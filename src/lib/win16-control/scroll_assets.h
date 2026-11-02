#ifndef WIN16_CONTROL_SCROLL_ASSETS_H
#define WIN16_CONTROL_SCROLL_ASSETS_H
#include "updown_paint.h"
enum win16_control_scroll_asset {
    WIN16_SCROLL_UP, WIN16_SCROLL_DOWN, WIN16_SCROLL_LEFT, WIN16_SCROLL_RIGHT,
    WIN16_SCROLL_UP_PRESSED, WIN16_SCROLL_DOWN_PRESSED, WIN16_SCROLL_LEFT_PRESSED, WIN16_SCROLL_RIGHT_PRESSED,
    WIN16_SCROLL_UP_DISABLED, WIN16_SCROLL_DOWN_DISABLED, WIN16_SCROLL_LEFT_DISABLED, WIN16_SCROLL_RIGHT_DISABLED
};
ntw16s_updown_glyph *const *win16_control_scroll_glyphs(void);
const char (*win16_control_scroll_glyph(enum win16_control_scroll_asset asset))[18];
#endif
