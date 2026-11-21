#include "late_process.h"
#include "intercept.h"
#include "lib-win32-style/access/runtime/process.h"
static style_runtime_process process;
static DWORD prepare(DWORD (*attach)(void *,void *),BOOL reuse,DWORD flags,void *context)
{
    (void)context;DWORD error=reuse?ERROR_SUCCESS:(DWORD)ntw16s_attach_processes_using(attach);
    if(!error && (flags&NTWSHK_POLICY_STYLE))error=(DWORD)ntw16s_attach_titles_using(attach);
    if(!error && (flags&NTWSHK_POLICY_STYLE))error=(DWORD)ntw16s_attach_shell_dialogs_using(attach,FALSE);
    return error;
}
static void discard(void *context){(void)context;ntw16s_discard_processes();}
#ifdef NTWSHK_UNIFIED
static const style_runtime_process_ops operations={L"ntwshk",prepare,discard,NULL};
#else
static const style_runtime_process_ops operations={L"ntw16s",prepare,discard,NULL};
#endif
DWORD ntw16s_late_process_init(PCWSTR directory,DWORD cookie)
{return style_runtime_process_init(&process,&ntw16s_process_context,&operations,directory,cookie,
    NTWSHK_POLICY_STYLE|NTWSHK_POLICY_PROPAGATE,FALSE);}
DWORD ntw16s_late_process_init_mode(PCWSTR directory,DWORD cookie,DWORD flags)
{return style_runtime_process_init(&process,&ntw16s_process_context,&operations,directory,cookie,flags,TRUE);}
