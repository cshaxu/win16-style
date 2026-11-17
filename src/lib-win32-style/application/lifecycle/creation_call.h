#ifndef STYLE_CREATION_CALL_H
#define STYLE_CREATION_CALL_H
#include "creation.h"
/* Zero-initialize once. The caller fixes valid callbacks and an immutable
 * creation policy before use; this context and callback code outlive calls.
 * The library owns once/creation/end state, not the installed appearances. */
typedef struct style_creation_context {
    INIT_ONCE once;
    PINIT_ONCE_FN prepare;
    BOOL (*active)(void);
    const style_creation_ops *operations;
} style_creation_context;
typedef struct style_creation_call {
    style_creation_scope *creation;
    DWORD failure;
    BOOL enabled;
} style_creation_call;
BOOL style_creation_call_begin(style_creation_call *,BOOL modal,style_creation_context *);
void style_creation_call_end(void *);
HRESULT style_creation_call_invoke(style_creation_context *,HRESULT (*)(void *),void *);
#endif
