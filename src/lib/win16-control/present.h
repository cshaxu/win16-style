#ifndef WIN16_CONTROL_PRESENT_H
#define WIN16_CONTROL_PRESENT_H
#include "palette.h"
typedef struct win16_control_present_view {
    const BOOL *detached;
    const win16_style_palette *palette;
    void *context;
    void (*paint)(HWND,void *,HDC);
} win16_control_present_view;
void win16_control_present(HWND window,const win16_control_present_view *view,HDC target);
void win16_control_print_client_and_children(HWND window,const BOOL *detached,HDC dc,LPARAM flags);
#endif
