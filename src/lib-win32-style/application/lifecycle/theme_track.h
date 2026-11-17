#ifndef STYLE_THEME_TRACK_H
#define STYLE_THEME_TRACK_H
#include <windows.h>
#include <uxtheme.h>

/* Native class IDs currently used by callers; unrecognized classes remain
 * OTHER. This bounded reader does not choose any renderer or visual style. */
enum style_theme_class {STYLE_THEME_OTHER,STYLE_THEME_BUTTON,STYLE_THEME_TEXTGLOW,STYLE_THEME_COMBOBOX};
int style_theme_classify(LPCWSTR);

/* A ticket is valid only while no theme open/close has crossed the draw. */
typedef struct style_theme_track_ticket {
    ULONGLONG epoch;
    BOOL eligible;
    HTHEME theme;
} style_theme_track_ticket;

/* Owns cancellation of one in-flight native open/close call. The caller
 * begins the existing tracker operation and clears active after normal
 * completion. End is also callback-compatible with its existing unwinder;
 * it never catches/retries or calls a native theme API. */
typedef struct style_theme_track_scope {BOOL active,closing;HTHEME handle;} style_theme_track_scope;
void style_theme_track_scope_end(void *);

/* Kind values are supplied by the caller; the tracker owns no style policy.
 * Each linked module has one process-wide tracker, not one per window. */
style_theme_track_ticket style_theme_track_open_begin(void);
void style_theme_track_open_end(style_theme_track_ticket, HTHEME, int);
void style_theme_track_open_cancel(void);
void style_theme_track_close_begin(HTHEME);
void style_theme_track_close_end(HTHEME, BOOL);
style_theme_track_ticket style_theme_track_begin(HTHEME, int);
BOOL style_theme_track_end(HTHEME, style_theme_track_ticket, int);
#endif
