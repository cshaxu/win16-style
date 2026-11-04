#ifndef WIN16_CONTROL_PALETTE_H
#define WIN16_CONTROL_PALETTE_H
#include "control_api.h"

typedef struct win16_style_palette {
    COLORREF active, inactive, active_text, inactive_text;
    COLORREF face, light, shadow, edge;
} win16_style_palette;
NTW16S_API void win16_style_default_palette(win16_style_palette *palette);
void win16_control_resolve_palette(const win16_style_palette *source,win16_style_palette *palette);
LRESULT win16_control_dialog_colors(HDC dc,const win16_style_palette *palette);
#endif
