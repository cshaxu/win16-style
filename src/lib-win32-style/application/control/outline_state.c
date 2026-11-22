#include "lib-win32-style/application/control/control_redraw.h"
#include "outline_state.h"
#include "lib-win32-style/application/control/control_class.h"
#include <commctrl.h>
#include <stdlib.h>
void style_outline_control_updown_colors(style_outline_control *s)
{
    const style_window_palette *p=&s->palette;
    const style_frame_colors *f=&s->frame_colors;
    COLORREF light=f->present?f->light:p->light,edge=f->present?f->edge:p->edge;
    s->updown.palette=(style_relief_palette){.face=f->present?f->face:p->face,.light=light,.highlight=light,
        .shadow=f->present?f->shadow:p->shadow,.dark=edge,.text=edge};
}

style_outline_control *style_outline_control_state_create(const style_control_fonts *fonts)
{
    style_outline_control *s=calloc(1,sizeof(*s));if(!s)return NULL;
    style_instance_init(&s->instance,s,style_outline_control_state_dispose);style_control_state_init(&s->state,fonts);
    style_control_bindings_init(&s->bindings);return s;
}
void style_outline_control_state_dispose(void *context)
{
    style_outline_control *s=context;
    style_control_bindings_dispose(&s->bindings,&s->state);
    style_control_state_dispose(&s->state);
    if(s->choice_module)FreeLibrary(s->choice_module);
    free(s);
}
BOOL style_outline_control_state_prepare_font(HWND w,void *context,style_control_message *call)
{
    style_outline_control *s=context;UINT m=call->message;WPARAM *a=&call->parameter;BOOL native_font=call->native_font;
    style_control_font_mode mode=STYLE_CONTROL_FONT_NONE;
    if(!s->bindings.scroll.resources.frame && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT) &&
        (native_font || style_control_vb_frame(w) || style_control_vb_edit(w) || style_control_vb_button(w) || style_control_vb_combo(w) || style_control_vb_list(w)))mode=STYLE_CONTROL_FONT_OBSERVE;
    if(m==WM_SETFONT && s->combo_passthrough)
        mode=((DWORD)GetWindowLongPtrW(w,GWL_STYLE)&3)==CBS_DROPDOWNLIST?STYLE_CONTROL_FONT_CACHE:STYLE_CONTROL_FONT_NORMALIZE;
    if(m==WM_SETFONT && !s->bindings.scroll.resources.frame && !s->combo_passthrough)mode=STYLE_CONTROL_FONT_REPLACE;
    return style_control_font_message(w,&s->instance,&s->state,a,mode,native_font,
        mode==STYLE_CONTROL_FONT_REPLACE && (native_font || style_control_vb_checklist_owner(w)),&call->previous,&call->outbound);
}

typedef struct control_retire {
    style_outline_control *state;
    const style_outline_control_remove_services *services;
} control_retire;
static void finish_input(HWND window,void *context)
{control_retire *retire=context;retire->services->scroll_finish(window,retire->state,TRUE);}
static style_control_retired release_bindings(HWND w,void *context,BOOL terminal)
{
    control_retire *retire=context;style_outline_control *s=retire->state;
    const style_outline_control_remove_services *v=retire->services;
    const style_control_binding_ops ops={s,v->track_notify,v->toolbar_notify,v->lookup,v->remove_list,v->terminal_list,v->theme_remove,v->theme_terminal};
    return style_control_bindings_release(w,&s->instance,&s->state,&s->bindings,terminal,s->combo_passthrough,&ops);
}
static const style_control_retire_ops retire_operations={finish_input,release_bindings};
BOOL style_outline_control_remove(HWND w,const style_outline_control_remove_services *v)
{
    DWORD_PTR data;
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId() ||
        !GetWindowSubclass(w,v->control_proc,(UINT_PTR)v->control_proc,&data))return FALSE;
    style_outline_control *s=(style_outline_control *)data;control_retire retire={s,v};
    return style_control_retire(w,&s->instance,&s->state,FALSE,&retire,&retire_operations);
}
BOOL style_outline_control_terminal(HWND w,const style_outline_control_remove_services *v)
{
    DWORD_PTR data;if(!GetWindowSubclass(w,v->control_proc,(UINT_PTR)v->control_proc,&data))return TRUE;
    style_outline_control *s=(style_outline_control *)data;control_retire retire={s,v};
    return style_control_retire(w,&s->instance,&s->state,TRUE,&retire,&retire_operations);
}

static void set_scroll_palette(const style_scroll_binding *binding,const style_window_palette *palette)
{
    style_outline_control *s=binding->instance->context;
    s->palette=*palette;
    s->themed=FALSE;
    if(s->bindings.updown.window)style_outline_control_updown_colors(s);
}
void style_outline_control_managed_frame_scrollbars_palette_owned(HWND w,const style_window_palette *palette,const void *owner,const style_scroll_managed_ops *ops)
{
    style_scroll_binding binding=ops->lookup(w);
    if(binding.instance && binding.scroll->resources.frame && binding.instance->owner==owner)
        set_scroll_palette(&binding,palette);
}
void style_outline_control_managed_frame_scrollbars_palette(HWND w,const style_window_palette *palette,const style_scroll_managed_ops *ops)
{
    style_scroll_binding binding=ops->lookup(w);
    if(binding.instance && binding.scroll->resources.frame)set_scroll_palette(&binding,palette);
}

void style_outline_control_scrollbars_paint(HWND w,HDC target,const style_window_palette *palette,SUBCLASSPROC proc,
    void (*paint)(HWND,style_outline_control *,HDC))
{
    style_outline_control s={0};DWORD_PTR attached;
    if(palette)s.palette=*palette;else style_window_palette_default(&s.palette);
    if(GetWindowSubclass(w,proc,(UINT_PTR)proc,&attached)){
        style_outline_control *owner=(style_outline_control *)attached;
        s.scroll_art=owner->scroll_art;
        if(!palette){
            s.palette=owner->palette;
            s.themed=owner->themed;
            s.scroll_arrow_colors=owner->scroll_arrow_colors;
            s.scroll_colors=owner->scroll_colors;
        }
        s.bindings.scroll.resources.list=owner->bindings.scroll.resources.list;s.bindings.scroll.resources.edit=owner->bindings.scroll.resources.edit;s.bindings.scroll.track=owner->bindings.scroll.track;
        s.bindings.scroll.resources.frame=owner->bindings.scroll.resources.frame;
        s.bindings.scroll.object=owner->bindings.scroll.object;
    }
    paint(w,&s,target);
}
