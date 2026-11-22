#ifndef STYLE_CONTROL_VCL_PAINT_H
#define STYLE_CONTROL_VCL_PAINT_H
#include "lib-win32-style/application/adapters/win32/runtime_roles.h"
#include "control_chrome.h"
typedef struct style_control_vcl_paint_view {
    HWND window;
    style_instance *instance;
    style_runtime_child_state *state;
    const style_window_palette *palette;
    const style_control_push_appearance *push;
    const classic_combo_arrow_art *combo_arrow;
} style_control_vcl_paint_view;
void style_control_vcl_inplace_draw(const style_control_vcl_paint_view *,HDC,int,int,const RECT *,const RECT *,const void *,BOOL);
void style_control_vcl_nav_draw(const style_control_vcl_paint_view *,HDC,int,int,const RECT *,const RECT *,const void *);
#endif
