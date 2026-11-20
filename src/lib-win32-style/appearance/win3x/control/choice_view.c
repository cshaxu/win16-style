#include "choice_view.h"
#include "choice_paint.h"
#include "other_paint.h"
#include "../../../application/control/control_protocol.h"
#include "../../../application/graphics/icon_size.h"
#include "lib-win32-style/application/control/button_snapshot.h"
#include <uxtheme.h>

static style_control_button_role choice_role(HWND window,void *context,DWORD style)
{
    const win16_control_choice_view *view=context;
    UINT type=view->button_type(window,style);BOOL label=view->is_static(window);
    return (style_control_button_role){type,label};
}
static HFONT choice_font(HWND window,void *context)
{
    const win16_control_choice_view *view=context;
    return view->state->font && (view->vb_frame(window) || view->vb_button(window) || style_control_protocol_is(window,L"BUTTON"))?
        view->state->font:(HFONT)SendMessageW(window,WM_GETFONT,0,0);
}

void win16_control_draw_choice(HWND w,const win16_control_choice_view *view,HDC dc)
{
    RECT r,glyph,label,focus;int saved,x,y,row,column,height;
    UINT dpi=GetDpiForWindow(w);style_control_button_snapshot snapshot;
    const style_control_button_reader reader={(void *)view,choice_role,choice_font};
    if(!style_control_button_snapshot_read(w,view->instance,&reader,&snapshot))return;
    DWORD style=snapshot.style;UINT type=snapshot.role.type;LRESULT state=snapshot.state,check=snapshot.check;
    WCHAR *text=snapshot.text;HFONT font=snapshot.font;TEXTMETRICW tm;
    BOOL radio=type==BS_RADIOBUTTON || type==BS_AUTORADIOBUTTON;
    BOOL label_control=snapshot.role.label;
    if(label_control && (style&SS_TYPEMASK)==SS_ICON){
        /* statctl.c retains image ownership/sizing in native STATIC. The
         * painter borrows its current handle, never reads image data as text.
         * CENTERIMAGE is a host extension; its natural-size image is clipped. */
        HICON icon=snapshot.icon;int cx,cy;
        saved=SaveDC(dc);if(!saved){style_control_button_snapshot_clear(&snapshot);return;}
        GetClientRect(w,&r);cx=r.right;cy=r.bottom;x=y=0;
        IntersectClipRect(dc,r.left,r.top,r.right,r.bottom);
        SetDCBrushColor(dc,view->palette->light);FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));
        if(icon && (style&SS_CENTERIMAGE)){
            if(style_icon_size(icon,&cx,&cy)){x=(r.right-cx)/2;y=(r.bottom-cy)/2;}
        }
        if(icon)DrawIconEx(dc,x,y,icon,cx,cy,0,NULL,DI_NORMAL);
        RestoreDC(dc,saved);style_control_button_snapshot_clear(&snapshot);return;
    }
    if(!font)font=(HFONT)GetStockObject(SYSTEM_FONT);
    saved=SaveDC(dc);if(!saved){style_control_button_snapshot_clear(&snapshot);return;}
    GetClientRect(w,&r);SetDCBrushColor(dc,view->palette->light);
    /* A group box surrounds sibling controls, not an opaque child surface.
     * Repainting its interior erases those siblings during ordinary motion. */
    if(label_control || type!=BS_GROUPBOX){
        BOOL check_parent=view->group_choice(w);
        BOOL parent_painted=check_parent && SUCCEEDED(DrawThemeParentBackground(w,dc,&r));
        if(view->instance->detached || !IsWindow(w) || (check_parent && !view->group_choice(w))){RestoreDC(dc,saved);style_control_button_snapshot_clear(&snapshot);return;}
        if(!parent_painted)FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));
    }
    if(label_control || type==BS_PUSHBUTTON || type==BS_DEFPUSHBUTTON || type==BS_GROUPBOX || (style&BS_PUSHLIKE)){
        win16_style_palette palette=*view->palette;
        UINT static_type=style&SS_TYPEMASK;
        BOOL static_shape=(static_type>=SS_BLACKRECT && static_type<=SS_WHITEFRAME) ||
            (static_type>=SS_ETCHEDHORZ && static_type<=SS_ETCHEDFRAME);
        if(label_control && static_shape && view->frame && view->frame->present){
            palette.edge=view->frame->edge;
            palette.shadow=view->frame->shadow;
            palette.light=view->frame->light;
        }else if(label_control && view->label && view->label->present){
            palette.edge=view->label->normal;
            palette.shadow=view->label->disabled;
        }else if(!label_control && type==BS_GROUPBOX && view->group && view->group->text.present){
            palette.edge=style&WS_DISABLED?view->group->text.disabled:view->group->text.normal;
            palette.shadow=view->group->shadow;
        }
        if(view->push && !label_control && (type<=BS_DEFPUSHBUTTON || (style&BS_PUSHLIKE))){
            BOOL pressed=(state&BST_PUSHED) || ((style&BS_PUSHLIKE) && check!=BST_UNCHECKED);
            classic_surface_colors colors=classic_push_color(view->push,(style&WS_DISABLED)!=0,pressed,
                type==BS_DEFPUSHBUTTON);
            palette.face=colors.face;
            palette.edge=colors.edge;
        }
        win16_control_other(w,view->instance,&palette,dc,r,font,text,label_control,view->button_type,view->group_push,view->controls);
        RestoreDC(dc,saved);style_control_button_snapshot_clear(&snapshot);return;
    }
    height=MulDiv(13,(int)dpi,96);glyph=r;glyph.top+=(r.bottom-height)/2;
    glyph.bottom=glyph.top+height;glyph.right=glyph.left+MulDiv(14,(int)dpi,96);
    if(style&BS_LEFTTEXT)OffsetRect(&glyph,r.right-glyph.right,0);
    row=radio?1:check==BST_INDETERMINATE?2:0;
    column=(check!=BST_UNCHECKED?1:0)+((state&BST_PUSHED)?2:0);
    win16_style_palette tile=*view->palette;
    const classic_choice_surface *entry=classic_choice_entry(view->choice,radio,check,
        (style&WS_DISABLED)!=0,state);
    const classic_surface_colors *colors=classic_choice_color(entry);
    if(colors){
        tile.light=colors->face;
        tile.edge=colors->edge;
    }
    const char *image=entry?classic_choice_art_get(view->module,entry->image,view->art):NULL;
    if(view->instance->detached){RestoreDC(dc,saved);style_control_button_snapshot_clear(&snapshot);return;}
    win16_control_choice_glyph(dc,glyph,dpi,row,column,&tile,image);
    label=r;if(style&BS_LEFTTEXT)label.right=glyph.left-MulDiv(4,(int)dpi,96);
    else label.left=glyph.right+MulDiv(4,(int)dpi,96);
    SelectObject(dc,font);GetTextMetricsW(dc,&tm);SetBkMode(dc,TRANSPARENT);SetTextColor(dc,view->palette->edge);
    label.top+=(r.bottom-tm.tmHeight)/2;label.bottom=label.top+tm.tmHeight;
    if(IsWindowEnabled(w))DrawTextW(dc,text,-1,&label,DT_LEFT|DT_SINGLELINE);
    else {HBRUSH brush=CreateSolidBrush(view->palette->edge);
        if(brush){DrawStateW(dc,brush,NULL,(LPARAM)text,0,label.left,label.top,
            max(0,label.right-label.left),tm.tmHeight,DST_PREFIXTEXT|DSS_UNION|DSS_MONO);DeleteObject(brush);}}
    if(state&BST_FOCUS){focus=label;DrawTextW(dc,text,-1,&focus,DT_CALCRECT|DT_SINGLELINE);
        focus.right=min(focus.right,label.right);InflateRect(&focus,1,1);DrawFocusRect(dc,&focus);}
    RestoreDC(dc,saved);style_control_button_snapshot_clear(&snapshot);
}
