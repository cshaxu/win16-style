#ifndef STYLE_CONTROL_REBAR_H
#define STYLE_CONTROL_REBAR_H
#include <windows.h>
#include <commctrl.h>
/* Native procedure/context/window are borrowed and pinned by the caller on
 * its GUI thread. Qualification is a whole operation, not private field views.
 * Draw metadata is borrowed synchronously. Rejected style bits are policy. */
typedef struct style_control_rebar_scope {
    HWND window;void *owner;WNDPROC native;BOOL (*valid)(HWND,void *);
} style_control_rebar_scope;
/* Owns values only; child/bitmap are borrowed. The two UTF16 cells preserve
 * the existing bounded empty-text probe, not an arbitrary text snapshot. */
typedef struct style_control_rebar_probe {
    RECT band,border,client,child_rect;HWND child;HBITMAP bitmap;
    DWORD style,text_capacity;WCHAR text[2];int image;
    COLORREF bar_back,bar_fore,back,fore;UINT dpi;
} style_control_rebar_probe;
BOOL style_control_rebar_read(const style_control_rebar_scope *,const NMCUSTOMDRAW *,DWORD,style_control_rebar_probe *);
/* Run after caller's content/style policy accepts the native data. Geometry
 * commits only after the original final validity/DPI checks succeed. */
BOOL style_control_rebar_geometry(const style_control_rebar_scope *,style_control_rebar_probe *);
#endif
