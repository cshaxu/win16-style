#ifndef NTW16S_PRINT_EXTENDED_H
#define NTW16S_PRINT_EXTENDED_H
#include <windows.h>
#include <commdlg.h>
typedef BOOL (WINAPI *ntw16s_print_dialog_a)(PRINTDLGA *);
BOOL ntw16s_print_extended_a(PRINTDLGEXA *parameters,ntw16s_print_dialog_a print,HRESULT *result);
typedef BOOL (WINAPI *ntw16s_print_dialog_w)(PRINTDLGW *);
BOOL ntw16s_print_extended_w(PRINTDLGEXW *parameters,ntw16s_print_dialog_w print,HRESULT *result);
#endif
