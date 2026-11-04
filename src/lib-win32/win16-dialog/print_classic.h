#ifndef WIN16_DIALOG_PRINT_CLASSIC_H
#define WIN16_DIALOG_PRINT_CLASSIC_H
#include <windows.h>
#include <commdlg.h>
BOOL win16_dialog_print_prepare_a(const PRINTDLGA *,PRINTDLGA *);
BOOL win16_dialog_print_prepare_w(const PRINTDLGW *,PRINTDLGW *);
void win16_dialog_print_result_a(PRINTDLGA *,PRINTDLGA *);
void win16_dialog_print_result_w(PRINTDLGW *,PRINTDLGW *);
#endif
