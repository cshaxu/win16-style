#ifndef WIN16_UWP_FRAME_H
#define WIN16_UWP_FRAME_H
#include <windows.h>

/* Local GUI-thread API. The caller retains the containing module until every
 * frame has been removed and callbacks have returned. No process hooks, XAML
 * changes, target file writes or registry writes occur here. */
typedef struct win16_uwp_frame_action {
    PCWSTR label;
    void (*invoke)(HWND frame,void *context);
    void *context;
} win16_uwp_frame_action;
/* Optional action is copied; label/context remain borrowed until removal.
 * Called on the owner GUI thread after menu tracking and temporary-item cleanup. */
BOOL win16_uwp_frame_apply_action(HWND frame,HWND native_title,const win16_uwp_frame_action *action);
BOOL win16_uwp_frame_apply(HWND frame,HWND native_title);
BOOL win16_uwp_frame_remove(HWND frame);
HWND win16_uwp_frame_surface(HWND frame);
#endif
