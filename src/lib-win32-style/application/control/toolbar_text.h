#ifndef STYLE_TOOLBAR_TEXT_H
#define STYLE_TOOLBAR_TEXT_H
#include "../window/palette.h"
#include "control_colors.h"
#include <windows.h>
#include <commctrl.h>
/* Existing bounded text-item protocol. Caller first verifies toolbar/native
 * eligibility. accept sees raw metadata before any text query and supplies
 * the presentation support policy. iString stays opaque, never dereferenced.
 * Values/text/rect belong to the complete caller-owned snapshot. */
typedef struct style_toolbar_text_item {TBBUTTON button;WCHAR text[512];RECT rect;} style_toolbar_text_item;
BOOL style_toolbar_text_read(HWND,int,style_toolbar_text_item *,BOOL (*)(HWND,const TBBUTTON *));
BOOL style_toolbar_text_supported(HWND,BOOL (*)(HWND,const TBBUTTON *));
BOOL style_toolbar_text_find(HWND,DWORD_PTR,style_toolbar_text_item *,BOOL (*)(HWND,const TBBUTTON *));

/* Borrow theme colors and the supplied font provider for a synchronous paint. */
typedef struct style_toolbar_outline_view {
    const style_window_palette *palette;
    const style_frame_colors *frame;
    const classic_bar_colors *colors;
    unsigned font_points;
    HFONT (*acquire_font)(HFONT,unsigned);
    void (*release_font)(HFONT);
} style_toolbar_outline_view;
BOOL style_toolbar_outline_supported(HWND);
BOOL style_toolbar_outline_part(HWND,const NMCUSTOMDRAW *,const void *);
#endif
