#include "frame_paint.h"
#include "../resources/frame_buttons.h"

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

static void paint_button(HDC dc,RECT r,int kind,BOOL down,BOOL zoom,BOOL child,int u,
    const win16_style_palette *p,const win16_caption_art *art)
{
    BITMAPINFO bi={0};DWORD pixels[19*18];int x,y;
    const BOOL system=kind==HTSYSMENU;
    const int width=system?18:19;
    const unsigned role=system?(child?CLASSIC_IMAGE_ROLE_COUNT:CLASSIC_IMAGE_SYSTEM):
        kind==HTMINBUTTON?CLASSIC_IMAGE_MINIMIZE:zoom?CLASSIC_IMAGE_RESTORE:CLASSIC_IMAGE_MAXIMIZE;
    const BYTE *roles=art && art->available?art->roles[role][!!down]:NULL;
    const char *fallback=system?&system_menu_bitmap[0][child?18:0]:&frame_buttons[role+(down?3:0)][0][0];
    const int stride=system?37:20;
    COLORREF palette[4]={p->edge,p->light,p->shadow,p->face};
    if(IsRectEmpty(&r))return;
    win16_control_fill(dc,r,system&&down?p->shadow:p->face);win16_control_outline(dc,r,u,p->edge);
    if(!roles && system && down){
        /* The original CLOSE atlas has one half per window role. */
        palette[0]=p->light;palette[1]=p->edge;
        palette[2]=p->face;palette[3]=p->shadow;
    }
    for(y=0;y<18;++y)for(x=0;x<width;++x){
        COLORREF c=palette[roles?roles[y*width+x]:fallback[y*stride+x]-'0'];
        pixels[y*width+x]=((DWORD)GetRValue(c)<<16)|((DWORD)GetGValue(c)<<8)|GetBValue(c);
    }
    bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    bi.bmiHeader.biWidth=width;bi.bmiHeader.biHeight=-18;
    bi.bmiHeader.biPlanes=1;bi.bmiHeader.biBitCount=32;
    /* Original arrow artwork includes its left separator; CLOSE does not.
     * The caption supplies the other separators. Scale the raster once. */
    SetStretchBltMode(dc,COLORONCOLOR);
    StretchDIBits(dc,r.left,r.top,r.right-r.left-(system?u:0),r.bottom-r.top,
        0,0,width,18,pixels,&bi,DIB_RGB_COLORS,SRCCOPY);
}

void win16_style_merged_control(HDC dc,RECT bounds,BOOL system,BOOL selected,int unit,
    const win16_style_palette *palette,const win16_caption_art *art)
{
    paint_button(dc,bounds,system?HTSYSMENU:HTMAXBUTTON,selected,!system,system,unit,palette,art);
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

void win16_control_frame_caption(HDC dc,const win16_style_layout *layout,const win16_style_palette *palette,
    const win16_frame_paint_state *snapshot,PCWSTR title,BOOL child,BOOL zoomed)
{
    win16_style_layout l=*layout;win16_style_palette p=*palette;RECT r;
    win16_frame_paint_state state=*snapshot;
    win16_control_fill(dc,l.caption,state.active?p.active:p.inactive);
    r=l.caption;r.top=r.bottom;r.bottom+=l.unit;win16_control_fill(dc,r,p.edge);
    paint_button(dc,l.system,HTSYSMENU,state.system_menu||(state.bar_keyboard&&state.bar_selected==-2)||(state.button.pressed==HTSYSMENU&&state.button.hot),FALSE,
        child,l.unit,&p,state.art);
    paint_button(dc,l.minimize,HTMINBUTTON,state.button.pressed==HTMINBUTTON&&state.button.hot,FALSE,FALSE,l.unit,&p,state.art);
    paint_button(dc,l.maximize,HTMAXBUTTON,state.button.pressed==HTMAXBUTTON&&state.button.hot,zoomed,FALSE,l.unit,&p,state.art);
    if(title){
        SIZE extent;int len=lstrlenW(title);
        r=l.title;
        SelectObject(dc,state.fonts->title);SetBkMode(dc,TRANSPARENT);
        SetTextColor(dc,state.active?p.active_text:p.inactive_text);
        /* The role font is realized at this window DPI. Do not scale it a
         * second time; retain centered-or-clipped, no-ellipsis text. */
        if(GetTextExtentPoint32W(dc,title,len,&extent))
            ExtTextOutW(dc,r.left+max(1,(r.right-r.left-extent.cx)/2),
                r.top+max(0,(r.bottom-r.top-extent.cy)/2),ETO_CLIPPED,&r,title,(UINT)len,NULL);
    }
}
