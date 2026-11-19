#ifndef STYLE_CLASSIC_FILE_CONTROLLER_H
#define STYLE_CLASSIC_FILE_CONTROLLER_H
#include <windows.h>
#include <commdlg.h>
#include "observation.h"
/* One fixed policy per linked controller. Callback code remains valid for
 * all native calls/hooks. The caller owns the patch transaction. Subclass
 * observation must match the caller's creation-scope mode. */
typedef struct style_classic_file_options {
    BOOL (*active)(void);
    HRESULT (*scope)(HRESULT (*)(void *),void *);
    void (*chrome)(HWND);
    BOOL subclass_observer;
    BOOL (*alternate_active)(void);
    BOOL (*alternate_a)(OPENFILENAMEA *,BOOL (WINAPI *)(OPENFILENAMEA *),const style_dialog_windows *);
    BOOL (*alternate_w)(OPENFILENAMEW *,BOOL (WINAPI *)(OPENFILENAMEW *),const style_dialog_windows *);
    style_dialog_windows alternate_windows;
} style_classic_file_options;
DWORD style_classic_file_attach(DWORD (*)(void *,void *),const style_classic_file_options *);
#endif
