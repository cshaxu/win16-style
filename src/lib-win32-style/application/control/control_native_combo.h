#ifndef STYLE_CONTROL_NATIVE_COMBO_H
#define STYLE_CONTROL_NATIVE_COMBO_H
#include <windows.h>
#include <commctrl.h>
#include "../lifecycle/instance.h"
/* GUI-thread native relation and first-paint observation state. Parent and
 * module are borrowed; the containing instance pins this object across queries.
 * Candidate admission belongs to the caller's existing presentation policy. */
typedef struct style_control_native_combo_state {
    BOOL candidate,attempted,installed;
    HWND parent;
    HMODULE owner;
    BOOL native_legacy_date;
} style_control_native_combo_state;
BOOL style_control_color_identity(HWND,const style_instance *,const style_control_native_combo_state *,COMBOBOXINFO *);
BOOL style_control_date_identity(HWND,const style_instance *,const style_control_native_combo_state *,DATETIMEPICKERINFO *);
/* Observe once at WM_PAINT, preserving entry LastError. Failed evidence is
 * not installed; no native state or application resource is replaced. */
void style_control_native_combo_settle(HWND,UINT,const style_instance *,style_control_native_combo_state *,BOOL date);
/* Value snapshot of the documented native button; no application content or
 * resource ownership is transferred. Caller pins instance/state during reads. */
typedef struct style_control_native_button {RECT bounds;BOOL pressed;} style_control_native_button;
BOOL style_control_native_button_read(HWND,const style_instance *,const style_control_native_combo_state *,BOOL date,style_control_native_button *);
typedef void (*style_control_native_button_painter)(HWND,const style_instance *,HDC,const RECT *,const style_control_native_button *);
/* The caller supplies the policy that recognizes an eligible parent combo. */
BOOL style_control_combo_edit(HWND,BOOL (*accept_parent)(HWND));
/* The classifier and eligible parent are supplied by the caller. */
BOOL style_control_dialog_combo_valid(HWND,HWND,BOOL (*is_combo)(HWND));
#endif
