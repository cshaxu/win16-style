#ifndef STYLE_EXPLORER_FILE_LAYOUT_H
#define STYLE_EXPLORER_FILE_LAYOUT_H
#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>
#include "metrics.h"
#include "lib-win32-style/application/window/window_identity.h"

typedef enum { STYLE_EXPLORER_FILE_NATIVE, STYLE_EXPLORER_FILE_COMPACT, STYLE_EXPLORER_FILE_PLACES } style_explorer_file_layout;

static inline BOOL style_explorer_file_stock(DWORD flags,DWORD extended)
{
    return (flags&OFN_EXPLORER) && !(flags&(OFN_ENABLEHOOK|OFN_ENABLETEMPLATE|OFN_ENABLETEMPLATEHANDLE|OFN_ALLOWMULTISELECT)) &&
        !(extended&~(DWORD)OFN_EX_NOPLACESBAR);
}

static inline style_explorer_file_layout style_explorer_file_layout_for(int width,int height,DWORD flags,DWORD extended,
    const classic_file_metrics *metrics)
{
    if(!metrics->compact_width || !style_explorer_file_stock(flags,extended))return STYLE_EXPLORER_FILE_NATIVE;
    if(width<metrics->compact_width || height<metrics->compact_height)return STYLE_EXPLORER_FILE_NATIVE;
    if(width<metrics->places_width || height<metrics->places_height)return STYLE_EXPLORER_FILE_COMPACT;
    return extended&OFN_EX_NOPLACESBAR?STYLE_EXPLORER_FILE_NATIVE:STYLE_EXPLORER_FILE_PLACES;
}

static inline style_explorer_file_layout style_explorer_file_layout_at(HWND owner,DWORD flags,DWORD extended,
    const classic_file_metrics *metrics)
{
    MONITORINFO monitor={.cbSize=sizeof(monitor)};
    if(!GetMonitorInfoW(MonitorFromWindow(owner,MONITOR_DEFAULTTOPRIMARY),&monitor))return STYLE_EXPLORER_FILE_NATIVE;
    UINT dpi=owner?GetDpiForWindow(owner):GetDpiForSystem();
    if(!dpi)dpi=96;
    const RECT *work=&monitor.rcWork;
    int width=MulDiv(work->right-work->left,96,(int)dpi);
    int height=MulDiv(work->bottom-work->top,96,(int)dpi);
    return style_explorer_file_layout_for(width,height,flags,extended,metrics);
}

static inline void style_explorer_file_fit_compact(HWND dialog)
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

static inline HWND style_explorer_file_toolbar(HWND dialog,int id)
{
    for(HWND child=GetWindow(dialog,GW_CHILD);child;child=GetWindow(child,GW_HWNDNEXT)){
        if(GetDlgCtrlID(child)==id && win32_style_window_class(child,L"ToolbarWindow32"))return child;
    }
    return NULL;
}

static inline RECT style_explorer_file_child_bounds(HWND dialog,HWND child)
{
    RECT bounds={0};
    if(GetWindowRect(child,&bounds))MapWindowPoints(NULL,dialog,(POINT *)&bounds,2);
    return bounds;
}

static inline void style_explorer_file_place_child(HWND child,const RECT *bounds)
{
    SetWindowPos(child,NULL,bounds->left,bounds->top,bounds->right-bounds->left,bounds->bottom-bounds->top,
        SWP_NOZORDER|SWP_NOACTIVATE);
}

static inline void style_explorer_file_fit_place_rows(HWND places)
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
static inline void style_explorer_file_fit_places(HWND dialog,const RECT *work,const classic_file_metrics *metrics)
{
    HWND places=style_explorer_file_toolbar(dialog,1184),navigation=style_explorer_file_toolbar(dialog,1088);
    HWND view=GetDlgItem(dialog,1121),name=GetDlgItem(dialog,1148),filter=GetDlgItem(dialog,1136);
    HWND drives=GetDlgItem(dialog,1137),ok=GetDlgItem(dialog,IDOK),cancel=GetDlgItem(dialog,IDCANCEL);
    if(!places || !navigation || !filter || !drives || !ok || !cancel ||
       !win32_style_window_class(dialog,L"#32770") ||
       !win32_style_window_class(view,L"SHELLDLL_DefView") ||
       !win32_style_window_class(name,L"ComboBoxEx32"))return;

    RECT reference={0,0,metrics->client_dlu_width,metrics->client_dlu_height},client,outer;
    if(!MapDialogRect(dialog,&reference) || !GetClientRect(dialog,&client) || !GetWindowRect(dialog,&outer))return;
    LONG width=reference.right+outer.right-outer.left-client.right;
    LONG height=reference.bottom+outer.bottom-outer.top-client.bottom;
    if(width>work->right-work->left || height>work->bottom-work->top)return;
    LONG x=max(work->left,min(outer.left,work->right-width));
    LONG y=max(work->top,min(outer.top,work->bottom-height));
    if(!SetWindowPos(dialog,NULL,x,y,width,height,SWP_NOZORDER|SWP_NOACTIVATE|SWP_NOSENDCHANGING) ||
       !GetClientRect(dialog,&client))return;

    int dpi=(int)GetDpiForWindow(dialog);
    LONG gap=MulDiv(metrics->gap,dpi,96),reserve=MulDiv(metrics->bottom_reserve,dpi,96);
    RECT cancel_bounds=style_explorer_file_child_bounds(dialog,cancel),name_bounds=style_explorer_file_child_bounds(dialog,name);
    LONG dx=max(0,cancel_bounds.right-client.right+gap);
    LONG bottom=max(cancel_bounds.bottom,style_explorer_file_child_bounds(dialog,filter).bottom);
    HWND readonly=GetDlgItem(dialog,1040);
    if(IsWindowVisible(readonly))bottom=max(bottom,style_explorer_file_child_bounds(dialog,readonly).bottom);
    LONG dy=max(0,bottom-client.bottom+reserve);

    static const int fields[]={1090,1089,1148,1136,1040,IDOK,IDCANCEL,1038};
    for(unsigned i=0;i<ARRAYSIZE(fields);++i){
        HWND child=GetDlgItem(dialog,fields[i]);
        if(!IsWindowVisible(child))continue;
        RECT bounds=style_explorer_file_child_bounds(dialog,child);
        OffsetRect(&bounds,0,-dy);
        if(fields[i]==IDOK || fields[i]==IDCANCEL || fields[i]==1038)OffsetRect(&bounds,-dx,0);
        else if(fields[i]==1148 || fields[i]==1136 || fields[i]==1040)bounds.right=max(bounds.left+1,bounds.right-dx);
        style_explorer_file_place_child(child,&bounds);
    }

    RECT nav=style_explorer_file_child_bounds(dialog,navigation),drive=style_explorer_file_child_bounds(dialog,drives);
    OffsetRect(&nav,min(0,client.right-gap-nav.right),0);
    drive.right=min(drive.right,nav.left-gap);
    style_explorer_file_place_child(navigation,&nav);
    style_explorer_file_place_child(drives,&drive);
    RECT pane=style_explorer_file_child_bounds(dialog,view),sidebar=style_explorer_file_child_bounds(dialog,places);
    pane.right=client.right-gap;
    pane.bottom=name_bounds.top-dy-gap;
    sidebar.bottom=client.bottom-reserve;
    style_explorer_file_place_child(view,&pane);
    style_explorer_file_place_child(places,&sidebar);
    style_explorer_file_fit_place_rows(places);
}
#endif
