#ifndef WIN16_CONTROL_SCROLL_NATIVE_H
#define WIN16_CONTROL_SCROLL_NATIVE_H
#include "scroll_layout.h"
#include "lib-win32-style/application/control/scroll_track.h"
#include "control_state.h"
#include "lib-win32-style/application/control/scroll_managed.h"
BOOL win16_scroll_measure_native(const RECT *bar,BOOL vertical,UINT dpi,
    const SCROLLINFO *info,style_scroll_layout *layout);
RECT win16_scroll_caret_rectangle(HWND window,const style_scroll_layout *layout,const style_scroll_track *track);
extern const style_scroll_policy win16_scroll_policy;

typedef BOOL (*win16_scroll_info_fn)(HWND,int,LPSCROLLINFO);
BOOL win16_scroll_control_layout(HWND window,win16_scroll_info_fn get_info,style_scroll_layout *layout);
BOOL win16_scroll_embedded_layout(HWND window,LONG object,win16_scroll_info_fn get_info,
    style_scroll_layout *layout,POINT *screen_origin);
void win16_control_managed_frame_scrollbars_palette_owned(HWND,const win16_style_palette *,const void *,const style_scroll_managed_ops *);
void win16_control_managed_frame_scrollbars_palette(HWND,const win16_style_palette *,const style_scroll_managed_ops *);
#endif
