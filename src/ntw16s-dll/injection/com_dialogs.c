/* Product policy and protocol identity; COM ownership lives in the library. */
#include "intercept.h"
#include "lib-win32-style/application/dialog/classic_com_controller.h"
#include "lib-win32-style/application/dialog/observation.h"

static HRESULT win2k_scope(HRESULT (*show)(void *),void *context)
{return style_dialog_observe(show,context,ntw16s_win2k_dialog_initialize);}

LONG ntw16s_attach_com_dialogs_using(DWORD (*attach)(void *,void *))
{
    if(!attach)return ERROR_INVALID_PARAMETER;
    if(!(ntw16s_process_context.flags & NTWSHK_POLICY_STYLE))return NO_ERROR;
    const style_classic_com_options options={
        .session_active=ntw16s_process_session_active,.scope=ntw16s_dialog_style_scope,
        .explorer_active=ntw16s_win2k_session_active,.explorer_scope=win2k_scope,
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
