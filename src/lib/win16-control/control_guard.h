#ifndef WIN16_CONTROL_GUARD_H
#define WIN16_CONTROL_GUARD_H
#include "control_state.h"
typedef struct win16_control_guard_services {
    BOOL (*ntw16s_rich_valid)(HWND),(*ntw16s_vcl_combo_valid)(HWND),(*ntw16s_group_button_valid)(HWND),(*ntw16s_grid_valid)(HWND);
    BOOL (*ntw16s_vb_data_valid)(HWND),(*ntw16s_vb_user_edge_valid)(HWND),(*ntw16s_vcl_toolbar_valid)(HWND),(*ntw16s_vcl_tab_valid)(HWND),(*ntw16s_vb_user_edge_dormant)(HWND);
    BOOL (*is_exact_native_button)(HWND),(*ntw16s_vb_checkbox)(HWND),(*ntw16s_vb_option)(HWND),(*ntw16s_vb_plain_choice)(HWND),(*ntw16s_vb_graphical_choice)(HWND),(*ntw16s_vb_graphical_button)(HWND);
    BOOL (*remove_window)(HWND),(*remove_control)(HWND);
} win16_control_guard_services;
BOOL win16_control_guard_message(HWND,control_style *,UINT,WPARAM,LPARAM,const win16_control_guard_services *,LRESULT *);
#endif
