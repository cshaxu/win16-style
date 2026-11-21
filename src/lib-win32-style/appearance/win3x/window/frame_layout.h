#ifndef WIN16_CONTROL_FRAME_LAYOUT_H
#define WIN16_CONTROL_FRAME_LAYOUT_H
#include "lib-win32-style/application/window/full_client_layout.h"
#include "lib-win32-style/application/lifecycle/instance.h"
#include "lib-win32-style/application/graphics/font_pair.h"
/* Borrow whole roots through native menu callbacks; values stay live. */
BOOL win16_control_measure_window(HWND window,const style_instance *,const style_font_pair *,
    const RECT *outer,const classic_button_group *buttons,const classic_caption_metrics *metrics,
    const style_frame_geometry *frame,unsigned icon_mode,const style_menu_metrics *menu,style_full_client_layout *layout);
BOOL win16_control_frame_hit_test(HWND window,const style_full_client_layout *layout,LPARAM point,LRESULT *result);
#endif
