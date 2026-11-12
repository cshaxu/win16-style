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
static window_op win2k_attached,win2k_apply,win2k_remove;
static BOOL CALLBACK load_win2k(PINIT_ONCE once,void *parameter,void **result)
{
    (void)once;(void)parameter;(void)result;
    static const char *const names[]={"Ntw2ksWindowAttached","Ntw2ksWindowAttach","Ntw2ksWindowRemove"};
    FARPROC entries[ARRAYSIZE(names)];
    if(!win32_style_companion((const void *)&load_win2k,
        sizeof(void *)==8?L"ntw2ks64.dll":L"ntw2ks32.dll",names,entries,ARRAYSIZE(entries),1))return FALSE;
    win2k_attached=(window_op)(void *)entries[0];win2k_apply=(window_op)(void *)entries[1];
    win2k_remove=(window_op)(void *)entries[2];return TRUE;
}
static BOOL win2k_is_attached(HWND window){return win2k_attached(window);}
static BOOL win2k_attach(HWND window){return win2k_apply(window);}
static BOOL win2k_detach(HWND window){return win2k_remove(window);}
static const style_global_window_ops win2k_ops={win2k_is_attached,win2k_attach,win2k_detach,
    ntw16s_global_policy_active,NTW16S_GLOBAL_CONTROL,NTW16S_GLOBAL_QUERY,NTW16S_GLOBAL_ATTACHED};
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
BOOL ntw16s_global_window_attached(HWND window)
{return style_global_window_attached(window,&ops);}
BOOL ntw16s_global_window_apply(HWND window)
{return style_global_window_apply(window,&ops);}
BOOL ntw16s_global_window_remove(HWND window)
{return style_global_window_remove(window,&ops);}
