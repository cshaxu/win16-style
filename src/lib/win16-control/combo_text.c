#include "combo_text.h"
#include "font.h"

/* Keep native-painter borrowed font returns alive through reentrant setters.
 * Finite cache follows the existing 32-lease VB text scope limit. */
static HFONT combo_edit_font(control_style *s,HFONT source)
{
    HFONT font=win16_style_vb_font_acquire(source);if(!font)return source;
    for(unsigned i=0;i<s->combo_font_count;++i)if(s->combo_fonts[i]==font){win16_style_system_release(font);return font;}
    if(s->combo_font_count==ARRAYSIZE(s->combo_fonts)){win16_style_system_release(font);return source;}
    s->combo_fonts[s->combo_font_count++]=font;return font;
}
void win16_control_combo_text_unbind(control_style *,BOOL,SUBCLASSPROC);
LRESULT win16_control_combo_text_message(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,control_style *s,SUBCLASSPROC combo_proc,SUBCLASSPROC control_proc)
{
    ++s->calls;LRESULT result;
    HFONT pinned=s->combo_edit_font,outbound=NULL;win16_style_system_retain(pinned);
    BOOL related=!s->detached && s->combo_edit==w && GetParent(w)==s->combo_parent;
    BOOL painting=related && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT);if(painting)++s->combo_paints;
    if(m==WM_NCDESTROY){RemoveWindowSubclass(w,combo_proc,id);if(s->combo_edit==w){s->combo_edit=NULL;if(s->combo_bound){s->combo_bound=FALSE;--s->calls;}}}
    if(related && m==WM_GETFONT)result=(LRESULT)(s->combo_paints && s->combo_edit_font?s->combo_edit_font:s->combo_edit_source);
    else {
        if(related && m==WM_SETFONT){
            HFONT source=win16_style_system_source((HFONT)a),font=combo_edit_font(s,source);
            s->combo_edit_source=source;s->combo_edit_font=font;
            outbound=s->combo_edit_font;win16_style_system_retain(outbound);a=(WPARAM)outbound;
        }
        result=DefSubclassProc(w,m,a,b);
    }
    /* A native setter can reenter removal; restore only after it returns. */
    if(!related && m!=WM_NCDESTROY && !s->detached && s->combo_edit==w)win16_control_combo_text_unbind(s,FALSE,combo_proc);
    if(m==WM_SETFONT && s->detached && IsWindow(w)){
        DWORD_PTR successor;HFONT source=s->combo_edit_source;
        if(GetWindowSubclass(w,combo_proc,(UINT_PTR)combo_proc,&successor))source=((control_style *)successor)->combo_edit_source;
        else if(GetWindowSubclass(w,control_proc,(UINT_PTR)control_proc,&successor))source=((control_style *)successor)->original_font;
        SendMessageW(w,WM_SETFONT,(WPARAM)source,b);
    }
    if(painting)--s->combo_paints;
    win16_style_system_release(outbound);win16_style_system_release(pinned);
    if(!--s->calls && s->detached)win16_control_state_dispose(s);
    return result;
}
void win16_control_combo_text_unbind(control_style *s,BOOL terminal,SUBCLASSPROC combo_proc)
{
    HWND edit=s->combo_edit;DWORD_PTR data;
    if(!s->combo_bound)return;
    BOOL owned=edit && GetWindowSubclass(edit,combo_proc,(UINT_PTR)combo_proc,&data) && data==(DWORD_PTR)s;
    if(owned && !RemoveWindowSubclass(edit,combo_proc,(UINT_PTR)combo_proc))return;
    s->combo_edit=NULL;s->combo_bound=FALSE;--s->calls;
    if(owned && !terminal && IsWindow(edit)){
        SendMessageW(edit,WM_SETFONT,(WPARAM)s->combo_edit_source,FALSE);
        if(IsWindow(edit))InvalidateRect(edit,NULL,TRUE);
    }
}
