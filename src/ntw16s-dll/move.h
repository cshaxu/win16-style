#ifndef WIN16_STYLE_MOVE_H
#define WIN16_STYLE_MOVE_H
#include <windows.h>

/* Private GUI-thread transaction. No state is kept in the window after return. */
typedef BOOL (*win16_style_move_valid)(HWND,const void *);
BOOL win16_style_move(HWND,UINT,BOOL,LPARAM,const void *,win16_style_move_valid);
void win16_style_move_cancel(HWND);
/* Called only by the startup-policy-owned default-procedure detours. */
BOOL win16_style_default_move(HWND,WPARAM,LPARAM);
#endif
