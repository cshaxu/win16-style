#ifndef STYLE_CONTROL_CHOICE_GLYPH_H
#define STYLE_CONTROL_CHOICE_GLYPH_H
#include "control_colors.h"
#include "lib-win32-style/application/window/palette.h"

typedef struct style_check_glyph {
    classic_choice_art art;
    COLORREF ink,paper;
} style_check_glyph;
typedef BOOL (*style_check_glyph_read)(UINT flags,style_check_glyph *);

BOOL style_control_checklist_glyph(HDC,const RECT *,UINT,style_check_glyph_read);
void style_control_choice_glyph(HDC,RECT,UINT dpi,UINT,const style_window_palette *,const char *);
#endif
