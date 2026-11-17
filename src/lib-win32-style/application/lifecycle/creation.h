#ifndef STYLE_CREATION_H
#define STYLE_CREATION_H
#include <windows.h>
/* Same GUI thread, strictly nested; FALSE masks the outer enabled scope.
 * The immutable operations and their code stay alive until all scopes and
 * callbacks finish. One observer mode is used throughout a nested chain.
 * End releases observer resources, not the installed window styles. */
typedef struct style_creation_scope style_creation_scope;
typedef struct style_creation_result {DWORD installed,unsupported,failure;} style_creation_result;
typedef struct style_creation_ops {
    BOOL (*prepare)(HWND,DWORD original_style);
    BOOL (*install)(HWND,DWORD original_style);
    BOOL (*attached)(HWND);
    void (*cancel)(HWND);
    void (*unsupported)(HWND);
    BOOL subclass_dialogs;
} style_creation_ops;
style_creation_scope *style_creation_begin(BOOL enabled,BOOL dialog,const style_creation_ops *);
BOOL style_creation_end(style_creation_scope *,style_creation_result *);
/* Optional subclass-mode initialized-dialog observation, nested by caller. */
void (*style_creation_dialog_observer(void (*)(HWND)))(HWND);
#endif
