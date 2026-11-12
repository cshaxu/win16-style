#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include "lib-style/common/global_window.h"

typedef BOOL (__cdecl *window_op)(HWND);
static window_op attach16,attached16,remove16;
static BOOL attempted,applied;
static BOOL active(DWORD cookie){return cookie==42;}
static BOOL apply(HWND window){return attach16(window);}
static BOOL attached(HWND window){return attached16(window);}
static BOOL remove_style(HWND window){return remove16(window);}
static const style_global_window_ops ops={attached,apply,remove_style,active,L"StyleRace.Control",1,1};
static LRESULT CALLBACK observer(int code,WPARAM wp,LPARAM lp)
{
    const CWPSTRUCT *message=(const CWPSTRUCT *)lp;
    if(code>=0 && message && message->message==WM_SHOWWINDOW && message->wParam){
        WCHAR name[64];
        if(GetClassNameW(message->hwnd,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"Win2kGlobalRace")){
            attempted=TRUE;applied=style_global_window_apply(message->hwnd,&ops);
        }
    }
    return CallNextHookEx(NULL,code,wp,lp);
}
int main(void)
{
    const WCHAR *name=sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll";
    HMODULE module=LoadLibraryW(name);if(!module)return 2;
    attach16=(void *)GetProcAddress(module,"Ntw16sWindowAttach");
    attached16=(void *)GetProcAddress(module,"Ntw16sWindowAttached");
    remove16=(void *)GetProcAddress(module,"Ntw16sWindowRemove");
    if(!attach16 || !attached16 || !remove16)return 2;
    WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=DefWindowProcW,.lpszClassName=L"Win2kGlobalRace"};
    if(!RegisterClassW(&cls))return 2;
    HHOOK hook=SetWindowsHookExW(WH_CALLWNDPROC,observer,NULL,GetCurrentThreadId());if(!hook)return 2;
    HWND window=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                              100,100,240,160,NULL,NULL,cls.hInstance,NULL);
    BOOL has16=window && attached16(window);
    BOOL has2k=window && GetPropW(window,L"Win2kStyle.Frame")!=NULL;
    printf("WIN2K STYLE RACE bits=%u attempted=%d applied=%d win16=%d win2k=%d\n",
           (unsigned)(sizeof(void *)*8),attempted,applied,has16,has2k);
    UnhookWindowsHookEx(hook);
    if(has16)style_global_window_remove(window,&ops);
    if(window)DestroyWindow(window);
    UnregisterClassW(cls.lpszClassName,cls.hInstance);FreeLibrary(module);
    return attempted && applied && has16 && !has2k?0:1;
}
