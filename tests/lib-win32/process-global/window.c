#include "lib-win32/process-global/window.h"
#include <stdio.h>
typedef struct seen {HWND parent,child,grandchild;unsigned mask;BOOL cancel;} seen;
static BOOL visit(HWND w,void *context)
{
    seen *s=context;
    if(w==s->parent)s->mask|=1;
    if(w==s->child)s->mask|=2;
    if(w==s->grandchild)s->mask|=4;
    return !(s->cancel && w==s->parent);
}
static void discovered(HWND window,DWORD pid,DWORD tid,void *context)
{seen *s=context;if(pid==GetCurrentProcessId() && tid==GetCurrentThreadId())(void)visit(window,s);}
typedef struct hook_test {DWORD thread_id;HANDLE ready,stop;DWORD result;} hook_test;
static volatile LONG hook_hits;
static LRESULT CALLBACK callback(int code,WPARAM wp,LPARAM lp)
{if(code>=0)InterlockedIncrement(&hook_hits);return CallNextHookEx(NULL,code,wp,lp);}
static DWORD WINAPI hook_worker(void *context)
{
    hook_test *s=context;HANDLE waits[]={s->stop};
    s->result=win32_process_global_window_hook_wait(WH_CALLWNDPROC,callback,NULL,s->thread_id,s->ready,waits,1);return 0;
}
int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    HWND parent=CreateWindowW(L"STATIC",L"owned",WS_OVERLAPPED,0,0,120,100,NULL,NULL,instance,NULL);
    HWND child=CreateWindowW(L"STATIC",L"child",WS_CHILD,0,0,60,60,parent,NULL,instance,NULL);
    HWND grandchild=CreateWindowW(L"STATIC",L"grandchild",WS_CHILD,0,0,30,30,child,NULL,instance,NULL);
    if(!parent || !child || !grandchild)return 1;
    seen s={parent,child,grandchild,0,FALSE};int error=0;
    if(!win32_process_global_window_walk(GetCurrentThreadId(),GetTickCount64()+3000,visit,&s) || s.mask!=7)error=2;
    s.mask=0;if(!error && (!win32_process_global_window_walk(0,GetTickCount64()+3000,visit,&s) || s.mask!=7))error=3;
    s.mask=0;s.cancel=TRUE;if(!error && (win32_process_global_window_walk(GetCurrentThreadId(),GetTickCount64()+3000,visit,&s) || GetLastError()!=ERROR_CANCELLED || s.mask!=1))error=4;
    s.mask=0;s.cancel=FALSE;if(!error && (win32_process_global_window_walk(GetCurrentThreadId(),0,visit,&s) || GetLastError()!=ERROR_TIMEOUT || s.mask))error=5;
    if(!error && (win32_process_global_window_walk(0,0,NULL,NULL) || GetLastError()!=ERROR_INVALID_PARAMETER))error=6;
    ShowWindow(parent,SW_SHOW);UpdateWindow(parent);
    win32_process_global_window watcher={0};s.mask=0;
    if(!error && win32_process_global_window_start_ex(&watcher,discovered,&s,2)!=ERROR_INVALID_PARAMETER)error=14;
    if(!error && win32_process_global_window_start(&watcher,discovered,&s))error=15;
    if(!error && s.mask)error=16;
    if(watcher.hook && win32_process_global_window_stop(&watcher))error=17;
    s.mask=0;
    if(!error && win32_process_global_window_start_ex(&watcher,discovered,&s,WIN32_PROCESS_GLOBAL_WINDOW_INCLUDE_SELF))error=18;
    if(!error && !(s.mask&1))error=19;
    HWND later=NULL;
    if(!error){
        later=CreateWindowW(L"STATIC",L"later",WS_OVERLAPPEDWINDOW,0,0,100,80,NULL,NULL,instance,NULL);
        if(!later)error=21;
        else{
            s.parent=later;s.mask=0;ShowWindow(later,SW_SHOW);
            ULONGLONG deadline=GetTickCount64()+2000;
            while(!(s.mask&1) && GetTickCount64()<deadline){MSG msg;while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);}Sleep(10);}
            if(!(s.mask&1))error=22;
        }
    }
    if(watcher.hook && win32_process_global_window_stop(&watcher))error=20;
    if(later)DestroyWindow(later);
    HANDLE ready=CreateEventW(NULL,TRUE,FALSE,NULL),stop=CreateEventW(NULL,TRUE,FALSE,NULL),worker=NULL;
    hook_test hook={GetCurrentThreadId(),ready,stop,ERROR_GEN_FAILURE};
    if(!error && (!ready || !stop))error=7;
    if(!error){worker=CreateThread(NULL,0,hook_worker,&hook,0,NULL);if(!worker)error=8;}
    if(!error && WaitForSingleObject(ready,3000)!=WAIT_OBJECT_0)error=9;
    if(!error){SendMessageW(parent,WM_NULL,0,0);if(!InterlockedCompareExchange(&hook_hits,0,0))error=10;}
    if(stop)SetEvent(stop);
    if(worker){
        if(WaitForSingleObject(worker,3000)!=WAIT_OBJECT_0)error=11;
        else if(hook.result)error=12;
        CloseHandle(worker);
    }
    LONG before=InterlockedCompareExchange(&hook_hits,0,0);
    SendMessageW(parent,WM_NULL,0,0);
    if(!error && InterlockedCompareExchange(&hook_hits,0,0)!=before)error=13;
    if(ready)CloseHandle(ready);
    if(stop)CloseHandle(stop);
    DestroyWindow(parent);
    if(error){printf("WINDOW WALK FAIL %d\n",error);return error;}
    puts("WINDOW WALK PASS desktop=1 thread=1 children=1 cancel=1 timeout=1 hook=1 unhook=1 self_opt_in=1 self_new=1");return 0;
}
