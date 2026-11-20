/* Product activation and appearance policy; native call state is shared. */
#include "intercept.h"
#include "lib-win32-style/application/dialog/classic_file_controller.h"
#include "lib-win32-style/appearance/win2000/dialog/file_call.h"
#include "ntw16s-dll/global/session.h"
#include "ntw16s-dll/global/window.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
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

    extern IMAGE_DOS_HEADER __ImageBase;
    classic_theme_mapping source={0};
    HRESULT result=classic_theme_open_profile((HMODULE)&__ImageBase,profile,&source);
    if(SUCCEEDED(result)){
        unsigned fallback=source.family==CLASSIC_THEME_WIN2K?CLASSIC_DIALOG_WIN2K:CLASSIC_DIALOG_WIN16;
        policy=source.model.dialog_policy;
        if(!policy.appearance)policy=(classic_dialog_policy){fallback,fallback,fallback,fallback};
        classic_theme_close(&source);
    }else if(profile<CLASSIC_THEME_PROFILE_FIRST_CUSTOM &&
       (result==HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND) || result==HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))){
        /* Older standalone DLL fixtures may use embedded appearance resources. */
        unsigned fallback=classic_theme_profile_family(profile)==CLASSIC_THEME_WIN2K?
            CLASSIC_DIALOG_WIN2K:CLASSIC_DIALOG_WIN16;
        policy=(classic_dialog_policy){fallback,fallback,fallback,fallback};
    }else policy=(classic_dialog_policy){0};

    AcquireSRWLockExclusive(&lock);
    cached=policy;cached_profile=profile;
    ReleaseSRWLockExclusive(&lock);
    return policy;
}

classic_dialog_route ntw16s_dialog_route(ntw_dialog_kind kind)
{
    DWORD incoming=GetLastError();
    DWORD flags=__atomic_load_n(&ntw16s_process_context.flags,__ATOMIC_ACQUIRE);
    DWORD profile=classic_theme_process_profile();
    if(flags&NTWSHK_POLICY_GLOBAL_SESSION){
        DWORD cookie=__atomic_load_n(&ntw16s_process_context.session_cookie,__ATOMIC_ACQUIRE);
        DWORD current;
        classic_theme_selection selection;
        if(!style_global_policy_current(&ntw16s_global_namespace,cookie,&current) ||
           !classic_theme_decode_selection(current,&selection)){
            SetLastError(incoming);return CLASSIC_DIALOG_NATIVE;
        }
        profile=selection.id;
    }
    classic_dialog_policy policy=read_policy(profile);
    SetLastError(incoming);
    switch(kind){
    case NTW_DIALOG_FILE:return (classic_dialog_route)policy.file;
    case NTW_DIALOG_FOLDER:return (classic_dialog_route)policy.folder;
    case NTW_DIALOG_PRINT:return (classic_dialog_route)policy.print;
    }
    return CLASSIC_DIALOG_NATIVE;
}

static BOOL file_win16(void){return ntw16s_dialog_route(NTW_DIALOG_FILE)==CLASSIC_DIALOG_WIN16;}
static BOOL file_win2k(void){return ntw16s_dialog_route(NTW_DIALOG_FILE)==CLASSIC_DIALOG_WIN2K;}

void ntw16s_win2k_dialog_initialize(HWND window)
{
#ifdef NTWSHK_UNIFIED
    DWORD flags=__atomic_load_n(&ntw16s_process_context.flags,__ATOMIC_ACQUIRE);
    if(!(flags&NTWSHK_POLICY_GLOBAL_SESSION)){
        ntw2ks_dialog_initialize(window);return;
    }
#endif
    DWORD style;
    DWORD cookie=__atomic_load_n(&ntw16s_process_context.session_cookie,__ATOMIC_ACQUIRE);
    classic_theme_selection selection;
    if(ntw16s_global_window_eligible(window) &&
       style_global_policy_current(&ntw16s_global_namespace,cookie,&style) &&
       classic_theme_decode_selection(style,&selection)){
        if(selection.family!=CLASSIC_THEME_WIN2K)style=NTW16S_GLOBAL_STYLE_2K;
        ntw16s_global_window_apply_style(window,style);
        ntw16s_global_window_children(window,TRUE,style);
    }
}
static LONG attach_dialogs(DWORD (*attach)(void *,void *))
{
    if(!attach)return ERROR_INVALID_PARAMETER;
    const style_classic_file_options options={.active=file_win16,
        .scope=ntw16s_dialog_style_scope,.chrome=ntw16s_old_dialog_chrome,
        .alternate_active=file_win2k,.alternate_a=win2k_file_call_a,
        .alternate_w=win2k_file_call_w,.alternate_windows={ntw16s_win2k_dialog_initialize,ntw16s_global_win2k_attached,ntw16s_global_win2k_resources},
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
