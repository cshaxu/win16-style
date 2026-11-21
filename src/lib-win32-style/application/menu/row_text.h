#ifndef STYLE_MENU_ROW_TEXT_H
#define STYLE_MENU_ROW_TEXT_H
#include <windows.h>
#include <stdint.h>

typedef struct style_menu_metrics {
    uint16_t check_column, shortcut_gap, arrow_column, plain_column;
    uint16_t text_flags, bar_padding;
    uint8_t mdi_buttons, row_minimum, row_padding, mdi_inactive;
    uint8_t font_mode;
} style_menu_metrics;

enum style_menu_text_flags {
    STYLE_MENU_TEXT_STIPPLE_DISABLED=1, STYLE_MENU_TEXT_SHORTCUT_RIGHT=2,
    STYLE_MENU_TEXT_BOLD_DEFAULT=4, STYLE_MENU_TEXT_CENTER_BAR=8, STYLE_MENU_COMMAND_GLYPHS=16,
    STYLE_MENU_TEXT_DEFINED=0x8000
};

static inline unsigned style_menu_text_options(unsigned declared)
{
    return declared?declared:
        STYLE_MENU_TEXT_SHORTCUT_RIGHT|STYLE_MENU_TEXT_BOLD_DEFAULT|STYLE_MENU_COMMAND_GLYPHS;
}

typedef struct style_menu_text_layout {
    RECT label, shortcut;
    BOOL shortcut_right, shortcut_first, label_center;
} style_menu_text_layout;

/* Borrows the currently selected font; leaves the source string intact. */
BOOL style_menu_draw_labels(HDC dc,const WCHAR *text,const style_menu_text_layout *layout,
    BOOL stippled,COLORREF color);
BOOL style_menu_draw_label(HDC dc,const WCHAR *text,const RECT *bounds,BOOL centered,
    BOOL stippled,COLORREF color);
int style_menu_label_width(HDC dc,const WCHAR *text);
/* Returns an owned font, or NULL when the supplied font cannot be realized. */
HFONT style_menu_bold_font(HFONT font);

#endif
