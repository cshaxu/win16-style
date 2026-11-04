#ifndef WIN16_CONTROL_NATIVE_COMBO_IDENTITY_H
#define WIN16_CONTROL_NATIVE_COMBO_IDENTITY_H
#include <windows.h>
#include <commctrl.h>
typedef struct win16_control_native_combo_view {
    const BOOL *detached;
    const HWND *parent;
    const HMODULE *owner;
} win16_control_native_combo_view;
BOOL win16_control_color_identity(HWND,const win16_control_native_combo_view *,COMBOBOXINFO *);
void win16_control_color_button(HWND,const win16_control_native_combo_view *,HDC,const RECT *);
BOOL win16_control_date_identity(HWND,const win16_control_native_combo_view *,DATETIMEPICKERINFO *);
void win16_control_date_button(HWND,const win16_control_native_combo_view *,HDC,const RECT *);
#endif
