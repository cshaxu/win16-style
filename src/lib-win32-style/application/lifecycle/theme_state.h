#ifndef STYLE_THEME_STATE_H
#define STYLE_THEME_STATE_H
#include <windows.h>
/* Same GUI thread. Only theme requests routed through style_theme_set can
 * be restored; native_theme performs the real SetWindowTheme operation.
 * Suppression temporarily applies empty application and ID strings. */
typedef HRESULT (*style_native_theme_fn)(HWND,LPCWSTR,LPCWSTR);
BOOL style_theme_capture(HWND window);
BOOL style_theme_stage(HWND window);
BOOL style_theme_commit(HWND window);
void style_theme_cancel_stage(HWND window);
/* NULL owner retires this window's node; non-NULL guards an owned peer. */
BOOL style_theme_terminal(HWND window,const void *owner);
BOOL style_theme_suppress(HWND window,const void *owner,style_native_theme_fn native_theme);
BOOL style_theme_retained(HWND window);
BOOL style_theme_owned(HWND window,const void *owner);
void style_theme_remove(HWND window,const void *owner,style_native_theme_fn native_theme);
HRESULT style_theme_set(HWND window,LPCWSTR application,LPCWSTR ids,style_native_theme_fn native_theme);
/* Intercepted native entry; distinguishes the suppression call from an
 * application call made synchronously by the native implementation. */
HRESULT style_theme_intercept(HWND window,LPCWSTR application,LPCWSTR ids,style_native_theme_fn native_theme);
#endif
