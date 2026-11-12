#ifndef STYLE_COMMON_MENU_INPUT_H
#define STYLE_COMMON_MENU_INPUT_H
#include <windows.h>

typedef enum style_menu_key_action {
    STYLE_MENU_KEY_NONE, STYLE_MENU_KEY_PREVIOUS, STYLE_MENU_KEY_NEXT,
    STYLE_MENU_KEY_LEAVE, STYLE_MENU_KEY_OPEN
} style_menu_key_action;

static inline style_menu_key_action style_menu_key(WPARAM key)
{
    switch(key){
    case VK_LEFT:return STYLE_MENU_KEY_PREVIOUS;
    case VK_RIGHT:return STYLE_MENU_KEY_NEXT;
    case VK_ESCAPE:case VK_F10:return STYLE_MENU_KEY_LEAVE;
    case VK_DOWN:case VK_RETURN:return STYLE_MENU_KEY_OPEN;
    default:return STYLE_MENU_KEY_NONE;
    }
}

/* Classic menu labels escape a literal ampersand as &&. */
static inline BOOL style_menu_mnemonic(const WCHAR *text,WCHAR key,BOOL stop_at_tab)
{
    CharUpperBuffW(&key,1);
    for(;*text && (!stop_at_tab || *text!=L'\t');++text)if(*text==L'&' && text[1]){
        WCHAR letter=*++text;if(letter==L'&')continue;CharUpperBuffW(&letter,1);
        if(letter==key)return TRUE;
    }
    return FALSE;
}

/* Returns whether tracking should end and which item should open next. */
static inline BOOL style_menu_pointer(UINT message,int hit,int selected,int no_hit,
    int cancel,BOOL armed,int *target)
{
    BOOL down=message==WM_LBUTTONDOWN || message==WM_NCLBUTTONDOWN;
    BOOL move=message==WM_MOUSEMOVE || message==WM_NCMOUSEMOVE;
    if((!down && !move) || hit==no_hit || (hit==selected && (!down || !armed)))return FALSE;
    *target=hit==selected?cancel:hit;return TRUE;
}

/* USER hot-tracks HTMENU before dispatch; custom menu bars own that input. */
static inline LRESULT style_menu_neutral_hit(LRESULT hit)
{
    return hit==HTMENU?HTBORDER:hit;
}

static inline WPARAM style_menu_route_click(UINT message,WPARAM hit,BOOL inside)
{
    return inside && hit==HTBORDER &&
        (message==WM_NCLBUTTONDOWN || message==WM_NCLBUTTONDBLCLK)?HTMENU:hit;
}
#endif
