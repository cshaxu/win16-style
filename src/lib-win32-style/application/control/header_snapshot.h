#ifndef STYLE_CONTROL_HEADER_SNAPSHOT_H
#define STYLE_CONTROL_HEADER_SNAPSHOT_H
#include "native.h"
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
#endif
