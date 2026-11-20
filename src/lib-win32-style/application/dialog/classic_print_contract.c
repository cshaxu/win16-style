/* Preserve eligible PrintDlg contracts while selecting the traditional layout. */
#include "classic_print_contract.h"
static UINT_PTR CALLBACK classic_print_hook(HWND window,UINT message,WPARAM a,LPARAM b)
{(void)window;(void)message;(void)a;(void)b;return 0;}
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
} \
BOOL style_classic_print_call_##suffix(type *parameters,style_print_dialog_##suffix print) \
{ \
    type copy; \
    if(!style_classic_print_prepare_##suffix(parameters,&copy,STYLE_CLASSIC_PRINT_FLAGS,classic_print_hook)) \
        return print(parameters); \
    BOOL result=print(&copy);style_classic_print_result_##suffix(parameters,&copy);return result; \
}
PRINT_CONTRACT(a,PRINTDLGA)
PRINT_CONTRACT(w,PRINTDLGW)
