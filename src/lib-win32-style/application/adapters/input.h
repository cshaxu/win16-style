#ifndef STYLE_UWP_INPUT_H
#define STYLE_UWP_INPUT_H
#include <windows.h>

/* Call while attached; state belongs to the caller and must remain alive
 * during synchronous window-message reentry. All callbacks are required.
 * The style supplies geometry and commands; the host owns pointer routing. */
typedef struct style_uwp_input_view {
    HWND owner,surface;
    const BOOL *detached;
    int *pressed;
    BOOL *hot;
    void *context;
    LRESULT (*hit)(void *,POINT);
    BOOL (*button)(void *,int);
    void (*invoke)(void *,int,UINT);
    BOOL (*refresh)(void *);
} style_uwp_input_view;

BOOL style_uwp_input_message(const style_uwp_input_view *,UINT,WPARAM,LPARAM,LRESULT *);
#endif
