#ifndef STYLE_MSSTYLES_CLASSIC_STYLE_H
#define STYLE_MSSTYLES_CLASSIC_STYLE_H

#include <stddef.h>
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
} classic_scroll_metrics;

typedef struct classic_menu_metrics {
    uint16_t check_column, shortcut_gap, arrow_column, plain_column;
    uint16_t text_flags;
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

typedef enum classic_dialog_route {
    CLASSIC_DIALOG_NATIVE, CLASSIC_DIALOG_WIN16, CLASSIC_DIALOG_WIN2K
} classic_dialog_route;

typedef struct classic_dialog_policy {
    uint8_t appearance, file, folder, print;
} classic_dialog_policy;

/* Parse project RCDATA CLASSIC_STYLE v1. The caller's output changes only on success. */
enum classic_style_result classic_style_read(const unsigned char *bytes,size_t length,
    classic_button_group *group,classic_scroll_metrics *scroll,classic_menu_metrics *menu,classic_dialog_metrics *dialog,
    classic_dialog_policy *policy,style_caption_presentation *caption,style_border_presentation *border);

/* Encode a title-button group and optional metrics. Returns size or zero. */
size_t classic_style_write(unsigned char *bytes,size_t capacity,const classic_button_group *group,
    const classic_scroll_metrics *scroll,const classic_menu_metrics *menu,const classic_dialog_metrics *dialog,
    const classic_dialog_policy *policy,const style_caption_presentation *caption,const style_border_presentation *border);

enum { CLASSIC_STYLE_RESOURCE_CAPACITY=183 };

#endif
