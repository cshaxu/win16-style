#include "lib-style/uwp/native.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
int wmain(int argc,WCHAR **argv)
{
    (void)argc;(void)argv;
    WNDCLASSW cls={.lpfnWndProc=DefWindowProcW,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"ApplicationFrameTitleBarWindow"};
    CHECK(RegisterClassW(&cls));
    HWND root=CreateWindowExW(0,L"STATIC",L"Title selection fixture",WS_OVERLAPPEDWINDOW|WS_VISIBLE,0,0,500,300,NULL,NULL,NULL,NULL);
    CHECK(root);ShowWindow(root,SW_SHOWNOACTIVATE);
    HWND small=CreateWindowExW(0,cls.lpszClassName,L"Auxiliary",WS_CHILD|WS_VISIBLE,0,0,69,48,root,NULL,cls.hInstance,NULL);
    HWND wide=CreateWindowExW(0,cls.lpszClassName,L"Native title",WS_CHILD|WS_VISIBLE,0,0,500,48,root,NULL,cls.hInstance,NULL);
    HWND hidden=CreateWindowExW(0,cls.lpszClassName,L"Hidden title",WS_CHILD,0,0,700,48,root,NULL,cls.hInstance,NULL);
    CHECK(small&&wide&&hidden);
    HWND auxiliary=NULL;
    CHECK(style_uwp_title(root,&auxiliary)==wide&&auxiliary==small);
    ShowWindow(small,SW_HIDE);CHECK(style_uwp_title(root,&auxiliary)==wide&&!auxiliary);
    ShowWindow(small,SW_SHOW);ShowWindow(hidden,SW_SHOW);
    CHECK(!style_uwp_title(root,&auxiliary));
    CHECK(style_uwp_fullscreen_available(wide)==RPC_E_WRONG_THREAD);
    CHECK(style_uwp_fullscreen_available(NULL)==E_INVALIDARG);
    CHECK(DestroyWindow(root));CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance));
    puts("UWP_NATIVE_TITLE_PASS widest hidden auxiliary ambiguous wrong-thread invalid");return 0;
}
