#ifndef STYLE_CLASSIC_DIALOG_CONTROLS_H
#define STYLE_CLASSIC_DIALOG_CONTROLS_H
#include <windows.h>
typedef struct style_classic_dialog_controls {
    BOOL (*grid)(HWND,const void *);
    BOOL (*scroll)(HWND);
    BOOL (*remove)(HWND,const void *);
    BOOL (*combo)(HWND,const void *);
} style_classic_dialog_controls;
void style_classic_file_controls(HWND,const void *,const style_classic_dialog_controls *);
void style_classic_font_controls(HWND,const void *,const style_classic_dialog_controls *);
BOOL style_classic_font_template(HWND);
BOOL style_classic_color_template(HWND);
BOOL style_classic_color_swatch(HWND,HDC,const RECT *);
void style_classic_color_controls(HWND,const int *,unsigned,void (*)(HWND));
#endif
