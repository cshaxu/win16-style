#include "paint_frame.h"
#include "pixel_grid.h"

/* The bevel color roles follow USER DrawEdge. The stepped frame preserves
 * the corner and default-button geometry documented in the Win3.x review. */
BOOL style_paint_color(HDC dc,const RECT *rect,COLORREF color)
{
    if(rect->right<=rect->left || rect->bottom<=rect->top)return TRUE;
    COLORREF old=SetDCBrushColor(dc,color);if(old==CLR_INVALID)return FALSE;
    BOOL result=FillRect(dc,rect,(HBRUSH)GetStockObject(DC_BRUSH))!=0;
    if(SetDCBrushColor(dc,old)==CLR_INVALID)result=FALSE;
    return result;
}
BOOL style_paint_relief(HDC dc,RECT *bounds,UINT edge,UINT flags,const style_relief_palette *p)
{
    UINT sides=flags&BF_RECT;
    if(!dc || !bounds || !p || !sides || (flags&BF_DIAGONAL))return FALSE;
    RECT r=*bounds;BOOL soft=(flags&BF_SOFT)!=0,flat=(flags&(BF_FLAT|BF_MONO))!=0;
    for(unsigned pass=0;pass<2;++pass){
        UINT part=edge&(pass?BDR_INNER:BDR_OUTER);COLORREF tl,br;
        if(!part)continue;
        if(flat){tl=br=(flags&BF_MONO)?(pass?p->window:p->text):(pass?p->face:p->shadow);}
        else switch(part){
        case BDR_RAISEDOUTER:tl=soft?p->highlight:p->light;br=p->dark;break;
        case BDR_RAISEDINNER:tl=soft?p->light:p->highlight;br=p->shadow;break;
        case BDR_SUNKENOUTER:tl=soft?p->dark:p->shadow;br=p->highlight;break;
        case BDR_SUNKENINNER:tl=soft?p->shadow:p->dark;br=p->light;break;
        default:return FALSE;
        }
        if(!style_paint_edge_layer(dc,&r,sides,tl,br))return FALSE;
    }
    if((flags&BF_MIDDLE) && !style_paint_color(dc,&r,(flags&BF_MONO)?p->window:p->face))return FALSE;
    if(flags&BF_ADJUST)*bounds=r;
    return TRUE;
}

BOOL style_paint_dither(HDC dc,const RECT *r,const style_relief_palette *p)
{
    if(p->highlight!=RGB(255,255,255))return style_paint_color(dc,r,p->highlight);
    /* ntuser/kernel/init.c LW_BrushInit's original monochrome gray pattern. */
    return style_paint_checker(dc,r,p->face,p->highlight);
}

BOOL style_paint_line_frame(HDC dc,RECT *bounds,UINT edge,UINT flags,int width,
    COLORREF flat,style_line_profile profile,const style_relief_palette *p)
{
    if(profile==STYLE_LINE_RELIEF)return style_paint_relief(dc,bounds,edge,flags|BF_RECT,p);
    if(profile!=STYLE_LINE_FLAT || width<1)return FALSE;
    style_paint_solid_frame(dc,*bounds,flat,width);
    if(flags&BF_ADJUST)InflateRect(bounds,-width,-width);
    return TRUE;
}
static BOOL relief_button(HDC dc,RECT *bounds,UINT state,BOOL default_button,const style_relief_palette *p)
{
    if(!dc || !bounds || !p)return FALSE;
    RECT r=*bounds;
    if(default_button){
        HBRUSH border=CreateSolidBrush(p->text);if(!border)return FALSE;
        BOOL drawn=FrameRect(dc,&r,border)!=0;DeleteObject(border);if(!drawn)return FALSE;
        InflateRect(&r,-1,-1);
    }
    UINT flags=BF_RECT|BF_ADJUST|BF_SOFT;
    if(state&DFCS_FLAT)flags|=BF_FLAT;
    if(state&DFCS_MONO)flags|=BF_MONO;
    if(default_button && (state&DFCS_PUSHED))flags|=BF_FLAT;
    if(!style_paint_relief(dc,&r,(state&(DFCS_PUSHED|DFCS_CHECKED))?EDGE_SUNKEN:EDGE_RAISED,flags,p))return FALSE;
    if(state&DFCS_CHECKED){if(!style_paint_dither(dc,&r,p))return FALSE;}
    else if(!style_paint_color(dc,&r,p->face))return FALSE;
    if(state&DFCS_ADJUSTRECT)*bounds=r;
    return TRUE;
}

RECT style_paint_stepped_border(HDC dc,RECT r,const style_relief_palette *palette,int u,int border,BOOL pushed)
{
    RECT bounds=r,corner;
    style_paint_solid_frame(dc,bounds,palette->dark,border);
    SetDCBrushColor(dc,palette->highlight);
    SetRect(&corner,r.left,r.top,r.left+u,r.top+u);FillRect(dc,&corner,(HBRUSH)GetStockObject(DC_BRUSH));
    OffsetRect(&corner,r.right-r.left-u,0);FillRect(dc,&corner,(HBRUSH)GetStockObject(DC_BRUSH));
    OffsetRect(&corner,0,r.bottom-r.top-u);FillRect(dc,&corner,(HBRUSH)GetStockObject(DC_BRUSH));
    OffsetRect(&corner,-(r.right-r.left-u),0);FillRect(dc,&corner,(HBRUSH)GetStockObject(DC_BRUSH));
    InflateRect(&bounds,-border,-border);
    RECT strip=bounds;int band=pushed?u:2*u;
    SetDCBrushColor(dc,pushed?palette->shadow:palette->highlight);
    strip.bottom=strip.top+band;FillRect(dc,&strip,(HBRUSH)GetStockObject(DC_BRUSH));
    strip=bounds;strip.right=strip.left+band;FillRect(dc,&strip,(HBRUSH)GetStockObject(DC_BRUSH));
    if(!pushed){SetDCBrushColor(dc,palette->shadow);
        for(int i=0;i<2;++i){strip=bounds;InflateRect(&strip,-i*u,-i*u);
            strip.top=strip.bottom-u;FillRect(dc,&strip,(HBRUSH)GetStockObject(DC_BRUSH));
            strip=bounds;InflateRect(&strip,-i*u,-i*u);
            strip.left=strip.right-u;FillRect(dc,&strip,(HBRUSH)GetStockObject(DC_BRUSH));}}
    return bounds;
}

BOOL style_paint_push_frame(HDC dc,RECT *bounds,UINT state,BOOL default_button,int unit,
    style_push_profile profile,const style_relief_palette *p)
{
    if(!dc || !bounds || !p)return FALSE;
    if(profile==STYLE_PUSH_RELIEF)return relief_button(dc,bounds,state,default_button,p);
    if(profile!=STYLE_PUSH_STEPPED || unit<1)return FALSE;
    BOOL pressed=(state&(DFCS_PUSHED|DFCS_CHECKED))!=0;
    int border=default_button && !(state&DFCS_INACTIVE)?2*unit:unit;
    if(!style_paint_color(dc,bounds,p->face))return FALSE;
    RECT content=style_paint_stepped_border(dc,*bounds,p,unit,border,pressed);
    if(state&DFCS_ADJUSTRECT)*bounds=content;
    return TRUE;
}

/* DrawFrameControl geometry and the system font's arrow glyphs. */
BOOL style_paint_scroll_system_arrow(HDC dc,const RECT *bounds,UINT state,const style_relief_palette *p)
{
    UINT direction=state&0xffu;
    if(!dc || !bounds || !p || (direction>3u && direction!=DFCS_SCROLLCOMBOBOX) ||
       (state&~(0xffu|DFCS_PUSHED|DFCS_INACTIVE|DFCS_FLAT|DFCS_MONO)))return FALSE;
    RECT r=*bounds;if(r.right-r.left<=4 || r.bottom-r.top<=4)return FALSE;
    int saved=SaveDC(dc);if(!saved)return FALSE;
    BOOL result=FALSE;HFONT font=NULL;UINT flags=BF_RECT|BF_ADJUST;
    if(state&DFCS_FLAT)flags|=BF_FLAT;
    if(state&DFCS_MONO)flags|=BF_FLAT|BF_MONO;
    if(!style_paint_relief(dc,&r,(state&DFCS_PUSHED)?EDGE_SUNKEN:EDGE_RAISED,flags,p) || !style_paint_color(dc,&r,p->face))goto done;
    int size=min(r.right-r.left,r.bottom-r.top);
    font=CreateFontW(size,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,SYMBOL_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,0,L"Marlett");
    if(!font)goto done;
    HGDIOBJ old=SelectObject(dc,font);WCHAR family[32];
    if(!old || old==HGDI_ERROR || !GetTextFaceW(dc,ARRAYSIZE(family),family) || lstrcmpiW(family,L"Marlett"))goto done;
    if(!SetBkMode(dc,TRANSPARENT) || SetTextAlign(dc,TA_LEFT|TA_TOP)==GDI_ERROR ||
       IntersectClipRect(dc,bounds->left,bounds->top,bounds->right,bounds->bottom)==ERROR)goto done;
    WCHAR ch=(direction&DFCS_SCROLLLEFT)?L'3':L'5';if(direction&DFCS_SCROLLDOWN)++ch;
    int x=r.left+(r.right-r.left-size)/2,y=r.top+(r.bottom-r.top-size)/2;
    BOOL disabled=(state&DFCS_INACTIVE)!=0;
    if(state&(DFCS_INACTIVE|DFCS_PUSHED)){++x;++y;}
    if(SetTextColor(dc,disabled?p->highlight:p->text)==CLR_INVALID || !TextOutW(dc,x,y,&ch,1))goto done;
    if(disabled && (SetTextColor(dc,p->shadow)==CLR_INVALID || !TextOutW(dc,x-1,y-1,&ch,1)))goto done;
    result=TRUE;
done:
    if(!RestoreDC(dc,saved))result=FALSE;
    if(font && !DeleteObject(font))result=FALSE;
    return result;
}

BOOL style_paint_scroll_arrow(HDC dc,const RECT *r,UINT state,const style_relief_palette *palette,
    const style_scroll_arrow_art *art)
{
    if(!art || art->source!=STYLE_SCROLL_ARROW_INDEXED)
        return style_paint_scroll_system_arrow(dc,r,state,palette);
    unsigned direction=state&0xffu;
    if(!dc || !r || !palette || IsRectEmpty(r) || direction>=STYLE_SCROLL_ARROW_DIRECTIONS)return FALSE;
    unsigned status=(state&DFCS_INACTIVE)?2u:(state&DFCS_PUSHED)?1u:0u;
    const BYTE *packed=art->cells[status][direction];
    char pixels[STYLE_SCROLL_ARROW_SIDE*STYLE_SCROLL_ARROW_SIDE];
    for(unsigned i=0;i<ARRAYSIZE(pixels);++i)pixels[i]=(char)('0'+((packed[i/4]>>(2*(i%4)))&3));
    const COLORREF colors[]={palette->dark,palette->highlight,palette->shadow,palette->face};
    const style_pixel_grid grid={pixels,STYLE_SCROLL_ARROW_SIDE,STYLE_SCROLL_ARROW_SIDE,STYLE_SCROLL_ARROW_SIDE,
        -1,colors,{r->left,r->top},{r->right-r->left,r->bottom-r->top},{STYLE_SCROLL_ARROW_SIDE,STYLE_SCROLL_ARROW_SIDE}};
    COLORREF previous=GetDCBrushColor(dc);
    style_paint_pixel_grid(dc,&grid);
    SetDCBrushColor(dc,previous);
    return TRUE;
}
