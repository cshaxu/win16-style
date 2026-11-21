/* Product activation and appearance policy; native call state is shared. */
#include "intercept.h"
#include "lib-win32-style/application/dialog/classic_file_controller.h"
#include "lib-win32-style/application/dialog/explorer_file.h"
#include "ntw16s-dll/global/session.h"
#include "ntw16s-dll/global/window.h"
#include "ntw16s-dll/attachment.h"
#ifdef NTWSHK_UNIFIED
#include "ntwshk-dll/observed.h"
#endif
#include "lib-win32-style/application/lifecycle/global_window.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
extern IMAGE_DOS_HEADER __ImageBase;

#ifndef NTWSHK_UNIFIED
/* Only the retired standalone provider fixtures can run without a theme. */
static classic_dialog_policy compatibility_dialog_policy(classic_theme_family family)
{
    if(family==CLASSIC_THEME_WIN2K)return (classic_dialog_policy){CLASSIC_WINDOW_PROVIDER_NONCLIENT,
        CLASSIC_FILE_EXPLORER_LAYOUT,CLASSIC_FOLDER_OBSERVED_NATIVE,CLASSIC_PRINT_GENERAL_PAGE,
        CLASSIC_STOCK_OBSERVED_LAYOUT,CLASSIC_STOCK_OBSERVED_LAYOUT,CLASSIC_STOCK_OBSERVED_LAYOUT};
    return (classic_dialog_policy){CLASSIC_WINDOW_PROVIDER_FULL_CLIENT,
        CLASSIC_FILE_LEGACY_LAYOUT,CLASSIC_FOLDER_LEGACY_TREE,CLASSIC_PRINT_LEGACY_BRIDGE,
        CLASSIC_STOCK_OWNER_CHROME,CLASSIC_STOCK_OWNER_CHROME,CLASSIC_STOCK_OWNER_CHROME};
}
#endif

static classic_dialog_policy read_policy(classic_theme_selection selection)
{
    static SRWLOCK lock=SRWLOCK_INIT;
    static unsigned cached_selection=UINT_MAX;
    const unsigned key=classic_theme_encode_selection(selection);
    static classic_dialog_policy cached;
    AcquireSRWLockShared(&lock);
    BOOL match=cached_selection==key;
    classic_dialog_policy policy=cached;
    ReleaseSRWLockShared(&lock);
    if(match)return policy;

    classic_theme_mapping source={0};
    HRESULT opened=classic_theme_open_selected((HMODULE)&__ImageBase,selection,&source);
    policy=source.model.dialog_policy;
    classic_theme_close(&source);
#ifndef NTWSHK_UNIFIED
    if(selection.id<CLASSIC_THEME_PROFILE_FIRST_CUSTOM &&
       (opened==HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) || opened==HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND)))
        policy=compatibility_dialog_policy(selection.family);
#endif
    if(FAILED(opened))return policy;

    AcquireSRWLockExclusive(&lock);
    cached=policy;cached_selection=key;
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
    HRESULT active=classic_theme_active_selection(selection);
#ifndef NTWSHK_UNIFIED
    if(active==S_FALSE){
        *selection=(classic_theme_selection){0,CLASSIC_THEME_WIN16};
        return TRUE;
    }
#endif
    return active==S_OK;
}

classic_dialog_policy ntw16s_dialog_policy(void)
{
    DWORD incoming=GetLastError(),encoded;
    BOOL global;
    classic_theme_selection selection;
    classic_dialog_policy policy={0};
    if(dialog_selection(&selection,&encoded,&global))policy=read_policy(selection);
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
    unsigned layout=read_policy(selection).window_provider;
    if(layout==CLASSIC_WINDOW_PROVIDER_NONCLIENT){
        ntwshk_observed_dialog_initialize(window);
        return;
    }
    if(layout!=CLASSIC_WINDOW_PROVIDER_FULL_CLIENT)return;
#endif
    if(!Ntw16sWindowAttached(window))(void)Ntw16sWindowAttach(window);
    (void)style_global_window_visit_children(window,attach_dialog_child,NULL);
}

static BOOL observed_dialog_attached(HWND window)
{
    if(Ntw16sWindowAttached(window))return TRUE;
#ifdef NTWSHK_UNIFIED
    return ntwshk_observed_window_attached(window);
#else
    classic_theme_selection selection;
    DWORD encoded;
    BOOL global;
    return dialog_selection(&selection,&encoded,&global) && selection.family==CLASSIC_THEME_WIN2K &&
        ntw16s_global_win2k_attached(window);
#endif
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
    ntw16s_observed_dialog_initialize,observed_dialog_attached,observed_dialog_resources,classic_theme_dialog_metrics
};

LONG ntw16s_register_file_dialogs(DWORD (*attach)(void *,void *))
{
    if(!attach)return ERROR_INVALID_PARAMETER;
    const style_classic_file_options options={.active=file_legacy_layout,
        .scope=ntw16s_dialog_style_scope,.chrome=ntw16s_old_dialog_chrome,
        .alternate_active=file_explorer_layout,.alternate_a=style_explorer_file_a,
        .alternate_w=style_explorer_file_w,.alternate_windows=ntw16s_observed_dialog_windows,
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
        .subclass_observer=TRUE,
#endif
    };
    return (LONG)style_classic_file_attach(attach,&options);
}
