#include "lib-win32/process-global/targets.h"
#include "lib-win32/process-global/window.h"
#include <stdio.h>
#include <wchar.h>
typedef struct observed {win32_process_global_targets *targets;DWORD pids[2];unsigned executed,disposed;} observed;
static LRESULT CALLBACK wndproc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_APP+1)return GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll")?1:0;
    if(m==WM_CLOSE){DestroyWindow(w);return 0;}
    if(m==WM_DESTROY){PostQuitMessage(0);return 0;}
    return DefWindowProcW(w,m,a,b);
}
static BOOL expired(win32_process_global_target *t,void *p){(void)t;(void)p;return FALSE;}
static BOOL finished(win32_process_global_target *t,void *p){(void)t;(void)p;return FALSE;}
static BOOL retryable(win32_process_global_target *t,void *p){(void)t;(void)p;return FALSE;}
static DWORD execute(win32_process_global_target *t,void *p,BOOL *retained)
{observed *o=p;(void)t;*retained=FALSE;++o->executed;return ERROR_SUCCESS;}
static void dispose(win32_process_global_target *t,void *p){observed *o=p;(void)t;++o->disposed;}
static void discovered(HWND w,DWORD pid,DWORD tid,void *p)
{observed *o=p;if(pid==o->pids[0] || pid==o->pids[1])win32_process_global_targets_submit(o->targets,pid,tid,w);}
static int child(const WCHAR *event,const WCHAR *title)
{
    HINSTANCE h=GetModuleHandleW(NULL);WNDCLASSW c={.hInstance=h,.lpfnWndProc=wndproc,.lpszClassName=L"GlobalTargetsFixture"};
    if(!RegisterClassW(&c))return 2;
    HWND w=CreateWindowW(c.lpszClassName,title,WS_OVERLAPPEDWINDOW|WS_VISIBLE,20,20,160,120,NULL,NULL,h,NULL);
    HANDLE ready=OpenEventW(EVENT_MODIFY_STATE,FALSE,event);if(!w || !ready)return 3;
    SetEvent(ready);CloseHandle(ready);MSG msg;while(GetMessageW(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}return 0;
}
static BOOL spawn(unsigned i,PROCESS_INFORMATION *pi,HWND *window)
{
    WCHAR self[MAX_PATH],event[128],title[128],cmd[MAX_PATH+300];
    if(!GetModuleFileNameW(NULL,self,ARRAYSIZE(self)))return FALSE;
    swprintf(event,ARRAYSIZE(event),L"Local\\GlobalTargets.%lu.%u",GetCurrentProcessId(),i);
    swprintf(title,ARRAYSIZE(title),L"GlobalTargets-%lu-%u",GetCurrentProcessId(),i);
    HANDLE ready=CreateEventW(NULL,TRUE,FALSE,event);if(!ready)return FALSE;
    swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" child %ls %ls",self,event,title);
    STARTUPINFOW si={.cb=sizeof(si)};BOOL ok=CreateProcessW(self,cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,pi);
    if(ok)ok=WaitForSingleObject(ready,3000)==WAIT_OBJECT_0;
    if(ok){*window=FindWindowW(L"GlobalTargetsFixture",title);ok=*window!=NULL;}
    CloseHandle(ready);return ok;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==4 && !wcscmp(argv[1],L"child"))return child(argv[2],argv[3]);
    PROCESS_INFORMATION p[2]={{0}};HWND w[2]={0};HANDLE work=CreateEventW(NULL,FALSE,FALSE,NULL),changed=CreateEventW(NULL,FALSE,FALSE,NULL);
    win32_process_global_targets targets={0};win32_process_global_window watcher={0};observed o={.targets=&targets};
    const win32_process_global_target_ops ops={expired,finished,retryable,execute,dispose};DWORD error=1;BOOL watching=FALSE;
    if(!work || !changed || win32_process_global_targets_init(&targets,work,changed,sizeof(win32_process_global_target),&ops,&o))goto done;
    if(!spawn(0,&p[0],&w[0]))goto done;
    o.pids[0]=p[0].dwProcessId;
    if(win32_process_global_window_start(&watcher,discovered,&o))goto done;
    watching=TRUE;
    if(!win32_process_global_targets_contains(&targets,p[0].dwProcessId,p[0].dwThreadId))goto done;
    if(!spawn(1,&p[1],&w[1]))goto done;
    o.pids[1]=p[1].dwProcessId;
    if(win32_process_global_window_refresh(&watcher))goto done;
    if(!win32_process_global_targets_contains(&targets,p[1].dwProcessId,p[1].dwThreadId))goto done;
    win32_process_global_targets_submit(&targets,p[1].dwProcessId,p[1].dwThreadId,w[1]);
    while(win32_process_global_targets_next(&targets)){}
    if(o.executed!=2 || !win32_process_global_targets_idle(&targets) || targets.count!=2)goto done;
    for(unsigned i=0;i<2;++i){DWORD_PTR loaded=1;
        if(!SendMessageTimeoutW(w[i],WM_APP+1,0,0,SMTO_ABORTIFHUNG,1000,&loaded) || loaded)goto done;
    }
    error=0;
done:
    if(watching && win32_process_global_window_stop(&watcher))error=1;
    win32_process_global_targets_dispose(&targets);
    for(unsigned i=0;i<2;++i)if(p[i].hProcess){
        if(w[i])PostMessageW(w[i],WM_CLOSE,0,0);
        if(WaitForSingleObject(p[i].hProcess,3000)!=WAIT_OBJECT_0){TerminateProcess(p[i].hProcess,ERROR_TIMEOUT);error=1;}
        CloseHandle(p[i].hThread);CloseHandle(p[i].hProcess);
    }
    if(work)CloseHandle(work);
    if(changed)CloseHandle(changed);
    if(error || o.disposed!=2){printf("TARGETS FAIL error=%lu exec=%u dispose=%u\n",error,o.executed,o.disposed);return 1;}
    puts("TARGETS PASS existing=1 new=1 dedup=1 no_dll=1 cleanup=1");return 0;
}
