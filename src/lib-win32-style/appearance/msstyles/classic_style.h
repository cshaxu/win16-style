#ifndef STYLE_MSSTYLES_CLASSIC_STYLE_H
#define STYLE_MSSTYLES_CLASSIC_STYLE_H

#include <stddef.h>
#include <stdint.h>

enum classic_button_role {
    CLASSIC_BUTTON_SYSTEM=1, CLASSIC_BUTTON_MINIMIZE, CLASSIC_BUTTON_MAXIMIZE,
    CLASSIC_BUTTON_HELP, CLASSIC_BUTTON_CLOSE
};
enum classic_button_side { CLASSIC_BUTTON_LEFT, CLASSIC_BUTTON_RIGHT };

typedef struct classic_button_slot {
    uint8_t role, side;
    uint16_t gap_before; /* 96 DPI pixels; entries are in visual left-to-right order. */
} classic_button_slot;

typedef struct classic_button_group {
    uint16_t count, left_margin, right_margin;
    classic_button_slot slots[CLASSIC_BUTTON_CLOSE];
} classic_button_group;

enum classic_style_result {
    CLASSIC_STYLE_OK, CLASSIC_STYLE_INVALID, CLASSIC_STYLE_VERSION,
    CLASSIC_STYLE_REQUIRED_RECORD, CLASSIC_STYLE_NO_BUTTON_GROUP
};

/* Parse project RCDATA CLASSIC_STYLE v1. The caller's output changes only on success. */
enum classic_style_result classic_style_read(const unsigned char *bytes,size_t length,
    classic_button_group *group);

/* Encode a v1.0 resource with one required title-button group. Returns size or zero. */
size_t classic_style_write(unsigned char *bytes,size_t capacity,const classic_button_group *group);

enum { CLASSIC_STYLE_BUTTON_RESOURCE_CAPACITY=52 };

#endif
