#include "api.h"
BOOL ntw16s_bootstrap(void);

DWORD __cdecl Ntw16sGetAbiVersion(void)
{
    return NTW16S_ABI_VERSION;
}

BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID reserved)
{
    /* Static TLS and /MT require notifications. Only memory/API address
     * transactions for payload launches; no GUI, waiting or DLL loading. */
    (void)module;(void)reserved;
    if(reason==DLL_PROCESS_ATTACH)return ntw16s_bootstrap();
    return TRUE;
}
