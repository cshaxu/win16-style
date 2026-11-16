#ifndef WIN16_CONTROL_LIFETIME_H
#define WIN16_CONTROL_LIFETIME_H
#include "control_state.h"
#include <commctrl.h>
void win16_control_finish_message(HWND,control_style *,UINT,LPARAM,HWND,const RECT *,BOOL,HFONT,HFONT,HFONT,SUBCLASSPROC);
BOOL win16_control_updown_acquire(void *,SUBCLASSPROC);
void win16_control_updown_release(void *);
void win16_control_scrollbars_paint(HWND,HDC,const win16_style_palette *,SUBCLASSPROC,void (*paint)(HWND,control_style *,HDC));
#endif
