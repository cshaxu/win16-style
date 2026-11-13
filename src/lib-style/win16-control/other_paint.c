#include "other_paint.h"
#include "basic_paint.h"
#include "../common/icon_size.h"
#include "../common/paint_frame.h"
#include <commctrl.h>

static BOOL fit_push_text(HDC target,HFONT font,const WCHAR *text,const RECT *bounds,COLORREF face,COLORREF ink,SIZE *drawn)
{
    RECT measure={0,0,0,0};
    if(!DrawTextW(target,text,-1,&measure,DT_CALCRECT|DT_SINGLELINE))return FALSE;
    int width=measure.right,height=measure.bottom,available_width=bounds->right-bounds->left,available_height=bounds->bottom-bounds->top;
    if(width<=0 || height<=0 || width>4096 || height>256 || available_width<=0 || available_height<=0 ||
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
void win16_control_other(HWND w,const BOOL *detached,const win16_style_palette *palette,HDC dc,RECT r,HFONT font,const WCHAR *text,BOOL label,UINT (*button_type)(HWND,DWORD),BOOL (*group_push)(HWND))
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);UINT type=(label?(style&SS_TYPEMASK):button_type(w,style));
    int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));RECT bounds=r;TEXTMETRICW tm;
    LRESULT button_state=label?0:SendMessageW(w,BM_GETSTATE,0,0);
    UINT flags=DT_LEFT;BOOL pushed=(button_state&BST_PUSHED)!=0,fit=!label && group_push(w);SIZE fitted={0};
    if(!label && (style&BS_PUSHLIKE))pushed=pushed || SendMessageW(w,BM_GETCHECK,0,0)!=BST_UNCHECKED;
    if(*detached)return;
    SelectObject(dc,font);GetTextMetricsW(dc,&tm);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,(*palette).edge);
    if(label && type>=SS_ETCHEDHORZ && type<=SS_ETCHEDFRAME){
        /* A later STATIC separator has no Win3.x etched relief. Keep its
         * geometry and express it using the same flat border as SS_*FRAME. */
        if(type==SS_ETCHEDHORZ)r.bottom=min(r.bottom,r.top+u);
        if(type==SS_ETCHEDVERT)r.right=min(r.right,r.left+u);
        style_paint_solid_frame(dc,r,(*palette).edge,u);return;
    }
    if(label && type>=SS_BLACKRECT && type<=SS_WHITEFRAME){
        COLORREF color=(type==SS_BLACKRECT || type==SS_BLACKFRAME)?(*palette).edge:
            (type==SS_GRAYRECT || type==SS_GRAYFRAME)?(*palette).shadow:(*palette).light;
        if(type>=SS_BLACKFRAME)style_paint_solid_frame(dc,r,color,u);
        else {SetDCBrushColor(dc,color);FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));}return;
    }
    if(!label && type==BS_GROUPBOX){
        RECT title=r;title.left+=8*u;DrawTextW(dc,text,-1,&title,DT_CALCRECT|DT_SINGLELINE);
        bounds.top+=tm.tmHeight/2;style_paint_solid_frame(dc,bounds,(*palette).shadow,u);
        InflateRect(&title,u,0);SetDCBrushColor(dc,(*palette).light);FillRect(dc,&title,(HBRUSH)GetStockObject(DC_BRUSH));
        title.left+=u;DrawTextW(dc,text,-1,&title,DT_SINGLELINE);return;
    }
    if(!label){
        /* Retail USER 1:8E0C draws one continuous frame: two border units
         * for an enabled default, one otherwise. It does not add a spaced
         * second rectangle. USER 25:18BC suppresses default emphasis when
         * disabled. Native BUTTON/dialog code still owns all these states. */
        int border=(type==BS_DEFPUSHBUTTON && IsWindowEnabled(w)?2:1)*u;
        SetDCBrushColor(dc,(*palette).face);FillRect(dc,&bounds,(HBRUSH)GetStockObject(DC_BRUSH));
        bounds=win16_control_button_border(dc,r,&(*palette),u,border,pushed);
        InflateRect(&bounds,-3*u,fit?-u:-3*u);if(pushed)OffsetRect(&bounds,u,u);
        flags=DT_CENTER|DT_VCENTER|DT_SINGLELINE;
    }else {flags=type==SS_CENTER?DT_CENTER:type==SS_RIGHT?DT_RIGHT:DT_LEFT;
        /* StaticCallback expands tabs and preserves explicit line breaks.
         * LEFTNOWORDWRAP removes word wrapping, not multiline text. SIMPLE
         * is the distinct single-line, always-enabled original fast path. */
        flags|=type==SS_SIMPLE?DT_SINGLELINE:DT_EXPANDTABS|(type==SS_LEFTNOWORDWRAP?0:DT_WORDBREAK);
        if(style&SS_NOPREFIX)flags|=DT_NOPREFIX;}
    if(!IsWindowEnabled(w) && !(label && type==SS_SIMPLE))SetTextColor(dc,(*palette).shadow);
    if(!label && (style&(BS_ICON|BS_BITMAP))){
        UINT image_type=(style&BS_ICON)?IMAGE_ICON:IMAGE_BITMAP;
        HANDLE image=(HANDLE)SendMessageW(w,BM_GETIMAGE,image_type,0);
        BUTTON_IMAGELIST images={0};BOOL listed=FALSE;
        int cx=0,cy=0;BITMAP bits;
        if(*detached || !IsWindow(w))return;
        if(!image){
            listed=(BOOL)SendMessageW(w,BCM_GETIMAGELIST,0,(LPARAM)&images);
            if(*detached || !IsWindow(w))return;
            listed=listed && images.himl && images.himl!=BCCL_NOGLYPH &&
                ImageList_GetIconSize(images.himl,&cx,&cy);
        }
        if(image && image_type==IMAGE_ICON){
            (void)style_icon_size((HICON)image,&cx,&cy);
        }else if(image && GetObjectW(image,sizeof(bits),&bits)){cx=bits.bmWidth;cy=bits.bmHeight;}
        if((image || listed) && cx>0 && cy>0){
            int x=bounds.left+(bounds.right-bounds.left-cx)/2,y=bounds.top+(bounds.bottom-bounds.top-cy)/2;
            if(listed){
                bounds.left+=images.margin.left;bounds.top+=images.margin.top;
                bounds.right-=images.margin.right;bounds.bottom-=images.margin.bottom;
                x=bounds.left+(bounds.right-bounds.left-cx)/2;y=bounds.top+(bounds.bottom-bounds.top-cy)/2;
                if(images.uAlign==BUTTON_IMAGELIST_ALIGN_LEFT)x=bounds.left;
                else if(images.uAlign==BUTTON_IMAGELIST_ALIGN_RIGHT)x=bounds.right-cx;
                else if(images.uAlign==BUTTON_IMAGELIST_ALIGN_TOP)y=bounds.top;
                else if(images.uAlign==BUTTON_IMAGELIST_ALIGN_BOTTOM)y=bounds.bottom-cy;
            }else {
                if((style&BS_CENTER)==BS_LEFT)x=bounds.left;
                else if((style&BS_CENTER)==BS_RIGHT)x=bounds.right-cx;
                if((style&BS_VCENTER)==BS_TOP)y=bounds.top;
                else if((style&BS_VCENTER)==BS_BOTTOM)y=bounds.bottom-cy;
            }
            int saved=SaveDC(dc);
            if(saved){IntersectClipRect(dc,bounds.left,bounds.top,bounds.right,bounds.bottom);
                if(listed){
                    int count=ImageList_GetImageCount(images.himl),index=0;
                    if(count>1)index=!IsWindowEnabled(w)?3:pushed?2:(button_state&BST_HOT)?1:type==BS_DEFPUSHBUTTON?4:0;
                    if(!*detached && index<count)ImageList_DrawEx(images.himl,index,dc,x,y,cx,cy,CLR_NONE,
                        (*palette).shadow,count==1 && !IsWindowEnabled(w)?ILD_BLEND50:ILD_NORMAL);
                }else DrawStateW(dc,NULL,NULL,(LPARAM)image,0,x,y,cx,cy,
                        (image_type==IMAGE_ICON?DST_ICON:DST_BITMAP)|(IsWindowEnabled(w)?DSS_NORMAL:DSS_DISABLED));
                RestoreDC(dc,saved);}
        }
        if(GetFocus()==w){RECT focus=bounds;InflateRect(&focus,-u,-u);DrawFocusRect(dc,&focus);}
        return;
    }
    BOOL fitted_text=fit && fit_push_text(dc,font,text,&bounds,(*palette).face,IsWindowEnabled(w)?(*palette).edge:(*palette).shadow,&fitted);
    if(!fitted_text)DrawTextW(dc,text,-1,&bounds,flags);
    if(!label && GetFocus()==w){
        /* USER 25:15DE..1694 bounds focus around the measured label, not
         * around the whole button face. Default status is independent. */
        RECT focus=bounds,measure={0,0,0,0};int width,height;
        DrawTextW(dc,text,-1,&measure,DT_CALCRECT|DT_SINGLELINE);
        if(fitted_text){measure.right=fitted.cx;measure.bottom=fitted.cy;}
        width=min(bounds.right-bounds.left,measure.right+4*u);
        height=min(bounds.bottom-bounds.top,measure.bottom+3*u);
        focus.left+=(bounds.right-bounds.left-width)/2;focus.right=focus.left+width;
        focus.top=max(bounds.top,bounds.top+(bounds.bottom-bounds.top-measure.bottom)/2-u);
        focus.bottom=min(bounds.bottom,focus.top+height);
        DrawFocusRect(dc,&focus);
    }
}
