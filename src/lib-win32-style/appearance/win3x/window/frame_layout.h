#ifndef WIN16_CONTROL_FRAME_LAYOUT_H
#define WIN16_CONTROL_FRAME_LAYOUT_H
#include "../common/layout.h"
#include "lib-win32-style/application/lifecycle/instance.h"
#include "lib-win32-style/application/graphics/font_pair.h"
/* Borrow whole roots through native menu callbacks; values stay live. */
BOOL win16_control_measure_window(HWND window,const style_instance *,const style_font_pair *,
    const RECT *outer,const classic_button_group *buttons,const classic_caption_metrics *metrics,
    const style_frame_geometry *frame,win16_style_layout *layout);
BOOL win16_control_frame_hit_test(HWND window,const win16_style_layout *layout,LPARAM point,LRESULT *result);
#endif
