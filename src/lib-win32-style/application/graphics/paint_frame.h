#ifndef STYLE_PAINT_FRAME_H
#define STYLE_PAINT_FRAME_H
#include <windows.h>

/* Draw an inward solid frame without changing the caller's DC state. */
static inline void style_paint_solid_frame(HDC dc,RECT r,COLORREF color,int width)
{
    HBRUSH brush=CreateSolidBrush(color);
    if(!brush)return;
    for(int i=0;i<width && r.right>r.left && r.bottom>r.top;++i){
        FrameRect(dc,&r,brush);InflateRect(&r,-1,-1);
    }
    DeleteObject(brush);
}
typedef enum style_line_profile {
    STYLE_LINE_DEFAULT, STYLE_LINE_FLAT, STYLE_LINE_RELIEF
} style_line_profile;

typedef enum style_push_profile {
    STYLE_PUSH_DEFAULT,STYLE_PUSH_STEPPED,STYLE_PUSH_RELIEF
} style_push_profile;
typedef struct style_relief_palette {
    COLORREF face,light,highlight,shadow,dark,text,gray,window;
    COLORREF selection,selection_text;
} style_relief_palette;
BOOL style_paint_color(HDC,const RECT *,COLORREF);
BOOL style_paint_relief(HDC,RECT *,UINT,UINT,const style_relief_palette *);
BOOL style_paint_line_frame(HDC,RECT *,UINT,UINT,int,COLORREF,style_line_profile,const style_relief_palette *);
BOOL style_paint_dither(HDC,const RECT *,const style_relief_palette *);
RECT style_paint_stepped_border(HDC,RECT,const style_relief_palette *,int,int,BOOL);
BOOL style_paint_push_frame(HDC,RECT *,UINT,BOOL,int,style_push_profile,const style_relief_palette *);

enum {
    STYLE_SCROLL_ARROW_SIDE=17, STYLE_SCROLL_ARROW_DIRECTIONS=4, STYLE_SCROLL_ARROW_STATES=3,
    STYLE_SCROLL_ARROW_BYTES=(STYLE_SCROLL_ARROW_SIDE*STYLE_SCROLL_ARROW_SIDE+3)/4
};
typedef enum style_scroll_arrow_source {
    STYLE_SCROLL_ARROW_DEFAULT, STYLE_SCROLL_ARROW_INDEXED, STYLE_SCROLL_ARROW_SYMBOL
} style_scroll_arrow_source;
typedef struct style_scroll_arrow_art {
    style_scroll_arrow_source source;
    BYTE cells[STYLE_SCROLL_ARROW_STATES][STYLE_SCROLL_ARROW_DIRECTIONS][STYLE_SCROLL_ARROW_BYTES];
} style_scroll_arrow_art;
BOOL style_paint_scroll_system_arrow(HDC,const RECT *,UINT,const style_relief_palette *);
BOOL style_paint_scroll_arrow(HDC,const RECT *,UINT,const style_relief_palette *,const style_scroll_arrow_art *);

static inline BOOL style_paint_checker(HDC dc,const RECT *rect,COLORREF zero,COLORREF one)
{
    static const WORD bits[]={0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa};
    HBITMAP bitmap=CreateBitmap(8,8,1,1,bits);
    if(!bitmap)return FALSE;
    HBRUSH brush=CreatePatternBrush(bitmap);
    BOOL filled=FALSE;
    if(brush){
        COLORREF old_text=SetTextColor(dc,zero),old_background=SetBkColor(dc,one);
        filled=FillRect(dc,rect,brush)!=0;
        SetBkColor(dc,old_background);
        SetTextColor(dc,old_text);
        DeleteObject(brush);
    }
    DeleteObject(bitmap);
    return filled;
}

/* Draw one rectangular edge layer, with bottom/right owning the corners. */
static inline BOOL style_paint_edge_layer(HDC dc,RECT *r,UINT sides,COLORREF leading,COLORREF trailing)
{
    if(r->right>r->left && r->bottom>r->top){
        const RECT edges[]={
            {r->left,r->top,r->left+1,r->bottom},
            {r->left,r->top,r->right,r->top+1},
            {r->right-1,r->top,r->right,r->bottom},
            {r->left,r->bottom-1,r->right,r->bottom}
        };
        const UINT flags[]={BF_LEFT,BF_TOP,BF_RIGHT,BF_BOTTOM};
        COLORREF old=GetDCBrushColor(dc);
        BOOL ok=old!=CLR_INVALID;
        for(unsigned i=0;ok && i<ARRAYSIZE(edges);++i)
            if(sides&flags[i])ok=SetDCBrushColor(dc,i<2?leading:trailing)!=CLR_INVALID &&
                FillRect(dc,&edges[i],(HBRUSH)GetStockObject(DC_BRUSH));
        if(old!=CLR_INVALID && SetDCBrushColor(dc,old)==CLR_INVALID)ok=FALSE;
        if(!ok)return FALSE;
    }
    r->left+=!!(sides&BF_LEFT);r->top+=!!(sides&BF_TOP);
    r->right-=!!(sides&BF_RIGHT);r->bottom-=!!(sides&BF_BOTTOM);
    return TRUE;
}

static inline BOOL style_paint_separator(HDC dc,RECT r,UINT type,style_line_profile profile,
    int unit,COLORREF ink,COLORREF shadow,COLORREF light)
{
    if(profile==STYLE_LINE_FLAT){
        if(type==SS_ETCHEDHORZ)r.bottom=min(r.bottom,r.top+unit);
        if(type==SS_ETCHEDVERT)r.right=min(r.right,r.left+unit);
        style_paint_solid_frame(dc,r,ink,unit);
        return TRUE;
    }
    return style_paint_edge_layer(dc,&r,BF_RECT,shadow,light) &&
        (type!=SS_ETCHEDFRAME || style_paint_edge_layer(dc,&r,BF_RECT,light,shadow));
}
#endif
