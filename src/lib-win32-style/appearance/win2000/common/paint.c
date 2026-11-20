/* Rectangular edge behavior follows Win2000 ntuser/rtl/draw.c DrawEdge;
 * see the pinned source and original-image evidence for T27. */
#include "paint.h"
#include "lib-win32-style/application/graphics/paint_frame.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"

/* Win2000 rtl/draw.c DrawBox and DrawFrameControl; original screenshot hashes
 * validate the system font's covered outlines separately from the color rule. */
BOOL win2k_choice_glyph(HDC dc,const RECT *rect,UINT state,const win2k_palette *p,
    const classic_surface_colors *surface)
{
    if(!dc||!rect||!p||rect->right<=rect->left||rect->right-rect->left!=rect->bottom-rect->top)return FALSE;
    UINT kind=state&0xff;
    if((kind!=DFCS_BUTTONCHECK && kind!=DFCS_BUTTONRADIO && kind!=DFCS_BUTTON3STATE) ||
       (state&~(0xffu|DFCS_CHECKED|DFCS_PUSHED|DFCS_INACTIVE|DFCS_FLAT|DFCS_MONO)))return FALSE;
    int saved=SaveDC(dc);if(!saved)return FALSE;
    int size=rect->right-rect->left;BOOL result=FALSE;
    HFONT font=CreateFontW(size,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,SYMBOL_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,0,L"Marlett");
    WCHAR family[32];
    if(!font)goto done;
    HGDIOBJ previous=SelectObject(dc,font);
    if(!previous||previous==HGDI_ERROR||!GetTextFaceW(dc,ARRAYSIZE(family),family)||lstrcmpiW(family,L"Marlett"))goto done;
    if(!SetBkMode(dc,TRANSPARENT)||SetTextAlign(dc,TA_TOP|TA_LEFT)==GDI_ERROR||
       IntersectClipRect(dc,rect->left,rect->top,rect->right,rect->bottom)==ERROR)goto done;
    BOOL flat=(state&(DFCS_FLAT|DFCS_MONO))!=0;
    BOOL mixed=kind==DFCS_BUTTON3STATE && (state&DFCS_CHECKED) && !(state&(DFCS_PUSHED|DFCS_INACTIVE));
    COLORREF colors[6]={
        (state&(DFCS_PUSHED|DFCS_INACTIVE))?p->face:mixed?p->highlight:p->window,
        flat?p->text:p->light,flat?p->text:p->dark,flat?p->text:p->highlight,flat?p->text:p->shadow,
        (state&(DFCS_BUTTON3STATE|DFCS_INACTIVE))?p->shadow:p->text
    };
    if(surface){
        colors[0]=surface->face;
        colors[5]=surface->edge;
    }
    const WCHAR *layers=kind==DFCS_BUTTONRADIO?L"nmlkji":L"gfedcb";
    unsigned first=0;
    if(mixed && colors[0]==RGB(255,255,255) && p->highlight==RGB(255,255,255)){
        if(!style_paint_dither(dc,rect,p))goto done;
        first=1;
    }
    for(unsigned layer_index=first;layer_index<5u+!!(state&DFCS_CHECKED);++layer_index){
        if(SetTextColor(dc,colors[layer_index])==CLR_INVALID ||
           !TextOutW(dc,rect->left,rect->top,layers+layer_index,1))goto done;
    }
    result=TRUE;
done:
    if(!RestoreDC(dc,saved))result=FALSE;
    if(font && !DeleteObject(font))result=FALSE;
    return result;
}

BOOL win2k_draw_content(HDC dc,LPARAM value,WPARAM length,int x,int y,int width,int height,
    UINT type,DRAWSTATEPROC draw,BOOL disabled,BOOL monochrome,const win2k_palette *p)
{
    if(!disabled && !monochrome)return DrawStateW(dc,NULL,draw,value,length,x,y,width,height,type);
    HBRUSH light=disabled?CreateSolidBrush(p->highlight):NULL,gray=CreateSolidBrush(p->gray);
    BOOL result=gray && (!disabled || light);
    if(result && disabled)result=DrawStateW(dc,light,draw,value,length,x+1,y+1,width,height,type|DSS_MONO);
    if(result)result=DrawStateW(dc,gray,draw,value,length,x,y,width,height,type|DSS_MONO);
    if(light)DeleteObject(light);
    if(gray)DeleteObject(gray);
    return result;
}
