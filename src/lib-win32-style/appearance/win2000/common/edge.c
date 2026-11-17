/* Rectangular edge behavior follows Win2000 ntuser/rtl/draw.c DrawEdge;
 * see the pinned source and original-image evidence for T27. */
#include "edge.h"

BOOL win2k_control_fill(HDC dc,const RECT *rect,COLORREF color)
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
        if(!win2k_control_fill(dc,&left,top_left) || !win2k_control_fill(dc,&top,top_left) ||
           !win2k_control_fill(dc,&right,bottom_right) || !win2k_control_fill(dc,&bottom,bottom_right))return FALSE;
    }
    InflateRect(r,-1,-1);return TRUE;
}
BOOL win2k_control_edge(HDC dc,RECT *bounds,UINT edge,UINT flags,const win2k_control_palette *p)
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
    if((flags&BF_MIDDLE) && !win2k_control_fill(dc,&r,(flags&BF_MONO)?p->window:p->face))return FALSE;
    if(flags&BF_ADJUST)*bounds=r;
    return TRUE;
}
