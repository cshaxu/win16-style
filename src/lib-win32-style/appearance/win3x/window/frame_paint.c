#include "frame_paint.h"
#include "../resources/frame_buttons.h"
#include <stdlib.h>
#include "lib-win32-style/application/window/window_text.h"

void win16_control_fill(HDC dc,RECT r,COLORREF c)
{
    SetDCBrushColor(dc,c);FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));
}

void win16_control_outline(HDC dc,RECT r,int u,COLORREF c)
{
    RECT q=r;SetDCBrushColor(dc,c);HBRUSH brush=(HBRUSH)GetStockObject(DC_BRUSH);
    q.bottom=q.top+u;FillRect(dc,&q,brush);
    q=r;q.top=q.bottom-u;FillRect(dc,&q,brush);
    q=r;q.right=q.left+u;FillRect(dc,&q,brush);
    q=r;q.left=q.right-u;FillRect(dc,&q,brush);
}

void win16_control_frame_button(HDC dc,RECT r,int kind,BOOL down,BOOL zoom,BOOL child,int u,
    const win16_style_palette *p)
{
    BITMAPINFO bi={0};DWORD pixels[19*18];int x,y,width,stride;
    const char *bits;
    COLORREF palette[4]={p->edge,p->light,p->shadow,p->face};
    if(IsRectEmpty(&r))return;
    win16_control_fill(dc,r,kind==HTSYSMENU&&down?p->shadow:p->face);win16_control_outline(dc,r,u,p->edge);
    if(kind==HTSYSMENU){
        /* Original CLOSE atlas: top-level left half, WS_CHILD right half. */
        bits=&system_menu_bitmap[0][child?18:0];width=18;stride=37;
        /* VGA selection inverts palette roles, not RGB bytes: light grey
         * becomes dark grey, black becomes white. No second CLOSE bitmap. */
        if(down){palette[0]=p->light;palette[1]=p->edge;
            palette[2]=p->face;palette[3]=p->shadow;}
    }else{
        int index=(kind==HTMINBUTTON?0:zoom?2:1)+(down?3:0);
        bits=&frame_buttons[index][0][0];width=19;stride=20;
    }
    for(y=0;y<18;++y)for(x=0;x<width;++x){
        COLORREF c=palette[bits[y*stride+x]-'0'];
        pixels[y*width+x]=((DWORD)GetRValue(c)<<16)|((DWORD)GetGValue(c)<<8)|GetBValue(c);
    }
    bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth=width;bi.bmiHeader.biHeight=-18;
    bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;
    /* Original arrow artwork includes its left separator; CLOSE does not.
     * The caption supplies the other separators. Scale the raster once. */
    SetStretchBltMode(dc,COLORONCOLOR);
    StretchDIBits(dc,r.left,r.top,r.right-r.left-(kind==HTSYSMENU?u:0),r.bottom-r.top,
        0,0,width,18,pixels,&bi,DIB_RGB_COLORS,SRCCOPY);
}

void win16_style_merged_control(HDC dc,RECT bounds,BOOL system,BOOL selected,int unit,const win16_style_palette *palette)
{
    win16_control_frame_button(dc,bounds,system?HTSYSMENU:HTMAXBUTTON,selected,!system,system,unit,palette);
}


void win16_control_frame_sizing(HDC dc,const win16_style_layout *layout,const win16_style_palette *palette)
{
    win16_style_layout l=*layout;win16_style_palette p=*palette;RECT r;
    if(l.sizeable && !l.dialog_frame){
        /* Original USER 1:A06B..A1DE divides the flat sizing band into
         * corner and edge regions. These are strokes, not a bevel. */
        int corner=l.corner.cx-l.unit,vertical=l.corner.cy-l.unit,i;
        for(i=0;i<2;++i){
            int x=i?l.outer.right-corner-l.unit:l.outer.left+corner;
            int y=i?l.outer.bottom-vertical-l.unit:l.outer.top+vertical;
            if(l.outer.right-l.outer.left>2*corner+l.unit){
                SetRect(&r,x,l.outer.top+l.unit,x+l.unit,l.outer.top+l.border-l.unit);win16_control_fill(dc,r,p.edge);
                OffsetRect(&r,0,l.outer.bottom-l.outer.top-l.border);win16_control_fill(dc,r,p.edge);
            }
            if(l.outer.bottom-l.outer.top>2*vertical+l.unit){
                SetRect(&r,l.outer.left+l.unit,y,l.outer.left+l.border-l.unit,y+l.unit);win16_control_fill(dc,r,p.edge);
                OffsetRect(&r,l.outer.right-l.outer.left-l.border,0);win16_control_fill(dc,r,p.edge);
            }
        }
    }
}

void win16_control_frame_caption(HWND w,HDC dc,const win16_style_layout *layout,const win16_style_palette *palette,
    void *context,win16_frame_state_read read)
{
    win16_style_layout l=*layout;win16_style_palette p=*palette;RECT r;int len;WCHAR *title;
    win16_frame_paint_state state;read(context,&state);
    win16_control_fill(dc,l.caption,state.active?p.active:p.inactive);
    r=l.caption;r.top=r.bottom;r.bottom+=l.unit;win16_control_fill(dc,r,p.edge);
    win16_control_frame_button(dc,l.system,HTSYSMENU,state.system_menu||(state.bar_keyboard&&state.bar_selected==-2)||(state.button.pressed==HTSYSMENU&&state.button.hot),FALSE,
        (GetWindowLongPtr(w,GWL_STYLE)&WS_CHILD)!=0,l.unit,&p);
    win16_control_frame_button(dc,l.minimize,HTMINBUTTON,state.button.pressed==HTMINBUTTON&&state.button.hot,FALSE,FALSE,l.unit,&p);
    win16_control_frame_button(dc,l.maximize,HTMAXBUTTON,state.button.pressed==HTMAXBUTTON&&state.button.hot,IsZoomed(w),FALSE,l.unit,&p);
    title=style_window_text_copy(w);
    if(title){
        read(context,&state);
        SIZE extent;len=lstrlenW(title);
        r=l.title;
        SelectObject(dc,state.fonts->title);SetBkMode(dc,TRANSPARENT);
        SetTextColor(dc,state.active?p.active_text:p.inactive_text);
        /* The role font is realized at this window DPI. Do not scale it a
         * second time; retain centered-or-clipped, no-ellipsis text. */
        if(GetTextExtentPoint32W(dc,title,len,&extent))
            ExtTextOutW(dc,r.left+max(1,(r.right-r.left-extent.cx)/2),
                r.top+max(0,(r.bottom-r.top-extent.cy)/2),ETO_CLIPPED,&r,title,(UINT)len,NULL);
        free(title);
    }
}
