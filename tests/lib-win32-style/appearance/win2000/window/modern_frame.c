#include "lib-win32-style/appearance/win2000/window/modern_frame.h"
#include "lib-win32-style/appearance/win2000/window/caption.h"
#include "lib-win32-style/appearance/msstyles/classic_presets.h"
#include "ntw16s-dll/uwp/hook.h"
#include <commctrl.h>
#include <stdio.h>
#include <stdint.h>
#include <wchar.h>

#define CHECK(x) do{if(!(x)){fprintf(stderr,"WIN2K UWP FRAME FAIL line=%d error=%lu: %s\n", +    __LINE__,(unsigned long)GetLastError(),#x);return 1;}}while(0)

static UINT command;
static BOOL CALLBACK find_surface(HWND candidate,LPARAM parameter)
{
    HWND *match=(HWND *)parameter;
    if(GetWindow(candidate,GW_OWNER)==*match && (GetWindowLongPtrW(candidate,GWL_EXSTYLE)&WS_EX_LAYERED)){
        *match=candidate;
        return FALSE;
    }
    return TRUE;
}
static HWND surface_of(HWND root)
{
    HWND surface=root;
    EnumThreadWindows(GetCurrentThreadId(),find_surface,(LPARAM)&surface);
    return surface==root?NULL:surface;
}
static BOOL request(HWND root,HWND core,WPARAM action)
{
    UINT message=RegisterWindowMessageW(NTW16S_UWP_MESSAGE);
    SendMessageW(root,message,action,(LPARAM)core);
    return (uintptr_t)RemovePropW(root,NTW16S_UWP_RESULT)==1;
}
static LRESULT CALLBACK owner_proc(HWND window,UINT message,WPARAM a,LPARAM b)
{
    if(message==WM_SYSCOMMAND && (a&0xfff0)==SC_CLOSE){command=SC_CLOSE;return 0;}
    if(message==WM_SIZE && a!=SIZE_MINIMIZED){
        HWND title=GetDlgItem(window,100);
        if(title)SetWindowPos(title,NULL,0,0,LOWORD(b),48,SWP_NOACTIVATE|SWP_NOZORDER);
    }
    return DefWindowProcW(window,message,a,b);
}

static BOOL missing_theme_retains(HWND root,HWND core,PCWSTR descriptor,WPARAM action,HWND surface)
{
    WCHAR backup[MAX_PATH];
    if(swprintf_s(backup,ARRAYSIZE(backup),L"%ls.fixture",descriptor)<0 ||
       !MoveFileW(descriptor,backup))return FALSE;
    BOOL applied=request(root,core,action);
    BOOL restored=MoveFileW(backup,descriptor);
    return restored && !applied && surface_of(root)==surface && IsWindow(surface);
}

static int mixed_profile(HWND root,HWND core,HMODULE module,PCWSTR folder)
{
    classic_theme_mapping source={0};
    classic_theme_family family;
    CHECK(SUCCEEDED(classic_theme_open_folder(module,folder,&source,&family)) && family==CLASSIC_THEME_WIN2K);
    CHECK(source.model.caption_presentation.button_source==STYLE_CAPTION_BUTTON_INDEXED &&
        source.model.title_buttons.right_margin==30);
    const WPARAM profile=(WPARAM)classic_theme_folder_id(folder)<<NTW16S_UWP_PROFILE_SHIFT;
    CHECK(request(root,core,profile|NTW16S_UWP_APPLY_2K));
    HWND surface=surface_of(root);
    RECT rect;
    CHECK(surface && GetWindowRect(surface,&rect));
    UINT dpi=GetDpiForWindow(root);
    const int border=style_frame_border_width(&source.model.border_presentation.geometry,
        STYLE_FRAME_SIZEABLE,dpi,dpi);
    RECT band={border,border,rect.right-rect.left-border,border+win2k_caption_theme_height(dpi,FALSE,&source.model)};
    RECT buttons[WIN2K_CAPTION_PARTS];
    const BOOL present[]={TRUE,TRUE,FALSE,TRUE};
    CHECK(win2k_caption_buttons_with_group(&band,dpi,FALSE,present,&source.model.title_buttons,
        &source.model.caption_metrics,buttons));
    classic_theme_close(&source);
    const RECT item=buttons[WIN2K_CLOSE];
    LPARAM point=MAKELPARAM(item.left+1,(item.top+item.bottom)/2);
    command=0;
    SendMessageW(surface,WM_LBUTTONDOWN,MK_LBUTTON,point);
    CHECK(GetCapture()==surface);
    SendMessageW(surface,WM_LBUTTONUP,0,point);
    MSG message;
    CHECK(PeekMessageW(&message,root,WM_SYSCOMMAND,WM_SYSCOMMAND,PM_REMOVE));
    DispatchMessageW(&message);
    CHECK(command==SC_CLOSE && !GetCapture());
    CHECK(request(root,core,profile|NTW16S_UWP_REMOVE_2K) && !IsWindow(surface));
    puts("UWP MIXED PROFILE PASS indexed source, relocated close command, restore");
    return 0;
}

static int product_modes(HWND root,HWND core,PCWSTR renderer,PCWSTR win16,PCWSTR win2k,PCWSTR mixed)
{
    HMODULE module=LoadLibraryW(renderer);
    CHECK(module);
    HOOKPROC callback=(HOOKPROC)(uintptr_t)GetProcAddress(module,"Ntw16sUwpFrameHook");
    CHECK(callback);
    HHOOK observer=SetWindowsHookExW(WH_CALLWNDPROC,callback,NULL,GetCurrentThreadId());
    CHECK(observer);
    CHECK(request(root,core,NTW16S_UWP_APPLY_2K));
    HWND classic=surface_of(root);
    CHECK(classic && request(root,core,NTW16S_UWP_APPLY_2K) && surface_of(root)==classic);
    if(win16)CHECK(missing_theme_retains(root,core,win16,NTW16S_UWP_APPLY,classic));
    CHECK(request(root,core,NTW16S_UWP_APPLY) && !IsWindow(classic));
    HWND legacy=surface_of(root);
    if(win2k)CHECK(legacy && missing_theme_retains(root,core,win2k,NTW16S_UWP_APPLY_2K,legacy));
    CHECK(legacy && request(root,core,NTW16S_UWP_APPLY_2K) && !IsWindow(legacy));
    classic=surface_of(root);
    CHECK(classic && request(root,core,NTW16S_UWP_REMOVE) && surface_of(root)==classic);
    CHECK(request(root,core,NTW16S_UWP_REMOVE_2K) && !IsWindow(classic) && !surface_of(root));
    CHECK(request(root,core,NTW16S_UWP_APPLY));
    legacy=surface_of(root);
    CHECK(legacy && request(root,core,NTW16S_UWP_REMOVE) && !IsWindow(legacy) && !surface_of(root));
    if(win16 && win2k){
        WPARAM first=(WPARAM)classic_theme_folder_id(L"Win16")<<NTW16S_UWP_PROFILE_SHIFT;
        WPARAM second=(WPARAM)classic_theme_folder_id(L"Win2K")<<NTW16S_UWP_PROFILE_SHIFT;
        CHECK(request(root,core,first|NTW16S_UWP_APPLY));
        legacy=surface_of(root);
        CHECK(legacy && !request(root,core,first|NTW16S_UWP_APPLY_2K) && surface_of(root)==legacy);
        CHECK(missing_theme_retains(root,core,win2k,second|NTW16S_UWP_APPLY_2K,legacy));
        CHECK(request(root,core,second|NTW16S_UWP_APPLY_2K) && !IsWindow(legacy));
        classic=surface_of(root);
        CHECK(classic && request(root,core,first|NTW16S_UWP_REMOVE) && surface_of(root)==classic);
        WCHAR backup[MAX_PATH];
        CHECK(swprintf_s(backup,ARRAYSIZE(backup),L"%ls.fixture",win2k)>0 && MoveFileW(win2k,backup));
        BOOL removed=request(root,core,second|NTW16S_UWP_REMOVE_2K);
        CHECK(MoveFileW(backup,win2k));
        CHECK(removed && !IsWindow(classic) && !surface_of(root));
        puts("UWP THEME ID PASS selected identity, mismatch preserves surface, removal without descriptor");
    }
    if(mixed)CHECK(!mixed_profile(root,core,module,mixed));
    CHECK(UnhookWindowsHookEx(observer) && FreeLibrary(module));
    puts("UWP PRODUCT MODE PASS two providers, reuse, switch both ways, style-specific removal");
    if(win16 && win2k)puts("UWP MISSING THEME PASS previous surface retained in both directions");
    return 0;
}

static int indexed_provider(HWND root,HWND title,PCWSTR descriptor,const classic_theme *original)
{
    WCHAR path[MAX_PATH];
    CHECK(wcscpy_s(path,ARRAYSIZE(path),descriptor)==0);
    WCHAR *extension=wcsrchr(path,L'.');
    CHECK(extension && wcscpy_s(extension,ARRAYSIZE(path)-(size_t)(extension-path),L".msstyles")==0);
    HMODULE resources=LoadLibraryExW(path,NULL,LOAD_LIBRARY_AS_DATAFILE|LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    CHECK(resources);
    classic_theme donor;
    CHECK(SUCCEEDED(classic_theme_read(resources,L"NormalSize",L"NormalColor",&donor)));
    classic_caption_art art={0};
    CHECK(SUCCEEDED(classic_caption_art_read(resources,&donor.caption_images,&art)));
    classic_theme theme=*original;
    theme.frame=donor.frame;
    theme.caption_images=donor.caption_images;
    theme.caption_presentation=donor.caption_presentation;
    theme.caption_presentation.button_source=STYLE_CAPTION_BUTTON_INDEXED;
    theme.caption_presentation.button_outline=STYLE_CAPTION_BUTTON_NO_OUTLINE;
    const classic_theme_mapping selection={.model=theme,.resources=resources};
    CHECK(win2k_uwp_frame_apply(root,title,NULL,0,&selection));
    CHECK(FreeLibrary(resources));

    HWND surface=style_uwp_frame_surface(root);
    CHECK(surface && SetWindowTextW(root,L"Indexed caption redraw") && style_uwp_frame_surface(root)==surface);
    DWORD_PTR data=0;
    CHECK(surface && GetWindowSubclass(surface,style_uwp_host_surface_proc,
        (UINT_PTR)style_uwp_host_surface_proc,&data));
    const style_bitmap *actual=&((const style_uwp_frame *)data)->bitmap;
    UINT dpi=GetDpiForWindow(root);
    RECT band={2,2,actual->size.cx-2,2+win2k_caption_theme_height(dpi,FALSE,&theme)};
    RECT buttons[WIN2K_CAPTION_PARTS];
    const BOOL present[]={TRUE,TRUE,FALSE,TRUE};
    CHECK(win2k_caption_buttons_with_group(&band,dpi,FALSE,present,
        &theme.title_buttons,&theme.caption_metrics,buttons));
    RECT item=buttons[WIN2K_MINIMIZE];
    SIZE size={item.right-item.left,item.bottom-item.top};
    style_bitmap expected={0};
    CHECK(style_bitmap_ensure(&expected,NULL,size));
    const style_caption_raster raster={.roles=art.roles[CLASSIC_IMAGE_MINIMIZE][0],
        .width=CLASSIC_CAPTION_ART_WIDTH,.height=CLASSIC_CAPTION_ART_HEIGHT,.stride=CLASSIC_CAPTION_ART_WIDTH};
    const COLORREF colors[]={theme.frame.edge,theme.frame.light,theme.frame.shadow,theme.frame.face};
    CHECK(style_caption_button_paint(expected.dc,(RECT){0,0,size.cx,size.cy},CLASSIC_BUTTON_MINIMIZE,
        FALSE,0,1,&theme.caption_presentation,&raster,colors));
    GdiFlush();
    const DWORD *wanted=(const DWORD *)expected.pixels,*observed=(const DWORD *)actual->pixels;
    for(LONG y=0;y<size.cy;++y)
        for(LONG x=0;x<size.cx;++x)
            CHECK((observed[(item.top+y)*actual->size.cx+item.left+x]&0xffffff)==(wanted[y*size.cx+x]&0xffffff));
    style_bitmap_dispose(&expected);
    CHECK(style_uwp_frame_remove(root) && !IsWindow(surface) && IsWindowVisible(title) &&
        SetWindowTextW(root,L"Hosted sample"));
    puts("UWP INDEXED CAPTION PASS shared pixels, copied resource, restored title");
    return 0;
}

int wmain(int argc,WCHAR **argv)
{
    if(argc!=1 && argc!=2 && argc!=4 && argc!=5)return 2;
    PCWSTR renderer=argc>=2?argv[1]:sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll";
    HINSTANCE module=GetModuleHandleW(NULL);
    WNDCLASSW cls={.lpfnWndProc=owner_proc,.hInstance=module,.lpszClassName=L"ApplicationFrameWindow"};
    CHECK(RegisterClassW(&cls));
    WNDCLASSW child={.lpfnWndProc=DefWindowProcW,.hInstance=module,.lpszClassName=L"ApplicationFrameTitleBarWindow"};
    CHECK(RegisterClassW(&child));
    child.lpszClassName=L"Windows.UI.Core.CoreWindow";
    CHECK(RegisterClassW(&child));
    HWND root=CreateWindowW(cls.lpszClassName,L"Hosted sample",WS_OVERLAPPEDWINDOW,
        80,80,400,300,NULL,NULL,module,NULL);
    CHECK(root);
    RECT client;CHECK(GetClientRect(root,&client));
    HWND title=CreateWindowW(L"ApplicationFrameTitleBarWindow",L"Native title",WS_VISIBLE|WS_CHILD,
        0,0,client.right,48,root,(HMENU)100,module,NULL);
    CHECK(title);
    HWND content=CreateWindowW(child.lpszClassName,L"Client stays native",WS_VISIBLE|WS_CHILD,
        0,60,client.right,30,root,NULL,module,NULL);
    CHECK(content);
    /* Keep the fixture's initial layered restack within its own windows. */
    HWND anchor=CreateWindowExW(WS_EX_NOACTIVATE|WS_EX_TOOLWINDOW,L"STATIC",L"",
        WS_POPUP,80,80,1,1,NULL,NULL,module,NULL);
    CHECK(anchor);
    ShowWindow(root,SW_SHOWNOACTIVATE);
    ShowWindow(anchor,SW_SHOWNOACTIVATE);
    CHECK(SetWindowPos(root,anchor,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE));
    RECT before,after,outer,title_screen;
    CHECK(GetWindowRect(content,&before) && GetWindowRect(root,&outer) && GetWindowRect(title,&title_screen));
    const classic_theme modified={
        .title_buttons=WIN2000_TITLE_BUTTON_GROUP,
        .caption_metrics={.height=25,.button={25,16}},
        .border_presentation.geometry={STYLE_FRAME_MONITOR_PIXELS,2,2,2,1,20,0},
        .active={RGB(32,64,96),RGB(32,64,96),RGB(255,255,255)},
        .inactive={RGB(64,32,96),RGB(64,32,96),RGB(255,255,255)}
    };
    const classic_theme_mapping selection={.model=modified};
    for(unsigned i=0;i<3;++i){
        const classic_theme *theme=i==1?&modified:NULL;
        CHECK(win2k_uwp_frame_apply(root,title,NULL,0,theme?&selection:NULL));
        HWND surface=style_uwp_frame_surface(root);
        CHECK(surface && IsWindowVisible(surface));
        CHECK(GetWindowRect(content,&after) && EqualRect(&before,&after));
        HRGN region=CreateRectRgn(0,0,0,0);
        CHECK(region && GetWindowRgn(surface,region)>NULLREGION);
        CHECK(PtInRegion(region,10,10) && !PtInRegion(region,before.left-outer.left+20,before.top-outer.top+5));
        DeleteObject(region);

        UINT dpi=GetDpiForWindow(root);
        int border=theme?2:4;
        RECT band={border,border,outer.right-outer.left-border,border+win2k_caption_theme_height(dpi,FALSE,theme)},buttons[4];
        BOOL present[]={TRUE,TRUE,FALSE,TRUE};
        if(theme){
            CHECK(win2k_caption_buttons_with_group(&band,dpi,FALSE,present,
                &theme->title_buttons,&theme->caption_metrics,buttons));
            CHECK(buttons[3].right-buttons[3].left==MulDiv(25,(int)dpi,96));
        }else CHECK(win2k_caption_buttons(&band,dpi,FALSE,present,buttons));
        LPARAM close=MAKELPARAM(buttons[3].left+1,(buttons[3].top+buttons[3].bottom)/2);
        SendMessageW(surface,WM_LBUTTONDOWN,MK_LBUTTON,close);
        CHECK(GetCapture()==surface);
        SendMessageW(surface,WM_LBUTTONUP,0,close);
        MSG message;
        CHECK(PeekMessageW(&message,root,WM_SYSCOMMAND,WM_SYSCOMMAND,PM_REMOVE));
        DispatchMessageW(&message);
        CHECK(command==SC_CLOSE && !GetCapture());
        if(i==0){
            ShowWindow(root,SW_MINIMIZE);
            CHECK(IsIconic(root) && !IsWindowVisible(surface));
            ShowWindow(root,SW_RESTORE);
            CHECK(SetWindowPos(root,anchor,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE));
            CHECK(!IsIconic(root) && IsWindowVisible(surface));
            ShowWindow(root,SW_MAXIMIZE);
            CHECK(SetWindowPos(root,anchor,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE));
            CHECK(IsZoomed(root) && IsWindowVisible(surface) && style_uwp_frame_surface(root)==surface);
            ShowWindow(root,SW_RESTORE);
            CHECK(SetWindowPos(root,anchor,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE));
            CHECK(!IsZoomed(root) && IsWindowVisible(surface));
        }
        CHECK(style_uwp_frame_remove(root) && !IsWindow(surface));
        CHECK(IsWindowVisible(title) && GetWindowRect(title,&after) && EqualRect(&title_screen,&after));
    }
    if(argc>=4)CHECK(!indexed_provider(root,title,argv[2],&modified));
    CHECK(!product_modes(root,content,renderer,argc>=4?argv[2]:NULL,argc>=4?argv[3]:NULL,argc==5?argv[4]:NULL));
    CHECK(win2k_uwp_frame_apply(root,title,NULL,0,NULL));
    HWND surface=style_uwp_frame_surface(root);
    CHECK(DestroyWindow(title) && !IsWindow(surface));
    CHECK(DestroyWindow(root) && DestroyWindow(anchor) && UnregisterClassW(cls.lpszClassName,module) &&
        UnregisterClassW(child.lpszClassName,module) && UnregisterClassW(L"ApplicationFrameTitleBarWindow",module));
    puts("WIN2K UWP FRAME PASS client hole, caption close, resize states, repeat, title restoration and destruction");
    return 0;
}
