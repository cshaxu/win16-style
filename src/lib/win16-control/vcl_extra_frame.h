#ifndef WIN16_CONTROL_VCL_EXTRA_FRAME_H
#define WIN16_CONTROL_VCL_EXTRA_FRAME_H
#include <windows.h>
typedef struct win16_control_bit_frame_view {
    void *owner;HWND window;HDC dc;RECT item;UINT state;const BOOL *detached;BOOL *verified;const HWND *parent;const HMODULE *module;
} win16_control_bit_frame_view;
typedef struct win16_control_controlbar_paint_view {
    void *owner;HWND window;const BOOL *candidate;BOOL *verified;const HMODULE *module;BOOL (*identity)(HWND,void *);
} win16_control_controlbar_paint_view;
void win16_control_bit_theme_frame(const win16_control_bit_frame_view *,HDC,int,const RECT *,const RECT *,const void *);
BOOL win16_control_controlbar_edge_allowed(UINT);
void win16_control_controlbar_edge(const win16_control_controlbar_paint_view *,HDC,const RECT *,UINT,UINT,const void *);
#endif
