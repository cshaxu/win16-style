/* Product policy and protocol identity; COM ownership lives in the library. */
#include "intercept.h"
#include "lib-win32-style/application/dialog/classic_com_controller.h"
#include "lib-win32-style/application/dialog/observation.h"

static HRESULT win2k_scope(HRESULT (*show)(void *),void *context)
{return style_dialog_observe(show,context,ntw16s_win2k_dialog_initialize);}

static style_classic_com_route dialog_route(BOOL folder)
{
    classic_dialog_route route=ntw16s_dialog_route(folder?NTW_DIALOG_FOLDER:NTW_DIALOG_FILE);
    if(route==CLASSIC_DIALOG_WIN16)return STYLE_COM_CLASSIC;
    if(route==CLASSIC_DIALOG_WIN2K)return STYLE_COM_EXPLORER;
    return STYLE_COM_NATIVE;
}

#ifdef NTWSHK_UNIFIED
static BOOL win2k_startup_bridge;
static BOOL global_win16(void)
{
    DWORD flags=__atomic_load_n(&ntw16s_process_context.flags,__ATOMIC_ACQUIRE);
    return (flags&NTWSHK_POLICY_GLOBAL_SESSION) && ntw16s_process_session_active();
}

DWORD ntw16s_attach_win2k_com_dialogs_using(DWORD (*attach)(void *,void *),
    HRESULT (*scope)(HRESULT (*)(void *),void *))
{
    const style_classic_com_options options={
        .session_active=global_win16,.scope=ntw16s_dialog_style_scope,
        .route=dialog_route,.explorer_scope=scope,
        .property=L"Win2kStyle.ComDialog",.folder_message=L"Win2kStyle.ComSetFolder",
        .hidden_message=L"Win2kStyle.HiddenFilesRefresh"
    };
    DWORD error=style_classic_com_attach(attach,&options);
    if(!error)win2k_startup_bridge=TRUE;
    return error;
}
#endif

LONG ntw16s_attach_com_dialogs_using(DWORD (*attach)(void *,void *))
{
    if(!attach)return ERROR_INVALID_PARAMETER;
#ifdef NTWSHK_UNIFIED
    /* Startup already installed the bridge and its original-call slots. */
    if(win2k_startup_bridge)return NO_ERROR;
#endif
    if(!(ntw16s_process_context.flags & NTWSHK_POLICY_STYLE))return NO_ERROR;
    const style_classic_com_options options={
        .session_active=ntw16s_process_session_active,.scope=ntw16s_dialog_style_scope,
        .route=dialog_route,.explorer_scope=win2k_scope,
        .property=L"Win16Style.T20.ComDialog",.folder_message=L"Win16Style.T20.ComSetFolder",
        .hidden_message=L"Win16Style.T20.HiddenFilesRefresh",
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
        .experimental_events=TRUE,
#endif
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_HIDDEN
        .experimental_hidden=TRUE,
#endif
    };
    return (LONG)style_classic_com_attach(attach,&options);
}
