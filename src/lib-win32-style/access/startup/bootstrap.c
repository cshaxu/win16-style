#include "startup.h"
#include <wchar.h>

DWORD style_startup_bootstrap(style_startup_context *context,PCWSTR module,BOOL session,
    BOOL *found,DWORD (*prepare)(void *),void *data)
{
    DWORD error=style_startup_read(context,found);
    if(error || !*found)return error;
    if(_wcsicmp(context->module,module) || (!session && (context->flags&STYLE_STARTUP_POLICY_GLOBAL_SESSION)))
        return ERROR_INVALID_DATA;
    error=process_hook_stage_restore();
    return error?error:win32_style_startup(prepare,data);
}
