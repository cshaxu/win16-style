#ifndef STYLE_WINDOW_FRAME_BORDER_H
#define STYLE_WINDOW_FRAME_BORDER_H

#include <stdint.h>
#include <windows.h>

enum style_border_fill { STYLE_BORDER_CLIPPED=1, STYLE_BORDER_PERIMETER };
enum style_border_anchor { STYLE_BORDER_OUTER, STYLE_BORDER_NEXT, STYLE_BORDER_INNER };
enum style_border_color {
    STYLE_BORDER_FACE, STYLE_BORDER_EDGE, STYLE_BORDER_LIGHT, STYLE_BORDER_SHADOW, STYLE_BORDER_TITLE,
    STYLE_BORDER_KEEP=255
};
typedef struct style_border_stroke {
    uint8_t anchor,leading,trailing;
} style_border_stroke;
enum style_frame_scale { STYLE_FRAME_LOGICAL=1, STYLE_FRAME_MONITOR_PIXELS };
enum style_frame_border_kind { STYLE_FRAME_SIZEABLE, STYLE_FRAME_FIXED, STYLE_FRAME_DIALOG, STYLE_FRAME_LINE };
typedef struct style_frame_geometry {
    uint8_t scale, sizeable, fixed, dialog, line, caption_floor, separator_units, scroll_shared_units;
} style_frame_geometry;
typedef struct style_border_presentation {
    uint8_t fill,dialog_fill;
    style_border_stroke outer,inner;
    uint8_t dialog_inner,divider;
    style_frame_geometry geometry;
} style_border_presentation;

typedef struct style_border_palette {
    COLORREF face,edge,light,shadow,title;
} style_border_palette;

/* DC coordinates. A clipped fill uses the caller's non-client clip. The
 * leading pair is top/left. The trailing pair is bottom/right. */
void style_frame_border_paint(HDC dc,RECT bounds,int border,int unit,BOOL dialog,const SIZE *resize_corners,
    const style_border_presentation *presentation,const style_border_palette *palette);
/* Extents in the caller's coordinate space. Monitor-pixel widths use the
 * monitor DPI as the source scale, even for DPI-virtualized windows. */
int style_frame_border_width(const style_frame_geometry *geometry,enum style_frame_border_kind kind,
    UINT coordinate_dpi,UINT monitor_dpi);
int style_frame_native_border_width(enum style_frame_border_kind kind,BOOL horizontal,UINT dpi);
int style_frame_window_border_width(HWND window,const style_frame_geometry *geometry,
    enum style_frame_border_kind kind,BOOL horizontal,UINT dpi);
int style_frame_caption_height(int logical_height,UINT dpi,int minimum);
SIZE style_frame_monitor_dpi(HWND window,UINT fallback);

#endif
