/* Preserve eligible PrintDlg contracts while selecting the traditional layout. */
#include "print_classic.h"
static UINT_PTR CALLBACK classic_print_hook(HWND window,UINT message,WPARAM wparam,LPARAM lparam)
{(void)window;(void)message;(void)wparam;(void)lparam;return 0;}
#define CLASSIC_PRINT_FLAGS (PD_ALLPAGES|PD_SELECTION|PD_PAGENUMS|PD_NOSELECTION|PD_NOPAGENUMS|PD_COLLATE|PD_PRINTTOFILE|PD_RETURNDC|PD_RETURNIC|PD_NOWARNING|PD_USEDEVMODECOPIESANDCOLLATE|PD_DISABLEPRINTTOFILE|PD_HIDEPRINTTOFILE|PD_NONETWORKBUTTON)
#define PRINT_CONTRACT(suffix,type) \
BOOL win16_dialog_print_prepare_##suffix(const type *parameters,type *copy) \
{ \
    if(!parameters || parameters->lStructSize!=sizeof(type) || (parameters->Flags & ~CLASSIC_PRINT_FLAGS))return FALSE; \
    *copy=*parameters;copy->Flags|=PD_ENABLEPRINTHOOK;copy->lpfnPrintHook=classic_print_hook;return TRUE; \
} \
void win16_dialog_print_result_##suffix(type *parameters,type *copy) \
{ \
    copy->Flags=(copy->Flags & ~PD_ENABLEPRINTHOOK)|(parameters->Flags & PD_ENABLEPRINTHOOK); \
    copy->lpfnPrintHook=parameters->lpfnPrintHook;*parameters=*copy; \
}
PRINT_CONTRACT(a,PRINTDLGA)
PRINT_CONTRACT(w,PRINTDLGW)
