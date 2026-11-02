#ifndef WIN16_CONTROL_CHOICE_PAINT_H
#define WIN16_CONTROL_CHOICE_PAINT_H
#include "palette.h"
BOOL win16_control_checklist_glyph(HDC dc,const RECT *bounds,UINT flags);
void win16_control_choice_glyph(HDC dc,RECT bounds,UINT dpi,int row,int column,const win16_style_palette *palette);
#endif
