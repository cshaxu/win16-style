#ifndef WIN16_CONTROL_VCL_IDENTITY_H
#define WIN16_CONTROL_VCL_IDENTITY_H
#include <windows.h>
BOOL win16_control_combo_identity(HWND w,HWND parent);
BOOL win16_control_group_button_identity(HWND w);
BOOL win16_control_grid_identity(HWND w,HMODULE owner,HWND parent);
BOOL win16_control_vcl_toolbar_identity(HWND w,HMODULE owner,HWND parent);
BOOL win16_control_inplace_identity(HWND w,HMODULE owner,HWND parent);
BOOL win16_control_lookup_identity(HWND w,HMODULE owner,HWND parent);
BOOL win16_control_inplace_or_lookup(HWND w,HMODULE owner,HWND parent);
BOOL win16_control_lookup_popup_identity(HWND w,HWND source,HWND source_parent,HMODULE owner);
BOOL win16_control_nav_identity(HWND w,HMODULE owner,HWND parent);
BOOL win16_control_bit_identity(HWND w,HMODULE owner);
BOOL win16_control_checklist_identity(HWND w);
BOOL win16_control_vb_data_role(HWND w,HWND parent);
BOOL win16_control_vb_user_edge_role(HWND w,HWND parent);
BOOL win16_control_vb_user_edge_identity(HWND w,HWND parent);
BOOL win16_control_box_identity(HWND w,HMODULE owner,HWND parent);
typedef struct win16_control_controlbar_view {
    const BOOL *detached,*retired;const HWND *parent;const HMODULE *owner;
} win16_control_controlbar_view;
BOOL win16_control_controlbar_identity(HWND,const win16_control_controlbar_view *);
BOOL win16_control_cool_class_owner(HMODULE);
typedef struct win16_control_rich_view {
    const BOOL *detached,*popup,*hotkey,*month,*unicode;const HWND *parent;const HMODULE *owner;
} win16_control_rich_view;
BOOL win16_control_rich_identity(HWND,const win16_control_rich_view *);
#endif
