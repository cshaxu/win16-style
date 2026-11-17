#include <windows.h>
#include <stdio.h>
#include "lib-win32-style/appearance/win3x/window/modern_frame.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"UWP HIDDEN FRAME FAIL line=%d error=%lu\n",__LINE__,(unsigned long)GetLastError());return 1;}}while(0)
int main(void)
{
    HWND owner=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,20,20,300,200,NULL,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(owner);
    HWND title=CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,100,20,owner,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(title);
    for(int i=0;i<3;++i){
        CHECK(win16_uwp_frame_apply_actions(owner,title,NULL,0));
        HWND surface=style_uwp_frame_surface(owner);CHECK(surface && IsWindow(surface));
        CHECK(SendMessageW(surface,WM_MOUSEACTIVATE,0,0)==MA_NOACTIVATE);
        CHECK(SendMessageW(surface,WM_NCHITTEST,0,0)==HTCLIENT);
        CHECK(style_uwp_frame_remove(owner));
        CHECK(!IsWindow(surface) && !style_uwp_frame_surface(owner));
    }
    CHECK(win16_uwp_frame_apply_actions(owner,title,NULL,0));
    HWND surface=style_uwp_frame_surface(owner);CHECK(surface);
    CHECK(DestroyWindow(title));CHECK(!IsWindow(surface) && !style_uwp_frame_surface(owner));
    CHECK(win16_uwp_frame_apply_nonclient(owner));
    surface=style_uwp_frame_surface(owner);CHECK(surface);
    CHECK(DestroyWindow(owner) && !IsWindow(surface));
    puts("UWP HIDDEN FRAME PASS repeat=3 remove=1 title_destroy=1 owner_destroy=1");return 0;
}
