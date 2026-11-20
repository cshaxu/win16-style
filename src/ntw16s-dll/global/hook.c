#include "ntw16s-dll/global/session.h"
#include "ntw16s-dll/global/window.h"
#include "ntw16s-dll/global/vb_text.h"
#include "ntw16s-dll/global/policy.h"
#include "ntw16s-dll/injection/late_process.h"
#include "ntwshk-dll/injection.h"
#include "lib-win32-style/access/global/worker.h"
#include <stdlib.h>
#include "lib-win32-style/access/global/receiver.h"
#include "lib-win32-style/access/global/host.h"
static DWORD initialize(HMODULE module,DWORD cookie,void *context)
{
    (void)context;ntwshk_context *paths=calloc(1,sizeof(*paths));
    if(!paths)return ERROR_NOT_ENOUGH_MEMORY;
    DWORD error=style_startup_paths(paths,module);
    if(!error)error=ntw16s_late_process_init(paths->directory,cookie);
    free(paths);return error;
}
static const style_global_late_ops late_ops={&ntw16s_global_namespace,initialize,
    NTW16S_GLOBAL_CONTROL,NTW16S_GLOBAL_APPLY,NULL};
static style_global_late late={.ops=&late_ops};
static DWORD worker_initialize(HMODULE module,DWORD cookie,void *context)
{
    DWORD error=initialize(module,cookie,context);
    if(!error)style_global_late_mark(&late,cookie);
    return error;
}
LRESULT CALLBACK Ntw16sGlobalHook(int,WPARAM,LPARAM);
DWORD WINAPI ntw16s_global_thread_worker(void *parameter)
{
    static const style_global_worker_ops ops={&ntw16s_global_namespace,Ntw16sGlobalHook,worker_initialize,NULL};
    return style_global_worker_run(&ops,parameter);
}
static void change(HWND window,DWORD style,BOOL apply,void *context)
{
    (void)context;
    if(apply){
        BOOL attached=ntw16s_global_window_apply_style(window,style),text_changed=FALSE;
        if(style==NTW16S_GLOBAL_STYLE_16 && attached && !ntw16s_global_vb_text(window,&text_changed) && text_changed)
            RedrawWindow(window,NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN);
        ntw16s_global_window_children(window,TRUE,style);
    }else{
        ntw16s_global_window_children(window,FALSE,style);
        (void)ntw16s_global_window_remove_style(window,style);
    }
}
LRESULT CALLBACK Ntw16sGlobalHook(int code,WPARAM a,LPARAM b)
{
    static const style_global_receiver_ops ops={&ntw16s_global_namespace,&late,
        NTW16S_GLOBAL_CONTROL,NTW16S_GLOBAL_APPLY,NTW16S_GLOBAL_REMOVE,ntw16s_global_image_allowed,
        ntw16s_global_window_eligible,change,NULL};
    static style_global_receiver receiver={.ops=&ops};
    return style_global_receiver_handle(&receiver,code,a,b);
}
static void report(enum style_global_host_notice notice,void *context)
{
    (void)context;
    if(notice==STYLE_GLOBAL_HOST_PENDING)OutputDebugStringW(L"run16s: global 32-bit startup attachment still pending\n");
    else OutputDebugStringW(L"run16s: global owner trace cleanup failed\n");
}
void CALLBACK Ntw16sGlobalHost(HWND window,HINSTANCE instance,LPSTR command,int show)
{
    (void)window;(void)instance;(void)command;(void)show;
    const style_global_host_spec spec={&ntw16s_global_manager_config,Ntw16sGlobalHook,
        report,NULL};
    style_global_manager *retained=NULL;
    DWORD error=style_global_host_run(&spec,&retained);if(error)ExitProcess(error);
}
