#ifndef WIN16_CONTROL_SCROLL_NATIVE_H
#define WIN16_CONTROL_SCROLL_NATIVE_H
#include "scroll_layout.h"
#include "scroll_track.h"
#include "control_state.h"
#include <commctrl.h>
BOOL win16_scroll_measure_native(const RECT *bar,BOOL vertical,UINT dpi,
    const SCROLLINFO *info,win16_scroll_layout *layout);
typedef struct win16_scroll_caret_view {
    BOOL *detached,*created,*shown;
    SIZE *size;
    const win16_scroll_track *track;
    BOOL (*is_scroll)(HWND);
    BOOL (*layout)(HWND,win16_scroll_layout *);
} win16_scroll_caret_view;
void win16_scroll_caret_destroy(HWND window,const win16_scroll_caret_view *view);
void win16_scroll_caret_sync(HWND window,const win16_scroll_caret_view *view);
typedef BOOL (*win16_scroll_info_fn)(HWND,int,LPSCROLLINFO);
BOOL win16_scroll_control_layout(HWND window,win16_scroll_info_fn get_info,win16_scroll_layout *layout);
BOOL win16_scroll_embedded_layout(HWND window,LONG object,win16_scroll_info_fn get_info,
    win16_scroll_layout *layout,POINT *screen_origin);
typedef struct win16_scroll_actions_view {
    void *context;
    BOOL *detached,*reporting,*frame,*edit,*list;
    unsigned *epoch;
    LONG *report,*object;
    UINT_PTR *timer;
    win16_scroll_track *track;
    void (*draw)(HWND,void *,HDC,LONG,BOOL);
    void (*nonclient)(HWND,void *,HDC);
    void (*caret)(HWND,void *);
} win16_scroll_actions_view;
void win16_scroll_apply_actions(HWND window,const win16_scroll_actions_view *view,
    const win16_scroll_actions *actions,unsigned epoch);
void win16_scroll_finish(HWND window,const win16_scroll_actions_view *view,HWND *previous,BOOL cancel);
typedef struct win16_scroll_input_view {
    win16_scroll_actions_view actions;
    HWND *previous_capture;
    BOOL *key;
    BOOL (*layout)(HWND,win16_scroll_layout *);
} win16_scroll_input_view;
BOOL win16_scroll_control_input(HWND window,const win16_scroll_input_view *view,
    UINT message,WPARAM wparam,LPARAM lparam);
typedef struct win16_scroll_embedded_view {
    win16_scroll_actions_view actions;
    HWND *previous_capture;
    BOOL (*has_owner)(void *);
    BOOL (*layout)(HWND,LONG,win16_scroll_layout *,POINT *);
    win16_scroll_info_fn get_info;
} win16_scroll_embedded_view;
BOOL win16_scroll_embedded_input(HWND window,const win16_scroll_embedded_view *view,
    UINT message,WPARAM wparam,LPARAM lparam);
typedef struct win16_control_managed_services {
    SUBCLASSPROC proc;
    BOOL (*remove)(HWND);
    BOOL (*get_info)(HWND,int,LPSCROLLINFO);
    int (*set_info)(HWND,int,LPCSCROLLINFO,BOOL);
    void (*draw)(HWND,control_style *,HDC,LONG,BOOL);
    BOOL (*is_list)(HWND),(*is_edit)(HWND);
} win16_control_managed_services;
void win16_control_managed_frame_scrollbars_palette_owned(HWND,const win16_style_palette *,const void *,const win16_control_managed_services *);
void win16_control_managed_frame_scrollbars_remove(HWND,const win16_control_managed_services *);
void win16_control_managed_frame_scrollbars_palette(HWND,const win16_style_palette *,const win16_control_managed_services *);
BOOL win16_control_managed_active(HWND,const win16_control_managed_services *);
BOOL win16_control_managed_get_scroll_info(HWND,int,LPSCROLLINFO,const win16_control_managed_services *);
int win16_control_managed_set_scroll_info(HWND,int,LPCSCROLLINFO,BOOL,const win16_control_managed_services *);
BOOL win16_control_managed_list_scrollbar_enable(HWND,const win16_control_managed_services *);
BOOL win16_control_managed_edit_scrollbars_enable(HWND,const win16_control_managed_services *);
#endif
