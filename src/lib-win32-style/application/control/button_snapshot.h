#ifndef STYLE_CONTROL_BUTTON_SNAPSHOT_H
#define STYLE_CONTROL_BUTTON_SNAPSHOT_H
#include "../lifecycle/instance.h"
typedef struct style_control_button_role {UINT type;BOOL label;} style_control_button_role;
typedef struct style_control_button_snapshot {
    DWORD style;
    style_control_button_role role;
    LRESULT state,check;
    HICON icon;
    HFONT font;
    WCHAR *text;
} style_control_button_snapshot;
typedef struct style_control_button_reader {
    void *context;
    style_control_button_role (*role)(HWND,void *,DWORD);
    HFONT (*font)(HWND,void *);
} style_control_button_reader;
/* Caller pins instance. read owns only the copied text; icon/font stay borrowed.
 * Failed reads clear owned text; clear also accepts a successful icon snapshot. */
BOOL style_control_button_snapshot_read(HWND,style_instance *,const style_control_button_reader *,style_control_button_snapshot *);
void style_control_button_snapshot_clear(style_control_button_snapshot *);
typedef struct style_control_button_image {
    UINT type;
    HANDLE image;
    BUTTON_IMAGELIST images;
    BOOL listed;
    SIZE size;
} style_control_button_image;
/* Image/list handles remain application-owned; native icon-size helper owns
 * and releases only the temporary bitmaps returned by GetIconInfo. */
BOOL style_control_button_image_read(HWND,style_instance *,UINT,style_control_button_image *);
#endif
