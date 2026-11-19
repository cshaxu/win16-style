#ifndef WIN16_CONTROL_CHOICE_PAINT_H
#define WIN16_CONTROL_CHOICE_PAINT_H
#include "../common/palette.h"
BOOL win16_control_checklist_glyph(HDC dc,const RECT *bounds,UINT flags);
typedef struct win16_choice_art {
    WORD image;
    BOOL available;
    char cell[14*13];
} win16_choice_art;
const char *win16_choice_art_get(HMODULE module,WORD image,win16_choice_art *art);
void win16_control_choice_glyph(HDC dc,RECT bounds,UINT dpi,int row,int column,const win16_style_palette *palette,
    const char *image);
#endif
