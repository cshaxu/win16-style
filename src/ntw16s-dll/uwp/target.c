#include "target.h"
#include "hook.h"
#include "native.h"
#include "lib-win32/process-image/image.h"
#include <dwmapi.h>
#include <stdint.h>
#include <wchar.h>

static BOOL class_is(HWND w,PCWSTR expected)
{WCHAR name[80];return GetClassNameW(w,name,ARRAYSIZE(name))&&!wcscmp(name,expected);}
static BOOL CALLBACK core_window(HWND w,LPARAM data)
{
    ntw16s_uwp_target *app=(ntw16s_uwp_target *)data;DWORD pid=0;GetWindowThreadProcessId(w,&pid);
    if((!app->pid||pid==app->pid)&&class_is(w,L"Windows.UI.Core.CoreWindow")){
        HWND root=GetAncestor(w,GA_ROOT);DWORD cloaked=1;
        if(root!=w&&class_is(root,L"ApplicationFrameWindow")&&IsWindowVisible(root)&&
           SUCCEEDED(DwmGetWindowAttribute(root,DWMWA_CLOAKED,&cloaked,sizeof(cloaked)))&&!cloaked){app->pid=pid;app->core=w;app->frame=root;return FALSE;}
    }
    return TRUE;
}
static BOOL CALLBACK top_window(HWND w,LPARAM data)
{core_window(w,data);if(!((ntw16s_uwp_target *)data)->frame)EnumChildWindows(w,core_window,data);return !((ntw16s_uwp_target *)data)->frame;}
BOOL ntw16s_uwp_target_find(DWORD pid,ntw16s_uwp_target *target)
{
    if(!target||!pid){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    *target=(ntw16s_uwp_target){.pid=pid};EnumWindows(top_window,(LPARAM)target);
    return target->frame!=NULL;
}
BOOL ntw16s_uwp_target_window(HWND window,ntw16s_uwp_target *target)
{
    if(!target||!window)return FALSE;
    *target=(ntw16s_uwp_target){0};
    HWND root=GetAncestor(window,GA_ROOT);
    if(!class_is(root,L"ApplicationFrameWindow"))return FALSE;
    core_window(window,(LPARAM)target);
    if(!target->frame)EnumChildWindows(root,core_window,(LPARAM)target);
    return target->frame!=NULL;
}
DWORD ntw16s_uwp_target_change(const ntw16s_uwp_target *app,PCWSTR path,BOOL apply)
{
    if(!app||!path||!*path||!class_is(app->core,L"Windows.UI.Core.CoreWindow")||
       !class_is(app->frame,L"ApplicationFrameWindow")||GetAncestor(app->core,GA_ROOT)!=app->frame)return ERROR_INVALID_WINDOW_HANDLE;
    DWORD core_pid=0;GetWindowThreadProcessId(app->core,&core_pid);
    if(core_pid!=app->pid)return ERROR_INVALID_WINDOW_HANDLE;
    DWORD host_pid=0,thread=GetWindowThreadProcessId(app->frame,&host_pid),error;
    HANDLE host=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,host_pid);
    if(!host)return GetLastError();
    WORD machine=0;error=win32_process_image_machine(host,&machine);CloseHandle(host);
    if(error)return error;
    if(machine!=IMAGE_FILE_MACHINE_AMD64)return ERROR_NOT_SUPPORTED;
    WPARAM action=NTW16S_UWP_REMOVE;
    if(apply){
        HWND auxiliary=NULL,title=ntw16s_uwp_title(app->frame,&auxiliary);
        if(!title)return ERROR_NOT_SUPPORTED;
        HRESULT capability=ntw16s_uwp_fullscreen_available(title);
        if(FAILED(capability))return (DWORD)capability;
        action=capability==S_OK?NTW16S_UWP_APPLY_FULLSCREEN:NTW16S_UWP_APPLY;
    }
    error=win32_process_image_validate(path,machine,TRUE);if(error)return error;
    HMODULE module=LoadLibraryExW(path,NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    if(!module)return GetLastError();
    HOOKPROC callback=(HOOKPROC)(uintptr_t)GetProcAddress(module,"Ntw16sUwpFrameHook");
    HHOOK hook=callback?SetWindowsHookExW(WH_CALLWNDPROC,callback,module,thread):NULL;
    error=hook?0:GetLastError();
    if(hook){
        RemovePropW(app->frame,NTW16S_UWP_RESULT);DWORD_PTR ignored=0;
        UINT message=RegisterWindowMessageW(NTW16S_UWP_MESSAGE);
        if(message)SetLastError(ERROR_SUCCESS);
        if(!SendMessageTimeoutW(app->frame,message,action,(LPARAM)app->core,
            SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,3000,&ignored))error=GetLastError()?GetLastError():ERROR_TIMEOUT;
        else {uintptr_t result=(uintptr_t)GetPropW(app->frame,NTW16S_UWP_RESULT);error=result?(DWORD)(result-1):ERROR_INVALID_DATA;}
        RemovePropW(app->frame,NTW16S_UWP_RESULT);
        if(!UnhookWindowsHookEx(hook)&&!error)error=GetLastError();
    }
    FreeLibrary(module);return error;
}
