#include "target.h"
#include <dwmapi.h>
#include <wchar.h>
#include "lib-win32/style/window_identity.h"

BOOL style_uwp_host_image(PCWSTR image,PCWSTR windows_directory)
{
    WCHAR expected[MAX_PATH];
    int n=swprintf(expected,ARRAYSIZE(expected),L"%ls\\System32\\ApplicationFrameHost.exe",windows_directory);
    return n>0 && (size_t)n<ARRAYSIZE(expected) && !_wcsicmp(image,expected);
}

static BOOL CALLBACK core_window(HWND window,LPARAM data)
{
    style_uwp_target *app=(style_uwp_target *)data;DWORD pid=0;GetWindowThreadProcessId(window,&pid);
    if((!app->pid||pid==app->pid)&&win32_style_window_class(window,L"Windows.UI.Core.CoreWindow")){
        HWND root=GetAncestor(window,GA_ROOT);DWORD cloaked=1;
        if(root!=window&&win32_style_window_class(root,L"ApplicationFrameWindow")&&IsWindowVisible(root)&&
           SUCCEEDED(DwmGetWindowAttribute(root,DWMWA_CLOAKED,&cloaked,sizeof(cloaked)))&&!cloaked){app->pid=pid;app->core=window;app->frame=root;return FALSE;}
    }
    return TRUE;
}
static BOOL CALLBACK top_window(HWND window,LPARAM data)
{core_window(window,data);if(!((style_uwp_target *)data)->frame)EnumChildWindows(window,core_window,data);return !((style_uwp_target *)data)->frame;}
BOOL style_uwp_target_find(DWORD pid,style_uwp_target *target)
{
    if(!target||!pid){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    *target=(style_uwp_target){.pid=pid};EnumWindows(top_window,(LPARAM)target);
    return target->frame!=NULL;
}
BOOL style_uwp_target_window(HWND window,style_uwp_target *target)
{
    if(!target||!window)return FALSE;
    *target=(style_uwp_target){0};
    HWND root=GetAncestor(window,GA_ROOT);
    if(!win32_style_window_class(root,L"ApplicationFrameWindow"))return FALSE;
    core_window(window,(LPARAM)target);
    if(!target->frame)EnumChildWindows(root,core_window,(LPARAM)target);
    return target->frame!=NULL;
}
BOOL style_uwp_target_valid(const style_uwp_target *target)
{
    if(!target||!win32_style_window_class(target->core,L"Windows.UI.Core.CoreWindow")||
       !win32_style_window_class(target->frame,L"ApplicationFrameWindow")||GetAncestor(target->core,GA_ROOT)!=target->frame)return FALSE;
    DWORD pid=0;GetWindowThreadProcessId(target->core,&pid);return pid==target->pid;
}
