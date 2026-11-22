/* USER: btnctl.c xxxBNInitDC/BNCalcRect/xxxBNDrawText and
 * xxxButtonDrawCheck. Pixel references and the finite contract are in T27. */
#include "button_paint.h"
#include "background.h"
#include "lib-win32-style/application/graphics/paint_frame.h"
#include "lib-win32-style/application/control/button_snapshot.h"
#include "lib-win32-style/application/control/control_class.h"
#include "lib-win32-style/application/graphics/pixel_grid.h"
#include <wchar.h>

static style_control_button_role role(HWND window,void *context,DWORD style)
{(void)context;return (style_control_button_role){style_control_button_type(window,style),FALSE};}
static HFONT font(HWND window,void *context)
{
    (void)context;HFONT value=(HFONT)SendMessageW(window,WM_GETFONT,0,0);
    return value?value:(HFONT)GetStockObject(SYSTEM_FONT);
}
static BOOL push_style(UINT type,DWORD style)
{return type<=BS_DEFPUSHBUTTON || (style&BS_PUSHLIKE);}

typedef struct button_text {const WCHAR *value;UINT format;} button_text;
static BOOL CALLBACK multiline(HDC dc,LPARAM value,WPARAM length,int width,int height)
{
    const button_text *text=(const button_text *)value;RECT r={0,0,width,height};(void)length;
    return DrawTextW(dc,text->value,-1,&r,text->format)!=0;
}
static BOOL label(HWND window,style_instance *instance,HDC dc,const style_control_button_snapshot *s,
    RECT bounds,const RECT *client,BOOL push,UINT cues,const style_relief_palette *p)
{
    int width=0,height=0;UINT type;LPARAM value;WPARAM length=0;DRAWSTATEPROC draw=NULL;
    button_text text={s->text,DT_WORDBREAK|DT_EDITCONTROL};style_control_button_image image;
    if(cues&UISF_HIDEACCEL)text.format|=DT_HIDEPREFIX;
    UINT horizontal=s->style&BS_CENTER,vertical=s->style&BS_VCENTER;
    if(!horizontal)horizontal=push?BS_CENTER:BS_LEFT;
    if(horizontal==BS_RIGHT)text.format|=DT_RIGHT;
    else if(horizontal!=BS_LEFT)text.format|=DT_CENTER;
    if(vertical==BS_BOTTOM)text.format|=DT_BOTTOM;
    else if(vertical!=BS_TOP)text.format|=DT_VCENTER;
    if(s->style&(BS_ICON|BS_BITMAP)){
        UINT kind=(s->style&BS_BITMAP)?IMAGE_BITMAP:IMAGE_ICON;
        if(!style_control_button_image_read(window,instance,kind,&image) || instance->detached || image.listed)return FALSE;
        if(!image.image)return TRUE;
        width=image.size.cx;height=image.size.cy;value=(LPARAM)image.image;
        type=kind==IMAGE_ICON?DST_ICON:DST_BITMAP;
    }else{
        if(!*s->text)return TRUE;
        BOOL multi=(s->style&BS_MULTILINE)!=0;
        RECT measured={0,0,max(0,bounds.right-bounds.left-(multi?4:0)),max(0,bounds.bottom-bounds.top-(multi?2:0))};
        UINT format=multi?text.format:DT_SINGLELINE;
        if(!DrawTextW(dc,s->text,-1,&measured,format|DT_CALCRECT))return FALSE;
        width=measured.right-measured.left;height=measured.bottom-measured.top+1;
        value=(LPARAM)s->text;length=wcslen(s->text);type=DST_PREFIXTEXT;
        if(cues&UISF_HIDEACCEL)type|=DSS_HIDEPREFIX;
        if(multi){type=DST_COMPLEX;value=(LPARAM)&text;draw=multiline;}
    }
    int x=horizontal==BS_LEFT?bounds.left+2:horizontal==BS_RIGHT?bounds.right-width-2:(bounds.left+bounds.right-width)/2;
    int y=vertical==BS_TOP?bounds.top+1:vertical==BS_BOTTOM?bounds.bottom-height-1:(bounds.top+bounds.bottom-height)/2;
    if(push && (s->state&BST_PUSHED)){++x;++y;}
    BOOL result=style_paint_content(dc,value,length,x,y,width,height,type,draw,(s->style&WS_DISABLED)!=0,
        push && s->check==BST_INDETERMINATE,p);
    if(result && (s->state&BST_FOCUS) && !(cues&UISF_HIDEFOCUS)){
        if(push)InflateRect(&bounds,-1,-1);
        else{
            bounds.left=max(client->left,x-1);bounds.top=max(client->top,y-1);
            bounds.right=min(client->right,bounds.left+width+2);bounds.bottom=min(client->bottom,bounds.top+height+2);
        }
        result=DrawFocusRect(dc,&bounds);
    }
    return result;
}
BOOL style_observed_button_paint(HWND window,style_instance *instance,HDC dc,const classic_push_colors *colors,
    const classic_choice_colors *choice,const classic_group_colors *group_colors,style_choice_art_read_fn read_art,HMODULE module,classic_choice_art *art,
    classic_control_presentation controls,const style_relief_palette *base)
{
    style_relief_palette palette=*base;
    const style_relief_palette *p=&palette;
    int saved=SaveDC(dc);if(!saved)return FALSE;
    BOOL result=FALSE;RECT client,bounds,glyph;HBRUSH brush=NULL;
    DWORD initial_style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);HWND parent=GetParent(window);
    const style_control_button_reader reader={NULL,role,font};style_control_button_snapshot snapshot={0};
    /* Later image-list-only content remains native; never silently drop it. */
    if(!(initial_style&(BS_ICON|BS_BITMAP))){
        BUTTON_IMAGELIST images={0};BOOL listed=(BOOL)SendMessageW(window,BCM_GETIMAGELIST,0,(LPARAM)&images);
        if(instance->detached || (listed && images.himl && images.himl!=BCCL_NOGLYPH))goto done;
    }
    if(!GetClientRect(window,&client))goto done;
    UINT type=style_control_button_type(window,initial_style);
    if(!classic_button_presentation_enabled(&controls,type,initial_style))goto done;
    BOOL group=type==BS_GROUPBOX;
    if(group && group_colors && group_colors->text.present){
        palette.text=group_colors->text.normal;
        palette.gray=group_colors->text.disabled;
        palette.shadow=group_colors->shadow;
        palette.highlight=group_colors->highlight;
    }
    if(!push_style(type,initial_style)){
        brush=style_observed_control_brush(window,instance,dc,p);if(!brush)goto done;
        if(!group && !FillRect(dc,&client,brush))goto done;
    }
    if(!style_control_button_snapshot_read(window,instance,&reader,&snapshot))goto done;
    UINT cues=(UINT)SendMessageW(window,WM_QUERYUISTATE,0,0);
    if(instance->detached || !IsWindow(window) || initial_style!=snapshot.style ||
       (DWORD)GetWindowLongPtrW(window,GWL_STYLE)!=snapshot.style || GetParent(window)!=parent)goto done;
    HGDIOBJ previous=SelectObject(dc,snapshot.font);if(!previous || previous==HGDI_ERROR)goto done;
    if(IntersectClipRect(dc,client.left,client.top,client.right,client.bottom)==ERROR)goto done;
    if(group){
        SIZE zero;TEXTMETRICW system;RECT text={0};HGDIOBJ old=SelectObject(dc,GetStockObject(SYSTEM_FONT));
        if(!old||old==HGDI_ERROR||!GetTextMetricsW(dc,&system)||!SelectObject(dc,old)||!GetTextExtentPoint32W(dc,L"0",1,&zero))goto done;
        bounds=client;bounds.top+=zero.cy/2;
        if(!style_paint_line_frame(dc,&bounds,EDGE_ETCHED,(snapshot.style&BS_FLAT)?BF_FLAT|BF_MONO:0,
            1,p->shadow,controls.group,p))goto done;
        if(*snapshot.text){
            if(!DrawTextW(dc,snapshot.text,-1,&text,DT_CALCRECT|DT_SINGLELINE))goto done;
            int width=text.right-text.left+4;bounds=client;
            UINT align=snapshot.style&BS_CENTER;
            bounds.left=align==BS_RIGHT?client.right-(system.tmAveCharWidth-1)-width:
                align==BS_CENTER?(client.left+client.right-width)/2:client.left+system.tmAveCharWidth-1;
            bounds.right=bounds.left+width;bounds.bottom=bounds.top+text.bottom-text.top+2;
            if(!FillRect(dc,&bounds,brush))goto done;
        }
        result=label(window,instance,dc,&snapshot,bounds,&client,FALSE,cues|UISF_HIDEFOCUS,p);goto done;
    }
    POINT origin={0,0};
    if(!LPtoDP(dc,&origin,1)||!SetBrushOrgEx(dc,origin.x,origin.y,NULL))goto done;
    BOOL push=push_style(snapshot.role.type,snapshot.style);bounds=client;
    UINT state=(snapshot.state&BST_PUSHED)?DFCS_PUSHED:0;
    if(snapshot.style&BS_FLAT)state|=DFCS_FLAT|DFCS_MONO;
    if(push){
        if(colors){
            enum classic_push_state role=classic_push_state_from_button((snapshot.style&WS_DISABLED)!=0,
                (state&DFCS_PUSHED) || snapshot.check!=BST_UNCHECKED,snapshot.role.type==BS_DEFPUSHBUTTON);
            classic_surface_colors surface=colors->state[role];
            palette.face=surface.face;
            palette.dark=surface.edge;
            COLORREF ink=classic_push_text_color(colors,role);
            if(ink!=CLR_INVALID && !(snapshot.style&(BS_BITMAP|BS_ICON))){
                palette.text=ink;
                palette.gray=ink;
            }
        }
        SetBkMode(dc,TRANSPARENT);SetTextColor(dc,p->text);SetBkColor(dc,p->face);
        if(snapshot.check)state|=DFCS_CHECKED;
        if(snapshot.style&WS_DISABLED)state|=DFCS_INACTIVE;
        int unit=max(1,MulDiv(1,(int)GetDpiForWindow(window),96));
        if(!style_paint_push_frame(dc,&bounds,state|DFCS_ADJUSTRECT,snapshot.role.type==BS_DEFPUSHBUTTON,unit,controls.push,p))
            goto done;
        IntersectClipRect(dc,bounds.left,bounds.top,bounds.right,bounds.bottom);
    }else if(!style_choice_layout(dc,client,GetDpiForWindow(window),snapshot.style,controls.choice_metrics,&glyph,&bounds))
        goto done;
    result=label(window,instance,dc,&snapshot,bounds,&client,push,cues,p);
    if(result && !push){
        BOOL radio=snapshot.role.type==BS_RADIOBUTTON || snapshot.role.type==BS_AUTORADIOBUTTON;
        state|=radio?DFCS_BUTTONRADIO:snapshot.check==BST_INDETERMINATE?DFCS_BUTTON3STATE:DFCS_BUTTONCHECK;
        if(snapshot.check)state|=DFCS_CHECKED;
        if(snapshot.style&WS_DISABLED)state|=DFCS_INACTIVE;
        const classic_choice_surface *entry=classic_choice_entry(choice,radio,snapshot.check,
            (snapshot.style&WS_DISABLED)!=0,snapshot.state);
        const classic_surface_colors *surface=classic_choice_color(entry);
        const char *image=entry && read_art?read_art(module,entry->image,art):NULL;
        if(instance->detached){result=FALSE;goto done;}
        if(image){
            const COLORREF ink[2]={surface?surface->edge:p->text,surface?surface->face:p->window};
            const style_pixel_grid grid={image,CLASSIC_CHOICE_WIDTH,CLASSIC_CHOICE_HEIGHT,CLASSIC_CHOICE_WIDTH,
                -1,ink,{glyph.left,glyph.top},{glyph.right-glyph.left,glyph.bottom-glyph.top},
                {CLASSIC_CHOICE_WIDTH,CLASSIC_CHOICE_HEIGHT}};
            style_paint_pixel_grid(dc,&grid);
        }else if(controls.choice_fallback==CLASSIC_CHOICE_SYMBOLS){
            const COLORREF ink[2]={surface?surface->face:p->window,surface?surface->edge:p->text};
            result=style_paint_choice_symbols(dc,&glyph,state,p,surface?ink:NULL);
        }else result=DrawFrameControl(dc,&glyph,DFC_BUTTON,state);
    }
done:
    if(!RestoreDC(dc,saved))result=FALSE;
    style_control_button_snapshot_clear(&snapshot);return result;
}
