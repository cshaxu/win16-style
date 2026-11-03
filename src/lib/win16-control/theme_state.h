#ifndef WIN16_CONTROL_THEME_STATE_H
#define WIN16_CONTROL_THEME_STATE_H
#include <windows.h>
enum win16_control_theme_kind {WIN16_CONTROL_THEME_OTHER,WIN16_CONTROL_THEME_BUTTON,WIN16_CONTROL_THEME_TEXTGLOW,WIN16_CONTROL_THEME_COMBOBOX};
int win16_control_theme_kind(LPCWSTR);
typedef HRESULT (*win16_control_native_theme_fn)(HWND,LPCWSTR,LPCWSTR);
BOOL win16_control_theme_capture(HWND window);
BOOL win16_control_theme_stage(HWND window);
BOOL win16_control_theme_commit(HWND window);
void win16_control_theme_cancel_stage(HWND window);
BOOL win16_control_theme_terminal(HWND window,const void *owner);
BOOL win16_control_theme_enable(HWND window,const void *owner,win16_control_native_theme_fn native_theme);
BOOL win16_control_theme_retained(HWND window);
void win16_control_theme_remove(HWND window,const void *owner,win16_control_native_theme_fn native_theme);
HRESULT win16_control_theme_set(HWND window,LPCWSTR application,LPCWSTR ids,win16_control_native_theme_fn native_theme);
#endif
