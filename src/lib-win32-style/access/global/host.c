#include "host.h"
#include "trace.h"
#include <wchar.h>

DWORD style_global_host_run(const style_global_host_spec *spec,style_global_manager **retained)
{
    DWORD error=ERROR_SUCCESS;
    *retained=NULL;
    WCHAR name[STYLE_GLOBAL_NAME_CAP],ready_name[STYLE_GLOBAL_NAME_CAP+16],stop_name[STYLE_GLOBAL_NAME_CAP+16];
    if(!style_global_session_name(spec->manager->attach->start->space,name,ARRAYSIZE(name))){return GetLastError();}
    if(swprintf(ready_name,ARRAYSIZE(ready_name),L"%ls.ready",name)<0 || swprintf(stop_name,ARRAYSIZE(stop_name),L"%ls.stop",name)<0){return ERROR_INSUFFICIENT_BUFFER;}
    HANDLE mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name),ready=NULL,stop=NULL,owner=NULL;HHOOK hook=NULL;
    style_global_manager *manager=NULL;
    style_global_session *s=mapping?(style_global_session *)MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*s)):NULL;
    if(!s){error=GetLastError();if(!error)error=ERROR_INVALID_HANDLE;goto done;}
    if(!style_global_owner_alive(spec->manager->attach->start->space,s)){error=ERROR_INVALID_STATE;goto done;}
    owner=OpenProcess(SYNCHRONIZE|PROCESS_QUERY_LIMITED_INFORMATION,FALSE,s->owner_pid);if(!owner){error=GetLastError();goto done;}
    ready=OpenEventW(EVENT_MODIFY_STATE,FALSE,ready_name);if(!ready){error=GetLastError();goto done;}
    stop=OpenEventW(SYNCHRONIZE,FALSE,stop_name);if(!stop){error=GetLastError();goto done;}
    HMODULE module=NULL;if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCWSTR)(void *)spec->hook,&module)){error=GetLastError();goto done;}
    hook=SetWindowsHookExW(WH_CALLWNDPROC,spec->hook,module,0);
    if(!hook && GetLastError()!=ERROR_MOD_NOT_FOUND){error=GetLastError();goto done;}
    WCHAR path[32768];DWORD n=GetModuleFileNameW(module,path,ARRAYSIZE(path));
    WORD machine=sizeof(void*)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386;
    if(!n || n>=ARRAYSIZE(path)){error=n?ERROR_INSUFFICIENT_BUFFER:GetLastError();goto done;}
    error=style_global_manager_start(spec->manager,path,machine,&manager);if(error)goto done;
    DWORD idle=style_global_manager_wait_idle(manager,2000);
    if(idle==ERROR_TIMEOUT){if(spec->report)spec->report(STYLE_GLOBAL_HOST_PENDING,spec->context);}
    else if(idle){error=idle;goto done;}
    if(!SetEvent(ready)){error=GetLastError();goto done;}
    HANDLE waits[2]={stop,owner};
    for(;;){DWORD status=MsgWaitForMultipleObjects(2,waits,FALSE,INFINITE,QS_ALLINPUT);
        if(status==WAIT_OBJECT_0 || status==WAIT_OBJECT_0+1)break;
        if(status!=WAIT_OBJECT_0+2)break;
        MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
    }
done:
    if(hook)UnhookWindowsHookEx(hook);
    if(manager){DWORD stopped=style_global_manager_stop(manager,5000);if(stopped){*retained=manager;if(!error)error=stopped;}}
    if(owner && WaitForSingleObject(owner,0)==WAIT_OBJECT_0 && style_global_trace_stop_peer(spec->manager->trace_prefix,owner,s->owner_pid))
        if(spec->report)spec->report(STYLE_GLOBAL_HOST_OWNER_TRACE,spec->context);
    if(owner)CloseHandle(owner);
    if(stop)CloseHandle(stop);
    if(ready)CloseHandle(ready);
    if(s)UnmapViewOfFile(s);
    if(mapping)CloseHandle(mapping);
    return error;
}
