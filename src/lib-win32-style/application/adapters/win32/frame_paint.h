#ifndef STYLE_RUNTIME_FRAME_PAINT_H
#define STYLE_RUNTIME_FRAME_PAINT_H
#include "runtime_identity.h"
#include "../../control/runtime.h"
typedef struct style_runtime_frame_scope {
    BOOL group_background;int group_state;RECT group_rect;
    BOOL panel_first,panel_enabled;HWND panel_parent;HDC panel_dc;POINT panel_points[3];COLORREF panel_color;
} style_runtime_frame_scope;
/* Native paint association borrows complete public objects. The caller pins
 * their lifetime for synchronous renderer calls; no callback may retain done
 * or its context. A renderer commits only after its existing successful draw. */
typedef struct style_runtime_frame_paint {
    style_runtime_frame_scope *scope;
    style_instance *instance;
    style_runtime_frame_state *state;
    void *owner;HWND window;
    BOOL (*owned)(HWND,void *);
    BOOL (*current)(style_runtime_frame_scope *);
} style_runtime_frame_paint;
typedef struct style_runtime_frame_renderer {
    void (*group)(HDC,const RECT *,const RECT *,const RECT *,void (*)(void *),void *);
    void (*panel)(HDC,const POINT *,BOOL (WINAPI *)(HDC,const POINT *,int),void (*)(void *),void *);
} style_runtime_frame_renderer;
void style_runtime_group_frame(const style_runtime_frame_paint *,const style_runtime_frame_renderer *,HDC,int,int,const RECT *,const RECT *,LPCWSTR,int,DWORD,const void *,BOOL);
void style_runtime_panel_polyline(const style_runtime_frame_paint *,const style_runtime_frame_renderer *,HDC,const POINT *,int,const void *,BOOL (WINAPI *)(HDC,const POINT *,int));
BOOL style_runtime_controlbar_edge_allowed(UINT);
void style_runtime_controlbar_edge(HWND,style_instance *,style_runtime_frame_state *,HDC,const RECT *,UINT,UINT,const void *,BOOL (*)(HDC,const RECT *,const RECT *));
typedef struct style_runtime_bit_frame_paint {
    HWND window;HDC dc;RECT item;UINT state;style_instance *instance;
    style_runtime_verification *verification;const style_control_runtime_report *report;
} style_runtime_bit_frame_paint;
/* Owner-draw payload, native parent/caller/DC and successful renderer commit.
 * All handles and complete state are borrowed through the synchronous draw. */
void style_runtime_bit_frame(const style_runtime_bit_frame_paint *,HDC,int,const RECT *,const RECT *,const void *,BOOL (*)(HWND,HDC,const RECT *,const RECT *,UINT));
#endif
