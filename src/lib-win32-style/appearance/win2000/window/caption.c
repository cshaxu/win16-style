#include "caption.h"

BOOL win2k_caption_paint(HDC dc,const RECT *bounds,PCWSTR title,HFONT font,
                         const win2k_caption_colors *colors)
{
    if(!dc || !bounds || !title || !font || !colors ||
       bounds->right<=bounds->left || bounds->bottom<=bounds->top)return FALSE;
    int saved=SaveDC(dc);if(!saved)return FALSE;
    BOOL ok=TRUE;
    if(colors->gradient){
        TRIVERTEX vertices[2]={
            {bounds->left,bounds->top,(COLOR16)(GetRValue(colors->left)<<8),
             (COLOR16)(GetGValue(colors->left)<<8),(COLOR16)(GetBValue(colors->left)<<8),0},
            {bounds->right,bounds->bottom,(COLOR16)(GetRValue(colors->right)<<8),
             (COLOR16)(GetGValue(colors->right)<<8),(COLOR16)(GetBValue(colors->right)<<8),0}};
        GRADIENT_RECT gradient={0,1};
        ok=GradientFill(dc,vertices,2,&gradient,1,GRADIENT_FILL_RECT_H);
    }else{
        HBRUSH brush=CreateSolidBrush(colors->left);
        if(!brush)ok=FALSE;
        else{ok=FillRect(dc,bounds,brush)!=0;DeleteObject(brush);}
    }
    if(ok){
        RECT text=*bounds;
        if(!SelectObject(dc,font) || SetBkMode(dc,TRANSPARENT)==0 || SetTextColor(dc,colors->text)==CLR_INVALID)ok=FALSE;
        else if(*title && !DrawTextW(dc,title,-1,&text,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOPREFIX))ok=FALSE;
    }
    if(!RestoreDC(dc,saved))ok=FALSE;
    return ok;
}
