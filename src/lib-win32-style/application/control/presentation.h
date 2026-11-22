#ifndef STYLE_CONTROL_PRESENTATION_H
#define STYLE_CONTROL_PRESENTATION_H
#include <stdint.h>
#include "roles.h"
#include "progress_paint.h"
#include "trackbar_paint.h"

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

static inline unsigned classic_button_native_roles(const classic_control_presentation *p)
{
    return (!p->group?STYLE_ROLE_GROUP:0) | (!p->push?STYLE_ROLE_PUSH:0);
}

/* A missing style record leaves the corresponding native drawing intact. */
static inline unsigned classic_control_native_roles(const classic_control_presentation *p)
{
    return classic_button_native_roles(p) | (!p->separator?STYLE_ROLE_SEPARATOR:0) | (!p->edit?STYLE_ROLE_EDIT:0) |
        (!(p->progress.flags&STYLE_PROGRESS_BORDER)?STYLE_ROLE_PROGRESS_BORDER:0) |
        (!p->trackbar?STYLE_ROLE_TRACKBAR:0) | (!p->header?STYLE_ROLE_HEADER:0) |
        (!p->tab?STYLE_ROLE_TAB:0) | (!p->native_edge?STYLE_ROLE_NATIVE_EDGE:0) |
        (!p->updown_mode?STYLE_ROLE_UPDOWN:0);
}
static inline BOOL classic_button_presentation_enabled(const classic_control_presentation *p,UINT type,DWORD style)
{
    return !(classic_button_native_roles(p)&style_control_button_paint_role(type,style));
}

#endif
