#include "choice_view.h"
#include "choice_paint.h"
#include "other_paint.h"
#include "../common/control_protocol.h"
#include "../common/icon_size.h"
#include "../common/window_text.h"
#include <uxtheme.h>
#include <stdlib.h>

void win16_control_draw_choice(HWND w,const win16_control_choice_view *view,HDC dc)
{
    RECT r,glyph,label,focus;int saved,x,y,row,column,height;
    UINT dpi=GetDpiForWindow(w);DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    UINT type=view->button_type(w,style);LRESULT state,check;WCHAR *text;HFONT font;TEXTMETRICW tm;
    BOOL radio=type==BS_RADIOBUTTON || type==BS_AUTORADIOBUTTON;
    BOOL label_control=view->is_static(w);
    if(label_control && (style&SS_TYPEMASK)==SS_ICON){
        /* statctl.c retains image ownership/sizing in native STATIC. The
         * painter borrows its current handle, never reads image data as text.
         * CENTERIMAGE is a host extension; its natural-size image is clipped. */
        HICON icon=(HICON)SendMessageW(w,STM_GETICON,0,0);int cx,cy;
        if(*view->detached || !IsWindow(w))return;
        saved=SaveDC(dc);if(!saved)return;
        GetClientRect(w,&r);cx=r.right;cy=r.bottom;x=y=0;
        IntersectClipRect(dc,r.left,r.top,r.right,r.bottom);
        SetDCBrushColor(dc,view->palette->light);FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));
        if(icon && (style&SS_CENTERIMAGE)){
            if(style_icon_size(icon,&cx,&cy)){x=(r.right-cx)/2;y=(r.bottom-cy)/2;}
        }
        if(icon)DrawIconEx(dc,x,y,icon,cx,cy,0,NULL,DI_NORMAL);
        RestoreDC(dc,saved);return;
    }
    state=label_control?0:SendMessageW(w,BM_GETSTATE,0,0);check=label_control?0:SendMessageW(w,BM_GETCHECK,0,0);
    text=style_window_text_copy(w);
    if(!text)return;
    font=*view->font && (view->vb_frame(w) || view->vb_button(w) ||
        style_control_protocol_is(w,L"BUTTON"))?
        *view->font:(HFONT)SendMessageW(w,WM_GETFONT,0,0);
    if(*view->detached || !IsWindow(w)){free(text);return;}
    if(!font)font=(HFONT)GetStockObject(SYSTEM_FONT);
    saved=SaveDC(dc);if(!saved){free(text);return;}
    GetClientRect(w,&r);SetDCBrushColor(dc,view->palette->light);
    /* A group box surrounds sibling controls, not an opaque child surface.
     * Repainting its interior erases those siblings during ordinary motion. */
    if(label_control || type!=BS_GROUPBOX){
        BOOL check_parent=view->group_choice(w);
        BOOL parent_painted=check_parent && SUCCEEDED(DrawThemeParentBackground(w,dc,&r));
        if(*view->detached || !IsWindow(w) || (check_parent && !view->group_choice(w))){RestoreDC(dc,saved);free(text);return;}
        if(!parent_painted)FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));
    }
    if(label_control || type==BS_PUSHBUTTON || type==BS_DEFPUSHBUTTON || type==BS_GROUPBOX || (style&BS_PUSHLIKE)){
        win16_control_other(w,view->detached,view->palette,dc,r,font,text,label_control,view->button_type,view->group_push);RestoreDC(dc,saved);free(text);return;}
    height=MulDiv(13,(int)dpi,96);glyph=r;glyph.top+=(r.bottom-height)/2;
    glyph.bottom=glyph.top+height;glyph.right=glyph.left+MulDiv(14,(int)dpi,96);
    if(style&BS_LEFTTEXT)OffsetRect(&glyph,r.right-glyph.right,0);
    row=radio?1:check==BST_INDETERMINATE?2:0;
    column=(check!=BST_UNCHECKED?1:0)+((state&BST_PUSHED)?2:0);
    win16_control_choice_glyph(dc,glyph,dpi,row,column,view->palette);
    label=r;if(style&BS_LEFTTEXT)label.right=glyph.left-MulDiv(4,(int)dpi,96);
    else label.left=glyph.right+MulDiv(4,(int)dpi,96);
    SelectObject(dc,font);GetTextMetricsW(dc,&tm);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,view->palette->edge);
    label.top+=(r.bottom-tm.tmHeight)/2;label.bottom=label.top+tm.tmHeight;
    if(IsWindowEnabled(w))DrawTextW(dc,text,-1,&label,DT_LEFT|DT_SINGLELINE);
    else {HBRUSH brush=CreateSolidBrush(view->palette->edge);
        if(brush){DrawStateW(dc,brush,NULL,(LPARAM)text,0,label.left,label.top,
            max(0,label.right-label.left),tm.tmHeight,DST_PREFIXTEXT|DSS_UNION|DSS_MONO);DeleteObject(brush);}}
    if(state&BST_FOCUS){focus=label;DrawTextW(dc,text,-1,&focus,DT_CALCRECT|DT_SINGLELINE);
        focus.right=min(focus.right,label.right);InflateRect(&focus,1,1);DrawFocusRect(dc,&focus);}
    RestoreDC(dc,saved);free(text);
}
