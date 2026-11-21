#ifndef STYLE_NATIVE_SCROLL_H
#define STYLE_NATIVE_SCROLL_H
#include "scroll_message.h"
#include "scroll_palette.h"
#include "lib-win32-style/application/graphics/paint_frame.h"

typedef struct style_native_scroll {
    style_scroll_controller controller;
    style_scroll_policy policy;
    style_instance *instance;
    style_relief_palette arrow_palette;
    style_scroll_arrow_art arrow_art;
    style_native_scroll_colors colors;
    style_scroll_metrics metrics;
} style_native_scroll;

void style_native_scroll_init(style_native_scroll *,style_instance *);
void style_native_scroll_configure(style_native_scroll *,style_relief_palette,const style_scroll_arrow_art *,
    const style_native_scroll_colors *,style_scroll_metrics);
style_scroll_ops style_native_scroll_operations(style_native_scroll *);
void style_native_scroll_paint_nc(HWND,style_native_scroll *,HDC);
void style_native_scroll_sizebox(HWND,void *,HDC,BOOL);
void style_native_scroll_retire(HWND,style_native_scroll *,BOOL terminal);
#endif
