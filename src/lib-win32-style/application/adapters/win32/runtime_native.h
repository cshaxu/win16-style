#ifndef STYLE_RUNTIME_NATIVE_H
#define STYLE_RUNTIME_NATIVE_H
#include "runtime_observer.h"
typedef struct style_runtime_native_call style_runtime_native_call;
/* One caller-owned manager per GUI thread. This observes calls only, and owns
 * no windows, fonts or hooks. Caller pins all referenced instances/runtime. */
typedef struct style_runtime_native_calls {style_runtime_native_call *current;} style_runtime_native_calls;
struct style_runtime_native_call {
    style_runtime_native_calls *manager;
    style_runtime_native_call *previous;
    HWND window;HMODULE native_module;
    const style_instance *instance;
    BOOL active;
};
/* Performs all existing native witness observations even on no-match. A
 * nested unverified call installs a barrier, so an outer font scope cannot
 * leak into it. FALSE means no scope was entered. End is also a native-unwind
 * cleanup callback; scopes must end in stack order while their storage lives. */
BOOL style_runtime_native_call_begin(style_runtime_native_calls *,style_runtime_native_call *,WNDPROC,HWND,UINT,const void *,const style_runtime_observation *,style_runtime_observation *);
void style_runtime_native_call_end(void *);
const style_runtime_native_call *style_runtime_native_current(const style_runtime_native_calls *,const style_runtime_observation *);
#endif
