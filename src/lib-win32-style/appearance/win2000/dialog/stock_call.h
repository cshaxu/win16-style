#ifndef WIN2K_DIALOG_STOCK_CALL_H
#define WIN2K_DIALOG_STOCK_CALL_H
#include <windows.h>
#include <commdlg.h>

typedef struct win2k_stock_windows {
    void (*initialized)(HWND);
    BOOL (*attached)(HWND);
} win2k_stock_windows;

#define STOCK_DECL(S,PRINT,SETUP,FONT,COLOR,FIND) \
BOOL win2k_print_##S(PRINT *,BOOL (WINAPI *)(PRINT *),const win2k_stock_windows *); \
BOOL win2k_page_setup_##S(SETUP *,BOOL (WINAPI *)(SETUP *),const win2k_stock_windows *); \
BOOL win2k_font_##S(FONT *,BOOL (WINAPI *)(FONT *),const win2k_stock_windows *); \
BOOL win2k_color_##S(COLOR *,BOOL (WINAPI *)(COLOR *),const win2k_stock_windows *); \
HWND win2k_find_##S(FIND *,HWND (WINAPI *)(FIND *),const win2k_stock_windows *,BOOL compact);
STOCK_DECL(a,PRINTDLGA,PAGESETUPDLGA,CHOOSEFONTA,CHOOSECOLORA,FINDREPLACEA)
STOCK_DECL(w,PRINTDLGW,PAGESETUPDLGW,CHOOSEFONTW,CHOOSECOLORW,FINDREPLACEW)
#undef STOCK_DECL
#endif
