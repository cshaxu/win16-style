#ifndef STYLE_STOCK_DIALOG_H
#define STYLE_STOCK_DIALOG_H
#include <windows.h>
#include <commdlg.h>

#include "lib-win32-style/application/dialog/observation.h"

#define STOCK_DECL(S,PRINT,SETUP,FONT,COLOR,FIND) \
BOOL style_observed_print_##S(PRINT *,BOOL (WINAPI *)(PRINT *),const style_dialog_windows *); \
BOOL style_observed_page_setup_##S(SETUP *,BOOL (WINAPI *)(SETUP *),const style_dialog_windows *); \
BOOL style_observed_font_##S(FONT *,BOOL (WINAPI *)(FONT *),const style_dialog_windows *); \
BOOL style_observed_color_##S(COLOR *,BOOL (WINAPI *)(COLOR *),const style_dialog_windows *); \
HWND style_observed_find_##S(FIND *,HWND (WINAPI *)(FIND *),const style_dialog_windows *,BOOL compact);
STOCK_DECL(a,PRINTDLGA,PAGESETUPDLGA,CHOOSEFONTA,CHOOSECOLORA,FINDREPLACEA)
STOCK_DECL(w,PRINTDLGW,PAGESETUPDLGW,CHOOSEFONTW,CHOOSECOLORW,FINDREPLACEW)
#undef STOCK_DECL
#endif
