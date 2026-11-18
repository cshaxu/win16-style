#ifndef WIN2K_DIALOG_FILE_H
#define WIN2K_DIALOG_FILE_H
#include <windows.h>
#include <commdlg.h>
#include "lib-win32-style/application/window/window_identity.h"

static inline BOOL win2k_file_compact(HWND owner,DWORD flags,DWORD extended)
{
    if(!(flags&OFN_EXPLORER) || (flags&(OFN_ENABLEHOOK|OFN_ENABLETEMPLATE|OFN_ENABLETEMPLATEHANDLE|OFN_ALLOWMULTISELECT)) ||
       (extended&~(DWORD)OFN_EX_NOPLACESBAR))return FALSE;
    MONITORINFO monitor={.cbSize=sizeof(monitor)};
    if(!GetMonitorInfoW(MonitorFromWindow(owner,MONITOR_DEFAULTTOPRIMARY),&monitor))return FALSE;
    UINT dpi=owner?GetDpiForWindow(owner):GetDpiForSystem();
    if(!dpi)dpi=96;
    const RECT *work=&monitor.rcWork;
    int width=MulDiv(work->right-work->left,96,(int)dpi);
    int height=MulDiv(work->bottom-work->top,96,(int)dpi);
    /* Normalize the minimum verified compact work area and the reference
     * 555-by-320 Places client plus its frame to the same 96 DPI units. */
    enum { compact_width=427, compact_height=272, places_width=565, places_height=345 };
    return width>=compact_width && height>=compact_height &&
        (width<places_width || height<places_height);
}

static inline void win2k_file_fit_compact(HWND dialog)
{
    if(!win32_style_window_class(dialog,L"#32770") || GetDlgItem(dialog,1184) ||
       !win32_style_window_class(GetDlgItem(dialog,1121),L"SHELLDLL_DefView") ||
       !win32_style_window_class(GetDlgItem(dialog,1148),L"ComboBoxEx32"))return;

    RECT outer;
    MONITORINFO monitor={.cbSize=sizeof(monitor)};
    if(!GetWindowRect(dialog,&outer) ||
       !GetMonitorInfoW(MonitorFromWindow(dialog,MONITOR_DEFAULTTONEAREST),&monitor))return;
    const RECT *work=&monitor.rcWork;
    LONG width=min(outer.right-outer.left,work->right-work->left);
    LONG height=min(outer.bottom-outer.top,work->bottom-work->top);
    LONG x=max(work->left,min(outer.left,work->right-width));
    LONG y=max(work->top,min(outer.top,work->bottom-height));
    if(x==outer.left && y==outer.top && width==outer.right-outer.left && height==outer.bottom-outer.top)return;

    /* The native sizing handler otherwise restores a minimum larger than
     * this monitor. WM_SIZE still lets the dialog lay out its own children. */
    SetWindowPos(dialog,NULL,x,y,width,height,SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSENDCHANGING);
}
#endif
