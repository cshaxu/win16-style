#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <dwmapi.h>
static unsigned allocations,releases;
static BOOL deny_allocation;
static BOOL deny_policy;
static HRESULT WINAPI set_policy(HWND window,DWORD attribute,LPCVOID value,DWORD size)
{
    if(deny_policy){SetLastError(17);return E_ACCESSDENIED;}
    return DwmSetWindowAttribute(window,attribute,value,size);
}
static void *allocate(size_t count,size_t size)
{if(deny_allocation)return NULL;void *value=calloc(count,size);if(value)++allocations;return value;}
static void release(void *value){if(value)++releases;free(value);}
#define calloc allocate
#define free release
#define DwmSetWindowAttribute set_policy
#include "lib-win32-style/application/window/observed_frame.c"
#undef DwmSetWindowAttribute
#undef calloc
#undef free

static DWORD native_theme(style_observed_frame_theme *theme,style_native_scroll *scroll)
{(void)theme;(void)scroll;return ERROR_SUCCESS;}
static HFONT native_font(const style_font_descriptor *font,UINT dpi,BOOL tool)
{
    (void)font;
    NONCLIENTMETRICSW metrics={.cbSize=sizeof(metrics)};
    if(!SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS,sizeof(metrics),&metrics,0,dpi))return NULL;
    return CreateFontIndirectW(tool?&metrics.lfSmCaptionFont:&metrics.lfCaptionFont);
}
static const style_observed_frame_provider provider={L"Win2kStyle.Frame",L"Win2kStyle.RecalculateFrame",native_theme,native_font};
static BOOL attach(HWND window)
{return style_observed_frame_attach(window,&provider);}

#define CHECK(x) do { if(!(x)){fprintf(stderr,"WIN2K_INSTANCE_FAIL line=%d allocations=%u releases=%u error=%lu\n",__LINE__,allocations,releases,GetLastError());return 1;} } while(0)
static BOOL reentrant_removed,reentrant_attached,retained_during_callback;
static BOOL remove_on_caption,caption_removed,caption_retained;
static BOOL testing_buttons;
static UINT last_command,commands;
static unsigned composed_paints;
static BOOL watch_composed_paints;
static BOOL extends_client;
static LRESULT CALLBACK native(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(extends_client && message==WM_NCCALCSIZE)return 0;
    if(watch_composed_paints && message==WM_NCPAINT){
        frame_window *frame=frame_state(window);
        if(frame && frame->composed_repaint.repainting)++composed_paints;
    }
    if(testing_buttons && message==WM_SYSCOMMAND){last_command=(UINT)wp&0xfff0;++commands;return 0;}
    if(message==WM_GETTITLEBARINFOEX && remove_on_caption){
        remove_on_caption=FALSE;
        style_instance *instance=style_instance_lookup(window,frame_proc);
        caption_retained=instance && instance->calls>=2;
        caption_removed=style_observed_frame_remove(window);
    }
    if(message==WM_APP){
        unsigned before=releases;
        reentrant_removed=style_observed_frame_remove(window);
        retained_during_callback=releases==before;
        reentrant_attached=attach(window);
    }
    return DefWindowProcW(window,message,wp,lp);
}
static BOOL pump_repaint(frame_window *frame)
{
    ULONGLONG deadline=GetTickCount64()+2000;
    MSG message;
    while(frame->composed_repaint.pending && GetTickCount64()<deadline){
        while(PeekMessageW(&message,NULL,0,0,PM_REMOVE))DispatchMessageW(&message);
        if(frame->composed_repaint.pending)MsgWaitForMultipleObjects(0,NULL,FALSE,20,QS_ALLINPUT);
    }
    return !frame->composed_repaint.pending;
}
int main(void)
{
    WNDCLASSW cls={.lpfnWndProc=native,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"StyleInstanceLifetime"};
    CHECK(RegisterClassW(&cls));
    for(unsigned i=0;i<8;++i){
        HWND window=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,cls.hInstance,NULL);
        CHECK(window && attach(window) && style_observed_frame_attached(window));
        CHECK(style_observed_frame_remove(window) && !style_observed_frame_attached(window) && allocations==releases);
        CHECK(attach(window));
        CHECK(DestroyWindow(window) && allocations==releases);
    }
    HWND window=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,cls.hInstance,NULL);
    CHECK(window && attach(window));
    unsigned before=releases;
    SendMessageW(window,WM_APP,0,0);
    CHECK(reentrant_removed && reentrant_attached && retained_during_callback && releases==before+1);
    CHECK(style_observed_frame_attached(window) && DestroyWindow(window) && allocations==releases);

    window=CreateWindowW(cls.lpszClassName,L"",WS_POPUP,0,0,200,100,NULL,NULL,cls.hInstance,NULL);
    CHECK(window && attach(window) && !style_observed_frame_attached(window));
    unsigned pending_allocations=allocations;
    CHECK(attach(window) && allocations==pending_allocations && !style_observed_frame_attached(window));
    CHECK(style_observed_frame_remove(window) && allocations==releases);
    CHECK(attach(window));
    LONG_PTR previous=GetWindowLongPtrW(window,GWL_STYLE);
    LONG_PTR returned=SetWindowLongPtrW(window,GWL_STYLE,WS_OVERLAPPEDWINDOW);
    printf("WIN2K_STYLE_VALUES previous=%llx returned=%llx\n",(unsigned long long)(UINT_PTR)previous,(unsigned long long)(UINT_PTR)returned);
    CHECK((DWORD)returned==(DWORD)previous);
    CHECK(style_observed_frame_attached(window) && DestroyWindow(window) && allocations==releases);

    window=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,cls.hInstance,NULL);
    CHECK(window);
    deny_allocation=TRUE;
    CHECK(!attach(window) && GetLastError()==ERROR_NOT_ENOUGH_MEMORY && allocations==releases);
    deny_allocation=FALSE;
    deny_policy=TRUE;
    CHECK(!attach(window) && GetLastError()==ERROR_ACCESS_DENIED && allocations==releases && !style_observed_frame_attached(window));
    CHECK(!GetPropW(window,L"Win2kStyle.Frame"));deny_policy=FALSE;
    style_instance foreign={0};
    CHECK(SetWindowSubclass(window,frame_proc,(UINT_PTR)frame_proc,(DWORD_PTR)&foreign));
    CHECK(!attach(window) && GetLastError()==ERROR_ALREADY_EXISTS && allocations==releases);
    CHECK(RemoveWindowSubclass(window,frame_proc,(UINT_PTR)frame_proc) && DestroyWindow(window));
    window=CreateWindowW(cls.lpszClassName,L"Caption reentry",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,cls.hInstance,NULL);
    CHECK(window && attach(window));
    remove_on_caption=TRUE;paint(window);
    CHECK(caption_removed && caption_retained && allocations==releases && !style_observed_frame_attached(window));
    CHECK(DestroyWindow(window));
    window=CreateWindowW(cls.lpszClassName,L"Caption commands",WS_OVERLAPPEDWINDOW,20,20,300,180,NULL,NULL,cls.hInstance,NULL);
    CHECK(window && attach(window));testing_buttons=TRUE;
    const UINT hits[]={HTMINBUTTON,HTMAXBUTTON,HTCLOSE},expected[]={SC_MINIMIZE,SC_MAXIMIZE,SC_CLOSE};
    const unsigned indices[]={2,3,5};
    for(unsigned i=0;i<3;++i){
        TITLEBARINFOEX info={.cbSize=sizeof(info)};
        SendMessageW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
        CHECK(!(info.rgstate[indices[i]]&(STATE_SYSTEM_INVISIBLE|STATE_SYSTEM_UNAVAILABLE)));
        SendMessageW(window,WM_NCLBUTTONDOWN,hits[i],0);
        CHECK(GetCapture()==window && !style_observed_frame_remove(window) && GetLastError()==ERROR_BUSY);
        SendMessageW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
        CHECK(info.rgstate[indices[i]]&STATE_SYSTEM_PRESSED);
        RECT bounds=info.rgrect[indices[i]];POINT point={(bounds.left+bounds.right)/2,(bounds.top+bounds.bottom)/2};
        CHECK(ScreenToClient(window,&point));
        SendMessageW(window,WM_LBUTTONUP,0,MAKELPARAM(point.x,point.y));
        CHECK(commands==i+1 && last_command==expected[i] && GetCapture()!=window);
    }
    SendMessageW(window,WM_NCLBUTTONDOWN,HTMINBUTTON,0);CHECK(GetCapture()==window);
    SendMessageW(window,WM_CANCELMODE,0,0);
    CHECK(GetCapture()!=window && commands==3 && style_observed_frame_remove(window));
    testing_buttons=FALSE;CHECK(DestroyWindow(window) && allocations==releases);

    WNDCLASSW bridge={.lpfnWndProc=DefWindowProcW,.hInstance=cls.hInstance,
                       .lpszClassName=L"Microsoft.UI.Content.DesktopChildSiteBridge"};
    CHECK(RegisterClassW(&bridge));
    extends_client=TRUE;
    window=CreateWindowW(cls.lpszClassName,L"Composed frame",WS_OVERLAPPEDWINDOW,20,20,300,180,
                         NULL,NULL,cls.hInstance,NULL);
    HWND child=CreateWindowW(bridge.lpszClassName,L"",WS_CHILD,0,0,100,30,window,NULL,cls.hInstance,NULL);
    RECT original_outer;
    CHECK(GetWindowRect(window,&original_outer));
    CHECK(window && child && attach(window));
    frame_window *frame=frame_state(window);
    CHECK(frame && frame->native.refresh_pending);
    SendMessageW(window,frame->native.refresh_message,0,0);
    CHECK(frame->native.refresh_pending && frame->composed_repaint.waiting && frame->composed_repaint.pending);
    ShowWindow(window,SW_SHOWNOACTIVATE);
    ShowWindow(child,SW_SHOWNOACTIVATE);
    CHECK(pump_repaint(frame) && !frame->native.refresh_pending && !frame->composed_repaint.waiting);
    RECT outer,client;
    CHECK(GetWindowRect(window,&outer));
    style_client_screen(window,&client);
    CHECK(client.top>=outer.top+caption_height(window) &&
          client.left==outer.left && client.right==outer.right && client.bottom==outer.bottom);
    watch_composed_paints=TRUE;
    HDC dc=GetDC(window);CHECK(dc);
    SendMessageW(window,WM_ERASEBKGND,(WPARAM)dc,0);
    SendMessageW(window,WM_ERASEBKGND,(WPARAM)dc,0);
    CHECK(frame->composed_repaint.pending && pump_repaint(frame) && composed_paints==1);
    watch_composed_paints=FALSE;
    SendMessageW(window,WM_ERASEBKGND,(WPARAM)dc,0);
    ReleaseDC(window,dc);
    HWND timer=frame->composed_repaint.timer;
    CHECK(frame->composed_repaint.pending && IsWindow(timer));
    CHECK(style_observed_frame_remove(window) && !IsWindow(timer) && allocations==releases);
    CHECK(GetWindowRect(window,&outer));
    style_client_screen(window,&client);
    CHECK(EqualRect(&outer,&client) && EqualRect(&outer,&original_outer));
    CHECK(DestroyWindow(window) && UnregisterClassW(bridge.lpszClassName,cls.hInstance));

    CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance));
    puts("WIN2K_INSTANCE_OK repeat=8 remove=1 terminal=1 reentrant_replace=1 pending=1 pending_repeat=1 allocation_failure=1 attach_failure=1 caption_reentry=1 caption_commands=3 caption_busy_cancel=1 composed_show=1 composed_repaint=1 composed_cancel=1 cleanup=1");
    return 0;
}
