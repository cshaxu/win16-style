#ifndef WIN16_CONTROL_GUARD_H
#define WIN16_CONTROL_GUARD_H
#include "control_state.h"
typedef struct win16_control_guard_services {
    BOOL (*rich_valid)(HWND),(*vcl_combo_valid)(HWND),(*group_button_valid)(HWND),(*grid_valid)(HWND);
    BOOL (*vb_data_valid)(HWND),(*vb_user_edge_valid)(HWND),(*vcl_toolbar_valid)(HWND),(*vcl_tab_valid)(HWND),(*vb_user_edge_dormant)(HWND);
    BOOL (*is_exact_native_button)(HWND),(*vb_checkbox)(HWND),(*vb_option)(HWND),(*vb_plain_choice)(HWND),(*vb_graphical_choice)(HWND),(*vb_graphical_button)(HWND);
    BOOL (*remove_window)(HWND),(*remove_control)(HWND);
} win16_control_guard_services;
BOOL win16_control_supported(HWND,HWND);
BOOL win16_control_guard_message(HWND,control_style *,UINT,WPARAM,LPARAM,const win16_control_guard_services *,LRESULT *);
#endif
