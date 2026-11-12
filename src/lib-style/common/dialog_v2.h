#ifndef STYLE_DIALOG_V2_H
#define STYLE_DIALOG_V2_H
#include <windows.h>
#include <shobjidl.h>
#include "lib-style/common/dialog_customize.h"
/* The owner supplies COM identity, dispatch and fallback state.  Adapters
 * borrow their native interfaces and all provided state for their lifetime. */
typedef enum style_dialog_v2_operation {
    STYLE_DIALOG_V2_SHOW,STYLE_DIALOG_V2_TYPES,STYLE_DIALOG_V2_SET_INDEX,STYLE_DIALOG_V2_GET_INDEX,STYLE_DIALOG_V2_ADVISE,STYLE_DIALOG_V2_UNADVISE,STYLE_DIALOG_V2_SET_FOLDER,STYLE_DIALOG_V2_GET_FOLDER,
    STYLE_DIALOG_V2_PRIMARY_GET_SELECTION,STYLE_DIALOG_V2_PRIMARY_CLOSE,STYLE_DIALOG_V2_PRIMARY_GET_RESULTS,
    STYLE_DIALOG_V2_GET_SELECTION,STYLE_DIALOG_V2_SET_NAME,STYLE_DIALOG_V2_GET_NAME,STYLE_DIALOG_V2_TITLE,STYLE_DIALOG_V2_LABEL,STYLE_DIALOG_V2_RESULT,STYLE_DIALOG_V2_EXTENSION,STYLE_DIALOG_V2_CLOSE
} style_dialog_v2_operation;
typedef HRESULT (*style_dialog_v2_call)(void *,style_dialog_v2_operation,ULONG_PTR,ULONG_PTR);
typedef struct style_dialog_v2 {
    IFileDialog2 iface;void *owner;IFileDialog2 *native;BOOL *unsupported;WCHAR **cancel_label;
    const style_dialog_identity_services *identity;style_dialog_v2_call call;
} style_dialog_v2;
void style_dialog_v2_init(style_dialog_v2 *,void *,IFileDialog2 *,BOOL *,WCHAR **,
    const style_dialog_identity_services *,style_dialog_v2_call);
#endif
