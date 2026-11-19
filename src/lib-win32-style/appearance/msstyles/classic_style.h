#ifndef STYLE_MSSTYLES_CLASSIC_STYLE_H
#define STYLE_MSSTYLES_CLASSIC_STYLE_H

#include <stddef.h>
#include "lib-win32-style/application/window/caption_slots.h"

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

/* Parse project RCDATA CLASSIC_STYLE v1. The caller's output changes only on success. */
enum classic_style_result classic_style_read(const unsigned char *bytes,size_t length,
    classic_button_group *group,classic_scroll_metrics *scroll);

/* Encode a title-button group and an optional scroll metric. Returns size or zero. */
size_t classic_style_write(unsigned char *bytes,size_t capacity,const classic_button_group *group,
    const classic_scroll_metrics *scroll);

enum { CLASSIC_STYLE_RESOURCE_CAPACITY=64 };

#endif
