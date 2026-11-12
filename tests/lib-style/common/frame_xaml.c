#include "lib-style/common/frame.h"
static LRESULT CALLBACK proc(HWND w,UINT m,WPARAM a,LPARAM b)
{return DefWindowProcW(w,m,a,b);}
int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    const WCHAR *classes[]={L"Windows.UI.Composition.DesktopWindowContentBridge",L"Microsoft.UI.Content.DesktopChildSiteBridge"};
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,50,50,NULL,NULL,instance,NULL);
    if(!parent)return 1;
    if(style_frame_xaml_bridge(parent,FALSE)){DestroyWindow(parent);return 2;}
    for(unsigned i=0;i<2;++i){
        WNDCLASSW c={.lpfnWndProc=proc,.hInstance=instance,.lpszClassName=classes[i]};
        if(!RegisterClassW(&c)){DestroyWindow(parent);return 3;}
        HWND child=CreateWindowW(classes[i],L"",WS_CHILD,0,0,10,10,parent,NULL,instance,NULL);
        if(!child || !style_frame_xaml_bridge(parent,FALSE) || style_frame_xaml_bridge(parent,TRUE)){DestroyWindow(parent);return 4;}
        DestroyWindow(child);UnregisterClassW(classes[i],instance);
    }
    DestroyWindow(parent);return 0;
}
