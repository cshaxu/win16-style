#ifndef STYLE_WINDOW_INSTALL_H
#define STYLE_WINDOW_INSTALL_H
#include "lib-win32-style/application/lifecycle/instance.h"

typedef struct style_frame_install_ops {
    void *context;
    const BOOL *cancelled;
    BOOL (*setup)(HWND,void *);
    void (*rollback)(HWND,void *);
} style_frame_install_ops;

/* Consumes a new, initialized instance with no outstanding calls. The common
 * transaction owns publication, cancellation and callback lifetime. Rendering
 * preparation stays with the provider; setup adds its complete live services.
 * Failure before publication disposes it. After publication rollback only acts
 * on this identity; a failed detach retains window ownership, never unloads. */
BOOL style_frame_install(style_instance *,HWND,SUBCLASSPROC,const void *,const style_frame_install_ops *);
#endif
