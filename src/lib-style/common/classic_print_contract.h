#ifndef STYLE_CLASSIC_PRINT_CONTRACT_H
#define STYLE_CLASSIC_PRINT_CONTRACT_H
#include <windows.h>
#include <commdlg.h>
/* The caller supplies the supported flag mask and temporary hook. The result
 * preserves the caller's original hook and hook flag. */
BOOL style_classic_print_prepare_a(const PRINTDLGA *,PRINTDLGA *,DWORD,LPPRINTHOOKPROC);
BOOL style_classic_print_prepare_w(const PRINTDLGW *,PRINTDLGW *,DWORD,LPPRINTHOOKPROC);
void style_classic_print_result_a(PRINTDLGA *,PRINTDLGA *);
void style_classic_print_result_w(PRINTDLGW *,PRINTDLGW *);
#endif
