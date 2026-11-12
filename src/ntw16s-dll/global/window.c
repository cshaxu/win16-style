#include "ntw16s-dll/global/window.h"
#include "ntw16s-dll/attachment.h"
#include "ntw16s-dll/global/session.h"
#include "lib-win32/style-common/global_window.h"
#include "lib-win32/process-global/module.h"
#include <wchar.h>

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
    HMODULE self=NULL,module=NULL,pinned=NULL;WCHAR path[32768],actual[32768];
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCWSTR)(void *)&load_win2k,&self))return FALSE;
    DWORD length=GetModuleFileNameW(self,path,ARRAYSIZE(path));
    if(!length || length>=ARRAYSIZE(path)){SetLastError(ERROR_INSUFFICIENT_BUFFER);return FALSE;}
    WCHAR *slash=wcsrchr(path,L'\\');if(!slash){SetLastError(ERROR_INVALID_NAME);return FALSE;}
    size_t remaining=ARRAYSIZE(path)-(size_t)(slash+1-path);
    if(wcscpy_s(slash+1,remaining,sizeof(void *)==8?L"ntw2ks64.dll":L"ntw2ks32.dll")){SetLastError(ERROR_INSUFFICIENT_BUFFER);return FALSE;}
    module=LoadLibraryW(path);if(!module)return FALSE;
    length=GetModuleFileNameW(module,actual,ARRAYSIZE(actual));
    if(!length || length>=ARRAYSIZE(actual) || !win32_process_global_files_equal(path,actual)){
        FreeLibrary(module);SetLastError(ERROR_INVALID_IMAGE_HASH);return FALSE;
    }
    window_op attached=(window_op)(void *)GetProcAddress(module,"Ntw2ksWindowAttached");
    window_op apply=(window_op)(void *)GetProcAddress(module,"Ntw2ksWindowAttach");
    window_op remove=(window_op)(void *)GetProcAddress(module,"Ntw2ksWindowRemove");
    if(!attached || !apply || !remove || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
                                                            (LPCWSTR)(void *)apply,&pinned)){
        DWORD error=GetLastError();FreeLibrary(module);SetLastError(error?error:ERROR_PROC_NOT_FOUND);return FALSE;
    }
    win2k_attached=attached;win2k_apply=apply;win2k_remove=remove;FreeLibrary(module);return TRUE;
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
