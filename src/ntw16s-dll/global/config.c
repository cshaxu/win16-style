#include "session.h"
#include "policy.h"
const style_global_attach_config ntw16s_global_attach_config={
    &ntw16s_global_start_config,sizeof(void*)==8?L"ntwshk64.dll":L"ntwshk32.dll",
    "process_hook_startup_begin",ntw16s_global_image_allowed,NTW16S_GLOBAL_CONTROL,
    NTW16S_GLOBAL_APPLY,NTW16S_GLOBAL_REMOVE,NTW16S_GLOBAL_QUERY,NTW16S_GLOBAL_ATTACHED
};
