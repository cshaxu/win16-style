#ifndef WIN16_CONTROL_STATUS_FRAME_H
#define WIN16_CONTROL_STATUS_FRAME_H
#include <windows.h>
typedef struct win16_control_status_frame_view {
    void *owner; HWND window; HDC dc; HMODULE native;
    unsigned epoch,count; const unsigned *owner_epoch; const BOOL *detached; const RECT *panes;
    BOOL (*identity)(HWND,void *,HMODULE *);
} win16_control_status_frame_view;
void win16_control_status_text_frame(const win16_control_status_frame_view *,HDC,const RECT *,UINT,const void *);
void win16_control_status_icon_frame(const win16_control_status_frame_view *,HDC,int,int,int,int,HICON,const void *);
void win16_control_status_frame(const win16_control_status_frame_view *,HDC,const RECT *,const RECT *,const void *);
typedef struct win16_control_status_snapshot_view {
    void *owner; const BOOL *detached; const unsigned *owner_epoch; unsigned epoch;
    HMODULE *native; BOOL *simple; int *simple_length; WCHAR *simple_text;
    unsigned *text_count; RECT *text_panes; unsigned *count; RECT *panes;
    BOOL (*identity)(HWND,void *,HMODULE *);
} win16_control_status_snapshot_view;
void win16_control_status_invalidate(UINT,BOOL *,unsigned *);
void win16_control_status_snapshot(HWND,const win16_control_status_snapshot_view *);
#endif
