/* Preserve eligible PrintDlg contracts while selecting the traditional layout. */
#include "classic_print_contract.h"
#define PRINT_CONTRACT(suffix,type) \
BOOL style_classic_print_prepare_##suffix(const type *parameters,type *copy,DWORD compatible,LPPRINTHOOKPROC hook) \
{ \
    if(!parameters || !copy || !hook || parameters->lStructSize!=sizeof(type) || (parameters->Flags & ~compatible))return FALSE; \
    *copy=*parameters;copy->Flags|=PD_ENABLEPRINTHOOK;copy->lpfnPrintHook=hook;return TRUE; \
} \
void style_classic_print_result_##suffix(type *parameters,type *copy) \
{ \
    copy->Flags=(copy->Flags & ~PD_ENABLEPRINTHOOK)|(parameters->Flags & PD_ENABLEPRINTHOOK); \
    copy->lpfnPrintHook=parameters->lpfnPrintHook;*parameters=*copy; \
}
PRINT_CONTRACT(a,PRINTDLGA)
PRINT_CONTRACT(w,PRINTDLGW)
