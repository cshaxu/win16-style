/* Rectangular edge behavior follows Win2000 ntuser/rtl/draw.c DrawEdge;
 * see the pinned source and original-image evidence for T27. */
#include "paint.h"

BOOL win2k_fill(HDC dc,const RECT *rect,COLORREF color)
{
    if(rect->right<=rect->left || rect->bottom<=rect->top)return TRUE;
    COLORREF old=SetDCBrushColor(dc,color);if(old==CLR_INVALID)return FALSE;
    BOOL result=FillRect(dc,rect,(HBRUSH)GetStockObject(DC_BRUSH))!=0;
    if(SetDCBrushColor(dc,old)==CLR_INVALID)result=FALSE;
    return result;
}
static BOOL layer(HDC dc,RECT *r,COLORREF top_left,COLORREF bottom_right)
{
    if(r->right>r->left && r->bottom>r->top){
        RECT left={r->left,r->top,r->left+1,r->bottom};
        RECT top={r->left,r->top,r->right,r->top+1};
        RECT right={r->right-1,r->top,r->right,r->bottom};
        RECT bottom={r->left,r->bottom-1,r->right,r->bottom};
        /* USER favors bottom/right at the corners. */
        if(!win2k_fill(dc,&left,top_left) || !win2k_fill(dc,&top,top_left) ||
           !win2k_fill(dc,&right,bottom_right) || !win2k_fill(dc,&bottom,bottom_right))return FALSE;
    }
    InflateRect(r,-1,-1);return TRUE;
}
BOOL win2k_edge(HDC dc,RECT *bounds,UINT edge,UINT flags,const win2k_palette *p)
{
    if(!dc || !bounds || !p || (flags&BF_RECT)!=BF_RECT || (flags&BF_DIAGONAL))return FALSE;
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
        if(!layer(dc,&r,tl,br))return FALSE;
    }
    if((flags&BF_MIDDLE) && !win2k_fill(dc,&r,(flags&BF_MONO)?p->window:p->face))return FALSE;
    if(flags&BF_ADJUST)*bounds=r;
    return TRUE;
}

BOOL win2k_dither(HDC dc,const RECT *r,const win2k_palette *p)
{
    if(p->highlight!=RGB(255,255,255))return win2k_fill(dc,r,p->highlight);
    /* ntuser/kernel/init.c LW_BrushInit's original monochrome gray pattern. */
    static const WORD pattern[8]={0x55,0xaa,0x55,0xaa,0x55,0xaa,0x55,0xaa};
    HBITMAP bitmap=CreateBitmap(8,8,1,1,pattern);if(!bitmap)return FALSE;
    HBRUSH brush=CreatePatternBrush(bitmap);BOOL result=FALSE;
    if(brush){
        COLORREF back=SetBkColor(dc,p->highlight),text=SetTextColor(dc,p->face);
        result=FillRect(dc,r,brush)!=0;
        SetBkColor(dc,back);SetTextColor(dc,text);DeleteObject(brush);
    }
    DeleteObject(bitmap);return result;
}
BOOL win2k_push_button_frame(HDC dc,RECT *bounds,UINT state,BOOL default_button,const win2k_palette *p)
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
    if(!win2k_edge(dc,&r,(state&(DFCS_PUSHED|DFCS_CHECKED))?EDGE_SUNKEN:EDGE_RAISED,flags,p))return FALSE;
    if(state&DFCS_CHECKED){if(!win2k_dither(dc,&r,p))return FALSE;}
    else if(!win2k_fill(dc,&r,p->face))return FALSE;
    if(state&DFCS_ADJUSTRECT)*bounds=r;
    return TRUE;
}

BOOL win2k_scroll_arrow_button(HDC dc,const RECT *bounds,UINT state,const win2k_palette *p)
{
    UINT direction=state&0xffu;
    if(!dc || !bounds || !p || (direction>3u && direction!=DFCS_SCROLLCOMBOBOX) ||
       (state&~(0xffu|DFCS_PUSHED|DFCS_INACTIVE|DFCS_FLAT|DFCS_MONO)))return FALSE;
    RECT r=*bounds;if(r.right-r.left<=4 || r.bottom-r.top<=4)return FALSE;
    int saved=SaveDC(dc);if(!saved)return FALSE;
    BOOL result=FALSE;HFONT font=NULL;UINT flags=BF_RECT|BF_ADJUST;
    if(state&DFCS_FLAT)flags|=BF_FLAT;
    if(state&DFCS_MONO)flags|=BF_FLAT|BF_MONO;
    if(!win2k_edge(dc,&r,(state&DFCS_PUSHED)?EDGE_SUNKEN:EDGE_RAISED,flags,p) || !win2k_fill(dc,&r,p->face))goto done;
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

/* Win2000 rtl/draw.c DrawBox and DrawFrameControl; original screenshot hashes
 * validate the system font's covered outlines separately from the color rule. */
BOOL win2k_choice_glyph(HDC dc,const RECT *rect,UINT state,const win2k_palette *p)
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
    const WCHAR *layers=kind==DFCS_BUTTONRADIO?L"nmlkji":L"gfedcb";
    unsigned first=0;
    if(mixed && p->highlight==RGB(255,255,255)){
        if(!win2k_dither(dc,rect,p))goto done;
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
