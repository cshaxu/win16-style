/* Product policy and protocol identity; COM ownership lives in the library. */
#include "intercept.h"
#include "lib-win32-style/application/dialog/classic_com_controller.h"
#include "lib-win32-style/application/dialog/observation.h"

static HRESULT observed_scope(HRESULT (*show)(void *),void *context)
{return style_dialog_observe(show,context,ntw16s_observed_dialog_initialize);}

static style_classic_com_route dialog_route(BOOL folder)
{
    const classic_dialog_policy policy=ntw16s_dialog_policy();
    if(folder){
        if(policy.folder==CLASSIC_FOLDER_LEGACY_TREE)return STYLE_COM_CLASSIC;
        if(policy.folder==CLASSIC_FOLDER_OBSERVED_NATIVE)return STYLE_COM_EXPLORER;
    }else{
        if(policy.file==CLASSIC_FILE_LEGACY_LAYOUT)return STYLE_COM_CLASSIC;
        if(policy.file==CLASSIC_FILE_EXPLORER_LAYOUT)return STYLE_COM_EXPLORER;
    }
    return STYLE_COM_NATIVE;
}

/* Only observed startup owns a permanent registration before late attachment.
 * A late transaction is free to retry after rollback. */
static BOOL startup_shell_dialogs;
LONG ntw16s_attach_shell_dialogs_using(DWORD (*attach)(void *,void *),BOOL observed_startup)
{
    if(!attach)return ERROR_INVALID_PARAMETER;
    if(startup_shell_dialogs)return NO_ERROR;
    if(!observed_startup && !(ntw16s_process_context.flags&NTWSHK_POLICY_STYLE))return NO_ERROR;
    const style_classic_com_options options={
        .scope=ntw16s_dialog_style_scope,
        .route=dialog_route,.explorer_scope=observed_scope,
#ifdef NTWSHK_UNIFIED
        .property=L"Ntwshk.ComDialog",.folder_message=L"Ntwshk.ComSetFolder",
        .hidden_message=L"Ntwshk.HiddenFilesRefresh",
#else
        .property=L"Win16Style.T20.ComDialog",.folder_message=L"Win16Style.T20.ComSetFolder",
        .hidden_message=L"Win16Style.T20.HiddenFilesRefresh",
#endif
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
        .experimental_events=!observed_startup,
#endif
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_HIDDEN
        .experimental_hidden=!observed_startup,
#endif
    };
    LONG error=ntw16s_register_file_dialogs(attach);
    if(!error)error=(LONG)style_classic_com_attach(attach,&options);
    if(!error)error=ntw16s_register_folder_dialogs(attach);
    if(!error && observed_startup)startup_shell_dialogs=TRUE;
    return error;
}
