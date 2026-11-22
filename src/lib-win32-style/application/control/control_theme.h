#ifndef STYLE_CONTROL_THEME_H
#define STYLE_CONTROL_THEME_H
#include "control_colors.h"
#include "native_scroll.h"
#include "presentation.h"
#include "header_snapshot.h"
#include "tab_snapshot.h"
#include "lib-win32-style/application/window/palette.h"

/* Format-neutral recipe. An owned resource mapping transfers on successful
 * load; the instance retains it only when choice artwork is needed. */
typedef struct style_control_theme {
    HMODULE resources; /* An owned mapping transfers to the caller on success. */
    style_choice_art_read_fn read_choice_art;
    style_relief_palette palette,scroll_palette;
    style_window_palette window_palette;
    classic_surface_colors scroll_arrow;
    classic_combo_arrow_art combo_arrow;
    classic_control_presentation controls;
    classic_push_colors push;
    classic_choice_colors choice;
    classic_text_colors label;
    classic_edit_border edit[CLASSIC_EDIT_STATE_COUNT];
    classic_list_colors list;
    classic_report_item_colors report[CLASSIC_REPORT_ROLE_COUNT][CLASSIC_REPORT_STATE_COUNT];
    classic_bar_colors bars[CLASSIC_BAR_ROLE_COUNT];
    classic_combo_colors combo;
    style_tab_colors tab;
    style_header_colors header;
    classic_progress_colors progress[2];
    classic_trackbar_colors trackbar[2];
    classic_group_colors group;
    style_frame_colors frame;
    classic_calendar_colors calendar;
    COLORREF date_button_face;
    BOOL has_date_button_face;
    style_scroll_arrow_art scroll_art;
    style_native_scroll_colors scroll;
    style_scroll_metrics scroll_metrics;
} style_control_theme;
typedef DWORD (*style_control_theme_load)(style_control_theme *);

#endif
