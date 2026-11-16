#ifndef STYLE_CONTROL_COMBO_SNAPSHOT_H
#define STYLE_CONTROL_COMBO_SNAPSHOT_H
#include "../lifecycle/instance.h"
typedef struct style_control_combo_snapshot {
    COMBOBOXINFO info;
    WCHAR *text;
    HFONT font;
    BOOL pressed,simple,editable;
} style_control_combo_snapshot;
typedef struct style_control_combo_reader {
    void *context;
    BOOL (*get_info)(HWND,PCOMBOBOXINFO);
    HFONT (*font)(HWND,void *);
} style_control_combo_reader;
/* Caller pins instance. Only copied text is owned; child HWNDs/font are
 * borrowed native data. clear accepts empty or failed snapshots. */
BOOL style_control_combo_snapshot_read(HWND,style_instance *,const style_control_combo_reader *,style_control_combo_snapshot *);
void style_control_combo_snapshot_clear(style_control_combo_snapshot *);
#endif
