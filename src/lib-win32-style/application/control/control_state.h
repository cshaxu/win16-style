#ifndef STYLE_CONTROL_STATE_H
#define STYLE_CONTROL_STATE_H
#include "control_protocol.h"
#include "../lifecycle/instance.h"

/* Lease operations recognize their own variants; retain/release on a borrowed
 * application font are no-ops. Operations remain valid until state disposal. */
typedef struct style_control_fonts {
    HFONT (*source)(HFONT);
    HFONT (*acquire)(HWND,HFONT,BOOL);
    void (*retain)(HFONT);
    void (*release)(HFONT);
} style_control_fonts;
typedef struct style_control_state {
    BOOL removing,redraw_off;
    unsigned suppression,native_paints;
    style_control_protocol *protocol;
    style_instance *owner; /* borrowed parent identity for owned native children */
    HFONT font,original_font;
    int tab_count,*tab_positions;
    const style_control_fonts *fonts;
} style_control_state;
typedef enum style_control_font_mode {
    STYLE_CONTROL_FONT_NONE,STYLE_CONTROL_FONT_OBSERVE,STYLE_CONTROL_FONT_NORMALIZE,
    STYLE_CONTROL_FONT_CACHE,STYLE_CONTROL_FONT_REPLACE
} style_control_font_mode;

/* GUI-thread state, owned beside a public style_instance. The surrounding
 * instance pins state across callbacks; dispose after its last reference.
 * Disposal releases leases/protocol/tabs, never the application's font. */
void style_control_state_init(style_control_state *,const style_control_fonts *);
/* Prepare once on fresh initialized state; protocol and copied initial pixel
 * tabs belong to state even on failure, until the caller disposes it. */
BOOL style_control_state_prepare(HWND,style_control_state *,int,const int *);
void style_control_state_dispose(style_control_state *);
BOOL style_control_font_message(HWND,style_instance *,style_control_state *,WPARAM *,
    style_control_font_mode,BOOL,BOOL,HFONT *,HFONT *);
/* A retired native setter may finish after removal. The caller supplies the
 * current successor's source, or this state's source if no successor exists. */
void style_control_font_finish(HWND,const style_instance *,style_control_state *,UINT,LPARAM,BOOL,
    HFONT,HFONT,HFONT,HFONT);
/* Compute set-time pixels before native forwarding; commit only on success
 * while still attached. Native tab-stop messages keep their original units. */
LRESULT style_control_tabs_set(HWND,style_instance *,style_control_state *,WPARAM,LPARAM,
    LRESULT (*)(HWND,WPARAM,LPARAM));
#endif
