#include "control_redraw.h"
#include "combo.h"
#include "control_scope.h"

static LRESULT CALLBACK edit_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static HFONT edit_font(style_control_combo *combo,HFONT source)
{
    HFONT font=combo->ops->acquire(source,combo->state->font_recipe);if(!font)return source;
    for(unsigned i=0;i<combo->font_count;++i)if(combo->fonts[i]==font){combo->state->fonts->release(font);return font;}
    if(combo->font_count==ARRAYSIZE(combo->fonts)){combo->state->fonts->release(font);return source;}
    combo->fonts[combo->font_count++]=font;return font;
}
void style_control_combo_dispose(style_control_combo *combo,const style_control_state *state)
{
    for(unsigned i=0;i<combo->font_count;++i)state->fonts->release(combo->fonts[i]);
    *combo=(style_control_combo){0};
}
BOOL style_control_combo_bind(style_control_combo *combo,style_instance *instance,style_control_state *state,
    HWND parent,HWND edit,const style_control_combo_ops *ops)
{
    HFONT source=(HFONT)SendMessageW(edit,WM_GETFONT,0,0);DWORD_PTR existing;
    if(instance->detached){SetLastError(ERROR_CANCELLED);return FALSE;}
    combo->instance=instance;combo->state=state;combo->ops=ops;
    combo->parent=parent;combo->edit_source=state->fonts->source(source);
    if(GetWindowSubclass(edit,edit_proc,(UINT_PTR)edit_proc,&existing)){SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
    if(!SetWindowSubclass(edit,edit_proc,(UINT_PTR)edit_proc,(DWORD_PTR)combo))return FALSE;
    combo->edit=edit;combo->bound=TRUE;style_instance_retain(instance);
    SendMessageW(edit,WM_SETFONT,(WPARAM)combo->edit_source,FALSE);
    return TRUE;
}
void style_control_combo_unbind(style_control_combo *combo,BOOL terminal)
{
    HWND edit=combo->edit;HFONT source=combo->edit_source;style_instance *instance=combo->instance;
    if(!combo->bound)return;
    BOOL owned=style_control_subclass_owned(edit,edit_proc,(UINT_PTR)edit_proc,combo);
    if(owned && !RemoveWindowSubclass(edit,edit_proc,(UINT_PTR)edit_proc))return;
    combo->edit=NULL;combo->bound=FALSE;style_instance_release(instance);
    if(owned && !terminal && IsWindow(edit)){
        SendMessageW(edit,WM_SETFONT,(WPARAM)source,FALSE);
        if(IsWindow(edit))InvalidateRect(edit,NULL,TRUE);
    }
}
static LRESULT CALLBACK edit_proc(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{
    style_control_combo *combo=(style_control_combo *)data;style_instance *instance=combo->instance;
    style_control_state *state=combo->state;const style_control_fonts *fonts=state->fonts;
    style_instance_retain(instance);LRESULT result;
    HFONT pinned=combo->edit_font,outbound=NULL;fonts->retain(pinned);
    BOOL related=!instance->detached && combo->edit==window && GetParent(window)==combo->parent;
    BOOL painting=related && (message==WM_PAINT || message==WM_PRINT || message==WM_PRINTCLIENT);
    if(painting)++state->native_paints;
    if(message==WM_NCDESTROY){
        RemoveWindowSubclass(window,edit_proc,id);
        if(combo->edit==window){combo->edit=NULL;if(combo->bound){combo->bound=FALSE;style_instance_release(instance);}}
    }
    if(related && message==WM_GETFONT)result=(LRESULT)(state->native_paints && combo->edit_font?combo->edit_font:combo->edit_source);
    else {
        if(related && message==WM_SETFONT){
            HFONT source=fonts->source((HFONT)wp),font=edit_font(combo,source);
            combo->edit_source=source;combo->edit_font=font;
            outbound=font;fonts->retain(outbound);wp=(WPARAM)outbound;
        }
        result=DefSubclassProc(window,message,wp,lp);
    }
    if(!related && message!=WM_NCDESTROY && !instance->detached && combo->edit==window)style_control_combo_unbind(combo,FALSE);
    if(message==WM_SETFONT && instance->detached && IsWindow(window)){
        DWORD_PTR successor;HFONT source=combo->edit_source;
        if(GetWindowSubclass(window,edit_proc,(UINT_PTR)edit_proc,&successor))source=((style_control_combo *)successor)->edit_source;
        else {style_control_state *current=combo->ops->lookup(window);if(current)source=current->original_font;}
        SendMessageW(window,WM_SETFONT,(WPARAM)source,lp);
    }
    if(painting)--state->native_paints;
    fonts->release(outbound);fonts->release(pinned);style_instance_release(instance);return result;
}
void style_control_combo_list_bind(style_control_combo *combo,style_instance *instance,HWND list,style_control_state *(*lookup)(HWND))
{
    combo->list=list;style_control_state *child=lookup(list);if(child)child->owner=instance;
}
BOOL style_control_combo_list_retire(style_control_combo *combo,style_instance *instance,style_control_state *(*lookup)(HWND),BOOL (*retire)(HWND))
{
    HWND list=combo->list;combo->list=NULL;style_control_state *child=list?lookup(list):NULL;
    return child && child->owner==instance?retire(list):TRUE;
}

BOOL style_control_combo_setup(HWND window,style_instance *instance,style_control_combo *combo,const style_control_combo_ops *bindings,const style_control_combo_setup_ops *ops)
{
    COMBOBOXINFO info={.cbSize=sizeof(info)};BOOL result=TRUE;
    if(!ops->get_info(window,&info) || !ops->install_list(info.hwndList,ops->context)){
        if(!instance->detached)(void)ops->remove(window);
        result=FALSE;
    }else if(instance->detached){
        (void)ops->remove(info.hwndList);
    }else{
        style_control_combo_list_bind(combo,instance,info.hwndList,bindings->lookup);
        if(ops->edit && !ops->edit(window,info.hwndItem,ops->context)){
            DWORD error=GetLastError();if(!instance->detached)(void)ops->remove(window);
            SetLastError(error);result=FALSE;
        }
    }
    if(instance->detached){SetLastError(ERROR_CANCELLED);return FALSE;}
    return result;
}

LRESULT style_control_combo_message(HWND w,style_instance *instance,style_control_state *state,UINT m,WPARAM a,LPARAM b,const style_control_draw *draw)
{
    LRESULT result=0;
    if(m==WM_SETREDRAW){state->redraw_off=!a;result=DefSubclassProc(w,m,a,b);}
    else if(m==WM_PAINT){PAINTSTRUCT ps;HDC dc=BeginPaint(w,&ps);draw->render(w,draw->context,dc,TRUE);EndPaint(w,&ps);}
    else if(m==WM_PRINTCLIENT)draw->render(w,draw->context,(HDC)a,FALSE);
    else if(m==WM_ERASEBKGND)result=1;
    else {BOOL affects=m==CB_SETCURSEL || m==CB_SHOWDROPDOWN || m==WM_COMMAND || m==WM_ENABLE ||
            m==WM_SETFONT || m==WM_SETFOCUS || m==WM_KILLFOCUS || m==WM_KEYDOWN ||
            m==WM_LBUTTONDOWN || m==WM_LBUTTONUP;
        BOOL suppress=style_control_redraw_can_suppress(w,state->redraw_off,state->suppression) && affects;
        result=style_control_redraw_call(w,m,a,b,suppress,&state->suppression,&instance->detached,&state->redraw_off);
        if(!instance->detached && !state->suppression && !state->redraw_off && affects)
            RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);}
    return result;
}
