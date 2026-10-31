#include "global_thread.h"
#include "global_session.h"
#include <wchar.h>
#ifdef NTW16S_BUILD
#include "injection/late_process.h"
#include "injection/injection.h"
#include "global_late.h"
#include <stdlib.h>
#endif

LRESULT CALLBACK Ntw16sGlobalHook(int,WPARAM,LPARAM);

/* This worker runs in the target, never in DllMain. The caller retains its
 * packet until the worker returns and creates both events before starting it. */
DWORD WINAPI ntw16s_global_thread_worker(void *parameter)
{
    ntw16s_global_thread_packet packet;SIZE_T read=0;
    WCHAR name[NTW16S_GLOBAL_NAME_CAP],ready_name[NTW16S_GLOBAL_NAME_CAP+64];
    WCHAR stop_name[NTW16S_GLOBAL_NAME_CAP+16];
    HANDLE thread=NULL,mapping=NULL,owner=NULL,ready=NULL,stop=NULL;
    ntw16s_global_session *session=NULL;HHOOK hook=NULL;DWORD error=ERROR_SUCCESS;
    if(!parameter || !ReadProcessMemory(GetCurrentProcess(),parameter,&packet,sizeof(packet),&read) ||
       read!=sizeof(packet) || packet.bytes!=sizeof(packet) || packet.version!=1 ||
       !packet.thread_id || !packet.cookie)return ERROR_INVALID_PARAMETER;
    thread=OpenThread(THREAD_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,packet.thread_id);
    if(!thread)return GetLastError();
    if(GetProcessIdOfThread(thread)!=GetCurrentProcessId()){error=ERROR_INVALID_THREAD_ID;goto done;}
    HDESK desktop=GetThreadDesktop(packet.thread_id);
    if(!desktop || !SetThreadDesktop(desktop)){error=GetLastError();goto done;}
    if(!ntw16s_global_name(name,ARRAYSIZE(name)) ||
       swprintf(ready_name,ARRAYSIZE(ready_name),L"%ls.thread.%lu.%lu.%lu",name,
                GetCurrentProcessId(),packet.thread_id,packet.cookie)<0 ||
       swprintf(stop_name,ARRAYSIZE(stop_name),L"%ls.stop",name)<0){error=ERROR_INVALID_NAME;goto done;}
    mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
    if(!mapping){error=GetLastError();goto done;}
    session=(ntw16s_global_session *)MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*session));
    if(!session){error=GetLastError();goto done;}
    if(session->cookie!=packet.cookie || session->phase!=NTW16S_GLOBAL_ACTIVE ||
       session->owner_pid==GetCurrentProcessId() || session->host_pid==GetCurrentProcessId() ||
       !ntw16s_global_owner_alive(session)){error=ERROR_ACCESS_DENIED;goto done;}
    owner=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,session->owner_pid);
    if(!owner){error=GetLastError();goto done;}
    DWORD low=0,high=0;
    if(!ntw16s_global_birth(owner,&low,&high) || low!=session->owner_birth_low ||
       high!=session->owner_birth_high || WaitForSingleObject(owner,0)!=WAIT_TIMEOUT){error=ERROR_INVALID_STATE;goto done;}
#ifdef NTW16S_BUILD
    /* Global fallback already runs this worker in the target outside DllMain.
     * Retain the module permanently while any installed trampoline can run. */
    HMODULE module=NULL;
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)(void *)&ntw16s_global_thread_worker,&module)){error=GetLastError();goto done;}
    if(!ntw16s_global_owner_matches(session,module)){error=ERROR_ACCESS_DENIED;goto done;}
    HMODULE pinned=NULL;
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
            (LPCWSTR)(void *)&ntw16s_global_thread_worker,&pinned)){error=GetLastError();goto done;}
    ntw16s_context *paths=(ntw16s_context *)calloc(1,sizeof(*paths));
    if(!paths){error=ERROR_NOT_ENOUGH_MEMORY;goto done;}
    error=ntw16s_context_paths(paths,module);
    if(!error)error=ntw16s_late_process_init(paths->directory,packet.cookie);
    free(paths);if(error)goto done;
    ntw16s_global_late_mark_ready(packet.cookie);
#endif
    ready=OpenEventW(EVENT_MODIFY_STATE,FALSE,ready_name);
    if(!ready){error=GetLastError();goto done;}
    stop=OpenEventW(SYNCHRONIZE,FALSE,stop_name);
    if(!stop){error=GetLastError();goto done;}
    hook=SetWindowsHookExW(WH_CALLWNDPROC,Ntw16sGlobalHook,NULL,packet.thread_id);
    if(!hook){error=GetLastError();goto done;}
    if(!SetEvent(ready)){error=GetLastError();goto done;}
    HANDLE waits[]={stop,owner,thread};DWORD result=WaitForMultipleObjects(ARRAYSIZE(waits),waits,FALSE,INFINITE);
    if(result>=WAIT_OBJECT_0+ARRAYSIZE(waits))
        error=result==WAIT_FAILED?GetLastError():ERROR_INVALID_STATE;
done:
    if(hook && !UnhookWindowsHookEx(hook) && !error)error=GetLastError();
    if(stop)CloseHandle(stop);
    if(ready)CloseHandle(ready);
    if(owner)CloseHandle(owner);
    if(session)UnmapViewOfFile(session);
    if(mapping)CloseHandle(mapping);
    if(thread)CloseHandle(thread);
    return error;
}
