#ifndef STYLE_COMMON_GLOBAL_WINDOW_H
#define STYLE_COMMON_GLOBAL_WINDOW_H
#include <windows.h>

typedef struct style_global_window_ops {
    BOOL (*attached)(HWND);
    BOOL (*apply)(HWND);
    BOOL (*remove)(HWND);
    BOOL (*active)(DWORD);
    LPCWSTR control;
    LPARAM query;
    LRESULT acknowledgement;
} style_global_window_ops;

/* The operations table must outlive the attachment. Calls are GUI-thread only. */
BOOL style_global_window_attached(HWND window,const style_global_window_ops *ops);
/* Visible across style DLLs in the same process, including during attach. */
BOOL style_global_window_managed(HWND window);
BOOL style_global_window_apply(HWND window,const style_global_window_ops *ops);
BOOL style_global_window_remove(HWND window,const style_global_window_ops *ops);
/* A nonzero profile belongs to the installed record. A zero value denotes
 * compatibility attachments without a recorded profile. */
DWORD style_global_window_profile(HWND window);
BOOL style_global_window_remove_current(HWND window);
/* The caller may scope each renderer invocation to the recorded profile.
 * Unmarked direct attachments have profile zero. */
typedef BOOL (*style_global_window_apply_profile)(HWND window,BOOL (*apply)(HWND),DWORD profile);
BOOL style_global_window_transition(HWND window,const style_global_window_ops *selected,
    const style_global_window_ops *unmanaged_other,DWORD profile,BOOL refresh_direct,
    style_global_window_apply_profile apply_profile);
/* Enumerate descendants owned by this GUI thread. A NULL parent follows
 * EnumChildWindows semantics. The visitor chooses policy and may stop. */
BOOL style_global_window_visit_children(HWND parent,BOOL (*visit)(HWND,void *),void *context);
#endif
