#include "ntw16s-dll/global/window.h"
#include "ntw16s-dll/attachment.h"
#include "ntw16s-dll/global/session.h"
#include "lib-style/common/global_window.h"
#include "lib-win32/style/companion.h"

static const style_global_window_ops ops={
    Ntw16sWindowAttached,Ntw16sWindowAttach,Ntw16sWindowRemove,
    ntw16s_global_policy_active,NTW16S_GLOBAL_CONTROL,NTW16S_GLOBAL_QUERY,NTW16S_GLOBAL_ATTACHED
};
typedef BOOL (__cdecl *window_op)(HWND);
static INIT_ONCE win2k_once=INIT_ONCE_STATIC_INIT;
static style_global_window_ops win2k_ops={.active=ntw16s_global_policy_active,
    .control=NTW16S_GLOBAL_CONTROL,.query=NTW16S_GLOBAL_QUERY,.acknowledgement=NTW16S_GLOBAL_ATTACHED};
static BOOL CALLBACK load_win2k(PINIT_ONCE once,void *parameter,void **result)
{
    (void)once;(void)parameter;(void)result;
    static const char *const names[]={"Ntw2ksWindowAttached","Ntw2ksWindowAttach","Ntw2ksWindowRemove"};
    FARPROC entries[ARRAYSIZE(names)];
    if(!win32_style_companion((const void *)&load_win2k,
        sizeof(void *)==8?L"ntw2ks64.dll":L"ntw2ks32.dll",names,entries,ARRAYSIZE(entries),1))return FALSE;
    win2k_ops.attached=(window_op)(void *)entries[0];win2k_ops.apply=(window_op)(void *)entries[1];
    win2k_ops.remove=(window_op)(void *)entries[2];return TRUE;
}
static const style_global_window_ops *style_ops(DWORD style)
{
    if(style==NTW16S_GLOBAL_STYLE_16)return &ops;
    if(style==NTW16S_GLOBAL_STYLE_2K){
        if(InitOnceExecuteOnce(&win2k_once,load_win2k,NULL,NULL))return &win2k_ops;
        return NULL;
    }
    SetLastError(ERROR_INVALID_PARAMETER);return NULL;
}
BOOL ntw16s_global_window_apply_style(HWND window,DWORD style)
{
    const style_global_window_ops *selected=style_ops(style),*other=NULL;
    if(!selected)return FALSE;
    if(style==NTW16S_GLOBAL_STYLE_2K)other=&ops;
    else if(style==NTW16S_GLOBAL_STYLE_16 && GetModuleHandleW(sizeof(void *)==8?L"ntw2ks64.dll":L"ntw2ks32.dll")){
        other=style_ops(NTW16S_GLOBAL_STYLE_2K);
        if(!other)return FALSE;
    }
    return style_global_window_transition(window,selected,other);
}
BOOL ntw16s_global_window_remove_style(HWND window,DWORD style)
{const style_global_window_ops *selected=style_ops(style);return selected && style_global_window_remove(window,selected);}
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
