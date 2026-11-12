#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include "lib-style/common/global_window.h"

static const WCHAR property[]=L"StyleGlobalWindowTest.Attached";
static BOOL marker_on_apply,marker_on_remove;
static BOOL attached(HWND window){return GetPropW(window,property)!=NULL;}
static BOOL apply(HWND window){marker_on_apply=style_global_window_managed(window);return SetPropW(window,property,(HANDLE)1);}
static BOOL remove_style(HWND window){marker_on_remove=style_global_window_managed(window);return RemovePropW(window,property)!=NULL;}
static BOOL fail_apply(HWND window){(void)window;SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
static BOOL active(DWORD cookie){return cookie==42;}
static const style_global_window_ops ops={attached,apply,remove_style,active,L"StyleGlobalWindowTest.Control",3,1234};
static const style_global_window_ops other={attached,apply,remove_style,active,L"StyleGlobalWindowTest.Control",3,1234};
static const style_global_window_ops failing={attached,fail_apply,remove_style,active,L"StyleGlobalWindowTest.Control",3,1234};
static LRESULT CALLBACK procedure(HWND window,UINT message,WPARAM wp,LPARAM lp)
{return DefWindowProcW(window,message,wp,lp);}
typedef struct child_counter {HWND first,second,grandchild;unsigned seen;BOOL stop;} child_counter;
typedef struct foreign_child {HWND parent,window;HANDLE ready,stop;} foreign_child;
static DWORD WINAPI create_foreign_child(void *context)
{
    foreign_child *f=context;
    f->window=CreateWindowExW(WS_EX_NOPARENTNOTIFY,L"STATIC",L"",WS_CHILD,
        0,0,10,10,f->parent,NULL,GetModuleHandleW(NULL),NULL);
    SetEvent(f->ready);WaitForSingleObject(f->stop,5000);
    if(f->window)DestroyWindow(f->window);
    return 0;
}
static BOOL wait_with_messages(HANDLE handle)
{
    ULONGLONG deadline=GetTickCount64()+4000;
    do {
        DWORD result=MsgWaitForMultipleObjects(1,&handle,FALSE,50,QS_ALLINPUT);
        if(result==WAIT_OBJECT_0)return TRUE;
        if(result==WAIT_OBJECT_0+1){MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}}
        else if(result!=WAIT_TIMEOUT)return FALSE;
    } while(GetTickCount64()<deadline);
    return FALSE;
}
static BOOL visit_child(HWND window,void *context)
{
    child_counter *c=context;
    if(window==c->first)c->seen|=1;
    else if(window==c->second)c->seen|=2;
    else if(window==c->grandchild)c->seen|=4;
    else c->seen|=8;
    return !c->stop;
}
int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    WNDCLASSW cls={.lpfnWndProc=procedure,.hInstance=instance,.lpszClassName=L"StyleGlobalWindowTest"};
    if(!RegisterClassW(&cls))return 2;
    HWND window=CreateWindowW(cls.lpszClassName,L"Test",WS_OVERLAPPEDWINDOW,80,80,200,100,NULL,NULL,instance,NULL);
    if(!window)return 2;
    child_counter children={0};
    children.first=CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,20,20,window,NULL,instance,NULL);
    children.second=CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,20,20,window,NULL,instance,NULL);
    children.grandchild=CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,10,10,children.first,NULL,instance,NULL);
    foreign_child foreign={.parent=window};
    foreign.ready=CreateEventW(NULL,TRUE,FALSE,NULL);foreign.stop=CreateEventW(NULL,TRUE,FALSE,NULL);
    HANDLE thread=foreign.ready && foreign.stop?CreateThread(NULL,0,create_foreign_child,&foreign,0,NULL):NULL;
    BOOL foreign_ready=thread && wait_with_messages(foreign.ready) && foreign.window &&
        GetParent(foreign.window)==window && GetWindowThreadProcessId(foreign.window,NULL)!=GetCurrentThreadId();
    BOOL walked=children.first && children.second && children.grandchild &&
        style_global_window_visit_children(window,visit_child,&children) && children.seen==7;
    children.stop=TRUE;children.seen=0;
    (void)style_global_window_visit_children(window,visit_child,&children);
    BOOL stopped=children.seen && !(children.seen&(children.seen-1));
    BOOL rejected=!style_global_window_visit_children(window,NULL,NULL) && GetLastError()==ERROR_INVALID_PARAMETER;
    if(foreign.stop)SetEvent(foreign.stop);
    BOOL foreign_closed=thread && wait_with_messages(thread);
    if(thread && !foreign_closed)ExitProcess(4);
    if(thread)CloseHandle(thread);
    if(foreign.ready)CloseHandle(foreign.ready);
    if(foreign.stop)CloseHandle(foreign.stop);
    BOOL installed=style_global_window_apply(window,&ops) && style_global_window_apply(window,&ops) &&
                   style_global_window_attached(window,&ops) && attached(window);
    UINT control=RegisterWindowMessageW(ops.control);
    BOOL query=control && SendMessageW(window,control,42,3)==1234 && SendMessageW(window,control,41,3)!=1234;
    BOOL mismatch=!style_global_window_apply(window,&other) && GetLastError()==ERROR_BUSY;
    BOOL removed=style_global_window_remove(window,&ops) && !attached(window) && !style_global_window_attached(window,&ops) && !style_global_window_managed(window);
    BOOL repeated=!style_global_window_remove(window,&ops) && GetLastError()==ERROR_NOT_FOUND;
    BOOL reattached=style_global_window_apply(window,&other) && style_global_window_remove(window,&other);
    BOOL failed=!style_global_window_apply(window,&failing) && GetLastError()==ERROR_ACCESS_DENIED && !style_global_window_managed(window);
    BOOL rollback=style_global_window_apply(window,&ops) &&
                  !style_global_window_transition(window,&failing,&ops) && GetLastError()==ERROR_ACCESS_DENIED &&
                  attached(window) && style_global_window_attached(window,&ops) && style_global_window_remove(window,&ops);
    DestroyWindow(window);UnregisterClassW(cls.lpszClassName,instance);
    printf("STYLE GLOBAL WINDOW bits=%u installed=%d query=%d mismatch=%d removed=%d repeated=%d reattached=%d marker=%d failed_cleanup=%d rollback=%d children=%d stop=%d invalid=%d foreign=%d cleanup=%d\n",
           (unsigned)(sizeof(void *)*8),installed,query,mismatch,removed,repeated,reattached,marker_on_apply && marker_on_remove,failed,rollback,walked,stopped,rejected,foreign_ready,foreign_closed);
    return installed && query && mismatch && removed && repeated && reattached && marker_on_apply && marker_on_remove && failed && rollback && walked && stopped && rejected && foreign_ready && foreign_closed?0:3;
}
