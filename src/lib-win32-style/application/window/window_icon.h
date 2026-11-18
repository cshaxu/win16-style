#ifndef STYLE_WINDOW_ICON_H
#define STYLE_WINDOW_ICON_H
#include <windows.h>

/* Borrowed icon. Resolving it can reenter the owner through WM_GETICON. */
static inline HICON style_window_caption_icon(HWND window)
{
    HICON icon=(HICON)SendMessageW(window,WM_GETICON,ICON_SMALL2,0);
    if(!icon)icon=(HICON)SendMessageW(window,WM_GETICON,ICON_SMALL,0);
    if(!icon)icon=(HICON)SendMessageW(window,WM_GETICON,ICON_BIG,0);
    if(!icon)icon=(HICON)GetClassLongPtrW(window,GCLP_HICONSM);
    if(!icon)icon=(HICON)GetClassLongPtrW(window,GCLP_HICON);
    return icon?icon:LoadIconW(NULL,IDI_WINLOGO);
}
#endif
