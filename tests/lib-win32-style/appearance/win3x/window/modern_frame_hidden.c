#include <windows.h>
#include <stdio.h>
#include "lib-win32-style/appearance/win3x/window/modern_frame.h"
#ifdef STYLE_THEME_EXTERNAL_ONLY
#include "lib-win32-style/appearance/msstyles/modern_frame_style.h"
#endif
#define CHECK(x) do{if(!(x)){fprintf(stderr,"UWP HIDDEN FRAME FAIL line=%d error=%lu\n",__LINE__,(unsigned long)GetLastError());return 1;}}while(0)
#ifdef STYLE_THEME_EXTERNAL_ONLY
int wmain(int argc,WCHAR **argv)
#else
int main(void)
#endif
{
    const classic_theme_mapping *source=NULL;
#ifdef STYLE_THEME_EXTERNAL_ONLY
    classic_theme_mapping theme={0};
    CHECK(argc==2 && classic_theme_open(argv[1],&theme)==S_OK);
    source=&theme;
#endif
    HWND owner=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,20,20,300,200,NULL,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(owner);
    HWND title=CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,100,20,owner,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(title);
#ifdef STYLE_THEME_EXTERNAL_ONLY
    const classic_theme_mapping incomplete={.model=theme.model};
    CHECK(!win16_uwp_frame_apply_actions(owner,title,NULL,0,NULL) && GetLastError()==ERROR_RESOURCE_DATA_NOT_FOUND);
    CHECK(!win16_uwp_frame_apply_actions(owner,title,NULL,0,&incomplete) && GetLastError()==ERROR_RESOURCE_DATA_NOT_FOUND);
    CHECK(!style_uwp_frame_surface(owner));
    const unsigned provider=theme.model.dialog_policy.window_provider;
    theme.model.dialog_policy.window_provider=0xff;
    CHECK(!classic_modern_frame_apply(owner,title,NULL,0,&theme) && GetLastError()==ERROR_NOT_SUPPORTED);
    CHECK(!style_uwp_frame_surface(owner));
    theme.model.dialog_policy.window_provider=provider;
    CHECK(classic_modern_frame_apply(owner,title,NULL,0,&theme));
    CHECK(style_uwp_frame_surface(owner) && style_uwp_frame_remove(owner));
#endif
    for(int i=0;i<3;++i){
        CHECK(win16_uwp_frame_apply_actions(owner,title,NULL,0,source));
        HWND surface=style_uwp_frame_surface(owner);CHECK(surface && IsWindow(surface));
        CHECK(!style_uwp_frame_profile(owner));
        style_uwp_frame_set_profile(owner,123);
        CHECK(style_uwp_frame_profile(owner)==123);
        CHECK(SendMessageW(surface,WM_MOUSEACTIVATE,0,0)==MA_NOACTIVATE);
        CHECK(SendMessageW(surface,WM_NCHITTEST,0,0)==HTCLIENT);
        CHECK(style_uwp_frame_remove(owner));
        CHECK(!IsWindow(surface) && !style_uwp_frame_surface(owner) && !style_uwp_frame_profile(owner));
    }
    CHECK(win16_uwp_frame_apply_actions(owner,title,NULL,0,source));
    HWND surface=style_uwp_frame_surface(owner);CHECK(surface);
    CHECK(DestroyWindow(title));CHECK(!IsWindow(surface) && !style_uwp_frame_surface(owner));
#ifdef STYLE_THEME_EXTERNAL_ONLY
    CHECK(!win16_uwp_frame_apply_nonclient(owner) && GetLastError()==ERROR_RESOURCE_DATA_NOT_FOUND);
    CHECK(!style_uwp_frame_surface(owner));
    classic_theme_set_process_profile(classic_theme_folder_id(L"Win16"),CLASSIC_THEME_WIN16);
#endif
    CHECK(win16_uwp_frame_apply_nonclient(owner));
    surface=style_uwp_frame_surface(owner);CHECK(surface);
    CHECK(DestroyWindow(owner) && !IsWindow(surface));
#ifdef STYLE_THEME_EXTERNAL_ONLY
    classic_theme_close(&theme);
    puts("UWP CLIENT THEME PASS missing=3 external=1 active=1");
#endif
    puts("UWP HIDDEN FRAME PASS repeat=3 remove=1 title_destroy=1 owner_destroy=1");return 0;
}
