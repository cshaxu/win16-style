#ifndef STYLE_FULL_CLIENT_LAYOUT_H
#define STYLE_FULL_CLIENT_LAYOUT_H
#include <windows.h>
#include "../menu/row_text.h"
#include "lib-win32-style/application/window/caption_slots.h"
#include "lib-win32-style/application/window/frame_border.h"

/* Shared geometry for paint and hit-testing; all rectangles window-relative. */
typedef struct style_full_client_layout {
    RECT outer, client, caption, title, menu, icon;
    RECT buttons[CLASSIC_BUTTON_CLOSE+1];
    int border, unit;
    SIZE corner,button_size;
    style_menu_metrics menu_metrics;
    BOOL sizeable, dialog_frame;
} style_full_client_layout;
BOOL style_full_client_measure(int width,int height,UINT dpi,DWORD style,DWORD ex_style,
    BOOL maximized,const classic_button_group *group,const classic_caption_metrics *metrics,
    const style_frame_geometry *frame,UINT monitor_dpi,style_full_client_layout *layout);
void style_full_client_caption_icon(style_full_client_layout *layout,UINT dpi,DWORD style,DWORD extended,unsigned mode);
LRESULT style_full_client_hit(const style_full_client_layout *layout, POINT point);
#endif
