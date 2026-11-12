#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <dwmapi.h>
#include <commctrl.h>
#include <stdio.h>
#include "ntw2ks-dll/window.h"
#include "lib-win32/style-common/global_window.h"

static BOOL attached(HWND window){return Ntw2ksWindowAttached(window);}
static BOOL active(DWORD cookie){return cookie==42;}
static const style_global_window_ops ops={attached,Ntw2ksWindowAttach,Ntw2ksWindowRemove,active,
    L"Win2kFrameLifecycleTest.Control",3,1234};

static LRESULT CALLBACK procedure(HWND window,UINT message,WPARAM wp,LPARAM lp)
{return DefWindowProcW(window,message,wp,lp);}
static LRESULT CALLBACK resize_frame(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR ref)
{
    (void)id;(void)ref;
    LRESULT result=DefSubclassProc(window,message,wp,lp);
    if(message==WM_NCCALCSIZE && lp){
        RECT *rect=wp?&((NCCALCSIZE_PARAMS *)lp)->rgrc[0]:(RECT *)lp;
        rect->left-=4;rect->right+=4;rect->top-=10;rect->bottom+=4;
    }
    return result;
}
static COLORREF caption_sample(HWND window,const RECT *title)
{
    RECT outer;if(!GetWindowRect(window,&outer))return CLR_INVALID;
    HDC dc=GetWindowDC(window);if(!dc)return CLR_INVALID;
    int x=title->left-outer.left+(title->right-title->left)/2;
    int y=title->top-outer.top+4;
    COLORREF color=GetPixel(dc,x,y);ReleaseDC(window,dc);return color;
}
static DWORD WINAPI remove_other_thread(void *value)
{
    BOOL removed=Ntw2ksWindowRemove((HWND)value);
    return !removed && GetLastError()==ERROR_INVALID_THREAD_ID?0:1;
}

int main(int argc,char **argv)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    WNDCLASSW cls={.lpfnWndProc=procedure,.hInstance=instance,.lpszClassName=L"Win2kFrameLifecycleTest"};
    if(!RegisterClassW(&cls))return 2;
    HWND baseline=CreateWindowW(cls.lpszClassName,L"Native DWM",WS_OVERLAPPEDWINDOW,80,80,240,160,NULL,NULL,instance,NULL);
    BOOL native_before=FALSE,native_after=FALSE;DWORD mode=DWMNCRP_DISABLED;
    if(baseline){ShowWindow(baseline,SW_SHOWNOACTIVATE);UpdateWindow(baseline);
        DwmFlush();
        DwmGetWindowAttribute(baseline,DWMWA_NCRENDERING_ENABLED,&native_before,sizeof(native_before));
        DwmSetWindowAttribute(baseline,DWMWA_NCRENDERING_POLICY,&mode,sizeof(mode));
        SetWindowPos(baseline,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);mode=DWMNCRP_ENABLED;
        RedrawWindow(baseline,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN);
        HDC dc=GetWindowDC(baseline);if(dc){
            HDC mem=CreateCompatibleDC(dc);BITMAPINFO bi={0};void *pixels=NULL;
            bi.bmiHeader.biSize=sizeof(bi.bmiHeader);bi.bmiHeader.biWidth=20;bi.bmiHeader.biHeight=-20;
            bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;bi.bmiHeader.biCompression=BI_RGB;
            HBITMAP bmp=CreateDIBSection(dc,&bi,DIB_RGB_COLORS,&pixels,NULL,0);
            if(mem && bmp){HGDIOBJ old=SelectObject(mem,bmp);RECT rect={0,0,20,20};
                DrawFrameControl(mem,&rect,DFC_CAPTION,DFCS_CAPTIONMIN);
                DrawFrameControl(mem,&rect,DFC_CAPTION,DFCS_CAPTIONMAX);
                DrawFrameControl(mem,&rect,DFC_CAPTION,DFCS_CAPTIONCLOSE);BitBlt(dc,5,5,20,20,mem,0,0,SRCCOPY);
                if(old)SelectObject(mem,old);}
            if(bmp)DeleteObject(bmp);
            if(mem)DeleteDC(mem);
            GdiFlush();ReleaseDC(baseline,dc);}
        MSG baseline_message;while(PeekMessageW(&baseline_message,NULL,0,0,PM_REMOVE)){TranslateMessage(&baseline_message);DispatchMessageW(&baseline_message);}
        DwmSetWindowAttribute(baseline,DWMWA_NCRENDERING_POLICY,&mode,sizeof(mode));
        SetWindowPos(baseline,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
        TITLEBARINFOEX base_title={.cbSize=sizeof(base_title)};SendMessageW(baseline,WM_GETTITLEBARINFOEX,0,(LPARAM)&base_title);
        DwmGetWindowAttribute(baseline,DWMWA_NCRENDERING_ENABLED,&native_after,sizeof(native_after));DestroyWindow(baseline);}
    BOOL resized_restore=FALSE;
    baseline=CreateWindowW(cls.lpszClassName,L"Custom frame DWM",WS_OVERLAPPEDWINDOW,80,80,240,160,NULL,NULL,instance,NULL);
    if(baseline){ShowWindow(baseline,SW_SHOWNOACTIVATE);UpdateWindow(baseline);
        SetWindowSubclass(baseline,resize_frame,1,0);mode=DWMNCRP_DISABLED;
        DwmSetWindowAttribute(baseline,DWMWA_NCRENDERING_POLICY,&mode,sizeof(mode));
        SetWindowPos(baseline,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
        RemoveWindowSubclass(baseline,resize_frame,1);mode=DWMNCRP_ENABLED;
        DwmSetWindowAttribute(baseline,DWMWA_NCRENDERING_POLICY,&mode,sizeof(mode));
        SetWindowPos(baseline,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
        DwmGetWindowAttribute(baseline,DWMWA_NCRENDERING_ENABLED,&resized_restore,sizeof(resized_restore));DestroyWindow(baseline);}
    HWND window=CreateWindowW(cls.lpszClassName,L"Frame lifecycle",WS_OVERLAPPEDWINDOW,
                              80,80,420,260,NULL,NULL,instance,NULL);
    if(!window)return 2;
    ShowWindow(window,SW_SHOWNOACTIVATE);UpdateWindow(window);
    DwmFlush();
    printf("WIN2K WINDOW CONDITIONS style=%08lx ex=%08lx active=%d foreground=%d\n",
           (unsigned long)GetWindowLongW(window,GWL_STYLE),(unsigned long)GetWindowLongW(window,GWL_EXSTYLE),
           GetActiveWindow()==window,GetForegroundWindow()==window);
    TITLEBARINFOEX native={.cbSize=sizeof(native)},classic={.cbSize=sizeof(classic)},again={.cbSize=sizeof(again)};
    SendMessageW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)&native);
    COLORREF native_pixel=caption_sample(window,&native.rcTitleBar);
    BOOL before=FALSE,after=FALSE;
    BOOL observed=SUCCEEDED(DwmGetWindowAttribute(window,DWMWA_NCRENDERING_ENABLED,&before,sizeof(before)));
    BOOL direct=argc>1 && !lstrcmpA(argv[1],"--direct");
    BOOL installed=direct?Ntw2ksWindowAttach(window):
        style_global_window_apply(window,&ops) && style_global_window_attached(window,&ops) && attached(window);
    ULONG_PTR captured=(ULONG_PTR)GetPropW(window,L"Win2kStyle.PreviousDwmState");
    BOOL after_attach=FALSE,after_pump=FALSE;
    DwmGetWindowAttribute(window,DWMWA_NCRENDERING_ENABLED,&after_attach,sizeof(after_attach));
    MSG message;if(argc<2 || lstrcmpA(argv[1],"--no-pump"))
        while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
    DwmGetWindowAttribute(window,DWMWA_NCRENDERING_ENABLED,&after_pump,sizeof(after_pump));
    ULONG_PTR captured_after_pump=(ULONG_PTR)GetPropW(window,L"Win2kStyle.PreviousDwmState");
    SendMessageW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)&classic);
    COLORREF classic_pixel=caption_sample(window,&classic.rcTitleBar);
    RECT client_before,client_after;POINT origin={0,0};
    GetClientRect(window,&client_before);ClientToScreen(window,&origin);OffsetRect(&client_before,origin.x,origin.y);
    BOOL removed=installed && (direct?Ntw2ksWindowRemove(window):style_global_window_remove(window,&ops)) && !attached(window) &&
                 (direct || !style_global_window_attached(window,&ops));
    LONG_PTR style_after=GetWindowLongPtrW(window,GWL_STYLE),ex_after=GetWindowLongPtrW(window,GWL_EXSTYLE);
    BOOL immediate=FALSE;
    HRESULT immediate_status=DwmGetWindowAttribute(window,DWMWA_NCRENDERING_ENABLED,&immediate,sizeof(immediate));
    SendMessageW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)&again);
    COLORREF after_pixel=caption_sample(window,&again.rcTitleBar);
    origin.x=origin.y=0;GetClientRect(window,&client_after);ClientToScreen(window,&origin);OffsetRect(&client_after,origin.x,origin.y);
    BOOL geometry=native.rcTitleBar.bottom>native.rcTitleBar.top && classic.rcTitleBar.bottom>classic.rcTitleBar.top &&
                  again.rcTitleBar.bottom>again.rcTitleBar.top && EqualRect(&client_before,&client_after);
    DwmFlush();
    BOOL restored=!observed || (SUCCEEDED(DwmGetWindowAttribute(window,DWMWA_NCRENDERING_ENABLED,&after,sizeof(after))) && before==after);
    BOOL repeated=!style_global_window_remove(window,&ops) && GetLastError()==ERROR_NOT_FOUND;
    BOOL reattached=style_global_window_apply(window,&ops) && style_global_window_remove(window,&ops);
    HANDLE thread=CreateThread(NULL,0,remove_other_thread,window,0,NULL);DWORD thread_result=1;
    if(thread){if(WaitForSingleObject(thread,3000)==WAIT_OBJECT_0)GetExitCodeThread(thread,&thread_result);CloseHandle(thread);}
    HWND bare=CreateWindowW(cls.lpszClassName,L"Pending frame",WS_POPUP,80,80,200,100,NULL,NULL,instance,NULL);
    BOOL pending=bare && Ntw2ksWindowAttach(bare) && Ntw2ksWindowRemove(bare) &&
                 !Ntw2ksWindowRemove(bare) && GetLastError()==ERROR_NOT_FOUND;
    if(bare)DestroyWindow(bare);
    DestroyWindow(window);UnregisterClassW(cls.lpszClassName,instance);
    printf("WIN2K NATIVE DWM before=%d after=%d custom_frame_restore=%d\n",native_before,native_after,resized_restore);
    printf("WIN2K DWM AFTER REMOVE immediate=%d status=%08lx\n",immediate,(unsigned long)immediate_status);
    printf("WIN2K WINDOW AFTER REMOVE style=%08lx ex=%08lx\n",(unsigned long)style_after,(unsigned long)ex_after);
    printf("WIN2K DWM STAGES attach=%d pump=%d policy_after_pump=%llu\n",after_attach,after_pump,(unsigned long long)captured_after_pump);
    printf("WIN2K CAPTION PIXEL native=%06lx classic=%06lx after=%06lx\n",
           (unsigned long)native_pixel,(unsigned long)classic_pixel,(unsigned long)after_pixel);
    printf("WIN2K TITLE RECTS native=%ld,%ld,%ld,%ld classic=%ld,%ld,%ld,%ld after=%ld,%ld,%ld,%ld\n",
           native.rcTitleBar.left,native.rcTitleBar.top,native.rcTitleBar.right,native.rcTitleBar.bottom,
           classic.rcTitleBar.left,classic.rcTitleBar.top,classic.rcTitleBar.right,classic.rcTitleBar.bottom,
           again.rcTitleBar.left,again.rcTitleBar.top,again.rcTitleBar.right,again.rcTitleBar.bottom);
    printf("WIN2K FRAME LIFECYCLE bits=%u attached=%d removed=%d observed=%d before=%d after=%d captured=%llu restored=%d geometry=%d repeated=%d reattached=%d thread=%lu pending=%d\n",
           (unsigned)(sizeof(void *)*8),installed,removed,observed,before,after,(unsigned long long)captured,restored,geometry,repeated,reattached,thread_result,pending);
    return installed && removed && restored && geometry && repeated && reattached && !thread_result && pending?0:3;
}
