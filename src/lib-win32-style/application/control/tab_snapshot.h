#ifndef STYLE_CONTROL_TAB_SNAPSHOT_H
#define STYLE_CONTROL_TAB_SNAPSHOT_H
#include "native.h"
#include "lib-win32-style/application/graphics/paint_frame.h"
#include <commctrl.h>
typedef struct style_control_tab_snapshot {
    RECT client,page;
    int count,selected,focus;
    UINT cues;
    unsigned epoch;
} style_control_tab_snapshot;
typedef struct style_control_tab_item {
    RECT rect;
    int index;
    BOOL active,image;
    TCITEMW native;
    const WCHAR *text;
    HIMAGELIST images;
    SIZE image_size;
} style_control_tab_item;
/* Caller pins instance/state. This operation owns native reads, text and DC
 * isolation; callbacks borrow all payloads for that call only. Surface runs
 * before selection queries, prepare afterwards. Item must not send application
 * messages while using its borrowed image list. No TCIF_PARAM is requested:
 * applications may enlarge their item-data contract with TCM_SETITEMEXTRA. */
typedef struct style_control_tab_painter {
    void *context;
    void (*surface)(HWND,void *,HDC,const style_control_tab_snapshot *);
    void (*prepare)(HWND,void *,HDC,const style_control_tab_snapshot *);
    void (*item)(HWND,void *,HDC,const style_control_tab_snapshot *,const style_control_tab_item *);
} style_control_tab_painter;
BOOL style_control_tab_paint(HWND,HDC,const style_instance *,const style_control_native_state *,const style_control_tab_painter *);
BOOL style_control_tab_supported(HWND);
int style_control_tab_hot_item(HWND);
BOOL style_control_tab_present(HWND,HDC,HFONT,const style_instance *,const style_control_native_state *,
    const style_tab_colors *,style_line_profile,style_tab_text,const style_relief_palette *);
#endif
