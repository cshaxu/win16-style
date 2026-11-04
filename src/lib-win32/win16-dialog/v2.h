#ifndef WIN16_DIALOG_V2_H
#define WIN16_DIALOG_V2_H
#include <windows.h>
#include <shobjidl.h>
#include "customize.h"
typedef enum win16_dialog_v2_operation {
    V2_SHOW,V2_TYPES,V2_SET_INDEX,V2_GET_INDEX,V2_ADVISE,V2_UNADVISE,V2_SET_FOLDER,V2_GET_FOLDER,
    V2_PRIMARY_GET_SELECTION,V2_PRIMARY_CLOSE,V2_PRIMARY_GET_RESULTS,
    V2_GET_SELECTION,V2_SET_NAME,V2_GET_NAME,V2_TITLE,V2_LABEL,V2_RESULT,V2_EXTENSION,V2_CLOSE
} win16_dialog_v2_operation;
typedef HRESULT (*win16_dialog_v2_call)(void *,win16_dialog_v2_operation,ULONG_PTR,ULONG_PTR);
typedef struct win16_dialog_v2 {
    IFileDialog2 iface;void *owner;IFileDialog2 *native;BOOL *unsupported;WCHAR **cancel_label;
    const win16_dialog_customize_services *identity;win16_dialog_v2_call call;
} win16_dialog_v2;
void win16_dialog_v2_init(win16_dialog_v2 *,void *,IFileDialog2 *,BOOL *,WCHAR **,
    const win16_dialog_customize_services *,win16_dialog_v2_call);
#endif
