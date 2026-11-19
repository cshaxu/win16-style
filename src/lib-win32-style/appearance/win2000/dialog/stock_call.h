#ifndef WIN2K_DIALOG_STOCK_CALL_H
#define WIN2K_DIALOG_STOCK_CALL_H
#include <windows.h>
#include <commdlg.h>

#include "lib-win32-style/application/dialog/observation.h"

#define STOCK_DECL(S,PRINT,SETUP,FONT,COLOR,FIND) \
BOOL win2k_print_##S(PRINT *,BOOL (WINAPI *)(PRINT *),const style_dialog_windows *); \
BOOL win2k_page_setup_##S(SETUP *,BOOL (WINAPI *)(SETUP *),const style_dialog_windows *); \
BOOL win2k_font_##S(FONT *,BOOL (WINAPI *)(FONT *),const style_dialog_windows *); \
BOOL win2k_color_##S(COLOR *,BOOL (WINAPI *)(COLOR *),const style_dialog_windows *); \
HWND win2k_find_##S(FIND *,HWND (WINAPI *)(FIND *),const style_dialog_windows *,BOOL compact);
STOCK_DECL(a,PRINTDLGA,PAGESETUPDLGA,CHOOSEFONTA,CHOOSECOLORA,FINDREPLACEA)
STOCK_DECL(w,PRINTDLGW,PAGESETUPDLGW,CHOOSEFONTW,CHOOSECOLORW,FINDREPLACEW)
#undef STOCK_DECL
#endif
