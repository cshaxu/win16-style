#include "lib-style/uwp/native.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static unsigned presses,releases;
static LRESULT CALLBACK title_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_LBUTTONDOWN && wp==MK_LBUTTON && LOWORD(lp)==34 && HIWORD(lp)==24)++presses;
    if(message==WM_LBUTTONUP && wp==0 && LOWORD(lp)==34 && HIWORD(lp)==24)++releases;
    return DefWindowProcW(window,message,wp,lp);
}
static void drain_clicks(HWND window)
{MSG message;while(PeekMessageW(&message,window,WM_LBUTTONDOWN,WM_LBUTTONUP,PM_REMOVE))DispatchMessageW(&message);}
int wmain(int argc,WCHAR **argv)
{
    (void)argc;(void)argv;
    WNDCLASSW cls={.lpfnWndProc=title_proc,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"ApplicationFrameTitleBarWindow"};
    CHECK(RegisterClassW(&cls));
    HWND root=CreateWindowExW(0,L"STATIC",L"Title selection fixture",WS_OVERLAPPEDWINDOW|WS_VISIBLE,0,0,500,300,NULL,NULL,NULL,NULL);
    CHECK(root);ShowWindow(root,SW_SHOWNOACTIVATE);
    HWND small=CreateWindowExW(0,cls.lpszClassName,L"Auxiliary",WS_CHILD|WS_VISIBLE,0,0,69,48,root,NULL,cls.hInstance,NULL);
    HWND wide=CreateWindowExW(0,cls.lpszClassName,L"Native title",WS_CHILD|WS_VISIBLE,0,0,500,48,root,NULL,cls.hInstance,NULL);
    HWND hidden=CreateWindowExW(0,cls.lpszClassName,L"Hidden title",WS_CHILD,0,0,700,48,root,NULL,cls.hInstance,NULL);
    CHECK(small&&wide&&hidden);
    HWND auxiliary=NULL;
    CHECK(style_uwp_title(root,&auxiliary)==wide&&auxiliary==small);
    style_uwp_application_menu(root,small);drain_clicks(small);CHECK(presses==1 && releases==1);
    style_uwp_application_menu(wide,small);drain_clicks(small);CHECK(presses==1 && releases==1);
    ShowWindow(small,SW_HIDE);style_uwp_application_menu(root,small);drain_clicks(small);
    CHECK(presses==1 && releases==1 && style_uwp_title(root,&auxiliary)==wide&&!auxiliary);
    ShowWindow(small,SW_SHOW);ShowWindow(hidden,SW_SHOW);
    CHECK(!style_uwp_title(root,&auxiliary));
    CHECK(style_uwp_fullscreen_available(wide)==RPC_E_WRONG_THREAD);
    CHECK(style_uwp_fullscreen_available(NULL)==E_INVALIDARG);
    CHECK(DestroyWindow(root));CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance));
    puts("UWP_NATIVE_TITLE_PASS widest hidden auxiliary ambiguous wrong-thread invalid");return 0;
}
