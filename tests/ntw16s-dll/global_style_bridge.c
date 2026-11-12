#include <windows.h>
#include <stdio.h>
#include "ntw16s-dll/global/window.h"
#include "ntw16s-dll/global/session.h"
#include "ntw16s-dll/attachment.h"

static LRESULT CALLBACK procedure(HWND window,UINT message,WPARAM wp,LPARAM lp)
{return DefWindowProcW(window,message,wp,lp);}
int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    WNDCLASSW cls={.lpfnWndProc=procedure,.hInstance=instance,.lpszClassName=L"GlobalStyleBridgeTest"};
    if(!RegisterClassW(&cls))return 2;
    HWND window=CreateWindowW(cls.lpszClassName,L"Bridge",WS_OVERLAPPEDWINDOW,80,80,300,200,NULL,NULL,instance,NULL);
    if(!window)return 2;
    BOOL apply2k=ntw16s_global_window_apply_style(window,NTW16S_GLOBAL_STYLE_2K) &&
                 GetPropW(window,L"Win2kStyle.Frame")!=NULL && !Ntw16sWindowAttached(window);
    BOOL wrong=!ntw16s_global_window_apply_style(window,NTW16S_GLOBAL_STYLE_16) && GetLastError()==ERROR_BUSY;
    BOOL remove2k=ntw16s_global_window_remove_style(window,NTW16S_GLOBAL_STYLE_2K) &&
                  GetPropW(window,L"Win2kStyle.Frame")==NULL;
    BOOL apply16=ntw16s_global_window_apply_style(window,NTW16S_GLOBAL_STYLE_16) && Ntw16sWindowAttached(window);
    BOOL remove16=ntw16s_global_window_remove_style(window,NTW16S_GLOBAL_STYLE_16) && !Ntw16sWindowAttached(window);
    BOOL reverse=ntw16s_global_window_apply_style(window,NTW16S_GLOBAL_STYLE_16) &&
                 ntw16s_global_window_remove_style(window,NTW16S_GLOBAL_STYLE_16) &&
                 ntw16s_global_window_apply_style(window,NTW16S_GLOBAL_STYLE_2K) &&
                 ntw16s_global_window_remove_style(window,NTW16S_GLOBAL_STYLE_2K);
    BOOL existing=Ntw16sWindowAttach(window) && Ntw16sWindowAttached(window);
    BOOL takeover=existing && ntw16s_global_window_apply_style(window,NTW16S_GLOBAL_STYLE_2K) &&
                  !Ntw16sWindowAttached(window) && GetPropW(window,L"Win2kStyle.Frame")!=NULL;
    if(takeover)(void)ntw16s_global_window_remove_style(window,NTW16S_GLOBAL_STYLE_2K);
    if(Ntw16sWindowAttached(window))(void)Ntw16sWindowRemove(window);
    HMODULE module=GetModuleHandleW(sizeof(void *)==8?L"ntw2ks64.dll":L"ntw2ks32.dll");
    BOOL (__cdecl *attach2k)(HWND)=module?(void *)GetProcAddress(module,"Ntw2ksWindowAttach"):NULL;
    BOOL existing2k=attach2k && attach2k(window);
    BOOL takeover16=existing2k && ntw16s_global_window_apply_style(window,NTW16S_GLOBAL_STYLE_16) &&
                    Ntw16sWindowAttached(window) && !GetPropW(window,L"Win2kStyle.Frame");
    if(takeover16)(void)ntw16s_global_window_remove_style(window,NTW16S_GLOBAL_STYLE_16);
    DestroyWindow(window);UnregisterClassW(cls.lpszClassName,instance);
    printf("GLOBAL STYLE BRIDGE bits=%u apply2k=%d wrong=%d remove2k=%d apply16=%d remove16=%d reverse=%d existing=%d takeover=%d existing2k=%d takeover16=%d\n",
           (unsigned)(sizeof(void *)*8),apply2k,wrong,remove2k,apply16,remove16,reverse,existing,takeover,existing2k,takeover16);
    return apply2k && wrong && remove2k && apply16 && remove16 && reverse && takeover && takeover16?0:3;
}
