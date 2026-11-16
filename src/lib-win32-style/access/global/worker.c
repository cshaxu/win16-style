#include "worker.h"
#include "lib-win32/process-global/window.h"
#include "lib-win32/process-global/identity.h"
#include <wchar.h>

/* The packet is copied before ready; the caller may release its allocation
 * once ready signals. This worker never runs in DllMain. */
DWORD style_global_worker_run(const style_global_worker_ops *ops,void *parameter)
{
    style_global_thread_packet packet;SIZE_T read=0;
    WCHAR name[STYLE_GLOBAL_NAME_CAP],ready_name[STYLE_GLOBAL_NAME_CAP+64];
    WCHAR stop_name[STYLE_GLOBAL_NAME_CAP+16];
    HANDLE thread=NULL,mapping=NULL,owner=NULL,ready=NULL,stop=NULL;
    style_global_session *session=NULL;DWORD error=ERROR_SUCCESS;
    if(!parameter || !ReadProcessMemory(GetCurrentProcess(),parameter,&packet,sizeof(packet),&read) ||
       read!=sizeof(packet) || packet.bytes!=sizeof(packet) || packet.version!=1 ||
       !packet.thread_id || !packet.cookie)return ERROR_INVALID_PARAMETER;
    thread=OpenThread(THREAD_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,packet.thread_id);
    if(!thread)return GetLastError();
    if(GetProcessIdOfThread(thread)!=GetCurrentProcessId()){error=ERROR_INVALID_THREAD_ID;goto done;}
    HDESK desktop=GetThreadDesktop(packet.thread_id);
    if(!desktop || !SetThreadDesktop(desktop)){error=GetLastError();goto done;}
    if(!style_global_session_name(ops->space,name,ARRAYSIZE(name)) ||
       swprintf(ready_name,ARRAYSIZE(ready_name),L"%ls.thread.%lu.%lu.%lu",name,
                GetCurrentProcessId(),packet.thread_id,packet.cookie)<0 ||
       swprintf(stop_name,ARRAYSIZE(stop_name),L"%ls.stop",name)<0){error=ERROR_INVALID_NAME;goto done;}
    mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
    if(!mapping){error=GetLastError();goto done;}
    session=(style_global_session *)MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*session));
    if(!session){error=GetLastError();goto done;}
    if(session->cookie!=packet.cookie || session->phase!=STYLE_GLOBAL_ACTIVE ||
       session->owner_pid==GetCurrentProcessId() || session->host_pid==GetCurrentProcessId() ||
       !style_global_owner_alive(ops->space,session)){error=ERROR_ACCESS_DENIED;goto done;}
    owner=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,session->owner_pid);
    if(!owner){error=GetLastError();goto done;}
    if(!win32_process_global_alive(owner,session->owner_birth_low,session->owner_birth_high)){
        error=ERROR_INVALID_STATE;goto done;
    }
    if(ops->initialize){
        /* Global fallback already runs this worker in the target outside DllMain.
         * Retain the module permanently while any installed trampoline can run. */
        HMODULE module=NULL;
        if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                (LPCWSTR)(void *)&style_global_worker_run,&module)){error=GetLastError();goto done;}
        if(!style_global_owner_matches(ops->space,session,module)){error=ERROR_ACCESS_DENIED;goto done;}
        HMODULE pinned=NULL;
        if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
                (LPCWSTR)(void *)&style_global_worker_run,&pinned)){error=GetLastError();goto done;}
        error=ops->initialize(module,packet.cookie,ops->context);if(error)goto done;
    }
    ready=OpenEventW(EVENT_MODIFY_STATE,FALSE,ready_name);
    if(!ready){error=GetLastError();goto done;}
    stop=OpenEventW(SYNCHRONIZE,FALSE,stop_name);
    if(!stop){error=GetLastError();goto done;}
    HANDLE waits[]={stop,owner,thread};
    error=win32_process_global_window_hook_wait(WH_CALLWNDPROC,ops->hook,NULL,packet.thread_id,ready,waits,ARRAYSIZE(waits));
done:
    if(stop)CloseHandle(stop);
    if(ready)CloseHandle(ready);
    if(owner)CloseHandle(owner);
    if(session)UnmapViewOfFile(session);
    if(mapping)CloseHandle(mapping);
    if(thread)CloseHandle(thread);
    return error;
}
