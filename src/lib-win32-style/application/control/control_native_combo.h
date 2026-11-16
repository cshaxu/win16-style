#ifndef STYLE_CONTROL_NATIVE_COMBO_H
#define STYLE_CONTROL_NATIVE_COMBO_H
#include <windows.h>
#include <commctrl.h>
/* Callers own these witnesses and keep them valid across synchronous queries. */
typedef struct style_control_native_combo_view {
    const BOOL *detached;
    const HWND *parent;
    const HMODULE *owner;
} style_control_native_combo_view;
BOOL style_control_color_identity(HWND,const style_control_native_combo_view *,COMBOBOXINFO *);
BOOL style_control_date_identity(HWND,const style_control_native_combo_view *,DATETIMEPICKERINFO *);
/* The caller supplies the policy that recognizes an eligible parent combo. */
BOOL style_control_combo_edit(HWND,BOOL (*accept_parent)(HWND));
/* The classifier and eligible parent are supplied by the caller. */
BOOL style_control_dialog_combo_valid(HWND,HWND,BOOL (*is_combo)(HWND));
#endif
