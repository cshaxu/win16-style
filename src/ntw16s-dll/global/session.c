#include "session.h"
/* Product namespace/protocol binding. active/style adapt the callback
 * signatures used by appearance and startup policy; no session state or
 * lifecycle algorithm is owned here. */
style_global_namespace ntw16s_global_namespace={
    .prefix=L"Local\\Win16Style.Global",.launcher=L"run16s.exe",
    .module=sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll",
    .magic=NTW16S_GLOBAL_MAGIC,.version=NTW16S_GLOBAL_VERSION
};
const style_global_start_config ntw16s_global_start_config={
    &ntw16s_global_namespace,"Ntw16sGlobalThread"
};
BOOL ntw16s_global_policy_active(DWORD cookie)
{return style_global_policy_active(&ntw16s_global_namespace,cookie);}
BOOL ntw16s_global_policy_style(DWORD cookie,DWORD style)
{return style_global_policy_style(&ntw16s_global_namespace,cookie,style);}
