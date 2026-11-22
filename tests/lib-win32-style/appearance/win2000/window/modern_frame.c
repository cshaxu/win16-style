#include "lib-win32-style/appearance/win2000/window/modern_frame.h"
#ifdef STYLE_THEME_EXTERNAL_ONLY
#include "lib-win32-style/appearance/msstyles/modern_frame_style.h"
#endif
#include "lib-win32-style/appearance/win2000/window/caption.h"
#include "lib-win32-style/appearance/msstyles/classic_presets.h"
#include "ntw16s-dll/uwp/hook.h"
#include <commctrl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

#define CHECK(x) do{if(!(x)){fprintf(stderr,"WIN2K UWP FRAME FAIL line=%d error=%lu: %s\n", +    __LINE__,(unsigned long)GetLastError(),#x);return 1;}}while(0)

static UINT command;
static HICON fixture_icon;
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
    if(message==WM_GETICON && fixture_icon)return (LRESULT)fixture_icon;
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
    const WPARAM directory=profile|NTW16S_UWP_DIRECTORY;
    CHECK(request(root,core,directory|NTW16S_UWP_APPLY));
    HWND surface=surface_of(root);
    RECT rect;
    CHECK(surface && GetWindowRect(surface,&rect));
    UINT dpi=GetDpiForWindow(root);
    const style_frame_geometry *geometry=&source.model.border_presentation.geometry;
    const int x=style_frame_window_border_width(root,geometry,STYLE_FRAME_SIZEABLE,TRUE,dpi);
    const int y=style_frame_window_border_width(root,geometry,STYLE_FRAME_SIZEABLE,FALSE,dpi);
    RECT band={x,y,rect.right-rect.left-x,y+classic_theme_caption_height(dpi,FALSE,&source.model)};
    RECT buttons[STYLE_NATIVE_CAPTION_BUTTONS];
    const BOOL present[]={TRUE,TRUE,FALSE,TRUE};
    CHECK(win2k_caption_buttons_with_group(&band,dpi,FALSE,present,&source.model.title_buttons,
        &source.model.caption_metrics,buttons));
    classic_theme_close(&source);
    const RECT item=buttons[STYLE_NATIVE_CAPTION_CLOSE];
    LPARAM point=MAKELPARAM(item.left+1,(item.top+item.bottom)/2);
    command=0;
    SendMessageW(surface,WM_LBUTTONDOWN,MK_LBUTTON,point);
    CHECK(GetCapture()==surface);
    SendMessageW(surface,WM_LBUTTONUP,0,point);
    MSG message;
    CHECK(PeekMessageW(&message,root,WM_SYSCOMMAND,WM_SYSCOMMAND,PM_REMOVE));
    DispatchMessageW(&message);
    CHECK(command==SC_CLOSE && !GetCapture());
    CHECK(!request(root,core,profile|NTW16S_UWP_REMOVE_2K) && surface_of(root)==surface);
    CHECK(request(root,core,directory|NTW16S_UWP_REMOVE) && !IsWindow(surface) && !surface_of(root));
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
    BOOL unified=GetProcAddress(module,"ntwshk_context_read")!=NULL;
    WPARAM first=0,second=0;
    if(unified){
        first=((WPARAM)classic_theme_folder_id(L"Win16")<<NTW16S_UWP_PROFILE_SHIFT)|NTW16S_UWP_DIRECTORY;
        second=((WPARAM)classic_theme_folder_id(L"Win2K")<<NTW16S_UWP_PROFILE_SHIFT)|NTW16S_UWP_DIRECTORY;
        CHECK(!request(root,core,NTW16S_UWP_APPLY) && !request(root,core,NTW16S_UWP_APPLY_2K));
        CHECK(!surface_of(root));
    }
    WPARAM apply_first=first|NTW16S_UWP_APPLY;
    WPARAM remove_first=first|NTW16S_UWP_REMOVE;
    WPARAM apply_second=second|(unified?NTW16S_UWP_APPLY:NTW16S_UWP_APPLY_2K);
    WPARAM remove_second=second|(unified?NTW16S_UWP_REMOVE:NTW16S_UWP_REMOVE_2K);
    CHECK(request(root,core,apply_second));
    HWND classic=surface_of(root);
    CHECK(classic && request(root,core,apply_second) && surface_of(root)==classic);
    if(win16)CHECK(missing_theme_retains(root,core,win16,apply_first,classic));
    CHECK(request(root,core,apply_first) && !IsWindow(classic));
    HWND legacy=surface_of(root);
    if(win2k)CHECK(legacy && missing_theme_retains(root,core,win2k,apply_second,legacy));
    CHECK(legacy && request(root,core,apply_second) && !IsWindow(legacy));
    classic=surface_of(root);
    CHECK(classic && request(root,core,remove_first) && surface_of(root)==classic);
    CHECK(request(root,core,remove_second) && !IsWindow(classic) && !surface_of(root));
    CHECK(request(root,core,apply_first));
    legacy=surface_of(root);
    CHECK(legacy && request(root,core,remove_first) && !IsWindow(legacy) && !surface_of(root));
    if(win16 && win2k){
        WPARAM first=(WPARAM)classic_theme_folder_id(L"Win16")<<NTW16S_UWP_PROFILE_SHIFT;
        WPARAM second=(WPARAM)classic_theme_folder_id(L"Win2K")<<NTW16S_UWP_PROFILE_SHIFT;
        CHECK(!request(root,core,NTW16S_UWP_DIRECTORY|NTW16S_UWP_APPLY));
        CHECK(!request(root,core,second|NTW16S_UWP_DIRECTORY|NTW16S_UWP_APPLY_2K));
        CHECK(request(root,core,first|NTW16S_UWP_DIRECTORY|NTW16S_UWP_APPLY));
        legacy=surface_of(root);
        CHECK(legacy && !request(root,core,first|NTW16S_UWP_APPLY_2K) && surface_of(root)==legacy);
        CHECK(missing_theme_retains(root,core,win16,first|NTW16S_UWP_DIRECTORY|NTW16S_UWP_APPLY,legacy));
        CHECK(request(root,core,first|NTW16S_UWP_DIRECTORY|NTW16S_UWP_APPLY) && surface_of(root)==legacy);
        CHECK(missing_theme_retains(root,core,win2k,second|NTW16S_UWP_DIRECTORY|NTW16S_UWP_APPLY,legacy));
        CHECK(request(root,core,second|NTW16S_UWP_DIRECTORY|NTW16S_UWP_APPLY) && !IsWindow(legacy));
        classic=surface_of(root);
        CHECK(classic && request(root,core,first|NTW16S_UWP_DIRECTORY|NTW16S_UWP_REMOVE) && surface_of(root)==classic);
        WCHAR backup[MAX_PATH];
        CHECK(swprintf_s(backup,ARRAYSIZE(backup),L"%ls.fixture",win2k)>0 && MoveFileW(win2k,backup));
        BOOL removed=request(root,core,second|NTW16S_UWP_DIRECTORY|NTW16S_UWP_REMOVE);
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

static int indexed_provider(HWND root,HWND title,PCWSTR descriptor,const classic_theme *original,BOOL input)
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
    CHECK(theme.title_buttons.count<ARRAYSIZE(theme.title_buttons.slots));
    memmove(theme.title_buttons.slots+1,theme.title_buttons.slots,
        theme.title_buttons.count*sizeof(theme.title_buttons.slots[0]));
    theme.title_buttons.slots[0]=(classic_button_slot){CLASSIC_BUTTON_SYSTEM,CLASSIC_BUTTON_LEFT,0};
    ++theme.title_buttons.count;
    theme.caption_presentation=donor.caption_presentation;
    theme.caption_presentation.button_source=STYLE_CAPTION_BUTTON_INDEXED;
    theme.caption_presentation.button_outline=STYLE_CAPTION_BUTTON_NO_OUTLINE;
    classic_theme_mapping selection={.model=theme,.resources=resources};
#ifdef STYLE_THEME_EXTERNAL_ONLY
    selection.model.dialog_policy.window_provider=CLASSIC_WINDOW_PROVIDER_NONCLIENT;
    CHECK(classic_modern_frame_apply(root,title,NULL,0,&selection));
#else
    CHECK(win2k_uwp_frame_apply(root,title,NULL,0,&selection));
#endif
    CHECK(FreeLibrary(resources));

    HWND surface=style_uwp_frame_surface(root);
    CHECK(surface && SetWindowTextW(root,L"Indexed caption redraw") && style_uwp_frame_surface(root)==surface);
    DWORD_PTR data=0;
    CHECK(surface && GetWindowSubclass(surface,style_uwp_host_surface_proc,
        (UINT_PTR)style_uwp_host_surface_proc,&data));
    const style_bitmap *actual=&((const style_uwp_frame *)data)->bitmap;
    UINT dpi=GetDpiForWindow(root);
    const style_frame_geometry *profile=&theme.border_presentation.geometry;
    const int x=style_frame_window_border_width(root,profile,STYLE_FRAME_SIZEABLE,TRUE,dpi);
    const int y=style_frame_window_border_width(root,profile,STYLE_FRAME_SIZEABLE,FALSE,dpi);
    RECT band={x,y,actual->size.cx-x,y+classic_theme_caption_height(dpi,FALSE,&theme)};
    RECT buttons[STYLE_NATIVE_CAPTION_BUTTONS];
    const BOOL present[]={TRUE,TRUE,FALSE,TRUE};
    RECT system;
    CHECK(style_native_caption_buttons_with_system(&band,dpi,FALSE,present,
        &theme.title_buttons,&theme.caption_metrics,buttons,&system));
    CHECK(!IsRectEmpty(&system));
    RECT item=buttons[STYLE_NATIVE_CAPTION_MINIMIZE];
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
    size=(SIZE){system.right-system.left,system.bottom-system.top};
    CHECK(style_bitmap_ensure(&expected,NULL,size));
    const style_caption_raster menu_raster={art.roles[CLASSIC_IMAGE_SYSTEM][0],
        CLASSIC_SYSTEM_ART_WIDTH,CLASSIC_CAPTION_ART_HEIGHT,CLASSIC_SYSTEM_ART_WIDTH};
    CHECK(style_caption_button_paint(expected.dc,(RECT){0,0,size.cx,size.cy},CLASSIC_BUTTON_SYSTEM,
        FALSE,0,1,&theme.caption_presentation,&menu_raster,colors));
    GdiFlush();
    wanted=(const DWORD *)expected.pixels;
    for(LONG y=0;y<size.cy;++y)
        for(LONG x=0;x<size.cx;++x)
            CHECK((observed[(system.top+y)*actual->size.cx+system.left+x]&0xffffff)==(wanted[y*size.cx+x]&0xffffff));
    style_bitmap_dispose(&expected);
    if(input){
        LPARAM button=MAKELPARAM(system.left+1,system.top+1);
        SendMessageW(surface,WM_LBUTTONDOWN,MK_LBUTTON,button);
        CHECK(GetCapture()==surface);
        SendMessageW(surface,WM_CANCELMODE,0,0);
        CHECK(GetCapture()!=surface);
        puts("UWP INDEXED SYSTEM PASS painted and hit");
    }
    CHECK(style_uwp_frame_remove(root) && !IsWindow(surface) && IsWindowVisible(title) &&
        SetWindowTextW(root,L"Hosted sample"));
    puts("UWP INDEXED CAPTION PASS shared pixels, copied resource, restored title");
    return 0;
}

static int external_icon(HWND root,HWND title,PCWSTR path,const classic_theme *geometry)
{
    classic_theme_mapping source={0};
    CHECK(classic_theme_open(path,&source)==S_OK);
    source.model.title_buttons=geometry->title_buttons;
    source.model.caption_metrics=geometry->caption_metrics;
    source.model.border_presentation.geometry=geometry->border_presentation.geometry;
    source.model.active=geometry->active;
    source.model.inactive=geometry->inactive;
    CHECK(SetWindowTextW(root,L""));
    BYTE mask[32]={0},pixels[32];
    FillMemory(pixels,sizeof(pixels),255);
    HICON icon=CreateIcon(GetModuleHandleW(NULL),16,16,1,1,mask,pixels);
    CHECK(icon);
    fixture_icon=icon;
    for(unsigned mode=STYLE_CAPTION_ICON_DEFAULT;mode<=STYLE_CAPTION_ICON_NATIVE;++mode){
        const BOOL native=mode!=STYLE_CAPTION_ICON_HIDDEN;
        source.model.caption_presentation.icon_mode=(uint8_t)mode;
        CHECK(win2k_uwp_frame_apply(root,title,NULL,0,&source));
        HWND surface=style_uwp_frame_surface(root);
        DWORD_PTR data=0;
        CHECK(surface && GetWindowSubclass(surface,style_uwp_host_surface_proc,
            (UINT_PTR)style_uwp_host_surface_proc,&data));
        const style_uwp_frame *frame=(const style_uwp_frame *)data;
        UINT dpi=GetDpiForWindow(root);
        const style_frame_geometry *profile=&source.model.border_presentation.geometry;
        int x=style_frame_window_border_width(root,profile,STYLE_FRAME_SIZEABLE,TRUE,dpi);
        int y=style_frame_window_border_width(root,profile,STYLE_FRAME_SIZEABLE,FALSE,dpi);
        RECT band={x,y,frame->bitmap.size.cx-x,y+classic_theme_caption_height(dpi,FALSE,&source.model)};
        RECT slot;
        CHECK(style_caption_icon_bounds(band,band.right,dpi,&slot));
        POINT center={(slot.left+slot.right)/2,(slot.top+slot.bottom)/2};
        CHECK(frame->ops->hit(frame->context,center)==(native?HTSYSMENU:HTCAPTION));
        const int corner=band.left+MulDiv(geometry->caption_metrics.button.cx,(int)dpi,96);
        const int bottom=frame->bitmap.size.cy-1;
        CHECK(frame->ops->hit(frame->context,(POINT){corner-1,bottom})==HTBOTTOMLEFT);
        CHECK(frame->ops->hit(frame->context,(POINT){corner,bottom})==HTBOTTOM);
        GdiFlush();
        const DWORD *observed=(const DWORD *)frame->bitmap.pixels;
        DWORD pixel=observed[center.y*frame->bitmap.size.cx+center.x]&0xffffff;
        printf("UWP ICON PIXEL mode=%u rgb=%06lx\n",mode,(unsigned long)pixel);
        CHECK((pixel==0xffffff)==native);
        if(native){
            HICON replacement=CreateIcon(GetModuleHandleW(NULL),16,16,1,1,mask,mask);
            CHECK(replacement);
            fixture_icon=replacement;
            SendMessageW(root,WM_SETICON,ICON_SMALL,0);
            GdiFlush();
            observed=(const DWORD *)frame->bitmap.pixels;
            CHECK((observed[center.y*frame->bitmap.size.cx+center.x]&0xffffff)==0);
            fixture_icon=icon;
            CHECK(DestroyIcon(replacement));
        }
        CHECK(style_uwp_frame_remove(root));
    }
    fixture_icon=NULL;
    CHECK(DestroyIcon(icon));
    classic_theme_close(&source);
    puts("UWP APPLICATION ICON PASS declared pixels, hit and live replacement");
    return 0;
}

int wmain(int argc,WCHAR **argv)
{
    BOOL external=argc==3 && !wcscmp(argv[1],L"--external-renderer");
    if(!external && argc!=1 && argc!=2 && argc!=4 && argc!=5)return 2;
    PCWSTR renderer=argc>=2?argv[1]:sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll";
    HINSTANCE module=GetModuleHandleW(NULL);
    WNDCLASSW cls={.lpfnWndProc=owner_proc,.hInstance=module,.lpszClassName=L"ApplicationFrameWindow"};
    CHECK(RegisterClassW(&cls));
    WNDCLASSW child={.lpfnWndProc=DefWindowProcW,.hInstance=module,.lpszClassName=L"ApplicationFrameTitleBarWindow"};
    CHECK(RegisterClassW(&child));
    child.lpszClassName=L"Windows.UI.Core.CoreWindow";
    CHECK(RegisterClassW(&child));
    int origin=external?-10000:80;
    HWND root=CreateWindowW(cls.lpszClassName,L"Hosted sample",WS_OVERLAPPEDWINDOW,
        origin,origin,400,300,NULL,NULL,module,NULL);
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
        WS_POPUP,origin,origin,1,1,NULL,NULL,module,NULL);
    CHECK(anchor);
    ShowWindow(root,SW_SHOWNOACTIVATE);
    ShowWindow(anchor,SW_SHOWNOACTIVATE);
    CHECK(SetWindowPos(root,anchor,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE));
    RECT before,after,outer,title_screen;
    CHECK(GetWindowRect(content,&before) && GetWindowRect(root,&outer) && GetWindowRect(title,&title_screen));
    const classic_theme modified={
        .title_buttons=WIN2000_TITLE_BUTTON_GROUP,
        .caption_metrics={.height=25,.button={25,16}},
        .border_presentation.geometry={STYLE_FRAME_MONITOR_PIXELS,2,2,2,1,20,0,0},
        .active={RGB(32,64,96),RGB(32,64,96),RGB(255,255,255)},
        .inactive={RGB(64,32,96),RGB(64,32,96),RGB(255,255,255)}
    };
    const classic_theme_mapping selection={.model=modified};
    if(external){
        CHECK(!win2k_uwp_frame_apply(root,title,NULL,0,NULL) && GetLastError()==ERROR_RESOURCE_DATA_NOT_FOUND);
        CHECK(!win2k_uwp_frame_apply(root,title,NULL,0,&selection) && GetLastError()==ERROR_RESOURCE_DATA_NOT_FOUND);
        CHECK(!style_uwp_frame_surface(root) && IsWindowVisible(title));
        CHECK(!indexed_provider(root,title,argv[2],&modified,FALSE));
        CHECK(!external_icon(root,title,argv[2],&modified));
        CHECK(GetWindowRect(content,&after) && EqualRect(&after,&before) && DestroyWindow(root) && DestroyWindow(anchor));
        puts("UWP EXTERNAL RENDERER PASS missing=2 external_pixels=2 title_restored=1");
        return 0;
    }
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
        const style_frame_geometry *profile=theme?&theme->border_presentation.geometry:NULL;
        const int pad=GetSystemMetricsForDpi(SM_CXPADDEDBORDER,dpi);
        const int x=theme?style_frame_window_border_width(root,profile,STYLE_FRAME_SIZEABLE,TRUE,dpi):
            GetSystemMetricsForDpi(SM_CXSIZEFRAME,dpi)+pad;
        const int y=theme?style_frame_window_border_width(root,profile,STYLE_FRAME_SIZEABLE,FALSE,dpi):
            GetSystemMetricsForDpi(SM_CYSIZEFRAME,dpi)+pad;
        RECT band={x,y,outer.right-outer.left-x,y+classic_theme_caption_height(dpi,FALSE,theme)},buttons[4];
        BOOL present[]={TRUE,TRUE,FALSE,TRUE};
        if(theme){
            CHECK(win2k_caption_buttons_with_group(&band,dpi,FALSE,present,
                &theme->title_buttons,&theme->caption_metrics,buttons));
            CHECK(buttons[3].right-buttons[3].left==MulDiv(25,(int)dpi,96));
        }else CHECK(win2k_caption_system_buttons(&band,dpi,FALSE,present,buttons));
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
    if(argc>=4)CHECK(!indexed_provider(root,title,argv[2],&modified,TRUE));
    CHECK(!product_modes(root,content,renderer,argc>=4?argv[2]:NULL,argc>=4?argv[3]:NULL,argc==5?argv[4]:NULL));
    CHECK(win2k_uwp_frame_apply(root,title,NULL,0,NULL));
    HWND surface=style_uwp_frame_surface(root);
    CHECK(DestroyWindow(title) && !IsWindow(surface));
    CHECK(DestroyWindow(root) && DestroyWindow(anchor) && UnregisterClassW(cls.lpszClassName,module) &&
        UnregisterClassW(child.lpszClassName,module) && UnregisterClassW(L"ApplicationFrameTitleBarWindow",module));
    puts("WIN2K UWP FRAME PASS client hole, caption close, resize states, repeat, title restoration and destruction");
    return 0;
}
