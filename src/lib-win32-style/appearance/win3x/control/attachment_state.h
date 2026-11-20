#ifndef WIN16_CONTROL_ATTACHMENT_STATE_H
#define WIN16_CONTROL_ATTACHMENT_STATE_H
#include "../common/palette.h"
#include "lib-win32-style/application/adapters/win32/attachment.h"
#include "lib-win32-style/application/lifecycle/instance.h"
#include <commctrl.h>
#include "status_font.h"
#include "lib-win32-style/application/control/runtime.h"
#include "lib-win32-style/application/control/control_status.h"
#include "lib-win32-style/application/control/control_native_combo.h"
#include "../../../application/control/control_protocol.h"
#include "../../../application/graphics/clip.h"
#include "vb_text.h"
#include "lib-win32-style/application/adapters/win32/paint_dispatch.h"
#include "vcl_identity.h"
#include "special_paint.h"
typedef struct attachment {
    style_runtime_attachment base;
    BOOL file_owner,picture,button_owner,checklist;
    win16_style_palette palette;
    win16_runtime_push push;
    classic_bar_colors grip;
    COLORREF calendar_selection;
    COLORREF status_edge;
    COLORREF date_button_face;
    HFONT month_cached_font;style_font_snapshot_cache status_font;
    style_font_cache fonts;
    /* Appended only to attachments whose native role is a rich month control. */
    style_scroll_arrow_art month_arrows[];
} attachment;
typedef struct win16_attachment_guard_services {
    BOOL (*host_attached)(HWND);
    BOOL (*cool_identity)(HWND,attachment *);
    BOOL (*remove)(HWND);
} win16_attachment_guard_services;
BOOL win16_attachment_vcl_combo_candidate(HWND,attachment *);
BOOL win16_attachment_group_push_button(HWND,attachment *);
BOOL win16_attachment_group_choice_button(HWND,attachment *);
BOOL win16_attachment_vb_data_valid(HWND,attachment *);
BOOL win16_attachment_vb_data_attached(HWND,attachment *);
BOOL win16_attachment_vb_user_label_target(HWND,attachment *);
BOOL win16_attachment_vb_user_edge_dormant(HWND,attachment *);
BOOL win16_attachment_vb_user_edge_valid(HWND,attachment *);
void win16_attachment_guard_cool(HWND,UINT,attachment *,const win16_attachment_guard_services *);
void win16_attachment_guard_roles(HWND,UINT,attachment *,const win16_attachment_guard_services *);
typedef BOOL (*win16_attachment_special_install)(HWND,const win16_style_palette *,const void *,const BOOL *);
typedef struct win16_attachment_install_services {
    win16_attachment_special_install legacy_progress,toolbar,updown,trackbar,progress,tree,picture,grid,frame_scrollbars;
    BOOL (*control)(HWND,BOOL,BOOL,BOOL,const win16_style_palette *,const void *,const BOOL *);
    win16_attachment_special_install frame;
} win16_attachment_install_services;
BOOL win16_attachment_install_render(HWND,attachment *,style_control_protocol *,DWORD,BOOL,BOOL,BOOL,BOOL,BOOL,BOOL,const win16_attachment_install_services *);
BOOL win16_attachment_inplace_paint(attachment *,HWND,HDC);
typedef struct win16_attachment_initial_classification {
    BOOL edit,control,report,tree,legacy_progress;
    style_control_protocol *protocol;
    HMODULE owner;
} win16_attachment_initial_classification;
BOOL win16_attachment_classify_initial(HWND,DWORD,BOOL,BOOL,BOOL,BOOL,win16_attachment_initial_classification *);
void win16_attachment_classify_vcl(HWND,DWORD,BOOL,BOOL,BOOL,BOOL,BOOL,const win16_attachment_initial_classification *,style_control_runtime_candidates *);
int win16_attachment_eligibility(DWORD,BOOL,BOOL,BOOL,BOOL,BOOL,const win16_attachment_initial_classification *,const style_control_runtime_candidates *,const style_control_runtime_extras *);
#endif
