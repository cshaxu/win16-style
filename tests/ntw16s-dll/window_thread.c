#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "ntw16s-dll/window_thread.h"

typedef BOOL (__cdecl *query_fn)(HWND);
#define CHECK(x) do { if(!(x)) { \
    fprintf(stderr,"window-thread failure at %d: %lu\n",__LINE__,GetLastError()); \
    return 1; \
} } while(0)

typedef struct surface_probe {HWND owner,surface;unsigned count;} surface_probe;
static BOOL CALLBACK find_surface(HWND window,LPARAM context)
{
    surface_probe *p=(surface_probe *)context;
    if(GetWindow(window,GW_OWNER)==p->owner && (GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_LAYERED)){
        p->surface=window;++p->count;
    }
    return TRUE;
}
static LRESULT CALLBACK window_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    return DefWindowProcW(window,message,wp,lp);
}

int wmain(int argc,WCHAR **argv)
{
    BOOL direct=argc>=3 && (!lstrcmpW(argv[2],L"--direct") || !lstrcmpW(argv[2],L"--direct-composed"));
    BOOL composed=argc>=3 && (!lstrcmpW(argv[2],L"--composed") || !lstrcmpW(argv[2],L"--direct-composed"));
    CHECK(argc==2 || ((argc==3 || argc==4) && (direct || composed)));
    WPARAM identity=argc==4?(WPARAM)wcstoul(argv[3],NULL,10):0;
    CHECK(argc!=4 || identity);
    HMODULE dll=LoadLibraryW(argv[1]);
    CHECK(dll);
    HOOKPROC callback=(HOOKPROC)(uintptr_t)GetProcAddress(dll,"Ntw16sWindowThreadHook");
    query_fn attached=(query_fn)(uintptr_t)GetProcAddress(dll,"Ntw16sWindowAttached");
    CHECK(callback && attached);
    WNDCLASSW wc={.lpfnWndProc=window_proc,.hInstance=GetModuleHandleW(NULL),
        .lpszClassName=composed?L"Chrome_WidgetWin_1":L"S3ThreadHookFixture"};
    CHECK(RegisterClassW(&wc));
    HWND first=CreateWindowExW(composed?WS_EX_NOREDIRECTIONBITMAP:0,wc.lpszClassName,L"one",WS_OVERLAPPEDWINDOW,
        0,0,400,300,NULL,NULL,wc.hInstance,NULL);
    HWND other=CreateWindowW(wc.lpszClassName,L"two",WS_OVERLAPPEDWINDOW,
        0,0,400,300,NULL,NULL,wc.hInstance,NULL);
    CHECK(first && other && !attached(first) && !attached(other));
    HHOOK hook=direct?NULL:SetWindowsHookExW(WH_CALLWNDPROC,callback,dll,GetCurrentThreadId());
    CHECK(direct || hook);
    UINT message=RegisterWindowMessageW(NTW16S_WINDOW_THREAD_MESSAGE);
    CHECK(message);
    CWPSTRUCT event={.hwnd=first,.message=message,
        .wParam=NTW16S_WINDOW_THREAD_ATTACH|(identity<<NTW16S_WINDOW_THREAD_PROFILE_SHIFT),.lParam=(LPARAM)other};
#define DISPATCH() do {if(direct){SetLastError(0x3456);callback(0,0,(LPARAM)&event);CHECK(GetLastError()==0x3456);}else SendMessageW(event.hwnd,event.message,event.wParam,event.lParam);} while(0)
    DISPATCH();
    CHECK(!attached(first) && !attached(other));
    CHECK(!GetPropW(first,NTW16S_WINDOW_THREAD_RESULT));
    event.lParam=(LPARAM)first;DISPATCH();
    CHECK((uintptr_t)GetPropW(first,NTW16S_WINDOW_THREAD_RESULT)==1);
    CHECK(attached(first) && !attached(other));
    RemovePropW(first,NTW16S_WINDOW_THREAD_RESULT);
    DISPATCH();
    CHECK((uintptr_t)GetPropW(first,NTW16S_WINDOW_THREAD_RESULT)==1 && attached(first));
    RemovePropW(first,NTW16S_WINDOW_THREAD_RESULT);
    surface_probe surface={.owner=first};
    if(composed){
        ShowWindow(first,SW_SHOWNOACTIVATE);
        CHECK(EnumThreadWindows(GetCurrentThreadId(),find_surface,(LPARAM)&surface));
        CHECK(surface.count==1 && IsWindowVisible(surface.surface));
    }
    event.wParam=NTW16S_WINDOW_THREAD_REMOVE;DISPATCH();
    CHECK((uintptr_t)GetPropW(first,NTW16S_WINDOW_THREAD_RESULT)==1);
    CHECK(!attached(first) && !attached(other));
    if(composed){
        CHECK(!IsWindow(surface.surface));
        CHECK(GetWindowLongPtrW(first,GWL_EXSTYLE)&WS_EX_NOREDIRECTIONBITMAP);
    }
    if(hook)CHECK(UnhookWindowsHookEx(hook));
    DestroyWindow(first);
    DestroyWindow(other);
    FreeLibrary(dll);
    printf("window-thread PASS width=%u direct=%d composed=%d surface_removed=%d\n",(unsigned)(8*sizeof(void *)),direct,composed,composed&&!IsWindow(surface.surface));
#undef DISPATCH
    return 0;
}
