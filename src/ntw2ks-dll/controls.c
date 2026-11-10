#include <windows.h>
#include <uxtheme.h>
#include <wchar.h>
#include "lib-win32/process-hook/startup_hooks.h"

static __typeof__(&CreateWindowExW) create_W;
static __typeof__(&CreateWindowExA) create_A;

static void classic_control(HWND window,DWORD style)
{
    if(!window || !(style&WS_CHILD))return;
    WCHAR name[32];
    if(!GetClassNameW(window,name,ARRAYSIZE(name)))return;
    static const WCHAR *const classes[]={L"Button",L"Edit",L"ComboBox",L"ListBox",L"ScrollBar"};
    for(unsigned i=0;i<ARRAYSIZE(classes);++i){
        if(!_wcsicmp(name,classes[i])){SetWindowTheme(window,L"",L"");break;}
    }
}

#define WINDOW_HOOK(S,T) \
static HWND WINAPI hook_##S(DWORD ex,T cls,T text,DWORD style,int x,int y,int width,int height,HWND parent,HMENU menu,HINSTANCE instance,void *data) \
{ HWND result=create_##S(ex,cls,text,style,x,y,width,height,parent,menu,instance,data);DWORD error=GetLastError(); \
  classic_control(result,style);SetLastError(error);return result; }
WINDOW_HOOK(W,LPCWSTR)
WINDOW_HOOK(A,LPCSTR)

DWORD ntw2ks_controls_attach(void)
{
    create_W=CreateWindowExW;create_A=CreateWindowExA;
    DWORD error=process_hook_startup_attach(&create_W,(void *)hook_W);
    if(!error)error=process_hook_startup_attach(&create_A,(void *)hook_A);
    return error;
}
