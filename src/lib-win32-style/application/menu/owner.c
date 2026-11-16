#include "owner.h"
#include <commctrl.h>
#include <stdlib.h>

struct style_menu_loop {
    style_menu_popup_scope *scope;
    unsigned depth;
    style_menu_loop *previous;
};
static void pop(style_menu_owner *s)
{
    style_menu_loop *loop=s->loops;
    if(loop){s->loops=loop->previous;style_menu_popup_end(loop->scope);free(loop);}
}
style_menu_popup_scope *style_menu_owner_scope(const style_menu_owner *s)
{return s->loops?s->loops->scope:NULL;}
BOOL style_menu_owner_enter(style_menu_owner *s,HWND window,const style_menu_owner_ops *ops,void *context)
{
    if(s->detached){SetLastError(ERROR_INVALID_STATE);return FALSE;}
    ++s->depth;
    style_menu_loop *loop=calloc(1,sizeof(*loop));
    if(!loop){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    loop->scope=ops->begin(context,window);
    if(!loop->scope){free(loop);return FALSE;}
    if(s->detached){style_menu_popup_end(loop->scope);free(loop);return FALSE;}
    loop->depth=s->depth;loop->previous=s->loops;s->loops=loop;return TRUE;
}
void style_menu_owner_exit(style_menu_owner *s)
{
    style_menu_restore_depth(s->depth,&s->items);
    if(s->loops && s->loops->depth==s->depth)pop(s);
    if(s->depth)--s->depth;
}
void style_menu_owner_stop(style_menu_owner *s)
{
    s->detached=TRUE;
    while(s->loops)pop(s);
    style_menu_restore(&s->items);s->depth=0;s->system=FALSE;
}
static void system_menu(style_menu_owner *s,HWND window,HMENU menu,
    const style_menu_owner_ops *ops,void *context)
{
    if(ops->system)ops->system(context,style_menu_owner_scope(s),menu);
    if(s->detached)return;
    s->system=TRUE;
    if(ops->selection)ops->selection(context,window,TRUE);
}
BOOL style_menu_owner_message(style_menu_owner *s,HWND w,UINT msg,WPARAM wp,LPARAM lp,
    const style_menu_owner_ops *ops,void *context,LRESULT *result)
{
    if(s->detached)return FALSE;
    switch(msg){
    case WM_ENTERMENULOOP:
        (void)style_menu_owner_enter(s,w,ops,context);return FALSE;
    case WM_ENTERIDLE:
        if(!ops->adapt_items && wp==MSGF_MENU)style_menu_popup_idle(style_menu_owner_scope(s));
        return FALSE;
    case WM_MEASUREITEM:
        if(!ops->measure || !ops->measure(context,s->items,(MEASUREITEMSTRUCT *)lp))return FALSE;
        *result=TRUE;return TRUE;
    case WM_DRAWITEM:
        if(!ops->draw || !ops->draw(context,s->items,(DRAWITEMSTRUCT *)lp))return FALSE;
        *result=TRUE;return TRUE;
    case WM_MENUCHAR:
        if(!ops->adapt_items)return FALSE;
        *result=DefSubclassProc(w,msg,wp,lp);
        if(!s->detached && HIWORD(*result)==MNC_IGNORE)style_menu_char(s->items,(HMENU)lp,(WCHAR)LOWORD(wp),result);
        return TRUE;
    case WM_INITMENUPOPUP:
        if(!ops->adapt_items){if(HIWORD(lp))system_menu(s,w,(HMENU)wp,ops,context);return FALSE;}
        style_menu_restore_popup((HMENU)wp,&s->items);
        *result=DefSubclassProc(w,msg,wp,lp);
        if(!s->detached && !style_menu_prepare((HMENU)wp,&s->items,s->depth))style_menu_restore_popup((HMENU)wp,&s->items);
        if(!s->detached && HIWORD(lp))system_menu(s,w,(HMENU)wp,ops,context);
        return TRUE;
    case WM_EXITMENULOOP:
        style_menu_owner_exit(s);
        if(ops->adapt_items)*result=DefSubclassProc(w,msg,wp,lp);
        if(!s->detached && s->system){s->system=FALSE;if(ops->selection)ops->selection(context,w,FALSE);}
        return ops->adapt_items;
    default:return FALSE;
    }
}
