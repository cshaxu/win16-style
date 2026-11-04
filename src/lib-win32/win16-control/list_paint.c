#include "list_paint.h"
#include "basic_paint.h"
#include <stdlib.h>
#include <limits.h>

/* LBSetTabStops stores set-time pixels using the alphabet average. */
LRESULT win16_control_set_list_tabs(HWND w,const win16_control_tabstops_view *s,WPARAM count,LPARAM values)
{
    int *pixels=NULL,average,i;LRESULT result;
    if(count>INT_MAX || (count && !values) || count>((size_t)-1)/sizeof(int))return FALSE;
    if(count){HDC dc;HFONT old;TEXTMETRICW tm;SIZE size;
        pixels=malloc((size_t)count*sizeof(int));if(!pixels)return FALSE;
        dc=GetDC(w);if(!dc){free(pixels);return FALSE;}
        old=SelectObject(dc,*s->font);
        if(!GetTextMetricsW(dc,&tm)){SelectObject(dc,old);ReleaseDC(w,dc);free(pixels);return FALSE;}
        average=tm.tmAveCharWidth;
        if(tm.tmPitchAndFamily&TMPF_FIXED_PITCH){
            if(!GetTextExtentPoint32W(dc,L"abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ",52,&size)){
                SelectObject(dc,old);ReleaseDC(w,dc);free(pixels);return FALSE;}
            average=((size.cx/26)+1)/2;
        }
        SelectObject(dc,old);ReleaseDC(w,dc);
        for(i=0;i<(int)count;++i){LONGLONG value=((LONGLONG)((const int *)values)[i]*average+2)/4;
            if(value<INT_MIN || value>INT_MAX){free(pixels);return FALSE;}pixels[i]=(int)value;}
    }
    result=s->forward(w,count,values);
    if(result && !*s->detached){free(*s->positions);*s->positions=pixels;*s->count=(int)count;}
    else free(pixels);
    return result;
}

void win16_control_draw_list(HWND w,const BOOL *detached,const win16_style_palette *palette,HFONT *font_slot,const int *tab_count,int *const *tab_positions,HDC dc,BOOL (*is_vb_list)(HWND))
{
    RECT client,row;int saved,index,count,caret;DWORD style;HFONT font;
    if(!dc)return;
    style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    index=(int)SendMessageW(w,LB_GETTOPINDEX,0,0);
    count=(int)SendMessageW(w,LB_GETCOUNT,0,0);
    caret=(int)SendMessageW(w,LB_GETCARETINDEX,0,0);
    font=*font_slot && is_vb_list(w)?*font_slot:(HFONT)SendMessageW(w,WM_GETFONT,0,0);
    if(*detached)return;
    saved=SaveDC(dc);if(!saved)return;
    GetClientRect(w,&client);IntersectClipRect(dc,0,0,client.right,client.bottom);
    SetDCBrushColor(dc,(*palette).light);FillRect(dc,&client,(HBRUSH)GetStockObject(DC_BRUSH));
    SelectObject(dc,font?font:GetStockObject(SYSTEM_FONT));SetBkMode(dc,TRANSPARENT);
    for(;index>=0 && index<count;++index){
        LRESULT length,selected;WCHAR *text;RECT label,visible;int row_saved;
        if(SendMessageW(w,LB_GETITEMRECT,index,(LPARAM)&row)==LB_ERR)break;
        /* Multicolumn indices advance down a column, then to its right.
         * Keep native item geometry/scroll origin; a below-client item does
         * not imply that the following column is invisible. */
        if(style&LBS_MULTICOLUMN){if(row.left>=client.right)break;}
        else if(row.top>=client.bottom)break;
        /* USER's single-column item rect extends its right edge by xOrigin;
         * native drawing separately offsets the DC by that origin. Preserve
         * the same content coordinates, not the public scrollbar's position
         * (SetScrollInfo alone does not change the list's content origin). */
        if(!(style&LBS_MULTICOLUMN))OffsetRect(&row,client.right-row.right,0);
        if(!IntersectRect(&visible,&row,&client))continue;
        length=SendMessageW(w,LB_GETTEXTLEN,index,0);
        if(*detached || length==LB_ERR)break;
        text=calloc((size_t)length+1,sizeof(WCHAR));if(!text)break;
        if(SendMessageW(w,LB_GETTEXT,index,(LPARAM)text)==LB_ERR){free(text);break;}
        selected=(style&(LBS_MULTIPLESEL|LBS_EXTENDEDSEL))?SendMessageW(w,LB_GETSEL,index,0):
            SendMessageW(w,LB_GETCURSEL,0,0)==index;
        if(*detached){free(text);break;}
        row_saved=SaveDC(dc);if(!row_saved){free(text);break;}
        IntersectClipRect(dc,visible.left,visible.top,visible.right,visible.bottom);
        SetDCBrushColor(dc,selected>0?(*palette).active:(*palette).light);
        FillRect(dc,&row,(HBRUSH)GetStockObject(DC_BRUSH));
        SetTextColor(dc,!IsWindowEnabled(w)?(*palette).shadow:selected>0?(*palette).active_text:(*palette).edge);
        /* Original LBPrintCallback starts multi-column text at0, ordinary
         * single-column text at2, relative to the native item rectangle. */
        label=row;if(!(style&LBS_MULTICOLUMN))label.left+=2;
        if(style&LBS_USETABSTOPS)
            TabbedTextOutW(dc,label.left,label.top,text,(int)length,*tab_count,*tab_positions,row.left);
        else DrawTextW(dc,text,-1,&label,DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);
        if(GetFocus()==w && caret==index)DrawFocusRect(dc,&row);
        RestoreDC(dc,row_saved);
        free(text);
    }
    RestoreDC(dc,saved);
}

void win16_control_edit_border(HWND w,BOOL edit_border,const win16_style_palette *palette,HDC target)
{
    HDC dc;RECT bounds;int saved;
    if(!edit_border)return;
    dc=target?target:GetDC(w);if(!dc)return;
    saved=SaveDC(dc);if(saved){
        GetClientRect(w,&bounds);win16_control_control_frame(dc,bounds,(*palette).edge,max(1,MulDiv(1,(int)GetDpiForWindow(w),96)));
        RestoreDC(dc,saved);
    }
    if(!target)ReleaseDC(w,dc);
}

void win16_control_list_border(HWND w,const win16_style_palette *palette,HDC target)
{
    RECT bounds,inside;HDC dc;int saved;BOOL edge=(GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_CLIENTEDGE)!=0;
    if(!(GetWindowLongPtrW(w,GWL_STYLE)&WS_BORDER) && !edge)return;
    if(!GetWindowRect(w,&bounds))return;
    OffsetRect(&bounds,-bounds.left,-bounds.top);
    dc=target?target:GetWindowDC(w);if(!dc)return;
    saved=SaveDC(dc);
    if(saved){
        if(edge){inside=bounds;InflateRect(&inside,-1,-1);win16_control_control_frame(dc,inside,(*palette).light,1);}
        win16_control_control_frame(dc,bounds,(*palette).edge,1);RestoreDC(dc,saved);}
    if(!target)ReleaseDC(w,dc);
}
