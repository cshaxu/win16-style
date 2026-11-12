#include <windows.h>
#include <stdio.h>
#include "lib-style/common/theme_state.h"

static WCHAR restored[64];
static HRESULT observe(HWND window,LPCWSTR application,LPCWSTR ids)
{
    (void)window;(void)ids;
    lstrcpynW(restored,application?application:L"<null>",ARRAYSIZE(restored));
    return S_OK;
}
int main(void)
{
    HMODULE common=LoadLibraryW(sizeof(void *)==8?L"ntwshk64.dll":L"ntwshk32.dll");
    HMODULE win16=LoadLibraryW(sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    if(!common || !win16)return 1;
    BOOL (*capture)(HWND)=(void *)GetProcAddress(common,"style_theme_capture");
    BOOL (*suppress)(HWND,const void *,style_native_theme_fn)=(void *)GetProcAddress(common,"style_theme_suppress");
    void (*remove)(HWND,const void *,style_native_theme_fn)=(void *)GetProcAddress(common,"style_theme_remove");
    HRESULT (__cdecl *set)(HWND,LPCWSTR,LPCWSTR)=(void *)GetProcAddress(win16,"Ntw16sSetWindowTheme");
    if(!capture || !suppress || !remove || !set)return 2;
    HWND window=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,40,40,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!window)return 3;
    int owner=0;
    BOOL ok=capture(window) && suppress(window,&owner,observe) &&
        SUCCEEDED(set(window,L"Explorer",NULL));
    remove(window,&owner,observe);
    ok=ok && !lstrcmpW(restored,L"Explorer");
    DestroyWindow(window);FreeLibrary(win16);FreeLibrary(common);
    if(!ok)return 4;
    puts("STYLE THEME SHARED PASS capture=common set=win16 restore=common");return 0;
}
