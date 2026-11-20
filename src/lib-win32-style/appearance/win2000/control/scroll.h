#ifndef WIN2K_SCROLL_H
#define WIN2K_SCROLL_H
#include "lib-win32-style/application/control/scroll_message.h"
#include "../common/palette.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"

typedef struct win2k_scroll {
    style_scroll_controller controller;
    style_scroll_policy policy;
    style_instance *instance;
    win2k_palette arrow_palette;
    style_scroll_arrow_art arrow_art;
    classic_scroll_colors colors;
    classic_scroll_metrics metrics;
} win2k_scroll;

void win2k_scroll_init(win2k_scroll *,style_instance *);
void win2k_scroll_theme(win2k_scroll *,const classic_theme *);
style_scroll_ops win2k_scroll_operations(win2k_scroll *);
void win2k_scroll_paint_nc(HWND,win2k_scroll *,HDC);
void win2k_scroll_sizebox(HWND,void *,HDC,BOOL);
void win2k_scroll_retire(HWND,win2k_scroll *,BOOL terminal);
#endif
