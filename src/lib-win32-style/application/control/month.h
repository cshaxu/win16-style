#ifndef STYLE_CONTROL_MONTH_H
#define STYLE_CONTROL_MONTH_H
#include "../adapters/win32/runtime_identity.h"
#include "../window/palette.h"
#include "../graphics/paint_frame.h"
#include <uxtheme.h>
typedef struct style_control_month_state {
    BOOL header_valid,header_known,nav_valid;RECT header,prev,next;
    BOOL selected;HTHEME theme;HDC dc;RECT rect,clip;BOOL has_clip;
} style_control_month_state;
typedef struct style_control_month_scope {
    void *context;HWND window;
    style_instance *instance;style_runtime_rich_state *rich;
    style_control_month_state *state;DWORD rejected_styles;
    BOOL (*current)(void *);
} style_control_month_scope;
typedef struct style_control_month_queries {BOOL active;} style_control_month_queries;
typedef struct style_control_month_selection {RECT rect,clip;BOOL has_clip;} style_control_month_selection;
/* Whole GUI-thread scope is borrowed and pinned through synchronous queries.
 * Queries use one caller-owned per-thread guard, including native unwind.
 * Header commit precedes navigation: preserve partial native query results.
 * Window/theme/DC are borrowed; no font or presentation resource is owned. */
void style_control_month_query(const style_control_month_scope *,style_control_month_queries *);
BOOL style_control_month_draw_identity(const style_control_month_scope *,HTHEME,HDC,const void *);
void style_control_month_selection_begin(const style_control_month_scope *,HTHEME,HDC,int,int,const RECT *,const RECT *,const void *);
BOOL style_control_month_selection_take(const style_control_month_scope *,HTHEME,HDC,int,int,const RECT *,DWORD,const void *,style_control_month_selection *);
BOOL style_control_month_title_fits(const RECT *,const RECT *,const RECT *,int);
BOOL style_control_month_text_length(LPCWSTR,int,int *);

/* Rendering uses the current theme data and a caller-owned font lease cache. */
typedef struct style_control_month_view {
    style_control_month_scope native;
    HFONT *cached_font;
    const style_window_palette *palette;
    const style_scroll_arrow_art *arrows;
    COLORREF selection;
    unsigned font_dpi,font_points;
    HFONT (*acquire_font)(unsigned density,unsigned points);
    BOOL (*retain_font)(HFONT);
} style_control_month_view;
void style_control_month_arrow(const style_control_month_view *,HTHEME,HDC,int,int,const RECT *,const RECT *,const void *);
HFONT style_control_month_text_borrow(const style_control_month_view *,HTHEME,HDC,int,int,DWORD,DWORD,const RECT *,const void *);
BOOL style_control_month_text_room(const style_control_month_view *,HTHEME,HDC,int,int,DWORD,DWORD,const RECT *,int,const void *);
void style_control_month_header_text(const style_control_month_view *,HTHEME,HDC,int,int,const RECT *,DWORD,const void *);
void style_control_month_selection_text(const style_control_month_view *,HTHEME,HDC,int,int,const RECT *,DWORD,const void *);
void style_control_month_paint_arrow(HDC,const RECT *,const RECT *,int part,int state,
    const style_window_palette *,const style_scroll_arrow_art *);
void style_control_month_fill(HDC,const RECT *,const RECT *,COLORREF);
/* The caller saves/restores the DC and owns the selected font lease. */
BOOL style_control_month_text(HTHEME,HDC,int part,int state,HFONT,LPCWSTR,int count,DWORD flags,const RECT *,
    int (WINAPI *draw)(HDC,LPCWSTR,int,LPRECT,UINT),BOOL (*room)(void *,int),void *context);
#endif
