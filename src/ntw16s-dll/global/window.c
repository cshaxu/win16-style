#include "ntw16s-dll/global/window.h"
#include "ntw16s-dll/attachment.h"
#include "ntw16s-dll/global/session.h"
#include "lib-win32-style/application/lifecycle/global_window.h"
#include "lib-win32-style/access/runtime/companion.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"

#ifdef NTWSHK_UNIFIED
#include "ntw2ks-dll/window.h"
extern IMAGE_DOS_HEADER __ImageBase;
#endif

static const style_global_window_ops ops={
    Ntw16sWindowAttached,Ntw16sWindowAttach,Ntw16sWindowRemove,
    ntw16s_global_policy_active,NTW16S_GLOBAL_CONTROL,NTW16S_GLOBAL_QUERY,NTW16S_GLOBAL_ATTACHED
};
typedef BOOL (__cdecl *window_op)(HWND);
static INIT_ONCE win2k_once=INIT_ONCE_STATIC_INIT;
static HMODULE win2k_module;
static style_global_window_ops win2k_ops={.active=ntw16s_global_policy_active,
    .control=NTW16S_GLOBAL_CONTROL,.query=NTW16S_GLOBAL_QUERY,.acknowledgement=NTW16S_GLOBAL_ATTACHED};
static BOOL CALLBACK load_win2k(PINIT_ONCE once,void *parameter,void **result)
{
    (void)once;(void)parameter;(void)result;
#ifdef NTWSHK_UNIFIED
    win2k_module=(HMODULE)&__ImageBase;
    win2k_ops.attached=Ntw2ksWindowAttached;
    win2k_ops.apply=Ntw2ksWindowAttach;
    win2k_ops.remove=Ntw2ksWindowRemove;
#else
    static const char *const names[]={"Ntw2ksWindowAttached","Ntw2ksWindowAttach","Ntw2ksWindowRemove"};
    FARPROC entries[ARRAYSIZE(names)];
    if(!win32_style_companion((const void *)&load_win2k,
        sizeof(void *)==8?L"ntw2ks64.dll":L"ntw2ks32.dll",names,entries,ARRAYSIZE(entries),1))return FALSE;
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCWSTR)(void *)entries[1],&win2k_module))return FALSE;
    win2k_ops.attached=(window_op)(void *)entries[0];win2k_ops.apply=(window_op)(void *)entries[1];
    win2k_ops.remove=(window_op)(void *)entries[2];
#endif
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
static const style_global_window_ops *style_ops(DWORD style)
{
    style&=1u;
    if(style==NTW16S_GLOBAL_STYLE_16)return &ops;
    if(style==NTW16S_GLOBAL_STYLE_2K){
        if(ntw16s_global_win2k_resources())return &win2k_ops;
        return NULL;
    }
    SetLastError(ERROR_INVALID_PARAMETER);return NULL;
}
BOOL ntw16s_global_window_apply_style(HWND window,DWORD style)
{
    static const WCHAR property[]=L"Win16Style.Global.Profile";
    const style_global_window_ops *selected=style_ops(style),*other=NULL;
    if(!selected)return FALSE;
    if(style&1u)other=&ops;
#ifdef NTWSHK_UNIFIED
    else other=style_ops(NTW16S_GLOBAL_STYLE_2K);
#else
    else if(style==NTW16S_GLOBAL_STYLE_16 && GetModuleHandleW(sizeof(void *)==8?L"ntw2ks64.dll":L"ntw2ks32.dll")){
        other=style_ops(NTW16S_GLOBAL_STYLE_2K);
        if(!other)return FALSE;
    }
#endif
    HANDLE mark=GetPropW(window,property),wanted=(HANDLE)(UINT_PTR)(style+1u);
    if(mark!=wanted && (style>=CLASSIC_THEME_PROFILE_FIRST_CUSTOM || mark) && selected->attached(window) &&
       !style_global_window_remove(window,selected))return FALSE;
    unsigned previous=classic_theme_enter_profile(style);
    BOOL applied=style_global_window_transition(window,selected,other);
    classic_theme_leave_profile(previous);
    if(applied)(void)SetPropW(window,property,wanted);
    return applied;
}
BOOL ntw16s_global_window_remove_style(HWND window,DWORD style)
{
    const style_global_window_ops *selected=style_ops(style);
    if(!selected || !style_global_window_remove(window,selected))return FALSE;
    RemovePropW(window,L"Win16Style.Global.Profile");return TRUE;
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
{return style_global_window_attached(window,&ops);}
BOOL ntw16s_global_window_apply(HWND window)
{return style_global_window_apply(window,&ops);}
BOOL ntw16s_global_window_remove(HWND window)
{return style_global_window_remove(window,&ops);}
