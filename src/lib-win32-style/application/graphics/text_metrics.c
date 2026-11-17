#include "text_metrics.h"

BOOL style_text_fit(HDC target,HFONT font,const WCHAR *text,const RECT *bounds,COLORREF face,COLORREF ink,SIZE limit,SIZE *drawn)
{
    RECT measure={0,0,0,0};
    if(!DrawTextW(target,text,-1,&measure,DT_CALCRECT|DT_SINGLELINE))return FALSE;
    int width=measure.right,height=measure.bottom,available_width=bounds->right-bounds->left,available_height=bounds->bottom-bounds->top;
    if(width<=0 || height<=0 || width>limit.cx || height>limit.cy || available_width<=0 || available_height<=0 ||
       (width<=available_width && height<=available_height))return FALSE;
    int out_width=width,out_height=height;
    if(out_width>available_width){out_height=MulDiv(out_height,available_width,out_width);out_width=available_width;}
    if(out_height>available_height){out_width=MulDiv(out_width,available_height,out_height);out_height=available_height;}
    if(out_width<=0 || out_height<=0)return FALSE;
    HDC memory=CreateCompatibleDC(target);if(!memory)return FALSE;
    HBITMAP bitmap=CreateCompatibleBitmap(target,width,height);if(!bitmap){DeleteDC(memory);return FALSE;}
    HGDIOBJ old=SelectObject(memory,bitmap),old_font=SelectObject(memory,font);BOOL ok=FALSE;
    if(old && old!=HGDI_ERROR && old_font && old_font!=HGDI_ERROR){
        RECT r={0,0,width,height};SetDCBrushColor(memory,face);FillRect(memory,&r,(HBRUSH)GetStockObject(DC_BRUSH));
        SetBkMode(memory,TRANSPARENT);SetTextColor(memory,ink);
        if(DrawTextW(memory,text,-1,&r,DT_SINGLELINE)){
            int saved=SaveDC(target);
            if(saved){SetStretchBltMode(target,COLORONCOLOR);
                int x=bounds->left+(available_width-out_width)/2,y=bounds->top+(available_height-out_height)/2;
                ok=StretchBlt(target,x,y,out_width,out_height,memory,0,0,width,height,SRCCOPY);
                RestoreDC(target,saved);
            }
        }
    }
    if(old_font && old_font!=HGDI_ERROR)SelectObject(memory,old_font);
    if(old && old!=HGDI_ERROR)SelectObject(memory,old);
    DeleteObject(bitmap);DeleteDC(memory);
    if(ok){drawn->cx=out_width;drawn->cy=out_height;}return ok;
}
