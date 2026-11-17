#ifndef STYLE_CONTROL_STATUS_PAINT_H
#define STYLE_CONTROL_STATUS_PAINT_H
#include "control_status.h"
/* Synchronous GUI-thread observation. Caller pins instance, state and context;
 * snapshot and theme text are owned values. DC, native modules and HWNDs are
 * borrowed. No renderer/font ownership or hook installation is implied. */
typedef struct style_control_status_scope {
    struct style_control_status_scope *previous;
    void *context;HWND window;HDC dc;
    const style_instance *instance;
    const style_control_status_state *state;
    LPCWSTR class_name;
    style_control_status_snapshot snapshot;
} style_control_status_scope;
typedef struct style_control_status_theme {
    struct style_control_status_theme *previous;
    const style_control_status_scope *paint;HDC dc;int length;BOOL valid;WCHAR text[513];
} style_control_status_theme;
typedef struct style_control_status_themes {style_control_status_theme *current;} style_control_status_themes;
typedef BOOL (*style_control_status_text_policy)(LPCWSTR,int);
BOOL style_control_status_text_frame(const style_control_status_scope *,HDC,const RECT *,UINT,const void *,RECT *);
BOOL style_control_status_icon_frame(const style_control_status_scope *,HDC,int,int,int,int,HICON,const void *,RECT *);
BOOL style_control_status_frame(const style_control_status_scope *,HDC,const RECT *,const void *,BOOL,HWND);
/* One caller-owned manager per GUI thread. A nested no-match still creates a
 * barrier; end pairs the active head only, including caller native unwind. */
BOOL style_control_status_theme_begin(style_control_status_themes *,style_control_status_theme *,const style_control_status_scope *,HDC,int,int,LPCWSTR,int,DWORD,const RECT *,const void *,style_control_status_text_policy);
void style_control_status_theme_end(style_control_status_themes *,style_control_status_theme *);
BOOL style_control_status_text_matches(const style_control_status_scope *,const style_control_status_theme *,HDC,LPCWSTR,int,UINT,const void *,style_control_status_text_policy);
#endif
