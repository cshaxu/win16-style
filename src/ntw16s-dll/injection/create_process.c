/* Product policy and payload adapter for the shared creation transaction. */
#include "intercept.h"
#include "startup_hooks.h"
#include "system_process.h"
#include "ntw16s-dll/global/session.h"
#include "lib-win32/process-hook/propagate.h"
BOOL ntw16s_process_session_active(void)
{return !(__atomic_load_n(&ntw16s_process_context.flags,__ATOMIC_ACQUIRE)&NTW16S_POLICY_GLOBAL_SESSION) || ntw16s_global_policy_active(__atomic_load_n(&ntw16s_process_context.session_cookie,__ATOMIC_ACQUIRE));}
static BOOL active(void *unused){(void)unused;return ntw16s_process_session_active();}
static BOOL accept_w(LPCWSTR application,LPWSTR command,void *unused)
{(void)unused;return !ntw16s_system_print_host_w(application,command);}
static BOOL accept_a(LPCSTR application,LPSTR command,void *unused)
{(void)unused;return !ntw16s_system_print_host_a(application,command);}
static DWORD install(HANDLE child,process_hook_create_w create,void *unused)
{(void)unused;return ntw16s_install(child,&ntw16s_process_context,create);}
LONG ntw16s_attach_processes_using(DWORD (*attach)(void *,void *))
{
    if(!attach)return ERROR_INVALID_PARAMETER;
    if(!(ntw16s_process_context.flags&NTW16S_POLICY_PROPAGATE))return NO_ERROR;
    const process_hook_propagation_ops ops={active,accept_w,accept_a,install,NULL};
    return (LONG)process_hook_propagation_attach(attach,&ops);
}
LONG ntw16s_attach_processes(void){return ntw16s_attach_processes_using(ntw16s_hooks_attach);}
