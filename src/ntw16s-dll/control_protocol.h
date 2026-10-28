#ifndef NTW16S_CONTROL_PROTOCOL_H
#define NTW16S_CONTROL_PROTOCOL_H
#include <windows.h>
/* A verified role belongs to one newly created HWND on its GUI thread. */
typedef struct ntw16s_control_protocol ntw16s_control_protocol;
ntw16s_control_protocol *ntw16s_control_protocol_bind(HWND);
/* Internal: owner is accepted only after the actual EDIT forwarding witness. */
ntw16s_control_protocol *ntw16s_control_protocol_bind_verified_edit(HWND,HMODULE);
ntw16s_control_protocol *ntw16s_control_protocol_bind_verified_scroll(HWND,HMODULE);
/* Internal: exact group button with a witnessed native BUTTON WM_PAINT. */
ntw16s_control_protocol *ntw16s_control_protocol_bind_verified_group_button(HWND,HMODULE);
/* Internal: the owner must have forwarded WM_PAINT to native TRACKBAR. */
ntw16s_control_protocol *ntw16s_control_protocol_bind_verified_trackbar(HWND,HMODULE);
ntw16s_control_protocol *ntw16s_control_protocol_bind_verified_tab(HWND,HMODULE);
BOOL ntw16s_control_protocol_runtime_trackbar(HWND);
ntw16s_control_protocol *ntw16s_control_protocol_acquire(HWND);
BOOL ntw16s_control_protocol_is(HWND,const WCHAR *);
HMODULE ntw16s_control_protocol_list_module(HWND);
HMODULE ntw16s_control_protocol_tree_module(HWND);
HMODULE ntw16s_control_protocol_updown_module(HWND);
BOOL ntw16s_control_protocol_toolbar(HWND);
ntw16s_control_protocol *ntw16s_control_protocol_bind_verified_toolbar(HWND,HMODULE);
BOOL ntw16s_control_protocol_runtime_toolbar(HWND);
BOOL ntw16s_control_protocol_legacy_progress(HWND);
void ntw16s_control_protocol_release(ntw16s_control_protocol *);
#endif
