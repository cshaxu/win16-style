#ifndef STYLE_CONTROL_COMBO_H
#define STYLE_CONTROL_COMBO_H
#include "basic.h"

typedef struct style_control_combo_ops {
    HFONT (*acquire)(HFONT);
    style_control_state *(*lookup)(HWND);
} style_control_combo_ops;
typedef struct style_control_combo {
    HWND list,edit,parent;
    HFONT edit_font,edit_source,fonts[32];
    unsigned font_count;
    BOOL bound;
    style_instance *instance;
    style_control_state *state;
    const style_control_combo_ops *ops;
} style_control_combo;

/* GUI thread only. The surrounding instance is pinned by the caller through
 * bind/unbind callbacks. An installed edit subclass adds its own reference;
 * synchronous messages add another. Fonts remain leased until final disposal,
 * including borrowed WM_GETFONT results. The existing pool limit is 32. */
BOOL style_control_combo_bind(style_control_combo *,style_instance *,style_control_state *,HWND,HWND,const style_control_combo_ops *);
void style_control_combo_unbind(style_control_combo *,BOOL);
void style_control_combo_dispose(style_control_combo *,const style_control_state *);
/* Child identity belongs to the public control state. Replaced attachments
 * do not inherit a retired parent's ownership. HWNDs remain borrowed. */
void style_control_combo_list_bind(style_control_combo *,style_instance *,HWND,style_control_state *(*)(HWND));
BOOL style_control_combo_list_retire(style_control_combo *,style_instance *,style_control_state *(*)(HWND),BOOL (*)(HWND));
typedef struct style_control_combo_setup_ops {
    void *context;
    BOOL (*get_info)(HWND,PCOMBOBOXINFO);
    BOOL (*install_list)(HWND,void *);
    BOOL (*remove)(HWND);
    BOOL (*edit)(HWND,HWND,void *);
} style_control_combo_setup_ops;
/* Caller pins the parent. This transaction owns child publication/cancellation
 * and parent rollback; edit provides the complete supported edit operation. */
BOOL style_control_combo_setup(HWND,style_instance *,style_control_combo *,const style_control_combo_ops *,const style_control_combo_setup_ops *);
/* Same native COMBOBOX redraw/input protocol, independent of edit-child binding. */
LRESULT style_control_combo_message(HWND,style_instance *,style_control_state *,UINT,WPARAM,LPARAM,const style_control_draw *);
#endif
