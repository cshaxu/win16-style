#include "lib-style/common/frame.h"
static LRESULT CALLBACK proc(HWND w,UINT m,WPARAM a,LPARAM b)
{return DefWindowProcW(w,m,a,b);}
int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    const WCHAR *classes[]={L"Windows.UI.Composition.DesktopWindowContentBridge",L"Microsoft.UI.Content.DesktopChildSiteBridge"};
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,50,50,NULL,NULL,instance,NULL);
    if(!parent)return 1;
    MONITORINFO monitor={.cbSize=sizeof(monitor)};RECT outer,wanted,client={0};
    if(!GetMonitorInfoW(MonitorFromWindow(parent,MONITOR_DEFAULTTONEAREST),&monitor) ||
       !SetWindowPos(parent,NULL,monitor.rcWork.left+10,monitor.rcWork.top+10,0,0,
                     SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE) || !GetWindowRect(parent,&outer)){DestroyWindow(parent);return 5;}
    wanted=outer;client.left=outer.left-monitor.rcWork.left+1;client.top=outer.top-monitor.rcWork.top+1;
    if(style_frame_xaml_bridge(parent,FALSE) || style_frame_keep_outer(parent,&wanted,&client)){DestroyWindow(parent);return 2;}
    for(unsigned i=0;i<2;++i){
        WNDCLASSW c={.lpfnWndProc=proc,.hInstance=instance,.lpszClassName=classes[i]};
        if(!RegisterClassW(&c)){DestroyWindow(parent);return 3;}
        HWND child=CreateWindowW(classes[i],L"",WS_CHILD,0,0,10,10,parent,NULL,instance,NULL);
        if(!child || !style_frame_xaml_bridge(parent,FALSE) || style_frame_xaml_bridge(parent,TRUE)){DestroyWindow(parent);return 4;}
        if(!style_frame_keep_outer(parent,&wanted,&client) || style_frame_keep_outer(parent,NULL,&client) ||
           style_frame_keep_outer(parent,&wanted,NULL)){DestroyWindow(parent);return 6;}
        RECT safe=wanted;safe.left=monitor.rcWork.left+client.left;safe.top=monitor.rcWork.top+client.top;
        if(style_frame_keep_outer(parent,&safe,&client)){DestroyWindow(parent);return 7;}
        RECT left=safe,top=safe;left.left=wanted.left;top.top=wanted.top;
        if(!style_frame_keep_outer(parent,&left,&client) || !style_frame_keep_outer(parent,&top,&client) ||
           style_frame_keep_outer(child,&wanted,&client)){DestroyWindow(parent);return 8;}
        DestroyWindow(child);UnregisterClassW(classes[i],instance);
    }
    DestroyWindow(parent);return 0;
}
