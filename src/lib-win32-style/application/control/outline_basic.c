#include "outline_basic.h"
#include "lib-win32-style/application/control/control_chrome.h"
#include "lib-win32-style/application/graphics/text_metrics.h"
#include "lib-win32-style/application/control/button_snapshot.h"
#include "../graphics/paint_frame.h"
#include <commctrl.h>

void style_outline_draw_basic(HWND w,style_instance *instance,const style_window_palette *palette,HDC dc,RECT r,HFONT font,const WCHAR *text,BOOL label,UINT (*button_type)(HWND,DWORD),BOOL (*group_push)(HWND),classic_control_presentation controls,COLORREF text_color)
{
    DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);UINT type=(label?(style&SS_TYPEMASK):button_type(w,style));
    int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));RECT bounds=r;TEXTMETRICW tm;
    LRESULT button_state=label?0:SendMessageW(w,BM_GETSTATE,0,0);
    UINT flags=DT_LEFT;BOOL pushed=(button_state&BST_PUSHED)!=0,fit=!label && group_push(w);SIZE fitted={0};
    BOOL checked=!label && (style&BS_PUSHLIKE) && (!pushed || controls.push==STYLE_PUSH_RELIEF) &&
        SendMessageW(w,BM_GETCHECK,0,0)!=BST_UNCHECKED;
    pushed=pushed || checked;
    if(instance->detached)return;
    SelectObject(dc,font);GetTextMetricsW(dc,&tm);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,(*palette).edge);
    if(label && type>=SS_ETCHEDHORZ && type<=SS_ETCHEDFRAME){
        style_paint_separator(dc,r,type,controls.separator,
            u,palette->edge,palette->shadow,palette->light);
        return;
    }
    if(label && type>=SS_BLACKRECT && type<=SS_WHITEFRAME){
        COLORREF color=(type==SS_BLACKRECT || type==SS_BLACKFRAME)?(*palette).edge:
            (type==SS_GRAYRECT || type==SS_GRAYFRAME)?(*palette).shadow:(*palette).light;
        if(type>=SS_BLACKFRAME)style_paint_solid_frame(dc,r,color,u);
        else {SetDCBrushColor(dc,color);FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));}return;
    }
    if(!label && type==BS_GROUPBOX){
        RECT title=r;title.left+=8*u;DrawTextW(dc,text,-1,&title,DT_CALCRECT|DT_SINGLELINE);
        bounds.top+=tm.tmHeight/2;
        style_relief_palette colors=style_control_outline_palette(palette);
        style_paint_line_frame(dc,&bounds,EDGE_ETCHED,0,u,palette->shadow,controls.group,&colors);
        InflateRect(&title,u,0);SetDCBrushColor(dc,(*palette).light);FillRect(dc,&title,(HBRUSH)GetStockObject(DC_BRUSH));
        title.left+=u;DrawTextW(dc,text,-1,&title,DT_SINGLELINE);return;
    }
    if(!label){
        UINT state=DFCS_ADJUSTRECT;
        if(button_state&BST_PUSHED)state|=DFCS_PUSHED;
        if(checked)state|=DFCS_CHECKED;
        if(style&BS_FLAT)state|=DFCS_FLAT|DFCS_MONO;
        if(!IsWindowEnabled(w))state|=DFCS_INACTIVE;
        style_relief_palette colors=style_control_outline_palette(palette);
        if(!style_paint_push_frame(dc,&bounds,state,type==BS_DEFPUSHBUTTON,u,controls.push,&colors))return;
        InflateRect(&bounds,-3*u,fit?-u:-3*u);if(pushed)OffsetRect(&bounds,u,u);
        flags=DT_CENTER|DT_VCENTER|DT_SINGLELINE;
    }else {flags=type==SS_CENTER?DT_CENTER:type==SS_RIGHT?DT_RIGHT:DT_LEFT;
        /* StaticCallback expands tabs and preserves explicit line breaks.
         * LEFTNOWORDWRAP removes word wrapping, not multiline text. SIMPLE
         * is the distinct single-line, always-enabled original fast path. */
        flags|=type==SS_SIMPLE?DT_SINGLELINE:DT_EXPANDTABS|(type==SS_LEFTNOWORDWRAP?0:DT_WORDBREAK);
        if(style&SS_NOPREFIX)flags|=DT_NOPREFIX;}
    if(text_color==CLR_INVALID)text_color=!IsWindowEnabled(w) && !(label && type==SS_SIMPLE)?palette->shadow:palette->edge;
    SetTextColor(dc,text_color);
    if(!label && (style&(BS_ICON|BS_BITMAP))){
        UINT image_type=(style&BS_ICON)?IMAGE_ICON:IMAGE_BITMAP;
        style_control_button_image snapshot;
        if(!style_control_button_image_read(w,instance,image_type,&snapshot))return;
        HANDLE image=snapshot.image;BUTTON_IMAGELIST images=snapshot.images;BOOL listed=snapshot.listed;
        int cx=snapshot.size.cx,cy=snapshot.size.cy;
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
                    if(!instance->detached && index<count)ImageList_DrawEx(images.himl,index,dc,x,y,cx,cy,CLR_NONE,
                        (*palette).shadow,count==1 && !IsWindowEnabled(w)?ILD_BLEND50:ILD_NORMAL);
                }else DrawStateW(dc,NULL,NULL,(LPARAM)image,0,x,y,cx,cy,
                        (image_type==IMAGE_ICON?DST_ICON:DST_BITMAP)|(IsWindowEnabled(w)?DSS_NORMAL:DSS_DISABLED));
                RestoreDC(dc,saved);}
        }
        if(GetFocus()==w){RECT focus=bounds;InflateRect(&focus,-u,-u);DrawFocusRect(dc,&focus);}
        return;
    }
    BOOL fitted_text=fit && style_text_fit(dc,font,text,&bounds,palette->face,text_color,(SIZE){4096,256},&fitted);
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
