#ifndef STYLE_CONTROL_CUSTOM_DRAW_H
#define STYLE_CONTROL_CUSTOM_DRAW_H
#include <windows.h>
#include <commctrl.h>
/* Only the current child may contribute a custom-draw notification. */
static inline NMCUSTOMDRAW *style_control_custom_draw_message(HWND parent,HWND child,UINT message,LPARAM parameter,BOOL detached)
{
    if(message!=WM_NOTIFY || !parameter || detached || GetParent(child)!=parent)return NULL;
    NMHDR *header=(NMHDR *)parameter;
    return header->hwndFrom==child && header->code==NM_CUSTOMDRAW?(NMCUSTOMDRAW *)parameter:NULL;
}
/* The application keeps its custom-draw result; request private stages only
 * after it selects the default route. State belongs to the calling window. */
LRESULT style_control_custom_draw_after(HWND window,const NMCUSTOMDRAW *draw,BOOL *active,BOOL internal,
    LRESULT result,const void *context,BOOL (*supported)(HWND),
    BOOL (*paint_part)(HWND,const NMCUSTOMDRAW *,const void *));
#endif
