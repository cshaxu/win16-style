#ifndef WIN16_CONTROL_UPDOWN_PAINT_H
#define WIN16_CONTROL_UPDOWN_PAINT_H
#include "../common/palette.h"
#include "lib-win32-style/application/control/updown.h"
struct classic_frame_colors;
typedef struct win16_control_updown {
    const win16_style_palette *palette;
    const struct classic_frame_colors *frame;
    const style_scroll_arrow_art *art;
    style_arrow_button_metrics metrics;
} win16_control_updown;
void win16_control_updown_register(win16_control_updown *,style_control_updown *,HWND,HMODULE,const win16_style_palette *,void *,BOOL (*)(void *),void (*)(void *));
#endif
