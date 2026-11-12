#include <windows.h>
#include <stdio.h>
#include "ntw16s-dll/global/window.h"
#include "ntw16s-dll/global/session.h"
#include "ntw16s-dll/attachment.h"

static LRESULT CALLBACK procedure(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(window,&ps);HBRUSH brush=CreateSolidBrush(RGB(0,128,0));
        if(brush){FillRect(dc,&ps.rcPaint,brush);DeleteObject(brush);}EndPaint(window,&ps);return 0;}
    return DefWindowProcW(window,message,wp,lp);
}
static void pump(void)
{MSG msg;while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);}}
static BOOL client_top_clean(HWND window)
{
    HDC dc=GetDC(window);if(!dc)return FALSE;BOOL clean=TRUE;
    for(int y=0;y<12 && clean;++y)for(int x=10;x<150;++x){COLORREF color=GetPixel(dc,x,y);if(color!=RGB(0,128,0)){printf("DIRTY x=%d y=%d color=%06lx\n",x,y,(unsigned long)color);clean=FALSE;break;}}
    ReleaseDC(window,dc);return clean;
}
int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    WNDCLASSW cls={.lpfnWndProc=procedure,.hInstance=instance,.lpszClassName=L"GlobalStyleBridgeTest"};
    if(!RegisterClassW(&cls))return 2;
    HWND window=CreateWindowW(cls.lpszClassName,L"Bridge",WS_OVERLAPPEDWINDOW,80,80,300,200,NULL,NULL,instance,NULL);
    if(!window)return 2;
    HMENU menu=CreateMenu(),file=CreatePopupMenu(),help=CreatePopupMenu();
    if(!menu || !file || !help || !AppendMenuW(file,MF_STRING,1,L"Item") || !AppendMenuW(help,MF_STRING,2,L"About") ||
       !AppendMenuW(menu,MF_POPUP,(UINT_PTR)file,L"Game") || !AppendMenuW(menu,MF_POPUP,(UINT_PTR)help,L"Help") || !SetMenu(window,menu))return 2;
    ShowWindow(window,SW_SHOWNOACTIVATE);UpdateWindow(window);pump();
    BOOL apply2k=ntw16s_global_window_apply_style(window,NTW16S_GLOBAL_STYLE_2K) &&
                 GetPropW(window,L"Win2kStyle.Frame")!=NULL && !Ntw16sWindowAttached(window);
    BOOL switched16=ntw16s_global_window_apply_style(window,NTW16S_GLOBAL_STYLE_16) &&
                    Ntw16sWindowAttached(window) && !GetPropW(window,L"Win2kStyle.Frame");
    BOOL switched2k=ntw16s_global_window_apply_style(window,NTW16S_GLOBAL_STYLE_2K) &&
                    !Ntw16sWindowAttached(window) && GetPropW(window,L"Win2kStyle.Frame")!=NULL;
    pump();UpdateWindow(window);BOOL client_clean=client_top_clean(window);
    MENUBARINFO native_menu={.cbSize=sizeof(native_menu)};POINT client_origin={0,0};RECT client_rect,overlap,update;
    BOOL overlap_exists=GetMenuBarInfo(window,OBJID_MENU,0,&native_menu) && GetClientRect(window,&client_rect) &&
        ClientToScreen(window,&client_origin);
    if(overlap_exists){OffsetRect(&client_rect,client_origin.x,client_origin.y);
        overlap_exists=IntersectRect(&overlap,&client_rect,&native_menu.rcBar);}
    ValidateRect(window,NULL);SendMessageW(window,WM_NCPAINT,1,0);
    BOOL refreshed=!overlap_exists || GetUpdateRect(window,&update,FALSE);
    if(overlap_exists)printf("OVERLAP screen=%ld,%ld,%ld,%ld origin=%ld,%ld update=%ld,%ld,%ld,%ld\n",overlap.left,overlap.top,overlap.right,overlap.bottom,client_origin.x,client_origin.y,update.left,update.top,update.right,update.bottom);
    UpdateWindow(window);BOOL clean_after=client_top_clean(window);
    BOOL remove2k=ntw16s_global_window_remove_style(window,NTW16S_GLOBAL_STYLE_2K) && GetPropW(window,L"Win2kStyle.Frame")==NULL;
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
    printf("GLOBAL STYLE BRIDGE bits=%u apply2k=%d switch16=%d switch2k=%d client_clean=%d clean_after=%d overlap=%d refreshed=%d remove2k=%d apply16=%d remove16=%d reverse=%d existing=%d takeover=%d existing2k=%d takeover16=%d\n",
           (unsigned)(sizeof(void *)*8),apply2k,switched16,switched2k,client_clean,clean_after,overlap_exists,refreshed,remove2k,apply16,remove16,reverse,existing,takeover,existing2k,takeover16);
    return apply2k && switched16 && switched2k && client_clean && clean_after && refreshed && remove2k && apply16 && remove16 && reverse && takeover && takeover16?0:3;
}
