#ifndef WIN2K_DIALOG_FILE_H
#define WIN2K_DIALOG_FILE_H
#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include "lib-win32-style/application/window/window_identity.h"

typedef enum { WIN2K_FILE_NATIVE, WIN2K_FILE_COMPACT, WIN2K_FILE_PLACES } win2k_file_layout;

static inline win2k_file_layout win2k_file_layout_for(int width,int height,DWORD flags,DWORD extended)
{
    if(!(flags&OFN_EXPLORER) || (flags&(OFN_ENABLEHOOK|OFN_ENABLETEMPLATE|OFN_ENABLETEMPLATEHANDLE|OFN_ALLOWMULTISELECT)) ||
       (extended&~(DWORD)OFN_EX_NOPLACESBAR))return WIN2K_FILE_NATIVE;
    if(width<427 || height<272)return WIN2K_FILE_NATIVE;
    if(width<565 || height<345)return WIN2K_FILE_COMPACT;
    return extended&OFN_EX_NOPLACESBAR?WIN2K_FILE_NATIVE:WIN2K_FILE_PLACES;
}

static inline win2k_file_layout win2k_file_layout_at(HWND owner,DWORD flags,DWORD extended)
{
    MONITORINFO monitor={.cbSize=sizeof(monitor)};
    if(!GetMonitorInfoW(MonitorFromWindow(owner,MONITOR_DEFAULTTOPRIMARY),&monitor))return WIN2K_FILE_NATIVE;
    UINT dpi=owner?GetDpiForWindow(owner):GetDpiForSystem();
    if(!dpi)dpi=96;
    const RECT *work=&monitor.rcWork;
    int width=MulDiv(work->right-work->left,96,(int)dpi);
    int height=MulDiv(work->bottom-work->top,96,(int)dpi);
    return win2k_file_layout_for(width,height,flags,extended);
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

static inline HWND win2k_file_toolbar(HWND dialog,int id)
{
    for(HWND child=GetWindow(dialog,GW_CHILD);child;child=GetWindow(child,GW_HWNDNEXT)){
        if(GetDlgCtrlID(child)==id && win32_style_window_class(child,L"ToolbarWindow32"))return child;
    }
    return NULL;
}

static inline RECT win2k_file_child_bounds(HWND dialog,HWND child)
{
    RECT bounds={0};
    if(GetWindowRect(child,&bounds))MapWindowPoints(NULL,dialog,(POINT *)&bounds,2);
    return bounds;
}

static inline void win2k_file_place_child(HWND child,const RECT *bounds)
{
    SetWindowPos(child,NULL,bounds->left,bounds->top,bounds->right-bounds->left,bounds->bottom-bounds->top,
        SWP_NOZORDER|SWP_NOACTIVATE);
}

static inline void win2k_file_fit_place_rows(HWND places)
{
    RECT bounds;
    if(!GetClientRect(places,&bounds))return;
    LRESULT count=SendMessageW(places,TB_BUTTONCOUNT,0,0);
    LRESULT size=SendMessageW(places,TB_GETBUTTONSIZE,0,0);
    if(count<=0 || !LOWORD(size))return;
    int row=bounds.bottom/(int)count;
    if(row>0 && row!=HIWORD(size))SendMessageW(places,TB_SETBUTTONSIZE,0,MAKELPARAM(LOWORD(size),row));
}

/* Reflow the installed Shell view and navigation controls. 1552 in the
 * Windows 2000 resource uses a 370 by 197 dialog-unit client. */
static inline void win2k_file_fit_places(HWND dialog,const RECT *work)
{
    HWND places=win2k_file_toolbar(dialog,1184),navigation=win2k_file_toolbar(dialog,1088);
    HWND view=GetDlgItem(dialog,1121),name=GetDlgItem(dialog,1148),filter=GetDlgItem(dialog,1136);
    HWND drives=GetDlgItem(dialog,1137),ok=GetDlgItem(dialog,IDOK),cancel=GetDlgItem(dialog,IDCANCEL);
    if(!places || !navigation || !filter || !drives || !ok || !cancel ||
       !win32_style_window_class(dialog,L"#32770") ||
       !win32_style_window_class(view,L"SHELLDLL_DefView") ||
       !win32_style_window_class(name,L"ComboBoxEx32"))return;

    RECT reference={0,0,370,197},client,outer;
    if(!MapDialogRect(dialog,&reference) || !GetClientRect(dialog,&client) || !GetWindowRect(dialog,&outer))return;
    LONG width=reference.right+outer.right-outer.left-client.right;
    LONG height=reference.bottom+outer.bottom-outer.top-client.bottom;
    if(width>work->right-work->left || height>work->bottom-work->top)return;
    LONG x=max(work->left,min(outer.left,work->right-width));
    LONG y=max(work->top,min(outer.top,work->bottom-height));
    if(!SetWindowPos(dialog,NULL,x,y,width,height,SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSENDCHANGING) ||
       !GetClientRect(dialog,&client))return;

    int dpi=(int)GetDpiForWindow(dialog);
    LONG gap=MulDiv(7,dpi,96),reserve=MulDiv(20,dpi,96);
    RECT cancel_bounds=win2k_file_child_bounds(dialog,cancel),name_bounds=win2k_file_child_bounds(dialog,name);
    LONG dx=max(0,cancel_bounds.right-client.right+gap);
    LONG bottom=max(cancel_bounds.bottom,win2k_file_child_bounds(dialog,filter).bottom);
    HWND readonly=GetDlgItem(dialog,1040);
    if(IsWindowVisible(readonly))bottom=max(bottom,win2k_file_child_bounds(dialog,readonly).bottom);
    LONG dy=max(0,bottom-client.bottom+reserve);

    static const int fields[]={1090,1089,1148,1136,1040,IDOK,IDCANCEL,1038};
    for(unsigned i=0;i<ARRAYSIZE(fields);++i){
        HWND child=GetDlgItem(dialog,fields[i]);
        if(!IsWindowVisible(child))continue;
        RECT bounds=win2k_file_child_bounds(dialog,child);
        OffsetRect(&bounds,0,-dy);
        if(fields[i]==IDOK || fields[i]==IDCANCEL || fields[i]==1038)OffsetRect(&bounds,-dx,0);
        else if(fields[i]==1148 || fields[i]==1136 || fields[i]==1040)bounds.right=max(bounds.left+1,bounds.right-dx);
        win2k_file_place_child(child,&bounds);
    }

    RECT nav=win2k_file_child_bounds(dialog,navigation),drive=win2k_file_child_bounds(dialog,drives);
    OffsetRect(&nav,min(0,client.right-gap-nav.right),0);
    drive.right=min(drive.right,nav.left-gap);
    win2k_file_place_child(navigation,&nav);
    win2k_file_place_child(drives,&drive);
    RECT pane=win2k_file_child_bounds(dialog,view),sidebar=win2k_file_child_bounds(dialog,places);
    pane.right=client.right-gap;
    pane.bottom=name_bounds.top-dy-gap;
    sidebar.bottom=client.bottom-reserve;
    win2k_file_place_child(view,&pane);
    win2k_file_place_child(places,&sidebar);
    win2k_file_fit_place_rows(places);
}
#endif
