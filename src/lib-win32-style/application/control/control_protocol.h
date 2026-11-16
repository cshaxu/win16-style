#ifndef STYLE_CONTROL_PROTOCOL_H
#define STYLE_CONTROL_PROTOCOL_H
#include <windows.h>
/* Bind and release on the window GUI thread. A successful bind returns one
 * reference; acquire adds one, release drops it. The registry and property
 * are private to this linked module and reject a foreign property value.
 * Initialized native controls may be checked at creation or late attachment. */
typedef struct style_control_protocol style_control_protocol;
/* Recognize an exact class registered by an already-loaded runtime module. */
static inline BOOL style_control_runtime_class(HWND window,const WCHAR *name,const WCHAR *module)
{
    WCHAR actual[64];HMODULE runtime=GetModuleHandleW(module);
    return runtime && (HMODULE)GetClassLongPtrW(window,GCLP_HMODULE)==runtime &&
        GetClassNameW(window,actual,ARRAYSIZE(actual)) && !lstrcmpiW(actual,name);
}
style_control_protocol *style_control_protocol_bind(HWND);
/* Internal: owner is accepted only after the actual EDIT forwarding witness. */
style_control_protocol *style_control_protocol_bind_verified_edit(HWND,HMODULE);
style_control_protocol *style_control_protocol_bind_verified_combo(HWND,HMODULE);
/* Internal: owner forwarded this control's WM_PAINT to system SysLink. */
style_control_protocol *style_control_protocol_bind_verified_link(HWND,HMODULE);
style_control_protocol *style_control_protocol_bind_verified_scroll(HWND,HMODULE);
/* Internal: exact group button with a witnessed native BUTTON WM_PAINT. */
style_control_protocol *style_control_protocol_bind_verified_group_button(HWND,HMODULE);
/* Internal: the owner must have forwarded WM_PAINT to native TRACKBAR. */
style_control_protocol *style_control_protocol_bind_verified_trackbar(HWND,HMODULE);
style_control_protocol *style_control_protocol_bind_verified_tab(HWND,HMODULE);
/* Internal: the owner forwarded this exact control paint to its native class. */
style_control_protocol *style_control_protocol_bind_verified_report(HWND,HMODULE,BOOL);
BOOL style_control_protocol_runtime_trackbar(HWND);
style_control_protocol *style_control_protocol_acquire(HWND);
BOOL style_control_protocol_is(HWND,const WCHAR *);
/* Recognize a registered superclass of the requested native class. */
BOOL style_control_native_superclass(HWND,const WCHAR *);
/* Same-runtime VCL combo in the plain, left-to-right modes; callers decide
 * whether this supported subset is eligible for their own rendering. */
BOOL style_control_vcl_plain_combo(HWND,HWND);
HMODULE style_control_protocol_list_module(HWND);
HMODULE style_control_protocol_tree_module(HWND);
HMODULE style_control_protocol_updown_module(HWND);
BOOL style_control_protocol_toolbar(HWND);
style_control_protocol *style_control_protocol_bind_verified_toolbar(HWND,HMODULE);
BOOL style_control_protocol_runtime_toolbar(HWND);
BOOL style_control_protocol_legacy_progress(HWND);
void style_control_protocol_release(style_control_protocol *);
#endif
