#include "control_dispatch.h"
#include "control_guard.h"
#include "control_class.h"
#include "special_paint.h"
#include "tab_paint.h"
#include "header_paint.h"
#include "progress_paint.h"
#include "lib-win32-style/application/control/control_class.h"
#include "../../../application/control/control_protocol.h"
#include <commctrl.h>

typedef struct control_render {
    control_style *state;
    const win16_control_dispatch_services *services;
    win16_control_state_painter painter;
} control_render;
static BOOL owner_input(HWND w,void *context,UINT m,WPARAM a,LPARAM b)
{const control_render *r=context;return ntw16s_vb_owner_file(w) && style_control_vb_directory(w) && r->services->fileowner->scroll_input(w,r->state,m,a,b);}
static style_control_owner_action owner_select(HWND w,void *context,UINT m,LPARAM b)
{
    const control_render *r=context;control_style *s=r->state;
    if(!(ntw16s_vb_owner_file(w) || (s->common_dialog_combo && style_control_dialog_combo_valid(w,s->common_dialog_parent,style_control_is_combo))))return STYLE_CONTROL_OWNER_NONE;
    if(m==WM_PAINT)return style_control_vb_drive(w) || s->common_dialog_combo?STYLE_CONTROL_OWNER_PAINT:STYLE_CONTROL_OWNER_FRAME;
    if(m==WM_PRINTCLIENT && (style_control_vb_drive(w) || s->common_dialog_combo))return STYLE_CONTROL_OWNER_PRINT;
    if((m==WM_NCPAINT || (m==WM_PRINT && (b&PRF_NONCLIENT))) && style_control_vb_directory(w))return STYLE_CONTROL_OWNER_FRAME;
    return STYLE_CONTROL_OWNER_NONE;
}
static void owner_draw(HWND w,void *context,HDC dc,style_control_owner_action action)
{
    const control_render *r=context;const win16_control_fileowner_services *ops=r->services->fileowner;
    if(action==STYLE_CONTROL_OWNER_FRAME){ops->border(w,r->state,dc);ops->scrollbars(w,r->state,dc);}
    else ops->combo(w,r->state,dc);
}
static void native_draw(HWND window,void *context,HDC dc,BOOL buffered)
{
    const control_render *render=context;
    if(buffered)render->services->present(window,render->state,dc,render->painter);
    else render->painter(window,render->state,dc);
}
static void draw_basic(HWND window,void *context,HDC dc,BOOL buffered)
{
    const control_render *render=context;
    if(buffered)render->services->present(window,render->state,dc,render->services->basic);
    else render->services->basic(window,render->state,dc);
}
static BOOL list_input(HWND window,void *context,UINT message,WPARAM parameter,LPARAM value)
{const control_render *render=context;return render->services->list->scroll_input(window,render->state,message,parameter,value);}
static void list_draw(HWND window,void *context,HDC dc,BOOL buffered)
{
    const control_render *render=context;const win16_control_list_services *list=render->services->list;
    if(buffered)list->present(window,render->state,dc,list->paint);
    else list->paint(window,render->state,dc);
}
static void list_border(HWND window,void *context,HDC dc)
{const control_render *render=context;render->services->list->border(window,render->state,dc);}
static void list_scrollbars(HWND window,void *context,HDC dc)
{const control_render *render=context;render->services->list->scrollbars(window,render->state,dc);}
static void combo_draw(HWND window,void *context,HDC dc,BOOL buffered)
{
    const control_render *render=context;
    if(buffered)render->services->present(window,render->state,dc,render->services->combo);
    else render->services->combo(window,render->state,dc);
}
static BOOL edit_input(HWND window,void *context,UINT message,WPARAM parameter,LPARAM value)
{const control_render *render=context;return render->services->edit->scroll_input(window,render->state,message,parameter,value);}
static void edit_scrollbars(HWND window,void *context,HDC dc)
{const control_render *render=context;render->services->edit->scrollbars(window,render->state,dc);}
static void edit_border(HWND window,void *context,HDC dc)
{
    const control_render *render=context;
    if(style_control_vb_edit(window) || style_control_native_superclass(window,L"EDIT") ||
        (GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_CLIENTEDGE))
        render->services->edit->border(window,render->state,dc);
}
static void edit_client_border(HWND window,void *context,HDC dc)
{const control_render *render=context;render->services->edit->client_border(window,render->state,dc);}
static void frame_border(HWND window,void *context,HDC dc)
{const control_render *render=context;render->services->frame_border(window,render->state,dc);}
static void frame_overlay(HWND window,void *context,HDC dc)
{
    const control_render *render=context;
    if(render->state->legacy_progress_border)win16_control_legacy_progress_border(window,&render->state->palette,dc);
}
static void sizebox_draw(HWND window,void *context,HDC dc,BOOL buffered)
{
    const control_render *render=context;
    if(buffered)render->services->present(window,render->state,dc,render->services->sizebox);
    else render->services->sizebox(window,render->state,dc);
}
LRESULT win16_control_dispatch_message(HWND w,control_style *s,UINT *message,WPARAM *wparam,LPARAM *lparam,BOOL tab,BOOL header,BOOL progress,const win16_control_dispatch_services *v)
{
    UINT m=*message;WPARAM a=*wparam;LPARAM b=*lparam;LRESULT result;
    if(m==WM_NCDESTROY){(void)v->terminal(w);
        result=DefSubclassProc(w,m,a,b);}
    else if(tab || header || progress){
        style_control_native_kind kind=tab?STYLE_CONTROL_TAB:header?STYLE_CONTROL_HEADER:STYLE_CONTROL_PROGRESS;
        style_control_native_state *state=tab?&s->bindings.tab:header?&s->bindings.header:&s->bindings.progress;
        control_render render={s,v,tab?v->tab:header?v->header:v->progress};
        const style_control_draw draw={&render,native_draw};
        result=style_control_native_message(w,&s->instance,state,kind,m,a,b,
            tab?win16_style_tab_supported:header?win16_style_header_supported:win16_style_progress_supported,&draw);
    }
    else if(style_control_class_is(w,UPDOWN_CLASSW)){result=DefSubclassProc(w,m,a,b);}
    else if(style_control_class_is(w,TRACKBAR_CLASSW) || style_control_protocol_is(w,TRACKBAR_CLASSW)){
        result=style_control_track_message(w,&s->instance,&s->bindings.track,m,a,b,v->track_notify,s,v->track_calls);
    }
    else if(style_control_is_link(w)){
        /* Preserve native markup, hit testing, focus and NM_CLICK/NM_RETURN.
         * Only the shared WM_SETFONT path above changes link typography. */
        result=DefSubclassProc(w,m,a,b);
    }
    else if(s->bindings.scroll.resources.frame){
        control_render render={s,v,NULL};const style_scroll_frame_ops frame={&render,TRUE,frame_border,frame_overlay};
        result=style_scroll_frame_message(w,&s->instance,&s->state,&s->bindings.scroll,m,a,b,v->scroll,&frame);
    }
    else if(style_control_is_scroll(w)){
        if(GetWindowLongPtrW(w,GWL_STYLE)&SBS_SIZEBOX){
            control_render render={s,v,NULL};const style_control_draw draw={&render,sizebox_draw};
            result=style_scroll_sizebox_message(w,m,a,b,&draw);
        }else result=style_scroll_control_message(w,&s->instance,&s->bindings.scroll,&m,&a,&b,v->scroll);
    }
    else if(s->file_owner){
        control_render render={s,v,NULL};const style_control_owner_ops ops={&render,owner_input,owner_select,owner_draw};
        result=style_control_owner_message(w,&s->instance,m,a,b,&ops);
    }
    else if(style_control_is_combo(w)){
        control_render render={s,v,NULL};const style_control_draw draw={&render,combo_draw};
        result=style_control_combo_message(w,&s->instance,&s->state,m,a,b,&draw);
    }
    else if(style_control_list_class(w)){
        control_render render={s,v,NULL};
        const style_control_list_ops operations={&render,TRUE,list_input,list_draw,list_border,list_scrollbars};
        result=style_control_list_message(w,&s->instance,&s->state,m,a,b,&operations);
    }
    else if(style_control_is_edit(w)){
        control_render render={s,v,NULL};
        const style_control_edit_ops operations={&render,TRUE,edit_input,edit_scrollbars,edit_border,edit_client_border};
        result=style_control_edit_message(w,&s->instance,&s->state,&s->bindings.scroll.resources,m,a,b,&operations);
    }
    else {control_render render={s,v,NULL};const style_control_draw draw={&render,draw_basic};
        result=style_control_basic_message(w,&s->instance,&s->state,m,a,b,win16_control_basic_kind(w),&draw);}
    *message=m;*wparam=a;*lparam=b;return result;
}
