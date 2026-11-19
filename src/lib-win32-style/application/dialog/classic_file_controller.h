#ifndef STYLE_CLASSIC_FILE_CONTROLLER_H
#define STYLE_CLASSIC_FILE_CONTROLLER_H
#include <windows.h>
#include <commdlg.h>
typedef struct style_file_window_ops {
    void (*initialized)(HWND);
    BOOL (*attached)(HWND);
} style_file_window_ops;
/* One fixed policy per linked controller. Callback code remains valid for
 * all native calls/hooks. The caller owns the patch transaction. Subclass
 * observation must match the caller's creation-scope mode. */
typedef struct style_classic_file_options {
    BOOL (*active)(void);
    HRESULT (*scope)(HRESULT (*)(void *),void *);
    void (*chrome)(HWND);
    BOOL subclass_observer;
    BOOL (*alternate_active)(void);
    BOOL (*alternate_a)(OPENFILENAMEA *,BOOL (WINAPI *)(OPENFILENAMEA *),const style_file_window_ops *);
    BOOL (*alternate_w)(OPENFILENAMEW *,BOOL (WINAPI *)(OPENFILENAMEW *),const style_file_window_ops *);
    style_file_window_ops alternate_windows;
} style_classic_file_options;
DWORD style_classic_file_attach(DWORD (*)(void *,void *),const style_classic_file_options *);
#endif
