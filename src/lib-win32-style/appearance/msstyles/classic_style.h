#ifndef STYLE_MSSTYLES_CLASSIC_STYLE_H
#define STYLE_MSSTYLES_CLASSIC_STYLE_H

#include <stddef.h>
#include "lib-win32-style/application/control/progress_paint.h"
#include "lib-win32-style/application/control/trackbar_paint.h"
#include "lib-win32-style/application/control/scroll_geometry.h"
#include "lib-win32-style/application/graphics/paint_frame.h"
#include "lib-win32-style/application/window/caption_slots.h"
#include "lib-win32-style/application/window/caption_paint.h"
#include "lib-win32-style/application/window/frame_border.h"

enum classic_style_result {
    CLASSIC_STYLE_OK, CLASSIC_STYLE_INVALID, CLASSIC_STYLE_VERSION,
    CLASSIC_STYLE_REQUIRED_RECORD, CLASSIC_STYLE_NO_BUTTON_GROUP
};

enum classic_scroll_extent_source {
    CLASSIC_SCROLL_EXTENT_DEFAULT, CLASSIC_SCROLL_EXTENT_CROSS_AXIS,
    CLASSIC_SCROLL_EXTENT_SYSTEM, CLASSIC_SCROLL_EXTENT_FIXED
};

typedef struct classic_scroll_metrics {
    uint16_t source;
    uint16_t logical_extent;
    style_scroll_thumb thumb;
    uint16_t initial_repeat_ms, repeat_ms;
    style_scroll_drag drag;
} classic_scroll_metrics;

enum classic_mdi_buttons {
    CLASSIC_MDI_SYSTEM=1, CLASSIC_MDI_RESTORE=2, CLASSIC_MDI_MINIMIZE=4, CLASSIC_MDI_CLOSE=8,
    CLASSIC_MDI_ALL=15, CLASSIC_MDI_DEFINED=128
};
typedef struct classic_menu_metrics {
    uint16_t check_column, shortcut_gap, arrow_column, plain_column;
    uint16_t text_flags, bar_padding;
    uint8_t mdi_buttons, row_minimum, row_padding, mdi_inactive;
} classic_menu_metrics;

typedef struct classic_file_metrics {
    uint16_t compact_width, compact_height, places_width, places_height;
    uint16_t client_dlu_width, client_dlu_height, gap, bottom_reserve;
} classic_file_metrics;

typedef struct classic_dialog_metrics {
    classic_file_metrics file;
    uint16_t print_width, print_full_height, print_large_height;
    uint16_t color_width, color_height;
    uint16_t find_width, find_native_height, find_classic_height;
    uint16_t font_width, font_height;
} classic_dialog_metrics;

/* Record 9 has four independent fields with stable wire values. */
enum classic_window_provider {
    CLASSIC_WINDOW_PROVIDER_UNSPECIFIED,
    CLASSIC_WINDOW_PROVIDER_FULL_CLIENT,
    CLASSIC_WINDOW_PROVIDER_NONCLIENT
};
enum classic_file_route {
    CLASSIC_FILE_NATIVE, CLASSIC_FILE_LEGACY_LAYOUT, CLASSIC_FILE_EXPLORER_LAYOUT
};
enum classic_folder_route {
    CLASSIC_FOLDER_NATIVE, CLASSIC_FOLDER_LEGACY_TREE, CLASSIC_FOLDER_OBSERVED_NATIVE
};
enum classic_print_route {
    CLASSIC_PRINT_NATIVE, CLASSIC_PRINT_LEGACY_BRIDGE, CLASSIC_PRINT_GENERAL_PAGE
};
enum classic_stock_route {
    CLASSIC_STOCK_DEFAULT, CLASSIC_STOCK_NATIVE, CLASSIC_STOCK_OWNER_CHROME, CLASSIC_STOCK_OBSERVED_LAYOUT
};

typedef struct classic_dialog_policy {
    uint8_t window_provider, file, folder, print;
    uint8_t color, font, find;
} classic_dialog_policy;

enum classic_choice_fallback {
    CLASSIC_CHOICE_NATIVE, CLASSIC_CHOICE_SYMBOLS
};

enum classic_updown_mode {
    CLASSIC_UPDOWN_NATIVE, CLASSIC_UPDOWN_ART, CLASSIC_UPDOWN_RECOLOR
};

typedef struct classic_control_presentation {
    style_line_profile separator,group,edit;
    style_push_profile push;
    style_progress_layout progress;
    style_trackbar_profile trackbar;
    style_line_profile header;
    style_line_profile tab,native_edge;
    style_tab_text tab_text;
    enum classic_choice_fallback choice_fallback;
    style_choice_metrics choice_metrics;
    style_arrow_button_metrics updown;
    enum classic_updown_mode updown_mode;
    uint8_t control_font_points;
} classic_control_presentation;

static inline BOOL classic_button_presentation_enabled(const classic_control_presentation *controls,UINT type,DWORD style)
{
    if(type==BS_GROUPBOX)return controls->group!=STYLE_LINE_DEFAULT;
    if(type==BS_PUSHBUTTON || type==BS_DEFPUSHBUTTON || (style&BS_PUSHLIKE))
        return controls->push!=STYLE_PUSH_DEFAULT;
    return TRUE;
}

/* Parse project RCDATA CLASSIC_STYLE v1. The caller's output changes only on success. */
enum classic_style_result classic_style_read(const unsigned char *bytes,size_t length,
    classic_button_group *group,classic_scroll_metrics *scroll,classic_menu_metrics *menu,classic_dialog_metrics *dialog,
    classic_dialog_policy *policy,style_caption_presentation *caption,style_border_presentation *border,classic_control_presentation *controls);

/* Encode a title-button group and optional metrics. Returns size or zero. */
size_t classic_style_write(unsigned char *bytes,size_t capacity,const classic_button_group *group,
    const classic_scroll_metrics *scroll,const classic_menu_metrics *menu,const classic_dialog_metrics *dialog,
    const classic_dialog_policy *policy,const style_caption_presentation *caption,const style_border_presentation *border,
    const classic_control_presentation *controls);

enum { CLASSIC_STYLE_RESOURCE_CAPACITY=410 };

#endif
