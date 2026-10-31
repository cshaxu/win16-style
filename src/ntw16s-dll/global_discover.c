#include "global_discover.h"

static _Thread_local ntw16s_global_discover *current;

static void report(ntw16s_global_discover *watcher,HWND window)
{
    DWORD process=0,thread;
    if(!watcher || !window || !IsWindowVisible(window))return;
    thread=GetWindowThreadProcessId(window,&process);
    if(!thread || !process || process==GetCurrentProcessId())return;
    ++watcher->dispatching;watcher->callback(window,process,thread,watcher->context);--watcher->dispatching;
}
static BOOL CALLBACK scan_child(HWND window,LPARAM parameter)
{report((ntw16s_global_discover *)parameter,window);return TRUE;}
static BOOL CALLBACK scan_top(HWND window,LPARAM parameter)
{
    ntw16s_global_discover *watcher=(ntw16s_global_discover *)parameter;
    report(watcher,window);EnumChildWindows(window,scan_child,parameter);return TRUE;
}
static void CALLBACK on_event(HWINEVENTHOOK hook,DWORD event,HWND window,LONG object,LONG child,DWORD thread,DWORD time)
{
    DWORD saved=GetLastError();
    (void)thread;(void)time;
    if(current && current->hook==hook && event==EVENT_OBJECT_SHOW && object==OBJID_WINDOW && child==CHILDID_SELF)
        report(current,window);
    SetLastError(saved);
}
DWORD ntw16s_global_discover_start(ntw16s_global_discover *watcher,ntw16s_global_discovered callback,void *context)
{
    if(!watcher || !callback)return ERROR_INVALID_PARAMETER;
    if(current || watcher->hook || watcher->callback || watcher->owner_thread)return ERROR_BUSY;
    watcher->callback=callback;watcher->context=context;watcher->owner_thread=GetCurrentThreadId();current=watcher;
    watcher->hook=SetWinEventHook(EVENT_OBJECT_SHOW,EVENT_OBJECT_SHOW,NULL,on_event,0,0,WINEVENT_OUTOFCONTEXT|WINEVENT_SKIPOWNPROCESS);
    if(!watcher->hook){DWORD error=GetLastError();current=NULL;ZeroMemory(watcher,sizeof(*watcher));return error?error:ERROR_GEN_FAILURE;}
    SetLastError(ERROR_SUCCESS);
    if(!EnumWindows(scan_top,(LPARAM)watcher)){
        DWORD error=GetLastError();UnhookWinEvent(watcher->hook);current=NULL;ZeroMemory(watcher,sizeof(*watcher));
        return error?error:ERROR_GEN_FAILURE;
    }
    return ERROR_SUCCESS;
}
DWORD ntw16s_global_discover_stop(ntw16s_global_discover *watcher)
{
    if(!watcher || !watcher->hook || current!=watcher || watcher->owner_thread!=GetCurrentThreadId())return ERROR_INVALID_PARAMETER;
    if(watcher->dispatching)return ERROR_BUSY;
    if(!UnhookWinEvent(watcher->hook)){DWORD error=GetLastError();return error?error:ERROR_GEN_FAILURE;}
    current=NULL;ZeroMemory(watcher,sizeof(*watcher));return ERROR_SUCCESS;
}
