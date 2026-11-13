#ifndef STYLE_MODERN_FILE_DIALOG_H
#define STYLE_MODERN_FILE_DIALOG_H
#include <windows.h>
typedef HRESULT (*style_dialog_scoped_call)(void *);
typedef HRESULT (*style_dialog_scope)(style_dialog_scoped_call,void *);
/* Thread-local state permits COM adapters to avoid wrapping this call again. */
BOOL style_dialog_modern_active(void);
/* kind: 1=open, 2=save, 3=folder. The scope must outlive the call. */
HRESULT style_dialog_show_modern(HWND owner,UINT kind,PWSTR path,DWORD capacity,
    style_dialog_scope scope);
#endif
