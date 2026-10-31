#include "ntw16s-dll/global_window.h"
#include "ntw16s-dll/ntw16s.h"
#include "isolated_process.h"
#include <commctrl.h>
#include <stdio.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"GLOBAL WINDOW FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;} } while(0)
static HWND wrong_window;
static BOOL destroy_on_position;
static LRESULT CALLBACK destroy_on_font(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;
    if(m==WM_GETFONT){DestroyWindow(w);return 0;}
    return DefSubclassProc(w,m,a,b);
}
static DWORD WINAPI wrong_thread(void *arg)
{
    (void)arg;
    if(ntw16s_global_window_apply(wrong_window) || GetLastError()!=ERROR_INVALID_THREAD_ID)return 1;
    if(ntw16s_global_window_remove(wrong_window) || GetLastError()!=ERROR_INVALID_THREAD_ID)return 2;
    return 0;
}
static LRESULT CALLBACK owner(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_WINDOWPOSCHANGED && destroy_on_position){destroy_on_position=FALSE;DestroyWindow(w);return 0;}
    if(m==WM_APP+1)return 314;
    return DefWindowProcW(w,m,a,b);
}
static int worker(int argc,WCHAR **argv)
{
    (void)argc;(void)argv;
    HINSTANCE instance=GetModuleHandleW(NULL);WNDCLASSW cls={0};
    cls.hInstance=instance;cls.lpfnWndProc=owner;cls.lpszClassName=L"Win16GlobalWindowFixture";
    CHECK(RegisterClassW(&cls));
    HWND root=CreateWindowW(cls.lpszClassName,L"Original",WS_OVERLAPPEDWINDOW,20,20,380,240,NULL,NULL,instance,NULL);
    CHECK(root);HWND button=CreateWindowW(L"BUTTON",L"Action",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,20,20,100,28,root,(HMENU)1,instance,NULL);CHECK(button);
    HWND edit=CreateWindowW(L"EDIT",L"Original text",WS_CHILD|WS_VISIBLE|WS_BORDER,20,60,180,28,root,(HMENU)2,instance,NULL);CHECK(edit);
    CHECK(ntw16s_global_window_apply(root));CHECK(ntw16s_global_window_attached(root));
    CHECK(ntw16s_global_window_apply(root));CHECK(ntw16s_global_window_apply(button));CHECK(ntw16s_global_window_attached(button));
    CHECK(ntw16s_global_window_apply(edit));CHECK(ntw16s_global_window_attached(edit));
    CHECK(SendMessageW(root,WM_APP+1,0,0)==314);
    wrong_window=root;HANDLE thread=CreateThread(NULL,0,wrong_thread,NULL,0,NULL);CHECK(thread);
    CHECK(WaitForSingleObject(thread,5000)==WAIT_OBJECT_0);DWORD result=1;CHECK(GetExitCodeThread(thread,&result)&&!result);CloseHandle(thread);
    CHECK(ntw16s_global_window_remove(edit));CHECK(!ntw16s_global_window_attached(edit));
    WCHAR text[32];CHECK(GetWindowTextW(edit,text,32)&&!wcscmp(text,L"Original text"));
    CHECK(ntw16s_global_window_remove(button));CHECK(!ntw16s_global_window_attached(button));
    CHECK(ntw16s_global_window_remove(root));CHECK(!ntw16s_global_window_attached(root));
    CHECK(!ntw16s_global_window_remove(root)&&GetLastError()==ERROR_NOT_FOUND);
    CHECK(ntw16s_global_window_apply(root));CHECK(DestroyWindow(root));
    HWND reentrant_parent=CreateWindowW(cls.lpszClassName,L"Reentrant",WS_OVERLAPPEDWINDOW,20,20,380,240,NULL,NULL,instance,NULL);CHECK(reentrant_parent);
    HWND reentrant=CreateWindowW(L"BUTTON",L"Reentrant",WS_CHILD|WS_VISIBLE,20,20,100,28,reentrant_parent,(HMENU)3,instance,NULL);CHECK(reentrant);
    CHECK(SetWindowSubclass(reentrant,destroy_on_font,1,0));
    CHECK(!ntw16s_global_window_apply(reentrant));CHECK(!IsWindow(reentrant));CHECK(DestroyWindow(reentrant_parent));
    ntw16s_creation_result creation={0};ntw16s_creation_scope *scope=Ntw16sCreationBegin(TRUE);CHECK(scope);
    HWND prior=CreateWindowW(cls.lpszClassName,L"Already styled",WS_OVERLAPPEDWINDOW,20,20,380,240,NULL,NULL,instance,NULL);CHECK(prior);
    CHECK(Ntw16sCreationEnd(scope,&creation));CHECK(Ntw16sWindowAttached(prior));
    CHECK(ntw16s_global_window_apply(prior));CHECK(!ntw16s_global_window_attached(prior));
    CHECK(!ntw16s_global_window_remove(prior));CHECK(Ntw16sWindowAttached(prior));CHECK(DestroyWindow(prior));
    CHECK(UnregisterClassW(cls.lpszClassName,instance));
    printf("GLOBAL WINDOW PASS bits=%u direct=1 control=1 edit=1 idempotent=1 wrong_thread=1 destroy=1 reentrant=1 existing=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
