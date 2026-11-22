#ifndef STYLE_ADAPTER_OUTLINE_ATTACHMENT_H
#define STYLE_ADAPTER_OUTLINE_ATTACHMENT_H
#include "lib-win32-style/application/window/palette.h"
#include "lib-win32-style/application/adapters/win32/attachment.h"
#include "lib-win32-style/application/lifecycle/instance.h"
#include <commctrl.h>
#include "lib-win32-style/application/graphics/font_cache.h"
#include "lib-win32-style/application/control/runtime.h"
#include "lib-win32-style/application/control/control_status.h"
#include "lib-win32-style/application/control/control_native_combo.h"
#include "lib-win32-style/application/control/control_protocol.h"
#include "lib-win32-style/application/graphics/clip.h"
#include "lib-win32-style/application/adapters/win32/runtime_text.h"
#include "lib-win32-style/application/adapters/win32/paint_dispatch.h"
#include "lib-win32-style/application/control/control_vcl_identity.h"
#include "lib-win32-style/application/control/outline_special.h"
#include "lib-win32-style/application/control/control_chrome.h"
typedef struct style_outline_attachment {
    style_runtime_attachment base;
    BOOL file_owner,picture,button_owner,checklist;
    style_window_palette palette;
    style_control_push_appearance push;
    classic_bar_colors grip;
    COLORREF calendar_selection;
    unsigned calendar_font_dpi,calendar_font_points;
    unsigned control_font_points;
    COLORREF status_edge;
    COLORREF date_button_face;
    HFONT month_cached_font;style_font_snapshot_cache status_font;
    style_font_cache fonts;
    /* Appended only to attachments whose native role is a rich month control. */
    classic_combo_arrow_art combo_arrow;
    style_scroll_arrow_art arrows[];
} style_outline_attachment;
typedef struct style_outline_attachment_guard_services {
    BOOL (*host_attached)(HWND);
    BOOL (*cool_identity)(HWND,style_outline_attachment *);
    BOOL (*remove)(HWND);
} style_outline_attachment_guard_services;
BOOL style_outline_attachment_vcl_combo_candidate(HWND,style_outline_attachment *);
BOOL style_outline_attachment_group_push_button(HWND,style_outline_attachment *);
BOOL style_outline_attachment_group_choice_button(HWND,style_outline_attachment *);
BOOL style_outline_attachment_vb_data_valid(HWND,style_outline_attachment *);
BOOL style_outline_attachment_vb_data_attached(HWND,style_outline_attachment *);
BOOL style_outline_attachment_vb_user_label_target(HWND,style_outline_attachment *);
BOOL style_outline_attachment_vb_user_edge_dormant(HWND,style_outline_attachment *);
BOOL style_outline_attachment_vb_user_edge_valid(HWND,style_outline_attachment *);
void style_outline_attachment_guard_cool(HWND,UINT,style_outline_attachment *,const style_outline_attachment_guard_services *);
void style_outline_attachment_guard_roles(HWND,UINT,style_outline_attachment *,const style_outline_attachment_guard_services *);
typedef BOOL (*style_outline_attachment_special_install)(HWND,const style_window_palette *,const void *,const BOOL *);
typedef struct style_outline_attachment_install_services {
    style_outline_attachment_special_install legacy_progress,toolbar,updown,trackbar,progress,tree,picture,grid,frame_scrollbars;
    BOOL (*control)(HWND,BOOL,BOOL,BOOL,const style_window_palette *,const void *,const BOOL *);
    style_outline_attachment_special_install frame;
} style_outline_attachment_install_services;
BOOL style_outline_attachment_install_render(HWND,style_outline_attachment *,style_control_protocol *,DWORD,BOOL,BOOL,BOOL,BOOL,BOOL,BOOL,const style_outline_attachment_install_services *);
BOOL style_outline_attachment_inplace_paint(style_outline_attachment *,HWND,HDC);
typedef struct style_outline_attachment_initial_classification {
    BOOL edit,control,report,tree,legacy_progress;
    style_control_protocol *protocol;
    HMODULE owner;
} style_outline_attachment_initial_classification;
BOOL style_outline_attachment_classify_initial(HWND,DWORD,BOOL,BOOL,BOOL,BOOL,style_outline_attachment_initial_classification *);
void style_outline_attachment_classify_vcl(HWND,DWORD,BOOL,BOOL,BOOL,BOOL,BOOL,const style_outline_attachment_initial_classification *,style_control_runtime_candidates *);
int style_outline_attachment_eligibility(DWORD,BOOL,BOOL,BOOL,BOOL,BOOL,const style_outline_attachment_initial_classification *,const style_control_runtime_candidates *,const style_control_runtime_extras *);
#endif
