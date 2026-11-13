#ifndef WIN16_CONTROL_ATTACHMENT_STATE_H
#define WIN16_CONTROL_ATTACHMENT_STATE_H
#include "../win16-common/palette.h"
#include <commctrl.h>
#include "status_font.h"
#include "../common/control_protocol.h"
#include "../common/clip.h"
#include "vb_text.h"
#include "vcl_identity.h"
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
    style_control_protocol *protocol;
} attachment;
typedef struct win16_attachment_guard_services {
    attachment *(*lookup)(HWND);
    BOOL (*mdi_identity)(HWND,HWND,HWND,HMODULE);
    BOOL (*cool_identity)(HWND,attachment *);
    BOOL (*remove)(HWND);
} win16_attachment_guard_services;
HMODULE win16_attachment_observe_native(HWND,UINT,WNDPROC,const void *,attachment *,HWND,
    attachment *,HWND,WNDPROC *,attachment *(*)(HWND));
BOOL win16_attachment_mdi_identity(HWND,HWND,HWND,HMODULE,attachment *(*)(HWND));
BOOL win16_attachment_combo_same_role(HWND,attachment *);
BOOL win16_attachment_vcl_combo_candidate(HWND,attachment *);
BOOL win16_attachment_vcl_combo_valid(HWND,attachment *);
BOOL win16_attachment_group_button_valid(HWND,attachment *);
BOOL win16_attachment_group_push_button(HWND,attachment *);
BOOL win16_attachment_group_choice_button(HWND,attachment *);
BOOL win16_attachment_vb_data_valid(HWND,attachment *);
BOOL win16_attachment_vb_data_attached(HWND,attachment *);
BOOL win16_attachment_vb_user_label_target(HWND,attachment *);
BOOL win16_attachment_vb_user_edge_dormant(HWND,attachment *);
BOOL win16_attachment_vb_user_edge_valid(HWND,attachment *);
BOOL win16_attachment_grid_valid(HWND,attachment *);
BOOL win16_attachment_vcl_toolbar_valid(HWND,attachment *);
BOOL win16_attachment_vcl_tab_valid(HWND,attachment *);
void win16_attachment_dbedit_ansi_witness(WNDPROC,HWND,UINT,const void *,attachment *,HWND,attachment *(*)(HWND));
void win16_attachment_dbcombo_ansi_witness(WNDPROC,HWND,UINT,const void *,attachment *,HWND,attachment *(*)(HWND));
void win16_attachment_retire_frames(HWND,attachment *);
void win16_attachment_guard_cool(HWND,UINT,attachment *,const win16_attachment_guard_services *);
void win16_attachment_guard_roles(HWND,UINT,attachment *,const win16_attachment_guard_services *);
typedef struct win16_attachment_post_services {
    BOOL (*grid_install)(HWND,const win16_style_palette *,const void *,const BOOL *);
    BOOL (*remove_owned)(HWND,const void *);
    BOOL (*rich_identity)(HWND,attachment *);
    BOOL (*get_scroll_info)(HWND,int,LPSCROLLINFO);
} win16_attachment_post_services;
void win16_attachment_post_box(HWND,attachment *,BOOL,const win16_attachment_post_services *);
void win16_attachment_post_controls(HWND,UINT,attachment *,const win16_attachment_post_services *);
typedef struct win16_attachment_verified_services {
    BOOL (*toolbar_install)(HWND,const win16_style_palette *,const void *,const BOOL *);
    BOOL (*control_install)(HWND,BOOL,BOOL,BOOL,const win16_style_palette *,const void *,const BOOL *);
    BOOL (*trackbar_install)(HWND,const win16_style_palette *,const void *,const BOOL *);
    BOOL (*remove_owned)(HWND,const void *);
    BOOL (*combo_same_role)(HWND,attachment *);
    BOOL (*theme_enable)(HWND,const void *);
    BOOL (*edit_scrollbars_enable)(HWND);
    void (*theme_remove)(HWND,const void *);
    void (*vcl_report_try_bind)(HWND,attachment *);
} win16_attachment_verified_services;
void win16_attachment_post_toolbar(HWND,UINT,attachment *,const win16_attachment_verified_services *);
void win16_attachment_post_verified(HWND,UINT,attachment *,const win16_attachment_verified_services *);
typedef struct win16_attachment_native_services {
    BOOL (*color_identity)(HWND,attachment *,COMBOBOXINFO *);
    BOOL (*status_identity)(HWND,attachment *,HMODULE *);
    BOOL (*date_identity)(HWND,attachment *,DATETIMEPICKERINFO *);
    void (*color_button)(HWND,attachment *,HDC,const RECT *);
    void (*date_button)(HWND,attachment *,HDC,const RECT *);
} win16_attachment_native_services;
void win16_attachment_native_prepare(HWND,UINT,WPARAM,attachment *,BOOL,RECT *,BOOL *,style_clip_snapshot *);
void win16_attachment_post_native(HWND,UINT,WPARAM,attachment *,style_clip_snapshot *,BOOL,const RECT *,const win16_attachment_native_services *);
typedef BOOL (*win16_attachment_special_install)(HWND,const win16_style_palette *,const void *,const BOOL *);
typedef struct win16_attachment_install_services {
    win16_attachment_special_install legacy_progress,toolbar,updown,trackbar,progress,tree,picture,grid,frame_scrollbars;
    BOOL (*control)(HWND,BOOL,BOOL,BOOL,const win16_style_palette *,const void *,const BOOL *);
    win16_attachment_special_install frame;
    BOOL (*get_combo_info)(HWND,PCOMBOBOXINFO);
    BOOL (*theme_enable)(HWND,const void *),(*list_scrollbar_enable)(HWND),(*edit_scrollbar_enable)(HWND);
    void (*theme_remove)(HWND,const void *);
    BOOL (*remove)(HWND);
} win16_attachment_install_services;
BOOL win16_attachment_install_style(HWND,attachment *,style_control_protocol *,DWORD,BOOL,BOOL,BOOL,BOOL,BOOL,BOOL,DWORD *,const win16_attachment_install_services *);
typedef struct win16_attachment_report_services {
    SUBCLASSPROC notify;
    win16_attachment_special_install tree_install,frame_install;
    BOOL (*remove)(HWND);
} win16_attachment_report_services;
void win16_attachment_report_bind(HWND,attachment *,const win16_attachment_report_services *);
void win16_attachment_vcl_report_bind(HWND,attachment *,const win16_attachment_report_services *);
void win16_attachment_report_unbind(attachment *,const win16_attachment_report_services *);
typedef struct win16_attachment_lifecycle_services {
    attachment *(*lookup)(HWND);
    BOOL (*cool_identity)(HWND,attachment *),(*mdi_identity)(HWND,HWND,HWND,HMODULE);
    BOOL (*controlbar_identity)(HWND,attachment *),(*rich_identity)(HWND,attachment *);
    BOOL (*vb_user_label_target)(HWND);
    SUBCLASSPROC proc;
    void (*report_unbind)(attachment *),(*cool_unbind)(attachment *);
    BOOL (*frame_remove_owned)(HWND,const void *),(*control_remove_owned)(HWND,const void *);
    void (*theme_remove)(HWND,const void *);
    BOOL (*frame_terminal)(HWND),(*control_terminal)(HWND),(*theme_terminal)(HWND,const void *);
    void (*release)(attachment *);
    void (*scrollbars_paint)(HWND,HDC,const win16_style_palette *);
} win16_attachment_lifecycle_services;
BOOL win16_attachment_attached(HWND,const win16_attachment_lifecycle_services *);
void win16_attachment_scroll_changed(HWND,const win16_attachment_lifecycle_services *);
BOOL win16_attachment_remove(HWND,const win16_attachment_lifecycle_services *);
BOOL win16_attachment_terminal(HWND,const win16_attachment_lifecycle_services *);
typedef struct checklist_draw_scope {
    struct checklist_draw_scope *previous;
    attachment *owner;
    HDC dc;
    RECT item;
} checklist_draw_scope;
BOOL win16_attachment_cool_identity(HWND,attachment *,attachment *(*)(HWND));
BOOL win16_attachment_inplace_paint(attachment *,HWND,HDC);
BOOL win16_attachment_checklist_draw_target(checklist_draw_scope *,HDC,const RECT *,const void *);
void win16_attachment_box_witness(HWND,UINT,const void *,attachment *,HWND,BOOL *);
BOOL win16_attachment_rich_valid(HWND,attachment *,BOOL (*)(HWND,attachment *));
BOOL win16_attachment_rich_witness(WNDPROC,HWND,UINT,const void *,BOOL,attachment *,HWND,BOOL (*)(HWND,attachment *));
typedef struct win16_attachment_notify_services {
    SUBCLASSPROC notify;
    LRESULT (*bit_draw_dispatch)(attachment *,HWND,UINT,WPARAM,LPARAM,const DRAWITEMSTRUCT *);
    void (*release)(attachment *);
    BOOL (*user_label_target)(HWND);
    checklist_draw_scope **checklist_current;
} win16_attachment_notify_services;
LRESULT win16_attachment_report_notify(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR,const win16_attachment_notify_services *);
typedef struct win16_attachment_initial_classification {
    BOOL edit,control,report,tree,legacy_progress;
    style_control_protocol *protocol;
    HMODULE owner;
} win16_attachment_initial_classification;
BOOL win16_attachment_classify_initial(HWND,DWORD,BOOL,BOOL,BOOL,BOOL,win16_attachment_initial_classification *);
void win16_attachment_classify_vcl(HWND,DWORD,BOOL,BOOL,BOOL,BOOL,BOOL,const win16_attachment_initial_classification *,win16_control_vcl_candidates *);
int win16_attachment_eligibility(DWORD,BOOL,BOOL,BOOL,BOOL,BOOL,const win16_attachment_initial_classification *,const win16_control_vcl_candidates *,const win16_control_vcl_extra_candidates *);
typedef struct win16_attachment_seed {
    const win16_attachment_initial_classification *initial;
    const win16_control_vcl_candidates *vcl;
    const win16_control_vcl_extra_candidates *extra;
    BOOL picture,checklist,status,spin,vcl_mdi_probe,lookup_popup_candidate;
    HWND popup_source,popup_source_parent;
    attachment *popup_source_attachment;
    attachment *(*lookup)(HWND);
} win16_attachment_seed;
void win16_attachment_initialize(HWND,DWORD,attachment *,const win16_attachment_seed *);
typedef enum win16_attachment_dispatch {
    WIN16_ATTACHMENT_DISPATCH_NATIVE,WIN16_ATTACHMENT_DISPATCH_COOL,WIN16_ATTACHMENT_DISPATCH_STATUS,
    WIN16_ATTACHMENT_DISPATCH_COLOR,WIN16_ATTACHMENT_DISPATCH_SPIN,WIN16_ATTACHMENT_DISPATCH_BOX,
    WIN16_ATTACHMENT_DISPATCH_LOOKUP,WIN16_ATTACHMENT_DISPATCH_STATIC
} win16_attachment_dispatch;
BOOL win16_attachment_static_paint(const attachment *,UINT);
BOOL win16_attachment_spin_message(const attachment *,UINT);
void win16_attachment_prepare_report(HWND,UINT,WPARAM,attachment *,ntw16s_vb_paint *,void (*)(HWND,attachment *));
void win16_attachment_reset_witness(attachment *,UINT);
win16_attachment_dispatch win16_attachment_dispatch_kind(HWND,attachment *,UINT,WPARAM,LPARAM,BOOL,BOOL,BOOL,BOOL);
#endif
