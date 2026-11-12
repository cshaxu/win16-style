#include "lib-style/uwp/target.h"
#include <dwmapi.h>
#include <stdio.h>

#define CHECK(x) do{if(!(x)){fprintf(stderr,"UWP_TARGET_FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    WNDCLASSW frame_class={.lpfnWndProc=DefWindowProcW,.hInstance=instance,.lpszClassName=L"ApplicationFrameWindow"};
    WNDCLASSW core_class={.lpfnWndProc=DefWindowProcW,.hInstance=instance,.lpszClassName=L"Windows.UI.Core.CoreWindow"};
    CHECK(RegisterClassW(&frame_class));CHECK(RegisterClassW(&core_class));
    HWND frame=CreateWindowW(frame_class.lpszClassName,L"Owned UWP target fixture",WS_OVERLAPPEDWINDOW,
                            80,80,260,180,NULL,NULL,instance,NULL);
    CHECK(frame);
    HWND core=CreateWindowW(core_class.lpszClassName,L"",WS_CHILD|WS_VISIBLE,0,0,100,100,frame,NULL,instance,NULL);
    CHECK(core);
    style_uwp_target target={0},found={0};
    CHECK(!style_uwp_target_find(0,&target) && GetLastError()==ERROR_INVALID_PARAMETER);
    CHECK(!style_uwp_target_window(NULL,&target));
    CHECK(!style_uwp_target_window(core,&target));
    ShowWindow(frame,SW_SHOWNOACTIVATE);UpdateWindow(frame);DwmFlush();
    CHECK(style_uwp_target_window(core,&target));
    CHECK(target.core==core && target.frame==frame && target.pid==GetCurrentProcessId());
    CHECK(style_uwp_target_valid(&target));
    CHECK(style_uwp_target_window(frame,&found) && found.core==core && found.frame==frame);
    CHECK(style_uwp_target_find(GetCurrentProcessId(),&found) && found.core==core && found.frame==frame);
    found.pid++;CHECK(!style_uwp_target_valid(&found));
    found=target;found.frame=core;CHECK(!style_uwp_target_valid(&found));
    CHECK(DestroyWindow(core));CHECK(!style_uwp_target_valid(&target));
    CHECK(DestroyWindow(frame));CHECK(UnregisterClassW(core_class.lpszClassName,instance));
    CHECK(UnregisterClassW(frame_class.lpszClassName,instance));
    puts("UWP_TARGET_PASS hidden discovery identity mismatch destroyed");return 0;
}
