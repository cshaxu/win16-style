#ifndef STYLE_OBSERVED_CONTROL_H
#define STYLE_OBSERVED_CONTROL_H
#include "lib-win32-style/application/lifecycle/theme_state.h"
#include "../control_theme.h"

/* The theme loader and native theme callback outlive GUI-thread attachments. */
BOOL style_observed_control_supported(HWND);
BOOL style_observed_control_attach(HWND,style_native_theme_fn,style_control_theme_load,BOOL *native_requested);
BOOL style_observed_control_attached(HWND);
BOOL style_observed_control_remove(HWND);
#endif
