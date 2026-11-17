#ifndef STYLE_RUNTIME_OBSERVER_H
#define STYLE_RUNTIME_OBSERVER_H
#include "../../control/runtime.h"
/* Caller-owned GUI paint scope. Complete state and native procedure/modules
 * are borrowed; caller pins instance/runtime and restores its nested chain on
 * normal and exceptional return. No rendering or hook installation occurs. */
typedef struct style_runtime_observation {
    struct style_runtime_observation *previous;
    style_instance *instance;
    style_control_runtime *runtime;
    void *owner;HWND window;
    BOOL (*current)(HWND,void *);
    WNDPROC native;
} style_runtime_observation;
HMODULE style_runtime_observe_native(HWND,UINT,WNDPROC,const void *,const style_runtime_observation *,style_runtime_observation *);
void style_runtime_observe_ansi(const style_runtime_observation *,WNDPROC,HWND,UINT,const void *,BOOL);
BOOL style_runtime_observe_box(HWND,UINT,const void *,const style_runtime_observation *);
BOOL style_runtime_observe_rich(WNDPROC,HWND,UINT,const void *,BOOL,const style_runtime_observation *,DWORD);
#endif
