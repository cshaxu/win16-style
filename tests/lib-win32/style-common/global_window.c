#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include "lib-win32/style-common/global_window.h"

static const WCHAR property[]=L"StyleGlobalWindowTest.Attached";
static BOOL marker_on_apply,marker_on_remove;
static BOOL attached(HWND window){return GetPropW(window,property)!=NULL;}
static BOOL apply(HWND window){marker_on_apply=style_global_window_managed(window);return SetPropW(window,property,(HANDLE)1);}
static BOOL remove_style(HWND window){marker_on_remove=style_global_window_managed(window);return RemovePropW(window,property)!=NULL;}
static BOOL fail_apply(HWND window){(void)window;SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
static BOOL active(DWORD cookie){return cookie==42;}
static const style_global_window_ops ops={attached,apply,remove_style,active,L"StyleGlobalWindowTest.Control",3,1234};
static const style_global_window_ops other={attached,apply,remove_style,active,L"StyleGlobalWindowTest.Control",3,1234};
static const style_global_window_ops failing={attached,fail_apply,remove_style,active,L"StyleGlobalWindowTest.Control",3,1234};
static LRESULT CALLBACK procedure(HWND window,UINT message,WPARAM wp,LPARAM lp)
{return DefWindowProcW(window,message,wp,lp);}
int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    WNDCLASSW cls={.lpfnWndProc=procedure,.hInstance=instance,.lpszClassName=L"StyleGlobalWindowTest"};
    if(!RegisterClassW(&cls))return 2;
    HWND window=CreateWindowW(cls.lpszClassName,L"Test",WS_OVERLAPPEDWINDOW,80,80,200,100,NULL,NULL,instance,NULL);
    if(!window)return 2;
    BOOL installed=style_global_window_apply(window,&ops) && style_global_window_apply(window,&ops) &&
                   style_global_window_attached(window,&ops) && attached(window);
    UINT control=RegisterWindowMessageW(ops.control);
    BOOL query=control && SendMessageW(window,control,42,3)==1234 && SendMessageW(window,control,41,3)!=1234;
    BOOL mismatch=!style_global_window_apply(window,&other) && GetLastError()==ERROR_BUSY;
    BOOL removed=style_global_window_remove(window,&ops) && !attached(window) && !style_global_window_attached(window,&ops) && !style_global_window_managed(window);
    BOOL repeated=!style_global_window_remove(window,&ops) && GetLastError()==ERROR_NOT_FOUND;
    BOOL reattached=style_global_window_apply(window,&other) && style_global_window_remove(window,&other);
    BOOL failed=!style_global_window_apply(window,&failing) && GetLastError()==ERROR_ACCESS_DENIED && !style_global_window_managed(window);
    DestroyWindow(window);UnregisterClassW(cls.lpszClassName,instance);
    printf("STYLE GLOBAL WINDOW bits=%u installed=%d query=%d mismatch=%d removed=%d repeated=%d reattached=%d marker=%d failed_cleanup=%d\n",
           (unsigned)(sizeof(void *)*8),installed,query,mismatch,removed,repeated,reattached,marker_on_apply && marker_on_remove,failed);
    return installed && query && mismatch && removed && repeated && reattached && marker_on_apply && marker_on_remove && failed?0:3;
}
