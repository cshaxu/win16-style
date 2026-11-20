#ifndef STYLE_CLASSIC_PRINT_CONTRACT_H
#define STYLE_CLASSIC_PRINT_CONTRACT_H
#include <windows.h>
#include <commdlg.h>
#include "print_extended.h"
#define STYLE_CLASSIC_PRINT_FLAGS (PD_ALLPAGES|PD_SELECTION|PD_PAGENUMS|PD_NOSELECTION|PD_NOPAGENUMS|PD_COLLATE|PD_PRINTTOFILE|PD_RETURNDC|PD_RETURNIC|PD_NOWARNING|PD_USEDEVMODECOPIESANDCOLLATE|PD_DISABLEPRINTTOFILE|PD_HIDEPRINTTOFILE|PD_NONETWORKBUTTON)
/* The caller supplies the supported flag mask and temporary hook. The result
 * preserves the caller's original hook and hook flag. */
BOOL style_classic_print_prepare_a(const PRINTDLGA *,PRINTDLGA *,DWORD,LPPRINTHOOKPROC);
BOOL style_classic_print_prepare_w(const PRINTDLGW *,PRINTDLGW *,DWORD,LPPRINTHOOKPROC);
BOOL style_classic_print_call_a(PRINTDLGA *,style_print_dialog_a);
BOOL style_classic_print_call_w(PRINTDLGW *,style_print_dialog_w);
void style_classic_print_result_a(PRINTDLGA *,PRINTDLGA *);
void style_classic_print_result_w(PRINTDLGW *,PRINTDLGW *);
#endif
