#ifndef STYLE_CAPTION_SLOTS_H
#define STYLE_CAPTION_SLOTS_H

#include <stdint.h>
#include <windows.h>

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

typedef struct classic_caption_metrics {
    int height, tool_height;
    SIZE button, tool_button;
} classic_caption_metrics;

/* The mask uses 1u << role. Omitted roles leave empty rectangles. The
 * clipping policy lets a right slot shrink on very narrow captions. */
BOOL style_caption_slots(const classic_button_group *group,const RECT *band,UINT dpi,
    SIZE dimensions,unsigned present,BOOL clip_right,RECT bounds[CLASSIC_BUTTON_CLOSE+1]);
/* Rectangles start at first and cover visible roles through CLOSE. */
int style_caption_hit(const RECT *bounds,enum classic_button_role first,unsigned visible,POINT point);
enum classic_button_role style_caption_role(int hit);
UINT style_caption_command(enum classic_button_role role,BOOL maximized);
UINT style_caption_gesture(int hit,UINT message,BOOL maximized,BOOL can_maximize,BOOL can_close);

#endif
