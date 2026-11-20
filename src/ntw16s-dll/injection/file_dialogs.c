/* Product activation and appearance policy; native call state is shared. */
#include "intercept.h"
#include "lib-win32-style/application/dialog/classic_file_controller.h"
#include "lib-win32-style/appearance/win2000/dialog/file_call.h"
#include "ntw16s-dll/global/session.h"
#include "ntw16s-dll/global/window.h"
void ntw16s_win2k_dialog_initialize(HWND window)
{
    DWORD style;
    DWORD cookie=__atomic_load_n(&ntw16s_process_context.session_cookie,__ATOMIC_ACQUIRE);
    if(ntw16s_win2k_session_active() && ntw16s_global_window_eligible(window) &&
       style_global_policy_current(&ntw16s_global_namespace,cookie,&style) && (style&1u)){
        ntw16s_global_window_apply_style(window,style);
        ntw16s_global_window_children(window,TRUE,style);
    }
}
LONG ntw16s_attach_file_dialogs_using(DWORD (*attach)(void *,void *))
{
    if(!attach)return ERROR_INVALID_PARAMETER;
    if(!(ntw16s_process_context.flags & NTWSHK_POLICY_STYLE))return NO_ERROR;
    const style_classic_file_options options={.active=ntw16s_process_session_active,
        .scope=ntw16s_dialog_style_scope,.chrome=ntw16s_old_dialog_chrome,
        .alternate_active=ntw16s_win2k_session_active,.alternate_a=win2k_file_call_a,
        .alternate_w=win2k_file_call_w,.alternate_windows={ntw16s_win2k_dialog_initialize,ntw16s_global_win2k_attached,ntw16s_global_win2k_resources},
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
        .subclass_observer=TRUE,
#endif
    };
    return (LONG)style_classic_file_attach(attach,&options);
}
