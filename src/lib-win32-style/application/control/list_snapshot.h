#ifndef STYLE_CONTROL_LIST_SNAPSHOT_H
#define STYLE_CONTROL_LIST_SNAPSHOT_H
#include "../lifecycle/instance.h"
typedef struct style_control_list_snapshot {
    RECT client;
    DWORD style;
    int first,count,caret;
    HFONT font;
} style_control_list_snapshot;
typedef struct style_control_list_item {
    int index,length;
    RECT row,visible;
    LRESULT selected;
    const WCHAR *text;
} style_control_list_item;
/* Caller qualifies the string-backed LB_GETTEXT protocol and pins instance.
 * Snapshots, strings and DC are borrowed only during
 * callbacks; this operation owns native reads, row strings and DC isolation. */
typedef struct style_control_list_painter {
    void *context;
    HFONT (*font)(HWND,void *);
    void (*begin)(HWND,void *,HDC,const style_control_list_snapshot *);
    void (*item)(HWND,void *,HDC,const style_control_list_snapshot *,const style_control_list_item *);
} style_control_list_painter;
void style_control_list_paint(HWND,style_instance *,HDC,const style_control_list_painter *);
#endif
