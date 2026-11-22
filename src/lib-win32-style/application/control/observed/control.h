#ifndef STYLE_OBSERVED_CONTROL_H
#define STYLE_OBSERVED_CONTROL_H
#include "lib-win32-style/application/lifecycle/theme_state.h"
#include "../control_colors.h"
#include "../native_scroll.h"
#include "../presentation.h"
#include "../header_snapshot.h"
#include "../tab_snapshot.h"
#include "lib-win32-style/application/window/palette.h"

typedef struct style_observed_control_theme {
    HMODULE resources; /* An owned mapping transfers to the caller on success. */
    style_choice_art_read_fn read_choice_art;
    style_relief_palette palette,scroll_palette;
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
} style_observed_control_theme;
typedef DWORD (*style_observed_control_theme_load)(style_observed_control_theme *);

/* The theme loader and native theme callback outlive GUI-thread attachments. */
BOOL style_observed_control_supported(HWND);
BOOL style_observed_control_attach(HWND,style_native_theme_fn,style_observed_control_theme_load,BOOL *native_requested);
BOOL style_observed_control_attached(HWND);
BOOL style_observed_control_remove(HWND);
#endif
