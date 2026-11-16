#include "propagate.h"
#include "install.h"
#include "target.h"

static BOOL active(void *context)
{
    const style_startup_propagation *policy=context;
    return !policy->active || policy->active(policy->context);
}
static BOOL accept_w(LPCWSTR app,LPWSTR command,void *context)
{(void)context;return !style_startup_print_host_w(app,command);}
static BOOL accept_a(LPCSTR app,LPSTR command,void *context)
{(void)context;return !style_startup_print_host_a(app,command);}
static DWORD install(HANDLE child,process_hook_create_w create,void *context)
{
    const style_startup_propagation *policy=context;
    return style_startup_install(child,policy->process,policy->dependency,create);
}
static process_hook_propagation_ops operations(const style_startup_propagation *policy)
{return (process_hook_propagation_ops){active,accept_w,accept_a,install,(void *)policy};}

DWORD style_startup_propagate(DWORD (*attach)(void *,void *),const style_startup_propagation *policy)
{
    if(!attach)return ERROR_INVALID_PARAMETER;
    if(!(__atomic_load_n(&policy->process->flags,__ATOMIC_ACQUIRE)&STYLE_STARTUP_POLICY_PROPAGATE))return ERROR_SUCCESS;
    process_hook_propagation_ops ops=operations(policy);
    DWORD error=process_hook_propagation_attach(attach,&ops);
    return error?error:process_hook_propagation_attach_as_user_w(attach);
}
void style_startup_propagation_discard(const style_startup_propagation *policy)
{
    process_hook_propagation_ops ops=operations(policy);
    process_hook_propagation_discard(&ops);
}
