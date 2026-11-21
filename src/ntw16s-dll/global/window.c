#include "ntw16s-dll/global/window.h"
#include "ntw16s-dll/attachment.h"
#include "ntw16s-dll/global/session.h"
#include "lib-win32-style/application/lifecycle/global_window.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"

#ifdef NTWSHK_UNIFIED
#include "ntwshk-dll/observed.h"
#else
#include "lib-win32-style/access/runtime/companion.h"
#endif

static const style_global_window_ops full_client_ops={
    Ntw16sWindowAttached,Ntw16sWindowAttach,Ntw16sWindowRemove,
    ntw16s_global_policy_active,NTW16S_GLOBAL_CONTROL,NTW16S_GLOBAL_QUERY,NTW16S_GLOBAL_ATTACHED
};
#ifdef NTWSHK_UNIFIED
static const style_global_window_ops nonclient_ops={
    ntwshk_observed_window_attached,ntwshk_observed_window_attach,ntwshk_observed_window_remove,
    ntw16s_global_policy_active,NTW16S_GLOBAL_CONTROL,NTW16S_GLOBAL_QUERY,NTW16S_GLOBAL_ATTACHED
};
#else
typedef BOOL (__cdecl *window_op)(HWND);
static INIT_ONCE win2k_once=INIT_ONCE_STATIC_INIT;
static HMODULE win2k_module;
static style_global_window_ops win2k_ops={.active=ntw16s_global_policy_active,
    .control=NTW16S_GLOBAL_CONTROL,.query=NTW16S_GLOBAL_QUERY,.acknowledgement=NTW16S_GLOBAL_ATTACHED};
static BOOL CALLBACK load_win2k(PINIT_ONCE once,void *parameter,void **result)
{
    (void)once;(void)parameter;(void)result;
    static const char *const names[]={"Ntw2ksWindowAttached","Ntw2ksWindowAttach","Ntw2ksWindowRemove"};
    FARPROC entries[ARRAYSIZE(names)];
    if(!win32_style_companion((const void *)&load_win2k,
        sizeof(void *)==8?L"ntw2ks64.dll":L"ntw2ks32.dll",names,entries,ARRAYSIZE(entries),1))return FALSE;
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCWSTR)(void *)entries[1],&win2k_module))return FALSE;
    win2k_ops.attached=(window_op)(void *)entries[0];win2k_ops.apply=(window_op)(void *)entries[1];
    win2k_ops.remove=(window_op)(void *)entries[2];
    return TRUE;
}
HMODULE ntw16s_global_win2k_resources(void)
{
    return InitOnceExecuteOnce(&win2k_once,load_win2k,NULL,NULL)?win2k_module:NULL;
}
BOOL ntw16s_global_win2k_attached(HWND window)
{
    return ntw16s_global_win2k_resources() && win2k_ops.attached(window);
}
#endif
static const style_global_window_ops *style_ops(enum classic_window_provider layout)
{
    switch(layout){
    case CLASSIC_WINDOW_PROVIDER_FULL_CLIENT:return &full_client_ops;
    case CLASSIC_WINDOW_PROVIDER_NONCLIENT:
#ifdef NTWSHK_UNIFIED
        return &nonclient_ops;
#else
        return ntw16s_global_win2k_resources()?&win2k_ops:NULL;
#endif
    default:
        SetLastError(ERROR_INVALID_PARAMETER);
        return NULL;
    }
}
static BOOL apply_profile(HWND window,BOOL (*apply)(HWND),DWORD profile)
{
    if(!profile)return apply(window);
    classic_theme_selection selection;
    if(!classic_theme_decode_selection(profile-1u,&selection)){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    unsigned previous=classic_theme_enter_profile(selection);
    BOOL applied=apply(window);
    classic_theme_leave_profile(previous);
    return applied;
}
BOOL ntw16s_global_window_apply_style(HWND window,DWORD style)
{
    classic_theme_selection selection;
    if(!classic_theme_decode_selection(style,&selection)){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    const style_global_window_ops *selected=style_ops(classic_theme_selection_layout(selection));
    if(!selected)return FALSE;
#ifdef NTWSHK_UNIFIED
    const style_global_window_ops *other=selected==&full_client_ops?&nonclient_ops:&full_client_ops;
#else
    const style_global_window_ops *other=NULL;
    if(selected!=&full_client_ops)other=&full_client_ops;
    else if(GetModuleHandleW(sizeof(void *)==8?L"ntw2ks64.dll":L"ntw2ks32.dll")){
        other=style_ops(CLASSIC_WINDOW_PROVIDER_NONCLIENT);
        if(!other)return FALSE;
    }
#endif
    return style_global_window_transition(window,selected,other,style+1u,
        selection.id>=CLASSIC_THEME_PROFILE_FIRST_CUSTOM,apply_profile);
}
BOOL ntw16s_global_window_remove_style(HWND window,DWORD style)
{
    if(style_global_window_remove_current(window))return TRUE;
    if(GetLastError()!=ERROR_NOT_FOUND)return FALSE;
    classic_theme_selection selection;
    if(!classic_theme_decode_selection(style,&selection)){
        SetLastError(ERROR_INVALID_PARAMETER);return FALSE;
    }
    const style_global_window_ops *selected=style_ops(classic_theme_selection_layout(selection));
    return selected && style_global_window_remove(window,selected);
}
BOOL ntw16s_global_window_eligible(HWND window)
{
    WCHAR name[64];DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    if(!GetClassNameW(window,name,ARRAYSIZE(name)))return FALSE;
    if(style&WS_CHILD)return TRUE;
    return (style&WS_CAPTION)==WS_CAPTION && !(GetWindowLongPtrW(window,GWL_EXSTYLE)&(WS_EX_TOOLWINDOW|WS_EX_LAYERED));
}
typedef struct child_action {BOOL apply;DWORD style;} child_action;
static BOOL child_window(HWND window,void *value)
{
    const child_action *action=value;
    if(action->apply && ntw16s_global_window_eligible(window))(void)ntw16s_global_window_apply_style(window,action->style);
    else if(!action->apply)(void)ntw16s_global_window_remove_style(window,action->style);
    return TRUE;
}
void ntw16s_global_window_children(HWND window,BOOL apply,DWORD style)
{child_action action={apply,style};(void)style_global_window_visit_children(window,child_window,&action);}
BOOL ntw16s_global_window_attached(HWND window)
{return style_global_window_attached(window,&full_client_ops);}
#ifndef NTWSHK_UNIFIED
BOOL ntw16s_global_window_apply(HWND window)
{return style_global_window_apply(window,&full_client_ops);}
BOOL ntw16s_global_window_remove(HWND window)
{return style_global_window_remove(window,&full_client_ops);}
#endif
