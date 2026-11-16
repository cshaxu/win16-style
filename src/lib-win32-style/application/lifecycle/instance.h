#ifndef STYLE_INSTANCE_H
#define STYLE_INSTANCE_H
#include <windows.h>
#include <commctrl.h>

/* GUI-thread state, embedded first in its containing object. An installed
 * subclass borrows the object; synchronous/reentrant users retain it. Detach
 * ends that borrow and the final release invokes dispose(context). Dispose
 * owns the containing allocation and its remaining resources. */
typedef struct style_instance {
    unsigned calls;
    BOOL detached;
    const void *owner;
    HWND window;
    SUBCLASSPROC procedure;
    void *context;
    void (*dispose)(void *);
} style_instance;

/* Init starts with zero calls. Failed attach leaves storage caller-owned;
 * a detached instance must be reinitialized before a new attachment. */
void style_instance_init(style_instance *,void *,void (*)(void *));
void style_instance_retain(style_instance *);
void style_instance_release(style_instance *);
style_instance *style_instance_lookup(HWND,SUBCLASSPROC);
BOOL style_instance_attach(style_instance *,HWND,SUBCLASSPROC,const void *);
/* Caller holds a reference through its own cleanup. Removal failure keeps
 * the instance attached; this does not restore appearance or unload a DLL. */
BOOL style_instance_detach(style_instance *);
#endif
