#ifndef STYLE_CONTROL_LIST_H
#define STYLE_CONTROL_LIST_H
#include <windows.h>

/* The native list retains selection and scrolling; consumers choose when to
 * take snapshots and what repaint policy to apply to a change. */
typedef struct style_control_list_view {
    LRESULT top,selection,caret;
} style_control_list_view;

static inline style_control_list_view style_control_list_view_read(HWND window)
{
    style_control_list_view view;
    view.top=SendMessageW(window,LB_GETTOPINDEX,0,0);
    view.selection=SendMessageW(window,LB_GETCURSEL,0,0);
    view.caret=SendMessageW(window,LB_GETCARETINDEX,0,0);
    return view;
}
static inline BOOL style_control_list_view_changed(const style_control_list_view *a,const style_control_list_view *b)
{return a->top!=b->top || a->selection!=b->selection || a->caret!=b->caret;}
#endif
