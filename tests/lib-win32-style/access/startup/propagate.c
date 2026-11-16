#include "lib-win32-style/access/startup/propagate.h"
#include "lib-win32-style/access/startup/install.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"propagation line=%d\n",__LINE__);return 1;}}while(0)
static process_hook_propagation_ops captured,discarded;
static DWORD attach_error,user_error,install_error;
static unsigned attaches,users;
static const style_startup_context *installed;
static PCWSTR dependency;
static HANDLE target;
static process_hook_create_w creator;
static DWORD attach(void *a,void *b){(void)a;(void)b;return 0;}
DWORD process_hook_propagation_attach(DWORD (*fn)(void *,void *),const process_hook_propagation_ops *ops)
{if(fn!=attach)return ERROR_INVALID_FUNCTION;captured=*ops;++attaches;return attach_error;}
DWORD process_hook_propagation_attach_as_user_w(DWORD (*fn)(void *,void *))
{if(fn!=attach)return ERROR_INVALID_FUNCTION;++users;return user_error;}
void process_hook_propagation_discard(const process_hook_propagation_ops *ops){discarded=*ops;}
DWORD style_startup_install(HANDLE child,const style_startup_context *process,PCWSTR name,process_hook_create_w create)
{target=child;installed=process;dependency=name;creator=create;return install_error;}
BOOL style_startup_print_host_a(LPCSTR app,LPCSTR command){return app==command;}
BOOL style_startup_print_host_w(LPCWSTR app,LPCWSTR command){return app==command;}
static BOOL enabled(void *context){return *(BOOL *)context;}
int main(void)
{
    static style_startup_context process;BOOL state=FALSE;
    style_startup_propagation policy={&process,L"dependency",NULL,NULL};
    CHECK(style_startup_propagate(NULL,&policy)==ERROR_INVALID_PARAMETER);
    CHECK(!style_startup_propagate(attach,&policy) && !attaches && !users);
    process.flags=STYLE_STARTUP_POLICY_PROPAGATE;
    CHECK(!style_startup_propagate(attach,&policy) && attaches==1 && users==1);
    CHECK(captured.active(captured.context));
    policy.active=enabled;policy.context=&state;
    CHECK(!captured.active(captured.context));state=TRUE;CHECK(captured.active(captured.context));
    CHECK(!captured.accept_w(NULL,NULL,captured.context) && captured.accept_w(L"app",NULL,captured.context));
    CHECK(!captured.accept_a(NULL,NULL,captured.context) && captured.accept_a("app",NULL,captured.context));
    install_error=ERROR_ACCESS_DENIED;
    CHECK(captured.install((HANDLE)(INT_PTR)42,CreateProcessW,captured.context)==install_error);
    CHECK(target==(HANDLE)(INT_PTR)42 && installed==&process && dependency==policy.dependency && creator==CreateProcessW);
    attach_error=ERROR_BUSY;
    CHECK(style_startup_propagate(attach,&policy)==attach_error && users==1);attach_error=0;
    user_error=ERROR_NOT_SUPPORTED;CHECK(style_startup_propagate(attach,&policy)==user_error && users==2);
    style_startup_propagation_discard(&policy);
    CHECK(discarded.context==captured.context && discarded.active==captured.active &&
        discarded.accept_a==captured.accept_a && discarded.accept_w==captured.accept_w && discarded.install==captured.install);
    puts("startup propagation pass: independent policy original creator failures discard ownership");return 0;
}
