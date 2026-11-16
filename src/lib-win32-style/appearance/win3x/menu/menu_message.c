#include "menu_message.h"
#include <commctrl.h>

void win16_style_menu_system(win16_style_menu_scope *,HMENU,HFONT,UINT);

BOOL win16_control_menu_message(HWND w,style_window *s,UINT msg,WPARAM wp,LPARAM lp,win16_control_menu_button_painter paint,LRESULT *result)
{
    switch(msg){
    case WM_MEASUREITEM:
        if(!win16_style_menu_size(s->menu_items,(MEASUREITEMSTRUCT *)lp,s->menu_font,s->dpi))return FALSE;
        *result=TRUE;return TRUE;
    case WM_DRAWITEM: {
        win16_style_palette palette;win16_control_resolve_palette(&s->palette,&palette);
        if(!win16_style_menu_draw(s->menu_items,(DRAWITEMSTRUCT *)lp,s->menu_font,s->dpi,&palette))return FALSE;
        *result=TRUE;return TRUE;
    }
    case WM_MENUCHAR:
        /* Application-owned items retain their existing mnemonic handler. */
        *result=DefSubclassProc(w,msg,wp,lp);
        if(!s->detached && HIWORD(*result)==MNC_IGNORE)
            style_menu_char(s->menu_items,(HMENU)lp,(WCHAR)LOWORD(wp),result);
        return TRUE;
    case WM_INITMENUPOPUP:
        /* Restore application-owned data before the owner refreshes items. */
        style_menu_restore_popup((HMENU)wp,&s->menu_items);
        *result=DefSubclassProc(w,msg,wp,lp);
        if(!s->detached && !style_menu_prepare((HMENU)wp,&s->menu_items,s->menu_depth))
            style_menu_restore_popup((HMENU)wp,&s->menu_items);
        if(!s->detached && HIWORD(lp)){
            if(s->menus)win16_style_menu_system(s->menus->scope,(HMENU)wp,s->menu_font,s->dpi);
            s->system_menu=TRUE;paint(w,s,HTSYSMENU);
        }
        return TRUE;
    case WM_EXITMENULOOP:
        win16_control_frame_state_exit_menu(s);
        *result=DefSubclassProc(w,msg,wp,lp);
        if(!s->detached && s->system_menu){s->system_menu=FALSE;paint(w,s,HTSYSMENU);}
        return TRUE;
    default:return FALSE;
    }
}
