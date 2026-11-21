#ifndef WIN16_CONTROL_VCL_NATIVE_PAINT_H
#define WIN16_CONTROL_VCL_NATIVE_PAINT_H
#include "lib-win32-style/application/adapters/win32/runtime_roles.h"
#include "../common/palette.h"
struct win16_runtime_push;
typedef struct win16_control_vcl_paint_view {
    HWND window;
    style_instance *instance;
    style_runtime_child_state *state;
    const win16_style_palette *palette;
    const struct win16_runtime_push *push;
    const classic_combo_arrow_art *combo_arrow;
} win16_control_vcl_paint_view;
void win16_control_vcl_inplace_draw(const win16_control_vcl_paint_view *,HDC,int,int,const RECT *,const RECT *,const void *,BOOL);
void win16_control_vcl_nav_draw(const win16_control_vcl_paint_view *,HDC,int,int,const RECT *,const RECT *,const void *);
#endif
