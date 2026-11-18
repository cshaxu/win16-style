#include "lib-win32-style/appearance/win2000/window/modern_frame.h"
#include "lib-win32-style/appearance/win2000/window/caption.h"
#include "ntw16s-dll/uwp/hook.h"
#include <stdio.h>
#include <stdint.h>

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

static int product_modes(HWND root,HWND core)
{
    HMODULE module=LoadLibraryW(sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    CHECK(module);
    HOOKPROC callback=(HOOKPROC)(uintptr_t)GetProcAddress(module,"Ntw16sUwpFrameHook");
    CHECK(callback);
    HHOOK observer=SetWindowsHookExW(WH_CALLWNDPROC,callback,NULL,GetCurrentThreadId());
    CHECK(observer);
    CHECK(request(root,core,NTW16S_UWP_APPLY_2K));
    HWND classic=surface_of(root);
    CHECK(classic && request(root,core,NTW16S_UWP_APPLY_2K) && surface_of(root)==classic);
    CHECK(request(root,core,NTW16S_UWP_APPLY) && !IsWindow(classic));
    HWND legacy=surface_of(root);
    CHECK(legacy && request(root,core,NTW16S_UWP_APPLY_2K) && !IsWindow(legacy));
    classic=surface_of(root);
    CHECK(classic && request(root,core,NTW16S_UWP_REMOVE) && surface_of(root)==classic);
    CHECK(request(root,core,NTW16S_UWP_REMOVE_2K) && !IsWindow(classic) && !surface_of(root));
    CHECK(request(root,core,NTW16S_UWP_APPLY));
    legacy=surface_of(root);
    CHECK(legacy && request(root,core,NTW16S_UWP_REMOVE) && !IsWindow(legacy) && !surface_of(root));
    CHECK(UnhookWindowsHookEx(observer) && FreeLibrary(module));
    puts("UWP PRODUCT MODE PASS two providers, reuse, switch both ways, style-specific removal");
    return 0;
}

int main(void)
{
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
    ShowWindow(root,SW_SHOWNOACTIVATE);
    RECT before,after,outer,title_screen;
    CHECK(GetWindowRect(content,&before) && GetWindowRect(root,&outer) && GetWindowRect(title,&title_screen));
    for(unsigned i=0;i<3;++i){
        CHECK(win2k_uwp_frame_apply(root,title,NULL,0));
        HWND surface=style_uwp_frame_surface(root);
        CHECK(surface && IsWindowVisible(surface));
        CHECK(GetWindowRect(content,&after) && EqualRect(&before,&after));
        HRGN region=CreateRectRgn(0,0,0,0);
        CHECK(region && GetWindowRgn(surface,region)>NULLREGION);
        CHECK(PtInRegion(region,10,10) && !PtInRegion(region,before.left-outer.left+20,before.top-outer.top+5));
        DeleteObject(region);

        UINT dpi=GetDpiForWindow(root);
        RECT band={4,4,outer.right-outer.left-4,4+win2k_caption_height(dpi,FALSE)},buttons[4];
        BOOL present[]={TRUE,TRUE,FALSE,TRUE};
        CHECK(win2k_caption_buttons(&band,dpi,FALSE,present,buttons));
        LPARAM close=MAKELPARAM((buttons[3].left+buttons[3].right)/2,(buttons[3].top+buttons[3].bottom)/2);
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
            CHECK(!IsIconic(root) && IsWindowVisible(surface));
            ShowWindow(root,SW_MAXIMIZE);
            CHECK(IsZoomed(root) && IsWindowVisible(surface) && style_uwp_frame_surface(root)==surface);
            ShowWindow(root,SW_RESTORE);
            CHECK(!IsZoomed(root) && IsWindowVisible(surface));
        }
        CHECK(style_uwp_frame_remove(root) && !IsWindow(surface));
        CHECK(IsWindowVisible(title) && GetWindowRect(title,&after) && EqualRect(&title_screen,&after));
    }
    CHECK(!product_modes(root,content));
    CHECK(win2k_uwp_frame_apply(root,title,NULL,0));
    HWND surface=style_uwp_frame_surface(root);
    CHECK(DestroyWindow(title) && !IsWindow(surface));
    CHECK(DestroyWindow(root) && UnregisterClassW(cls.lpszClassName,module) &&
        UnregisterClassW(child.lpszClassName,module) && UnregisterClassW(L"ApplicationFrameTitleBarWindow",module));
    puts("WIN2K UWP FRAME PASS client hole, caption close, resize states, repeat, title restoration and destruction");
    return 0;
}
