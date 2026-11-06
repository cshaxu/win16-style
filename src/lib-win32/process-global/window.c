#include "window.h"

static _Thread_local win32_process_global_window *current;

static void report(win32_process_global_window *watcher,HWND window)
{
    DWORD process=0,thread;
    if(!watcher || !window || !IsWindowVisible(window))return;
    thread=GetWindowThreadProcessId(window,&process);
    if(!thread || !process || (!(watcher->flags&WIN32_PROCESS_GLOBAL_WINDOW_INCLUDE_SELF) && process==GetCurrentProcessId()))return;
    ++watcher->dispatching;watcher->callback(window,process,thread,watcher->context);--watcher->dispatching;
}
static BOOL CALLBACK scan_child(HWND window,LPARAM parameter)
{report((win32_process_global_window *)parameter,window);return TRUE;}
static BOOL CALLBACK scan_top(HWND window,LPARAM parameter)
{
    win32_process_global_window *watcher=(win32_process_global_window *)parameter;
    report(watcher,window);EnumChildWindows(window,scan_child,parameter);return TRUE;
}
static void CALLBACK on_event(HWINEVENTHOOK hook,DWORD event,HWND window,LONG object,LONG child,DWORD thread,DWORD time)
{
    DWORD saved=GetLastError();
    (void)thread;(void)time;
    if(current && ((current->hook==hook && event==EVENT_OBJECT_SHOW) ||
       (current->uncloaked_hook==hook && event==EVENT_OBJECT_UNCLOAKED)) && object==OBJID_WINDOW && child==CHILDID_SELF)
        report(current,window);
    SetLastError(saved);
}
DWORD win32_process_global_window_start_ex(win32_process_global_window *watcher,win32_process_global_window_callback callback,void *context,DWORD flags)
{
    if(!watcher || !callback || (flags&~(WIN32_PROCESS_GLOBAL_WINDOW_INCLUDE_SELF|WIN32_PROCESS_GLOBAL_WINDOW_UNCLOAKED)))return ERROR_INVALID_PARAMETER;
    if(current || watcher->hook || watcher->uncloaked_hook || watcher->callback || watcher->owner_thread)return ERROR_BUSY;
    watcher->callback=callback;watcher->context=context;watcher->owner_thread=GetCurrentThreadId();watcher->flags=flags;current=watcher;
    DWORD options=WINEVENT_OUTOFCONTEXT;
    if(!(flags&WIN32_PROCESS_GLOBAL_WINDOW_INCLUDE_SELF))options|=WINEVENT_SKIPOWNPROCESS;
    watcher->hook=SetWinEventHook(EVENT_OBJECT_SHOW,EVENT_OBJECT_SHOW,NULL,on_event,0,0,options);
    if(!watcher->hook){DWORD error=GetLastError();current=NULL;ZeroMemory(watcher,sizeof(*watcher));return error?error:ERROR_GEN_FAILURE;}
    if(flags&WIN32_PROCESS_GLOBAL_WINDOW_UNCLOAKED){
        watcher->uncloaked_hook=SetWinEventHook(EVENT_OBJECT_UNCLOAKED,EVENT_OBJECT_UNCLOAKED,NULL,on_event,0,0,options);
        if(!watcher->uncloaked_hook){DWORD error=GetLastError(),cleanup=win32_process_global_window_stop(watcher);return cleanup?cleanup:(error?error:ERROR_GEN_FAILURE);}
    }
    SetLastError(ERROR_SUCCESS);
    if(!EnumWindows(scan_top,(LPARAM)watcher)){
        DWORD error=GetLastError();
        /* scan_top never stops enumeration; no windows also returns FALSE
         * without setting an error on an empty desktop. */
        if(error){DWORD cleanup=win32_process_global_window_stop(watcher);return cleanup?cleanup:error;}
    }
    return ERROR_SUCCESS;
}
DWORD win32_process_global_window_start(win32_process_global_window *watcher,win32_process_global_window_callback callback,void *context)
{return win32_process_global_window_start_ex(watcher,callback,context,0);}
DWORD win32_process_global_window_refresh(win32_process_global_window *watcher)
{
    if(!watcher || !watcher->hook || current!=watcher || watcher->owner_thread!=GetCurrentThreadId())return ERROR_INVALID_PARAMETER;
    SetLastError(ERROR_SUCCESS);
    if(!EnumWindows(scan_top,(LPARAM)watcher)){DWORD error=GetLastError();if(error)return error;}
    return ERROR_SUCCESS;
}
DWORD win32_process_global_window_stop(win32_process_global_window *watcher)
{
    if(!watcher || !watcher->hook || current!=watcher || watcher->owner_thread!=GetCurrentThreadId())return ERROR_INVALID_PARAMETER;
    if(watcher->dispatching)return ERROR_BUSY;
    if(watcher->uncloaked_hook){
        if(!UnhookWinEvent(watcher->uncloaked_hook)){DWORD error=GetLastError();return error?error:ERROR_GEN_FAILURE;}
        watcher->uncloaked_hook=NULL;
    }
    if(!UnhookWinEvent(watcher->hook)){DWORD error=GetLastError();return error?error:ERROR_GEN_FAILURE;}
    current=NULL;ZeroMemory(watcher,sizeof(*watcher));return ERROR_SUCCESS;
}

typedef struct walk_state {ULONGLONG deadline;win32_process_global_visit_window visit;void *context;DWORD stopped;} walk_state;
static BOOL CALLBACK walk_child(HWND window,LPARAM parameter)
{
    walk_state *s=(walk_state *)parameter;
    if(GetTickCount64()>=s->deadline){s->stopped=ERROR_TIMEOUT;return FALSE;}
    if(!s->visit(window,s->context)){s->stopped=ERROR_CANCELLED;return FALSE;}
    return TRUE;
}
static BOOL CALLBACK walk_top(HWND window,LPARAM parameter)
{
    walk_state *s=(walk_state *)parameter;
    if(!walk_child(window,parameter))return FALSE;
    EnumChildWindows(window,walk_child,parameter);
    if(s->stopped)return FALSE;
    if(GetTickCount64()>=s->deadline){s->stopped=ERROR_TIMEOUT;return FALSE;}
    return TRUE;
}
BOOL win32_process_global_window_walk(DWORD thread_id,ULONGLONG deadline,win32_process_global_visit_window visit,void *context)
{
    if(!visit){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    walk_state state={deadline,visit,context,FALSE};
    SetLastError(ERROR_SUCCESS);
    BOOL result=thread_id?EnumThreadWindows(thread_id,walk_top,(LPARAM)&state):EnumWindows(walk_top,(LPARAM)&state);
    if(state.stopped){SetLastError(state.stopped);return FALSE;}
    return result || GetLastError()==ERROR_SUCCESS;
}

DWORD win32_process_global_window_hook_wait(int type,HOOKPROC proc,HINSTANCE module,DWORD thread_id,
    HANDLE ready,const HANDLE *waits,DWORD count)
{
    if(!proc || !thread_id || !ready || !waits || !count || count>MAXIMUM_WAIT_OBJECTS)return ERROR_INVALID_PARAMETER;
    HHOOK hook=SetWindowsHookExW(type,proc,module,thread_id);
    if(!hook)return GetLastError();
    DWORD error=ERROR_SUCCESS;
    if(!SetEvent(ready))error=GetLastError();
    else{
        DWORD result=WaitForMultipleObjects(count,waits,FALSE,INFINITE);
        if(result>=WAIT_OBJECT_0+count)error=result==WAIT_FAILED?GetLastError():ERROR_INVALID_STATE;
    }
    if(!UnhookWindowsHookEx(hook) && !error)error=GetLastError();
    return error;
}
