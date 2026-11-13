#include "control_lifetime.h"
#include "lib-style/common/control_vcl_spin.h"
#include "lib-style/common/control_scope.h"
#include "../win16-common/font.h"
#include <commctrl.h>

void win16_control_finish_message(HWND w,control_style *s,UINT m,LPARAM b,HWND spin_button,const RECT *spin_before,BOOL combo_paint,HFONT previous,HFONT outbound,HFONT pinned,SUBCLASSPROC callback)
{
    style_control_vcl_spin_after(w,spin_button,spin_before,&s->detached);
    if(m==WM_SETFONT && s->detached && IsWindow(w)){
        /* A native setter can finish AFTER its callback removed this leaf,
         * overwriting the font restored during removal with our retired
         * variant. Settle only after that native call has returned. A new
         * independent owner, if installed reentrantly, keeps its own source. */
        DWORD_PTR successor;HFONT source=s->original_font;
        if(GetWindowSubclass(w,callback,(UINT_PTR)callback,&successor))
            source=((control_style *)successor)->original_font;
        SendMessageW(w,WM_SETFONT,(WPARAM)source,b);
    }
    if(combo_paint)--s->combo_paints;
    /* Native processing (including nested paint/remove/setfont) has finished
     * with the previous lease. Never delete the application's borrowed font. */
    win16_style_system_release(previous);
    win16_style_system_release(outbound);win16_style_system_release(pinned);
    if(!--s->calls && s->detached)win16_control_state_dispose(s);
}

BOOL win16_control_updown_acquire(void *owner,SUBCLASSPROC proc)
{
    control_style *s=owner;
    if(s->detached || !style_control_subclass_owned(s->updown.window,proc,(UINT_PTR)proc,s))return FALSE;
    ++s->calls;return TRUE;
}
void win16_control_updown_release(void *owner)
{control_style *s=owner;if(!--s->calls && s->detached)win16_control_state_dispose(s);}
void win16_control_scrollbars_paint(HWND w,HDC target,const win16_style_palette *palette,SUBCLASSPROC proc,
    void (*paint)(HWND,control_style *,HDC))
{
    control_style s={0};DWORD_PTR attached;
    if(palette)s.palette=*palette;else win16_style_default_palette(&s.palette);
    if(GetWindowSubclass(w,proc,(UINT_PTR)proc,&attached)){
        control_style *owner=(control_style *)attached;
        if(!palette)s.palette=owner->palette;
        s.list_scrollbar=owner->list_scrollbar;s.edit_scrollbars=owner->edit_scrollbars;s.scroll=owner->scroll;
        s.frame_scrollbars=owner->frame_scrollbars;
        s.embedded_object=owner->embedded_object;
    }
    paint(w,&s,target);
}
