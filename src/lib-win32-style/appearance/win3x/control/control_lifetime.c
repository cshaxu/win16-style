#include "control_lifetime.h"
#include "lib-win32-style/application/control/control_vcl_spin.h"
#include "lib-win32-style/application/control/control_scope.h"
#include "../common/font.h"
#include <commctrl.h>

void win16_control_finish_message(HWND w,control_style *s,UINT m,LPARAM b,HWND spin_button,const RECT *spin_before,BOOL combo_paint,HFONT previous,HFONT outbound,HFONT pinned,SUBCLASSPROC callback)
{
    style_control_vcl_spin_after(w,spin_button,spin_before,&s->instance.detached);
    HFONT source=s->state.original_font;
    if(m==WM_SETFONT && s->instance.detached && IsWindow(w)){
        DWORD_PTR successor;
        if(GetWindowSubclass(w,callback,(UINT_PTR)callback,&successor))source=((control_style *)successor)->state.original_font;
    }
    style_control_font_finish(w,&s->instance,&s->state,m,b,combo_paint,source,previous,outbound,pinned);
    style_instance_release(&s->instance);
}

BOOL win16_control_updown_acquire(void *owner,SUBCLASSPROC proc)
{
    control_style *s=owner;
    if(s->instance.detached || !style_control_subclass_owned(s->updown.window,proc,(UINT_PTR)proc,s))return FALSE;
    style_instance_retain(&s->instance);return TRUE;
}
void win16_control_updown_release(void *owner)
{control_style *s=owner;style_instance_release(&s->instance);}
void win16_control_scrollbars_paint(HWND w,HDC target,const win16_style_palette *palette,SUBCLASSPROC proc,
    void (*paint)(HWND,control_style *,HDC))
{
    control_style s={0};DWORD_PTR attached;
    if(palette)s.palette=*palette;else win16_style_default_palette(&s.palette);
    if(GetWindowSubclass(w,proc,(UINT_PTR)proc,&attached)){
        control_style *owner=(control_style *)attached;
        if(!palette)s.palette=owner->palette;
        s.scroll.resources.list=owner->scroll.resources.list;s.scroll.resources.edit=owner->scroll.resources.edit;s.scroll.track=owner->scroll.track;
        s.scroll.resources.frame=owner->scroll.resources.frame;
        s.scroll.object=owner->scroll.object;
    }
    paint(w,&s,target);
}
