#include "ntw16s-dll/ntw16s.h"
#include <stdio.h>
#include <wchar.h>
#include <dwmapi.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FRAME FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static LRESULT CALLBACK owner(HWND w,UINT m,WPARAM a,LPARAM b)
{if(m==WM_APP+8)return 808;return DefWindowProcW(w,m,a,b);}
static LRESULT CALLBACK custom_caption(HWND w,UINT m,WPARAM a,LPARAM b)
{if(m==WM_NCCALCSIZE)return 0;return DefWindowProcW(w,m,a,b);}
static unsigned xaml_paints,xaml_erases,xaml_timers;
static HWND repaint_watch;
static HWND destroy_on_repaint;
static unsigned repaint_messages;
static LRESULT CALLBACK watch_repaint(int code,WPARAM wp,LPARAM lp)
{
    if(code>=0 && lp){const CWPSTRUCT *message=(const CWPSTRUCT *)lp;
        if(message->hwnd==repaint_watch && message->message==WM_NCPAINT)++repaint_messages;
        if(message->hwnd==destroy_on_repaint && message->message==WM_NCPAINT){
            HWND root=destroy_on_repaint;destroy_on_repaint=NULL;DestroyWindow(root);}}
    return CallNextHookEx(NULL,code,wp,lp);
}
static LRESULT xaml_nccalc_result;
static HWND destroy_during_timer,created_timer,destroy_during_icon_style;
static LRESULT CALLBACK destroy_on_timer_create(int code,WPARAM wp,LPARAM lp)
{
    if(code==HCBT_CREATEWND && destroy_during_timer){
        WCHAR name[32]={0};GetClassNameW((HWND)wp,name,ARRAYSIZE(name));
        if(!lstrcmpiW(name,L"Static")){
            HWND root=destroy_during_timer;destroy_during_timer=NULL;created_timer=(HWND)wp;DestroyWindow(root);
        }
    }
    return CallNextHookEx(NULL,code,wp,lp);
}
static LRESULT CALLBACK xaml_frame(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_STYLECHANGED && destroy_during_icon_style==w && a==(WPARAM)(LONG_PTR)GWL_STYLE &&
       !(((STYLESTRUCT *)b)->styleNew&WS_DLGFRAME)){
        destroy_during_icon_style=NULL;DestroyWindow(w);return 0;
    }
    if(m==WM_NCCALCSIZE && a && xaml_nccalc_result){DefWindowProcW(w,m,a,b);return xaml_nccalc_result;}
    if(m==WM_ERASEBKGND){++xaml_erases;return 1;}
    if(m==WM_TIMER)++xaml_timers;
    if(m==WM_PAINT){PAINTSTRUCT ps;++xaml_paints;BeginPaint(w,&ps);EndPaint(w,&ps);return 0;}
    return DefWindowProcW(w,m,a,b);
}
static void pump_for(DWORD ms)
{
    ULONGLONG end=GetTickCount64()+ms;MSG msg;
    do{while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);}Sleep(1);}while(GetTickCount64()<end);
}
static void client_screen(HWND w,RECT *r)
{GetClientRect(w,r);MapWindowPoints(w,NULL,(POINT *)r,2);}

static int frame_worker(int argc,WCHAR **argv)
{
    static const DWORD styles[]={WS_OVERLAPPEDWINDOW,WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
        WS_POPUP|WS_THICKFRAME,WS_POPUP|WS_BORDER,WS_POPUP,WS_POPUP|WS_CAPTION|WS_SYSMENU};
    static const DWORD excluded[]={WS_EX_CLIENTEDGE,WS_EX_TOOLWINDOW,WS_EX_LAYERED};
    WCHAR text[80];
    WNDCLASSW cls={0};unsigned i;HWND w;RECT before,after;win16_style_palette palette;
    (void)argc;(void)argv;
    CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));
    cls.lpfnWndProc=owner;cls.lpszClassName=L"Win16FrameForms";cls.hInstance=GetModuleHandleW(NULL);
    CHECK(RegisterClassW(&cls));
    for(i=0;i<sizeof(styles)/sizeof(styles[0]);++i){
        DWORD ex=i==5?WS_EX_DLGMODALFRAME:0;LRESULT hit=0;POINT center;
        w=CreateWindowExW(ex,cls.lpszClassName,L"Unchanged title",styles[i],100,100,420,280,
            NULL,NULL,cls.hInstance,NULL);CHECK(w);
        client_screen(w,&before);CHECK(win16_style_install(w,NULL));client_screen(w,&after);
        CHECK(EqualRect(&before,&after));
        CHECK(GetWindowTextW(w,text,80) && !wcscmp(text,L"Unchanged title"));
        CHECK(SendMessageW(w,WM_APP+8,0,0)==808);
        center.x=(after.left+after.right)/2;center.y=(after.top+after.bottom)/2;
        CHECK(win16_style_hit_test(w,MAKELPARAM(center.x,center.y),&hit) && hit==HTCLIENT);
        CHECK(SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(center.x,center.y))==hit);
        win16_style_default_palette(&palette);palette.active=RGB(0,0,128);
        CHECK(win16_style_set_palette(w,&palette));
        CHECK(SetWindowTextW(w,L"Dynamic native title") && GetWindowTextW(w,text,80) && !wcscmp(text,L"Dynamic native title"));
        if(i==0){
            HWND native=CreateWindowW(cls.lpszClassName,L"Native state oracle",styles[i],
                100,100,420,280,NULL,NULL,cls.hInstance,NULL);CHECK(native);
            /* Restoring a minimized, previously maximized window may retain
             * maximization. Compare USER's actual state, not a guessed rule. */
            ShowWindow(native,SW_SHOWMAXIMIZED);ShowWindow(w,SW_SHOWMAXIMIZED);
            CHECK(IsZoomed(w)==IsZoomed(native) && IsIconic(w)==IsIconic(native));
            ShowWindow(native,SW_MINIMIZE);ShowWindow(w,SW_MINIMIZE);
            CHECK(IsZoomed(w)==IsZoomed(native) && IsIconic(w)==IsIconic(native));
            ShowWindow(native,SW_RESTORE);ShowWindow(w,SW_RESTORE);
            CHECK(IsZoomed(w)==IsZoomed(native) && IsIconic(w)==IsIconic(native));
            ShowWindow(native,SW_SHOWNORMAL);ShowWindow(w,SW_SHOWNORMAL);
            CHECK(!IsIconic(native) && !IsZoomed(native) && !IsIconic(w) && !IsZoomed(w));
            CHECK(DestroyWindow(native));
        }
        client_screen(w,&before);CHECK(win16_style_remove(w));client_screen(w,&after);
        CHECK(EqualRect(&before,&after));CHECK(!win16_style_remove(w) && GetLastError()==ERROR_NOT_FOUND);
        CHECK(DestroyWindow(w));
    }
    for(i=0;i<sizeof(excluded)/sizeof(excluded[0]);++i){
        w=CreateWindowExW(excluded[i],cls.lpszClassName,L"Unsupported form",WS_OVERLAPPEDWINDOW,
            100,100,420,280,NULL,NULL,cls.hInstance,NULL);CHECK(w);
        client_screen(w,&before);CHECK(!win16_style_install(w,NULL) && GetLastError()==ERROR_NOT_SUPPORTED);
        client_screen(w,&after);CHECK(EqualRect(&before,&after) && DestroyWindow(w));
    }
    {
        WNDCLASSW custom={.lpfnWndProc=custom_caption,.lpszClassName=L"Win16CustomCaptionFixture",.hInstance=cls.hInstance};
        RECT original_outer,restored_outer;
        CHECK(RegisterClassW(&custom));
        w=CreateWindowW(custom.lpszClassName,L"Custom caption fixture",WS_OVERLAPPEDWINDOW,
            100,100,420,280,NULL,NULL,custom.hInstance,NULL);CHECK(w);
        CHECK(GetWindowRect(w,&original_outer));client_screen(w,&before);
        CHECK(before.top==original_outer.top);
        CHECK(win16_style_install(w,NULL));client_screen(w,&after);CHECK(EqualRect(&before,&after));
        CHECK(win16_style_remove(w));client_screen(w,&after);CHECK(EqualRect(&before,&after));
        CHECK(GetWindowRect(w,&restored_outer) && EqualRect(&original_outer,&restored_outer));
        CHECK(DestroyWindow(w) && UnregisterClassW(custom.lpszClassName,custom.hInstance));
    }
    {
        WNDCLASSW root={.lpfnWndProc=xaml_frame,.lpszClassName=L"Win16XamlFrameFixture",.hInstance=cls.hInstance};
        WNDCLASSW bridge={.lpfnWndProc=DefWindowProcW,.lpszClassName=L"Windows.UI.Composition.DesktopWindowContentBridge",.hInstance=cls.hInstance};
        CHECK(RegisterClassW(&root) && RegisterClassW(&bridge));
        w=CreateWindowW(root.lpszClassName,L"XAML frame",WS_OVERLAPPEDWINDOW,100,100,420,280,NULL,NULL,root.hInstance,NULL);CHECK(w);
        HWND child=CreateWindowW(bridge.lpszClassName,L"",WS_CHILD|WS_VISIBLE,0,0,200,100,w,NULL,bridge.hInstance,NULL);CHECK(child);
        NCCALCSIZE_PARAMS params={0};WINDOWPOS pos={0};params.lppos=&pos;GetWindowRect(w,&params.rgrc[0]);
        xaml_nccalc_result=WVR_REDRAW;
        CHECK(SendMessageW(w,WM_NCCALCSIZE,TRUE,(LPARAM)&params)==WVR_REDRAW);
        CHECK(win16_style_install(w,NULL));ShowWindow(w,SW_SHOWNOACTIVATE);UpdateWindow(w);pump_for(100);
        RECT icon_before,icon_after,icon_outer_before,icon_outer_after;
        client_screen(w,&icon_before);CHECK(GetWindowRect(w,&icon_outer_before));
        LONG_PTR icon_style=GetWindowLongPtrW(w,GWL_STYLE);HICON sample=LoadIconW(NULL,IDI_WARNING);CHECK(sample);
        HICON initial=(HICON)(uintptr_t)SendMessageW(w,WM_GETICON,ICON_SMALL,0);
        HICON previous=(HICON)(uintptr_t)SendMessageW(w,WM_SETICON,ICON_SMALL,(LPARAM)sample);
        CHECK(previous==initial && (HICON)(uintptr_t)SendMessageW(w,WM_GETICON,ICON_SMALL,0)==sample);
        CHECK((HICON)(uintptr_t)SendMessageW(w,WM_SETICON,ICON_SMALL,(LPARAM)previous)==sample);
        CHECK((HICON)(uintptr_t)SendMessageW(w,WM_GETICON,ICON_SMALL,0)==initial);
        client_screen(w,&icon_after);CHECK(GetWindowRect(w,&icon_outer_after));
        CHECK(GetWindowLongPtrW(w,GWL_STYLE)==icon_style && EqualRect(&icon_before,&icon_after) && EqualRect(&icon_outer_before,&icon_outer_after));pump_for(150);
        const LRESULT flags[]={WVR_REDRAW,WVR_HREDRAW,WVR_VREDRAW,WVR_VALIDRECTS};
        for(unsigned n=0;n<sizeof(flags)/sizeof(flags[0]);++n){xaml_nccalc_result=flags[n];GetWindowRect(w,&params.rgrc[0]);
            CHECK(SendMessageW(w,WM_NCCALCSIZE,TRUE,(LPARAM)&params)==(flags[n]==WVR_VALIDRECTS?0:flags[n]));}
        xaml_nccalc_result=0;
        repaint_watch=w;repaint_messages=0;
        HHOOK repaint_hook=SetWindowsHookExW(WH_CALLWNDPROC,watch_repaint,NULL,GetCurrentThreadId());CHECK(repaint_hook);
        xaml_paints=xaml_erases=xaml_timers=0;HDC dc=GetDC(w);CHECK(dc);
        for(int n=0;n<5;++n){CHECK(SendMessageW(w,WM_ERASEBKGND,(WPARAM)dc,0)==1);}ReleaseDC(w,dc);
        CHECK(xaml_erases==5 && xaml_paints==0);pump_for(150);
        CHECK(xaml_erases==5 && xaml_paints==0 && xaml_timers==0 && repaint_messages>0);
        CHECK(UnhookWindowsHookEx(repaint_hook));repaint_watch=NULL;
        dc=GetDC(w);CHECK(dc);CHECK(SendMessageW(w,WM_ERASEBKGND,(WPARAM)dc,0)==1);ReleaseDC(w,dc);
        Sleep(50);MSG queued;CHECK(PeekMessageW(&queued,NULL,WM_TIMER,WM_TIMER,PM_NOREMOVE));CHECK(queued.hwnd!=w && IsWindow(queued.hwnd));
        HWND timer=queued.hwnd;CHECK(win16_style_remove(w));CHECK(!IsWindow(timer));UpdateWindow(w);xaml_timers=0;pump_for(100);CHECK(xaml_timers==0);
        CHECK(DestroyWindow(w));
        w=CreateWindowW(root.lpszClassName,L"XAML icon style destroy",WS_OVERLAPPEDWINDOW,100,100,420,280,NULL,NULL,root.hInstance,NULL);CHECK(w);
        child=CreateWindowW(bridge.lpszClassName,L"",WS_CHILD|WS_VISIBLE,0,0,200,100,w,NULL,bridge.hInstance,NULL);CHECK(child);
        CHECK(win16_style_install(w,NULL));destroy_during_icon_style=w;
        SendMessageW(w,WM_GETICON,ICON_SMALL,0);CHECK(!IsWindow(w) && !destroy_during_icon_style);
        w=CreateWindowW(root.lpszClassName,L"XAML destroy",WS_OVERLAPPEDWINDOW,100,100,420,280,NULL,NULL,root.hInstance,NULL);CHECK(w);
        child=CreateWindowW(bridge.lpszClassName,L"",WS_CHILD|WS_VISIBLE,0,0,200,100,w,NULL,bridge.hInstance,NULL);CHECK(child);
        CHECK(win16_style_install(w,NULL));ShowWindow(w,SW_SHOWNOACTIVATE);UpdateWindow(w);pump_for(100);
        dc=GetDC(w);CHECK(dc);CHECK(SendMessageW(w,WM_ERASEBKGND,(WPARAM)dc,0)==1);ReleaseDC(w,dc);
        Sleep(50);CHECK(PeekMessageW(&queued,NULL,WM_TIMER,WM_TIMER,PM_NOREMOVE));timer=queued.hwnd;CHECK(timer!=w && IsWindow(timer));
        CHECK(DestroyWindow(w));CHECK(!IsWindow(timer));xaml_timers=0;pump_for(100);CHECK(xaml_timers==0);
        w=CreateWindowW(root.lpszClassName,L"Classic frame",WS_OVERLAPPEDWINDOW,100,100,420,280,NULL,NULL,root.hInstance,NULL);CHECK(w);
        CHECK(win16_style_install(w,NULL));ShowWindow(w,SW_SHOWNOACTIVATE);UpdateWindow(w);
        xaml_paints=xaml_erases=0;dc=GetDC(w);CHECK(dc);
        CHECK(SendMessageW(w,WM_ERASEBKGND,(WPARAM)dc,0)==1);ReleaseDC(w,dc);
        CHECK(xaml_erases==1 && xaml_paints==0);
        CHECK(win16_style_remove(w) && DestroyWindow(w));
        w=CreateWindowW(root.lpszClassName,L"XAML repaint destroy",WS_OVERLAPPEDWINDOW,100,100,420,280,NULL,NULL,root.hInstance,NULL);CHECK(w);
        child=CreateWindowW(bridge.lpszClassName,L"",WS_CHILD|WS_VISIBLE,0,0,200,100,w,NULL,bridge.hInstance,NULL);CHECK(child);
        CHECK(win16_style_install(w,NULL));ShowWindow(w,SW_SHOWNOACTIVATE);UpdateWindow(w);pump_for(100);
        repaint_hook=SetWindowsHookExW(WH_CALLWNDPROC,watch_repaint,NULL,GetCurrentThreadId());CHECK(repaint_hook);
        dc=GetDC(w);CHECK(dc);CHECK(SendMessageW(w,WM_ERASEBKGND,(WPARAM)dc,0)==1);ReleaseDC(w,dc);
        Sleep(50);CHECK(PeekMessageW(&queued,NULL,WM_TIMER,WM_TIMER,PM_NOREMOVE));timer=queued.hwnd;CHECK(timer!=w && IsWindow(timer));
        destroy_on_repaint=w;pump_for(100);CHECK(!IsWindow(w) && !IsWindow(timer));CHECK(UnhookWindowsHookEx(repaint_hook));
        w=CreateWindowW(root.lpszClassName,L"XAML reentrant destroy",WS_OVERLAPPEDWINDOW,100,100,420,280,NULL,NULL,root.hInstance,NULL);CHECK(w);
        child=CreateWindowW(bridge.lpszClassName,L"",WS_CHILD|WS_VISIBLE,0,0,200,100,w,NULL,bridge.hInstance,NULL);CHECK(child);
        destroy_during_timer=w;created_timer=NULL;
        HHOOK cbt=SetWindowsHookExW(WH_CBT,destroy_on_timer_create,NULL,GetCurrentThreadId());CHECK(cbt);
        BOOL installed=win16_style_install(w,NULL);
        if(installed && IsWindow(w)){dc=GetDC(w);CHECK(dc);SendMessageW(w,WM_ERASEBKGND,(WPARAM)dc,0);ReleaseDC(w,dc);}
        UnhookWindowsHookEx(cbt);
        CHECK(!IsWindow(w));CHECK(created_timer);CHECK(!IsWindow(created_timer));pump_for(80);
        WNDCLASSW edge={.lpfnWndProc=custom_caption,.lpszClassName=L"Win16XamlEdgeFixture",.hInstance=cls.hInstance};
        MONITORINFO monitor={.cbSize=sizeof(monitor)};POINT origin={0,0};RECT edge_before,edge_after;
        CHECK(GetMonitorInfoW(MonitorFromPoint(origin,MONITOR_DEFAULTTONEAREST),&monitor));CHECK(RegisterClassW(&edge));
        for(unsigned edge_case=0;edge_case<2;edge_case++){
            w=CreateWindowW(edge.lpszClassName,L"XAML work-area edge",WS_OVERLAPPEDWINDOW,
                monitor.rcWork.left+(edge_case?0:100),monitor.rcWork.top+(edge_case?100:0),420,280,NULL,NULL,edge.hInstance,NULL);CHECK(w);
            child=CreateWindowW(bridge.lpszClassName,L"",WS_CHILD|WS_VISIBLE,0,0,200,100,w,NULL,bridge.hInstance,NULL);CHECK(child);
            CHECK(GetWindowRect(w,&edge_before));CHECK(win16_style_install(w,NULL));CHECK(GetWindowRect(w,&edge_after));
            CHECK(edge_after.top>=monitor.rcWork.top && edge_after.left>=monitor.rcWork.left);CHECK(win16_style_remove(w));CHECK(GetWindowRect(w,&edge_after));
            CHECK(EqualRect(&edge_before,&edge_after));CHECK(DestroyWindow(w));
        }
        CHECK(UnregisterClassW(edge.lpszClassName,edge.hInstance));
        CHECK(UnregisterClassW(bridge.lpszClassName,bridge.hInstance) && UnregisterClassW(root.lpszClassName,root.hInstance));
    }
    /* Read the documented query attribute; exercise both initial states and
     * two cycles so an apparently successful remove cannot hide disabled DWM. */
    for(DWORD policy=DWMNCRP_DISABLED;policy<=DWMNCRP_ENABLED;++policy){
        w=CreateWindowW(cls.lpszClassName,L"DWM restore",WS_OVERLAPPEDWINDOW,
            100,100,420,280,NULL,NULL,cls.hInstance,NULL);CHECK(w);
        CHECK(SUCCEEDED(DwmSetWindowAttribute(w,DWMWA_NCRENDERING_POLICY,&policy,sizeof(policy))));
        for(unsigned cycle=0;cycle<2;++cycle){
            BOOL original=FALSE,current=FALSE;
            CHECK(SUCCEEDED(DwmGetWindowAttribute(w,DWMWA_NCRENDERING_ENABLED,&original,sizeof(original))));
            CHECK(original==(policy==DWMNCRP_ENABLED));
            CHECK(win16_style_install(w,NULL));
            CHECK(SUCCEEDED(DwmGetWindowAttribute(w,DWMWA_NCRENDERING_ENABLED,&current,sizeof(current))) && !current);
            CHECK(win16_style_remove(w));
            CHECK(SUCCEEDED(DwmGetWindowAttribute(w,DWMWA_NCRENDERING_ENABLED,&current,sizeof(current))) && current==original);
        }
        CHECK(DestroyWindow(w));
    }
    CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance));
    puts("FRAME PASS six forms, client screen preservation, same geometry hit test, title/palette, maximize/minimize/restore, three exclusions");
    return 0;
}

int wmain(int argc,WCHAR **argv)
{return isolated_process(argc,argv,frame_worker);}
