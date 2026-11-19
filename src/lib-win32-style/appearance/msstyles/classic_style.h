#ifndef STYLE_MSSTYLES_CLASSIC_STYLE_H
#define STYLE_MSSTYLES_CLASSIC_STYLE_H

#include <stddef.h>
#include "lib-win32-style/application/window/caption_slots.h"

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
