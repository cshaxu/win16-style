#ifndef STYLE_CONTROL_HEADER_SNAPSHOT_H
#define STYLE_CONTROL_HEADER_SNAPSHOT_H
#include "native.h"
#include <commctrl.h>
#include "lib-win32-style/application/graphics/paint_frame.h"
BOOL style_header_supported(HWND,DWORD extra_exclusions);
void style_header_draw_text(HDC,RECT,PCWSTR,int format,const style_header_colors *,BOOL pressed,COLORREF fallback);
typedef struct style_header_item {RECT rect;WCHAR *text;int format;} style_header_item;
typedef struct style_header_snapshot {
    RECT client;
    int count;
    unsigned epoch;
    style_header_item *items;
} style_header_snapshot;
/* Owns copied text/items until clear; requests no application itemData.
 * Format support is profile policy. All native queries complete before paint.
 * Failure clears partial copies; caller pins instance/state for this call. */
BOOL style_header_snapshot_read(HWND,const style_instance *,const style_control_native_state *,UINT allowed_formats,style_header_snapshot *);
void style_header_snapshot_clear(style_header_snapshot *);
typedef struct style_header_notification {
    const style_instance *instance;
    const style_header_colors *colors;
    style_relief_palette palette;
    style_line_profile profile;
} style_header_notification;

BOOL style_header_client_supported(HWND);
BOOL style_header_native_supported(HWND);
BOOL style_header_client_paint(HWND,HDC,const style_relief_palette *,HFONT,const style_instance *,
    const style_control_native_state *,const style_header_colors *,style_line_profile);
BOOL style_header_native_part(HWND,const NMCUSTOMDRAW *,const void *);
#endif
