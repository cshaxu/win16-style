#ifndef WIN16_CONTROL_FRAME_LAYOUT_H
#define WIN16_CONTROL_FRAME_LAYOUT_H
#include "../win16-common/layout.h"
BOOL win16_control_measure_window(HWND window,const UINT *dpi,HFONT *menu_font,const BOOL *detached,
    const RECT *outer,win16_style_layout *layout);
BOOL win16_control_frame_hit_test(HWND window,const win16_style_layout *layout,LPARAM point,LRESULT *result);
void win16_control_frame_changed(HWND window);
void win16_control_client_screen(HWND window,RECT *client);
void win16_control_restore_client(HWND window,const RECT *wanted);
typedef HRESULT (WINAPI *win16_control_set_window_attribute)(HWND,DWORD,LPCVOID,DWORD);
void win16_control_frame_activate(HWND window,const BOOL *detached,const RECT *client,BOOL normal,win16_control_set_window_attribute set_attribute);
void win16_control_frame_restore(HWND window,const BOOL *pending,const BOOL *policy_saved,const DWORD *old_policy,const RECT *client,BOOL normal,win16_control_set_window_attribute set_attribute);
BOOL win16_control_frame_set_cursor(HWND window,WPARAM target,LPARAM hit);
#endif
