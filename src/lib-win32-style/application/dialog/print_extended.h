#ifndef STYLE_PRINT_EXTENDED_H
#define STYLE_PRINT_EXTENDED_H
#include <windows.h>
#include <commdlg.h>
typedef BOOL (WINAPI *style_print_dialog_a)(PRINTDLGA *);
BOOL style_print_extended_a(PRINTDLGEXA *parameters,style_print_dialog_a print,HRESULT *result);
typedef BOOL (WINAPI *style_print_dialog_w)(PRINTDLGW *);
BOOL style_print_extended_w(PRINTDLGEXW *parameters,style_print_dialog_w print,HRESULT *result);
#endif
