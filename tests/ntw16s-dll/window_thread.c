#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include "ntw16s-dll/window_thread.h"

typedef BOOL (__cdecl *query_fn)(HWND);
#define CHECK(x) do { if(!(x)) { \
    fprintf(stderr,"window-thread failure at %d: %lu\n",__LINE__,GetLastError()); \
    return 1; \
} } while(0)

static LRESULT CALLBACK window_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    return DefWindowProcW(window,message,wp,lp);
}

int wmain(int argc,WCHAR **argv)
{
    CHECK(argc==2);
    HMODULE dll=LoadLibraryW(argv[1]);
    CHECK(dll);
    HOOKPROC callback=(HOOKPROC)(uintptr_t)GetProcAddress(dll,"Ntw16sWindowThreadHook");
    query_fn attached=(query_fn)(uintptr_t)GetProcAddress(dll,"Ntw16sWindowAttached");
    CHECK(callback && attached);
    WNDCLASSW wc={.lpfnWndProc=window_proc,.hInstance=GetModuleHandleW(NULL),
        .lpszClassName=L"S3ThreadHookFixture"};
    CHECK(RegisterClassW(&wc));
    WNDCLASSW bridge_class={.lpfnWndProc=window_proc,.hInstance=wc.hInstance,
        .lpszClassName=L"Microsoft.UI.Content.DesktopChildSiteBridge"};
    CHECK(RegisterClassW(&bridge_class));
    HWND first=CreateWindowW(wc.lpszClassName,L"one",WS_OVERLAPPEDWINDOW,
        0,0,400,300,NULL,NULL,wc.hInstance,NULL);
    HWND other=CreateWindowW(wc.lpszClassName,L"two",WS_OVERLAPPEDWINDOW,
        0,0,400,300,NULL,NULL,wc.hInstance,NULL);
    CHECK(first && other && !attached(first) && !attached(other));
    HWND bridge=CreateWindowW(bridge_class.lpszClassName,L"",WS_CHILD,
        0,0,100,100,first,NULL,wc.hInstance,NULL);
    CHECK(bridge);
    HHOOK hook=SetWindowsHookExW(WH_CALLWNDPROC,callback,dll,GetCurrentThreadId());
    CHECK(hook);
    UINT message=RegisterWindowMessageW(NTW16S_WINDOW_THREAD_MESSAGE);
    CHECK(message);
    SendMessageW(first,message,NTW16S_WINDOW_THREAD_ATTACH,(LPARAM)other);
    CHECK(!attached(first) && !attached(other));
    CHECK(!GetPropW(first,NTW16S_WINDOW_THREAD_RESULT));
    SendMessageW(first,message,NTW16S_WINDOW_THREAD_ATTACH,(LPARAM)first);
    CHECK((uintptr_t)GetPropW(first,NTW16S_WINDOW_THREAD_RESULT)==1);
    CHECK(attached(first) && !attached(other));
    RemovePropW(first,NTW16S_WINDOW_THREAD_RESULT);
    SendMessageW(first,message,NTW16S_WINDOW_THREAD_ATTACH,(LPARAM)first);
    CHECK((uintptr_t)GetPropW(first,NTW16S_WINDOW_THREAD_RESULT)==1 && attached(first));
    CHECK(GetPropW(first,L"Ntw16s.XamlHost.v1"));
    CHECK(!GetPropW(other,L"Ntw16s.XamlHost.v1"));
    CHECK(DestroyWindow(bridge));
    CHECK(GetPropW(first,L"Ntw16s.XamlHost.v1"));
    RemovePropW(first,NTW16S_WINDOW_THREAD_RESULT);
    SendMessageW(first,message,NTW16S_WINDOW_THREAD_REMOVE,(LPARAM)first);
    CHECK((uintptr_t)GetPropW(first,NTW16S_WINDOW_THREAD_RESULT)==1);
    CHECK(!attached(first) && !attached(other));
    CHECK(!GetPropW(first,L"Ntw16s.XamlHost.v1"));
    CHECK(UnhookWindowsHookEx(hook));
    DestroyWindow(first);
    DestroyWindow(other);
    FreeLibrary(dll);
    printf("window-thread PASS width=%u\n",(unsigned)(8*sizeof(void *)));
    return 0;
}
