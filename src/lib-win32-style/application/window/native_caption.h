#ifndef STYLE_NATIVE_CAPTION_H
#define STYLE_NATIVE_CAPTION_H
#include <windows.h>
#include "lib-win32-style/application/window/caption_slots.h"
#include "caption_paint.h"
#include "frame_border.h"

#include "palette.h"

typedef struct style_native_caption_colors {
    COLORREF left, right, text;
    BOOL gradient;
} style_native_caption_colors;

style_native_caption_colors style_native_caption_palette(BOOL active);
enum style_native_caption_part { STYLE_NATIVE_CAPTION_MINIMIZE, STYLE_NATIVE_CAPTION_MAXIMIZE, STYLE_NATIVE_CAPTION_HELP, STYLE_NATIVE_CAPTION_CLOSE, STYLE_NATIVE_CAPTION_BUTTONS };
typedef struct style_native_caption_layout {
    RECT band, icon, buttons[STYLE_NATIVE_CAPTION_BUTTONS],system;
    UINT system_flags,left_buttons;
    BOOL system_on_right;
} style_native_caption_layout;

int style_native_caption_height(UINT dpi,BOOL tool);
BOOL style_native_caption_buttons_with_system(const RECT *title,UINT dpi,BOOL tool,
    const BOOL present[4],const classic_button_group *group,const classic_caption_metrics *metrics,
    RECT bounds[4],RECT *system);
unsigned style_native_caption_left_buttons(const classic_button_group *group);
BOOL style_native_caption_system_side(const classic_button_group *group,BOOL *on_right);
void style_native_caption_render(HDC dc,const style_native_caption_layout *layout,PCWSTR title,HICON icon,
    HFONT font,UINT dpi,style_native_caption_colors colors,const style_frame_colors *frame,
    const style_caption_presentation *presentation,BOOL maximized,
    const UINT button_flags[STYLE_NATIVE_CAPTION_BUTTONS],const classic_caption_art *art);
style_frame_colors style_native_frame_colors(const style_frame_colors *defined);
#endif
