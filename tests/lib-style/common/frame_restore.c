#include <windows.h>
#include "lib-style/common/frame.h"

static unsigned calculations;
static LRESULT CALLBACK procedure(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_NCCALCSIZE)++calculations;
    return DefWindowProcW(window,message,wp,lp);
}
int main(void)
{
    if(style_frame_resize_cursor(HTLEFT)!=(LPCWSTR)IDC_SIZEWE || style_frame_resize_cursor(HTRIGHT)!=(LPCWSTR)IDC_SIZEWE ||
       style_frame_resize_cursor(HTTOP)!=(LPCWSTR)IDC_SIZENS || style_frame_resize_cursor(HTBOTTOM)!=(LPCWSTR)IDC_SIZENS ||
       style_frame_resize_cursor(HTSIZE)!=(LPCWSTR)IDC_SIZENWSE || style_frame_resize_cursor(HTTOPLEFT)!=(LPCWSTR)IDC_SIZENWSE ||
       style_frame_resize_cursor(HTBOTTOMRIGHT)!=(LPCWSTR)IDC_SIZENWSE || style_frame_resize_cursor(HTTOPRIGHT)!=(LPCWSTR)IDC_SIZENESW ||
       style_frame_resize_cursor(HTBOTTOMLEFT)!=(LPCWSTR)IDC_SIZENESW || style_frame_resize_cursor(HTCAPTION)!=(LPCWSTR)IDC_ARROW)return 7;
    static const WCHAR name[]=L"StyleFrameRestoreTest";
    WNDCLASSW cls={0};cls.hInstance=GetModuleHandleW(NULL);cls.lpfnWndProc=procedure;cls.lpszClassName=name;
    if(!RegisterClassW(&cls))return 1;
    HWND window=CreateWindowW(name,L"",WS_OVERLAPPEDWINDOW,80,80,280,180,NULL,NULL,cls.hInstance,NULL);
    if(!window)return 2;
    MONITORINFO monitor={.cbSize=sizeof(monitor)};RECT outer,visible,unchanged={1,2,3,4};
    if(!GetMonitorInfoW(MonitorFromWindow(window,MONITOR_DEFAULTTONEAREST),&monitor)){DestroyWindow(window);return 8;}
    outer=monitor.rcWork;InflateRect(&outer,10,10);
    if(!style_frame_work_area(window,&outer,&visible) || !EqualRect(&visible,&monitor.rcWork)){DestroyWindow(window);return 9;}
    outer=monitor.rcWork;OffsetRect(&outer,monitor.rcWork.right-monitor.rcWork.left+100,0);visible=unchanged;
    if(style_frame_work_area(window,&outer,&visible) || !EqualRect(&visible,&unchanged)){DestroyWindow(window);return 10;}
    RECT original,changed,after;style_client_screen(window,&original);
    if(!SetWindowPos(window,NULL,0,0,340,230,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE)){DestroyWindow(window);return 3;}
    style_client_screen(window,&changed);
    if(EqualRect(&original,&changed)){DestroyWindow(window);return 4;}
    calculations=0;style_frame_changed_restore(window,&original,TRUE);style_client_screen(window,&after);
    if(!calculations || !EqualRect(&original,&after)){DestroyWindow(window);return 5;}
    calculations=0;style_frame_changed_restore(window,&changed,FALSE);style_client_screen(window,&after);
    if(!calculations || !EqualRect(&original,&after)){DestroyWindow(window);return 6;}
    DestroyWindow(window);UnregisterClassW(name,cls.hInstance);return 0;
}
