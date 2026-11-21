#ifndef WIN16_STYLE_LAYOUT_H
#define WIN16_STYLE_LAYOUT_H
#include <windows.h>
#include "lib-win32-style/appearance/msstyles/classic_style.h"
#include "lib-win32-style/application/window/caption_slots.h"
#include "lib-win32-style/application/window/frame_border.h"

/* Shared geometry for paint and hit-testing; all rectangles window-relative. */
typedef struct win16_style_layout {
    RECT outer, client, caption, title, menu, icon;
    RECT buttons[CLASSIC_BUTTON_CLOSE+1];
    int border, unit;
    SIZE corner,button_size;
    classic_menu_metrics menu_metrics;
    BOOL sizeable, dialog_frame;
} win16_style_layout;
BOOL win16_style_measure(int width, int height, UINT dpi, DWORD style, DWORD ex_style,
    BOOL maximized, win16_style_layout *layout);
BOOL win16_style_measure_with_buttons(int width,int height,UINT dpi,DWORD style,DWORD ex_style,
    BOOL maximized,const classic_button_group *group,const classic_caption_metrics *metrics,
    const style_frame_geometry *frame,UINT monitor_dpi,win16_style_layout *layout);
void win16_style_measure_caption_icon(win16_style_layout *layout,UINT dpi,DWORD style,DWORD extended,unsigned mode);
LRESULT win16_style_hit(const win16_style_layout *layout, POINT point);
#endif
