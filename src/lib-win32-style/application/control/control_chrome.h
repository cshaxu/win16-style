#ifndef STYLE_CONTROL_CHROME_H
#define STYLE_CONTROL_CHROME_H
#include "../window/palette.h"
#include "control_colors.h"
#include "lib-win32-style/application/graphics/paint_frame.h"
style_relief_palette style_control_outline_palette(const style_window_palette *);
void style_control_outline_scroll_arrow(HDC,RECT,UINT,const style_window_palette *,const style_scroll_arrow_art *);
void style_control_combo_arrow(HDC dc,int left,int top,int dpi,const classic_combo_arrow_art *art);
typedef struct style_control_push_appearance {
    classic_push_colors colors;
    style_push_profile profile;
} style_control_push_appearance;
BOOL style_control_bit_button_face(HWND window,HDC dc,const RECT *bounds,const RECT *clip,UINT state,
    const style_window_palette *,const style_control_push_appearance *);
BOOL style_control_push_frame(HWND,HDC,RECT,UINT,const style_window_palette *,const style_control_push_appearance *);

static inline style_relief_palette style_control_relief_palette(const style_frame_colors *frame,
    const classic_list_colors *list,const classic_text_colors *label)
{
    style_relief_palette palette=style_relief_native_palette();
    if(frame && frame->present){
        palette.face=palette.light=frame->face;
        palette.highlight=frame->light;
        palette.shadow=frame->shadow;
        palette.dark=frame->edge;
    }
    if(list && list->present){
        palette.dark=list->dark;
        palette.window=list->background;
        palette.selection=list->selection;
        palette.selection_text=list->selection_text;
    }
    if(label && label->present){
        palette.text=label->normal;
        palette.gray=label->disabled;
    }
    return palette;
}
#endif
