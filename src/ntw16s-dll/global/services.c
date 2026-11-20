#include "session.h"
#include "ntw16s-dll/uwp/target.h"
#include "lib-win32-style/access/global/manager.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include <stdio.h>
#include <stdint.h>
#include <wchar.h>
static BOOL uwp_active(void *context)
{return ntw16s_global_policy_active((DWORD)(uintptr_t)context);}
static DWORD uwp_change(const style_uwp_target *target,PCWSTR dll,BOOL apply,void *context)
{
    unsigned value=(unsigned)((uint64_t)(uintptr_t)context>>32);
    classic_theme_selection selection;
    if(!classic_theme_decode_selection(value,&selection))return ERROR_INVALID_DATA;
    return ntw16s_uwp_target_change(target,dll,apply,selection);
}
static void uwp_report(const style_uwp_target *target,DWORD error,void *context)
{
    (void)context;
#ifdef STYLE_GLOBAL_MANAGER_TEST
    printf("UWP_TARGET_RESULT pid=%lu frame=%p error=%lu\n",target->pid,(void *)target->frame,error);
#endif
    if(error && error!=ERROR_CANCELLED){WCHAR text[128];swprintf(text,ARRAYSIZE(text),L"run16s: UWP global frame %p failed (%lu)\n",(void *)target->frame,error);OutputDebugStringW(text);}
}
static BOOL uwp_services(DWORD style,DWORD cookie,WORD machine,style_uwp_global_services *services,void *context)
{
    (void)context;
    classic_theme_selection selection;
    if(sizeof(void *)!=8 || machine!=IMAGE_FILE_MACHINE_AMD64 ||
       !classic_theme_decode_selection(style,&selection))return FALSE;
    uint64_t context_bits=((uint64_t)style<<32)|cookie;
    *services=(style_uwp_global_services){machine,style_uwp_host_image,uwp_active,
        uwp_change,uwp_report,(void *)(uintptr_t)context_bits};
    return TRUE;
}
static void report(const style_global_report *detail,void *context)
{
    (void)context;WCHAR message[128];
    switch(detail->kind){
    case STYLE_GLOBAL_PROCESS_SCAN:
        if(swprintf(message,ARRAYSIZE(message),L"run16s: global process %lu scan failed (%lu)\n",detail->pid,detail->error)>0)OutputDebugStringW(message);
        break;
    case STYLE_GLOBAL_TRACE_UNAVAILABLE:
        OutputDebugStringW(L"run16s: global process events unavailable; using window discovery\n");break;
    case STYLE_GLOBAL_LOADER_RETAINED:
        fwprintf(stderr,L"run16s: global target %lu/%lu loader still running; remote path retained until target exit\n",detail->pid,detail->thread);break;
    case STYLE_GLOBAL_TARGET_STOP:
        fwprintf(stderr,L"run16s: global target %lu/%lu stop failed (%lu), worker=%lu loader=%lu phase=%lu\n",
            detail->pid,detail->thread,detail->error,detail->worker,detail->loader,detail->phase);break;
    case STYLE_GLOBAL_MONITOR_STOP:fwprintf(stderr,L"run16s: global monitor stop failed (%lu)\n",detail->error);break;
    case STYLE_GLOBAL_DISCOVERY_STOP:fwprintf(stderr,L"run16s: global discovery stop failed (%lu)\n",detail->error);break;
    case STYLE_GLOBAL_TARGETS_STOP:fwprintf(stderr,L"run16s: global targets stop failed (%lu)\n",detail->error);break;
    case STYLE_GLOBAL_UWP_STOP:fwprintf(stderr,L"run16s: global UWP stop failed (%lu)\n",detail->error);break;
    }
}
const style_global_manager_config ntw16s_global_manager_config={&ntw16s_global_attach_config,
    NTW16S_GLOBAL_TRACE_PREFIX,uwp_services,report,NULL};
