/* Win2000 kernel/drawfrm.c + caption.c NC edge ordering and ECGetBrush
 * color roles. RichEdit and HotKey keep their native client implementations. */
#include "edit_paint.h"
#include "../common/palette.h"
#include "lib-win32-style/application/control/control_class.h"
win2k_edit_kind win2k_edit_class(HWND window)
{
    WCHAR name[32];if(!GetClassNameW(window,name,ARRAYSIZE(name)))return WIN2K_EDIT_NONE;
    if(style_control_is_edit(window))return WIN2K_EDIT_STANDARD;
    static const WCHAR *const native[]={L"RichEdit20A",L"RichEdit20W",L"RICHEDIT50W",L"msctls_hotkey32"};
    for(unsigned i=0;i<ARRAYSIZE(native);++i)if(!lstrcmpiW(name,native[i]))return WIN2K_EDIT_NATIVE_CLIENT;
    return WIN2K_EDIT_NONE;
}
BOOL win2k_edit_border(HWND window,HDC target,const win2k_palette *p,style_line_profile profile)
{
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE),ex=(DWORD)GetWindowLongPtrW(window,GWL_EXSTYLE);
    RECT bounds;if(!GetWindowRect(window,&bounds))return FALSE;
    OffsetRect(&bounds,-bounds.left,-bounds.top);
    HDC dc=target?target:GetWindowDC(window);if(!dc)return FALSE;
    int saved=SaveDC(dc);BOOL result=FALSE;if(!saved)goto done;
    if(ex&WS_EX_WINDOWEDGE){
        if(!style_paint_line_frame(dc,&bounds,EDGE_RAISED,BF_ADJUST,2,p->text,profile,p))goto done;
    }else if(ex&WS_EX_STATICEDGE){
        if(!style_paint_line_frame(dc,&bounds,BDR_SUNKENOUTER,BF_ADJUST,1,p->text,profile,p))goto done;
    }
    if(style&(WS_BORDER|WS_DLGFRAME)){
        HBRUSH brush=CreateSolidBrush(p->text);if(!brush)goto done;
        BOOL framed=FrameRect(dc,&bounds,brush)!=0;DeleteObject(brush);if(!framed)goto done;
        InflateRect(&bounds,-1,-1);
    }
    result=!(ex&WS_EX_CLIENTEDGE) || style_paint_line_frame(dc,&bounds,EDGE_SUNKEN,0,2,p->text,profile,p);
done:
    {DWORD error=GetLastError();
     if(saved && !RestoreDC(dc,saved)){if(result)error=GetLastError();result=FALSE;}
     if(!target)ReleaseDC(window,dc);
     SetLastError(error);}
    return result;
}
HBRUSH win2k_edit_brush(HDC dc,HBRUSH brush,const win2k_palette *p)
{
    COLORREF native,wanted;
    if(brush==GetSysColorBrush(COLOR_BTNFACE)){native=GetSysColor(COLOR_BTNFACE);wanted=p->face;}
    else if(brush==GetSysColorBrush(COLOR_WINDOW)){native=GetSysColor(COLOR_WINDOW);wanted=p->window;}
    else return brush;
    if(native==wanted || !dc)return brush;
    COLORREF previous=SetDCBrushColor(dc,wanted);if(previous==CLR_INVALID)return brush;
    if(GetBkColor(dc)==native && SetBkColor(dc,wanted)==CLR_INVALID){SetDCBrushColor(dc,previous);return brush;}
    return (HBRUSH)GetStockObject(DC_BRUSH);
}
