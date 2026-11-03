#ifndef WIN16_CONTROL_VCL_FRAME_POLICY_H
#define WIN16_CONTROL_VCL_FRAME_POLICY_H
#include <windows.h>
typedef struct win16_control_vcl_frame_scope {
    BOOL group_background;int group_state;RECT group_rect;
    BOOL panel_first,panel_enabled;HWND panel_parent;HDC panel_dc;POINT panel_points[3];COLORREF panel_color;
} win16_control_vcl_frame_scope;
typedef struct win16_control_vcl_frame_result {void *owner;HWND parent;} win16_control_vcl_frame_result;
typedef struct win16_control_vcl_frame_view {
    win16_control_vcl_frame_scope *scope;void *owner;HWND window;
    const BOOL *detached,*candidate,*retired;const HWND *parent;
    BOOL (*owned)(HWND,void *);BOOL (*current)(win16_control_vcl_frame_scope *);void (*drawn)(void *);
} win16_control_vcl_frame_view;
void win16_control_vcl_group_frame(const win16_control_vcl_frame_view *,HDC,int,int,const RECT *,const RECT *,LPCWSTR,int,DWORD,const void *,BOOL);
void win16_control_vcl_panel_polyline(const win16_control_vcl_frame_view *,HDC,const POINT *,int,const void *,BOOL (WINAPI *)(HDC,const POINT *,int));
#endif
