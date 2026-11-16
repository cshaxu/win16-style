#ifndef STYLE_LIFECYCLE_INSTALL_H
#define STYLE_LIFECYCLE_INSTALL_H
#include "lib-win32-style/application/lifecycle/instance.h"

typedef struct style_instance_install_ops {
    void *context;
    const BOOL *cancelled;
    BOOL (*setup)(HWND,void *);
    void (*rollback)(HWND,void *);
} style_instance_install_ops;

/* Consumes a new, initialized instance with no outstanding calls. The common
 * transaction owns publication, cancellation and callback lifetime. Rendering
 * preparation stays with the provider; setup adds its complete live services.
 * Failure before publication disposes it. After publication the optional
 * rollback operation owns cleanup/detachment for this identity; without it
 * setup owns its failed attempt. Failed detach retains window ownership;
 * this transaction never unloads a module. */
BOOL style_instance_install(style_instance *,HWND,SUBCLASSPROC,const void *,const style_instance_install_ops *);
#endif
