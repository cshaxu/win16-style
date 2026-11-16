#include "control_dispatch.h"
#include "control_guard.h"
#include "special_paint.h"
#include "lib-win32-style/application/control/control_class.h"
#include "../../../application/control/control_protocol.h"
#include <commctrl.h>

typedef struct control_render {
    control_style *state;
    const win16_control_dispatch_services *services;
} control_render;
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
    else if(tab)result=win16_control_tab_message(w,s,m,a,b,v->present,v->tab);
    else if(header)result=win16_control_header_message(w,s,m,a,b,v->present,v->header);
    else if(progress)result=win16_control_progress_message(w,s,m,a,b,v->present,v->progress);
    else if(v->class_is(w,UPDOWN_CLASSW)){result=DefSubclassProc(w,m,a,b);}
    else if(v->class_is(w,TRACKBAR_CLASSW) || style_control_protocol_is(w,TRACKBAR_CLASSW)){
        if(style_control_protocol_runtime_trackbar(w) && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT))
            result=v->track_dispatch(s,w,m,a,b);
        else result=DefSubclassProc(w,m,a,b);
        if(!s->instance.detached && m==WM_STYLECHANGED)SendMessageW(w,WM_SYSCOLORCHANGE,0,0);
        if(!s->instance.detached && s->track_parent && GetParent(w)!=s->track_parent){
            v->track_unbind(s);SendMessageW(w,WM_SYSCOLORCHANGE,0,0);
        }
    }
    else if(v->is_link(w)){
        /* Preserve native markup, hit testing, focus and NM_CLICK/NM_RETURN.
         * Only the shared WM_SETFONT path above changes link typography. */
        result=DefSubclassProc(w,m,a,b);
    }
    else if(s->scroll.resources.frame){
        control_render render={s,v};const style_scroll_frame_ops frame={&render,TRUE,frame_border,frame_overlay};
        result=style_scroll_frame_message(w,&s->instance,&s->state,&s->scroll,m,a,b,v->scroll,&frame);
    }
    else if(v->is_scroll(w)){
        if(GetWindowLongPtrW(w,GWL_STYLE)&SBS_SIZEBOX){
            control_render render={s,v};const style_control_draw draw={&render,sizebox_draw};
            result=style_scroll_sizebox_message(w,m,a,b,&draw);
        }else result=style_scroll_control_message(w,&s->instance,&s->scroll,&m,&a,&b,v->scroll);
    }
    else if(s->file_owner){
        const win16_control_fileowner_services *services=v->fileowner;
        result=win16_control_fileowner_message(w,s,m,a,b,services);
    }
    else if(v->is_combo(w)){
        control_render render={s,v};const style_control_draw draw={&render,combo_draw};
        result=style_control_combo_message(w,&s->instance,&s->state,m,a,b,&draw);
    }
    else if(v->is_list(w)){
        control_render render={s,v};
        const style_control_list_ops operations={&render,TRUE,list_input,list_draw,list_border,list_scrollbars};
        result=style_control_list_message(w,&s->instance,&s->state,m,a,b,&operations);
    }
    else if(v->is_edit(w)){
        control_render render={s,v};
        const style_control_edit_ops operations={&render,TRUE,edit_input,edit_scrollbars,edit_border,edit_client_border};
        result=style_control_edit_message(w,&s->instance,&s->state,&s->scroll.resources,m,a,b,&operations);
    }
    else {control_render render={s,v};const style_control_draw draw={&render,draw_basic};
        result=style_control_basic_message(w,&s->instance,&s->state,m,a,b,win16_control_basic_kind(w),&draw);}
    *message=m;*wparam=a;*lparam=b;return result;
}
