/* Product activation and appearance policy; native call state is shared. */
#include "intercept.h"
#include "lib-win32-style/application/dialog/classic_file_controller.h"
#include "lib-win32-style/appearance/win2000/dialog/file_call.h"
#include "ntw16s-dll/global/session.h"
#include "ntw16s-dll/global/window.h"
#include "ntw16s-dll/attachment.h"
#include "lib-win32-style/application/lifecycle/global_window.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
extern IMAGE_DOS_HEADER __ImageBase;

static classic_dialog_policy read_policy(unsigned profile)
{
    static SRWLOCK lock=SRWLOCK_INIT;
    static unsigned cached_profile=UINT_MAX;
    static classic_dialog_policy cached;
    AcquireSRWLockShared(&lock);
    BOOL match=cached_profile==profile;
    classic_dialog_policy policy=cached;
    ReleaseSRWLockShared(&lock);
    if(match)return policy;

    policy=classic_theme_dialog_policy_for_profile((HMODULE)&__ImageBase,profile);

    AcquireSRWLockExclusive(&lock);
    cached=policy;cached_profile=profile;
    ReleaseSRWLockExclusive(&lock);
    return policy;
}

static BOOL dialog_selection(classic_theme_selection *selection,DWORD *encoded,BOOL *global)
{
    DWORD flags=__atomic_load_n(&ntw16s_process_context.flags,__ATOMIC_ACQUIRE);
    *global=(flags&NTWSHK_POLICY_GLOBAL_SESSION)!=0;
    if(*global){
        DWORD cookie=__atomic_load_n(&ntw16s_process_context.session_cookie,__ATOMIC_ACQUIRE);
        return style_global_policy_current(&ntw16s_global_namespace,cookie,encoded) &&
            classic_theme_decode_selection(*encoded,selection);
    }
    *selection=(classic_theme_selection){classic_theme_process_profile(),classic_theme_process_family()};
    return TRUE;
}

classic_dialog_policy ntw16s_dialog_policy(void)
{
    DWORD incoming=GetLastError(),encoded;
    BOOL global;
    classic_theme_selection selection;
    classic_dialog_policy policy={0};
    if(dialog_selection(&selection,&encoded,&global))policy=read_policy(selection.id);
    SetLastError(incoming);
    return policy;
}

static BOOL file_legacy_layout(void){return ntw16s_dialog_policy().file==CLASSIC_FILE_LEGACY_LAYOUT;}
static BOOL file_explorer_layout(void){return ntw16s_dialog_policy().file==CLASSIC_FILE_EXPLORER_LAYOUT;}

static BOOL attach_dialog_child(HWND window,void *context)
{
    (void)context;
    if(!Ntw16sWindowAttached(window))(void)Ntw16sWindowAttach(window);
    return TRUE;
}

void ntw16s_observed_dialog_initialize(HWND window)
{
    DWORD encoded;
    BOOL global;
    classic_theme_selection selection;
    if(!dialog_selection(&selection,&encoded,&global))return;
    if(global){
        if(ntw16s_global_window_eligible(window)){
            (void)ntw16s_global_window_apply_style(window,encoded);
            ntw16s_global_window_children(window,TRUE,encoded);
        }
        return;
    }
#ifdef NTWSHK_UNIFIED
    if(selection.family==CLASSIC_THEME_WIN2K){
        ntw2ks_dialog_initialize(window);
        return;
    }
#endif
    if(!Ntw16sWindowAttached(window))(void)Ntw16sWindowAttach(window);
    (void)style_global_window_visit_children(window,attach_dialog_child,NULL);
}

static BOOL observed_dialog_attached(HWND window)
{
    if(Ntw16sWindowAttached(window))return TRUE;
    classic_theme_selection selection;
    DWORD encoded;
    BOOL global;
    return dialog_selection(&selection,&encoded,&global) && selection.family==CLASSIC_THEME_WIN2K &&
        ntw16s_global_win2k_attached(window);
}

static HMODULE observed_dialog_resources(void)
{
#ifdef NTWSHK_UNIFIED
    return (HMODULE)&__ImageBase;
#else
    return ntw16s_global_win2k_resources();
#endif
}

const style_dialog_windows ntw16s_observed_dialog_windows={
    ntw16s_observed_dialog_initialize,observed_dialog_attached,observed_dialog_resources
};

static LONG attach_dialogs(DWORD (*attach)(void *,void *))
{
    if(!attach)return ERROR_INVALID_PARAMETER;
    const style_classic_file_options options={.active=file_legacy_layout,
        .scope=ntw16s_dialog_style_scope,.chrome=ntw16s_old_dialog_chrome,
        .alternate_active=file_explorer_layout,.alternate_a=win2k_file_call_a,
        .alternate_w=win2k_file_call_w,.alternate_windows=ntw16s_observed_dialog_windows,
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
        .subclass_observer=TRUE,
#endif
    };
    return (LONG)style_classic_file_attach(attach,&options);
}

#ifdef NTWSHK_UNIFIED
static BOOL win2k_startup_hooks;
LONG ntw16s_attach_win2k_file_dialogs_using(DWORD (*attach)(void *,void *))
{
    LONG error=attach_dialogs(attach);
    if(!error)win2k_startup_hooks=TRUE;
    return error;
}
#endif
LONG ntw16s_attach_file_dialogs_using(DWORD (*attach)(void *,void *))
{
    if(!attach)return ERROR_INVALID_PARAMETER;
#ifdef NTWSHK_UNIFIED
    if(win2k_startup_hooks)return NO_ERROR;
#endif
    return (ntw16s_process_context.flags&NTWSHK_POLICY_STYLE)?attach_dialogs(attach):NO_ERROR;
}
