#ifndef WIN16_CONTROL_ATTACHMENT_STATE_H
#define WIN16_CONTROL_ATTACHMENT_STATE_H
#include "status_font.h"
#include "control_protocol.h"
#include "vb_text.h"
typedef struct attachment {
    unsigned calls;
    BOOL detached,tree,file_owner,picture,button_owner,checklist,status,spin,static_candidate,static_verified;
    BOOL edit_candidate,edit_attempted,edit_installed,link_candidate,spinedit_candidate,combo_candidate,modern_spin,modern_spin_verified;
    BOOL scroll_candidate,scroll_attempted,scroll_installed; HWND scroll_parent; HMODULE scroll_witness;
    BOOL group_button_candidate,group_button_attempted,group_button_installed;
    BOOL grid_candidate,grid_attempted,grid_installed;
    BOOL vb_user_candidate,vb_user_attempted,vb_user_edge; HWND vb_user_parent;
    BOOL vb_data_candidate,vb_data_attempted,vb_data_edge; HWND vb_data_parent;
    BOOL vcl_toolbar_candidate,vcl_toolbar_attempted,vcl_toolbar_installed;
    BOOL track_candidate,track_attempted,track_installed;
    BOOL tab_candidate,tab_attempted,tab_installed;
    BOOL vcl_report_candidate,vcl_report_attempted,vcl_report_installed,vcl_report_tree;
    HWND vcl_report_parent; HMODULE vcl_report_owner,vcl_report_witness;
    BOOL color_candidate,color_attempted,color_installed;
    BOOL date_candidate,date_attempted,date_installed;
    BOOL status_candidate,status_attempted,status_installed;
    BOOL box_candidate,box_installed,controlbar_candidate,controlbar_verified,controlbar_retired; HWND box_parent,controlbar_parent; HMODULE box_owner,controlbar_owner;
    BOOL rich_candidate,rich_installed,rich_witness,rich_unicode,rich_hotkey,rich_month,rich_popup; HWND rich_parent; HMODULE rich_owner;
    BOOL bit_candidate,bit_verified; BOOL inplace_candidate,inplace_verified; HWND inplace_parent; HMODULE inplace_owner;
    BOOL nav_candidate,nav_verified; HWND nav_parent; HMODULE nav_owner;
    BOOL cool_candidate,cool_verified; HWND cool_parent; HMODULE cool_owner;
    BOOL mdi_candidate; HWND mdi_parent,mdi_host; HMODULE mdi_owner; struct attachment *mdi_host_attachment;
    BOOL lookup_popup_candidate; HWND lookup_popup_source,lookup_popup_source_parent; HMODULE lookup_popup_owner; struct attachment *lookup_popup_source_attachment;
    BOOL group_frame_candidate,group_frame_verified,group_frame_retired; HWND group_frame_parent;
    BOOL panel_frame_candidate,panel_frame_verified,panel_frame_retired; HWND panel_frame_parent;
    HMODULE status_witness_owner; HWND status_parent;
    HFONT month_cached_font;win16_control_status_font_cache status_font;
    DWORD edit_original_style,combo_original_type;
    HMODULE edit_witness_owner,group_button_witness_owner;
    HWND combo_parent,group_button_parent,grid_parent,vcl_toolbar_parent; HMODULE grid_owner,vcl_toolbar_owner,vcl_toolbar_witness;
    HMODULE track_witness_owner,tab_witness_owner; HWND tab_parent;
    HMODULE color_witness_owner;
    HWND color_parent;
    HMODULE date_witness_owner;
    HWND date_parent;
    unsigned status_epoch;
    HWND themed,report_window,report_parent;
    HMODULE report_module;
    ntw16s_vb_fonts fonts;
    ntw16s_control_protocol *protocol;
} attachment;
typedef struct win16_attachment_guard_services {
    attachment *(*lookup)(HWND);
    BOOL (*mdi_identity)(HWND,HWND,HWND,HMODULE);
    BOOL (*cool_identity)(HWND,attachment *);
    BOOL (*remove)(HWND);
} win16_attachment_guard_services;
void win16_attachment_retire_frames(HWND,attachment *);
void win16_attachment_guard_cool(HWND,UINT,attachment *,const win16_attachment_guard_services *);
void win16_attachment_guard_roles(HWND,UINT,attachment *,const win16_attachment_guard_services *);
#endif
