#ifndef STYLE_UWP_REPAINT_H
#define STYLE_UWP_REPAINT_H
#include <windows.h>

typedef struct style_uwp_repaint_services {
    void (*retain)(void *);
    void (*release)(void *);
    void (*activate)(HWND,void *);
} style_uwp_repaint_services;
typedef struct style_uwp_repaint {
    HWND timer,owner;
    BOOL pending,waiting,repainting;
    ULONGLONG deadline;
    const BOOL *detached;
    void *context;
    const style_uwp_repaint_services *services;
} style_uwp_repaint;
/* State and callbacks are caller-owned and must outlive queued callbacks.
 * Use on the owner UI thread; retain/release protect synchronous reentrancy.
 * A live scheduler must keep the same owner, context and services until cancel. */
BOOL style_uwp_repaint_queue(HWND,style_uwp_repaint *,const BOOL *,void *,const style_uwp_repaint_services *);
void style_uwp_repaint_cancel(style_uwp_repaint *);
#endif
