#ifndef STYLE_CONTROL_DIALOG_COMBO_H
#define STYLE_CONTROL_DIALOG_COMBO_H
#include <windows.h>
#include <commctrl.h>
/* Same GUI thread, synchronous callbacks borrowed only for install. The
 * pending identity is scoped to the primary combo operation; nested installs
 * restore it before installing native children. Child errors remain advisory.
 * No new exception recovery or rollback of installed appearance is implied. */
typedef enum style_dialog_combo_part {
    STYLE_DIALOG_COMBO,STYLE_DIALOG_COMBO_LIST,STYLE_DIALOG_COMBO_EDIT
} style_dialog_combo_part;
typedef struct style_dialog_combo_ops {
    BOOL (*apply)(HWND,style_dialog_combo_part,BOOL border,const void *owner);
    BOOL (*get_info)(HWND,PCOMBOBOXINFO);
} style_dialog_combo_ops;
HWND style_control_dialog_combo_pending(void);
BOOL style_control_dialog_combo_install(HWND,const void *,const style_dialog_combo_ops *);
#endif
