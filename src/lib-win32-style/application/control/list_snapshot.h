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

typedef struct style_list_appearance {
    HBRUSH parent_brush;
    HFONT font_override;
    COLORREF background,selection,text,selected_text,disabled_text;
    const int *tabs;
    int tab_count;
    BOOL use_tabs, honor_focus_cues;
} style_list_appearance;

/* The brush, font and tab stops are borrowed for this paint. A supplied brush
 * paints the whole background and remains visible in unselected rows. */
void style_control_list_strings(HWND,style_instance *,HDC,const style_list_appearance *);
#endif
