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
typedef struct win16_control_vcl_candidates {
    BOOL static_candidate,scroll_candidate,edit_candidate,tab_candidate,vcl_report_candidate;
    BOOL track_candidate,color_candidate,bit_candidate,inplace_candidate,nav_candidate;
    BOOL group_frame_candidate,panel_frame_candidate,group_button_candidate,grid_candidate;
    BOOL vcl_toolbar_candidate,status_candidate,date_candidate;
} win16_control_vcl_candidates;
void win16_control_vcl_classify(HWND,DWORD,BOOL,HMODULE,win16_control_vcl_candidates *);
typedef struct win16_control_vcl_extra_candidates {
    BOOL cool_candidate,box_candidate,controlbar_candidate;
    BOOL rich_month,rich_hotkey,rich_popup,rich_unicode,rich_candidate;
    BOOL vb_user_edge,vb_data_edge,vb_mdi_child;
} win16_control_vcl_extra_candidates;
void win16_control_vcl_classify_extra(HWND,DWORD,BOOL,HMODULE,win16_control_vcl_extra_candidates *);
#endif
