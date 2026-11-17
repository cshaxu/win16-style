/* Product activation and appearance policy; native call state is shared. */
#include "intercept.h"
#include "lib-win32-style/application/dialog/classic_file_controller.h"
LONG ntw16s_attach_file_dialogs_using(DWORD (*attach)(void *,void *))
{
    if(!attach)return ERROR_INVALID_PARAMETER;
    if(!(ntw16s_process_context.flags & NTWSHK_POLICY_STYLE))return NO_ERROR;
    const style_classic_file_options options={.active=ntw16s_process_session_active,
        .scope=ntw16s_dialog_style_scope,.chrome=ntw16s_old_dialog_chrome,
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
        .subclass_observer=TRUE,
#endif
    };
    return (LONG)style_classic_file_attach(attach,&options);
}
