#define COBJMACROS
#include "uwp.h"
#include "ntw16s-dll/uwp/hook.h"
#include "ntw16s-dll/uwp/native.h"
#include "lib-win32/process-image/image.h"
#include <shobjidl.h>
#include <dwmapi.h>
#include <stdint.h>
#include <wchar.h>
#include <appmodel.h>

static BOOL same_image(PCWSTR first,PCWSTR second)
{
    HANDLE a=CreateFileW(first,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,0,NULL);
    if(a==INVALID_HANDLE_VALUE)return FALSE;
    HANDLE b=CreateFileW(second,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,0,NULL);
    LARGE_INTEGER x,y;BOOL same=b!=INVALID_HANDLE_VALUE&&GetFileSizeEx(a,&x)&&GetFileSizeEx(b,&y)&&x.QuadPart==y.QuadPart;
    BYTE left[4096],right[4096];DWORD n=0,m=0;
    while(same){
        same=ReadFile(a,left,sizeof(left),&n,NULL)&&ReadFile(b,right,sizeof(right),&m,NULL)&&n==m&&!memcmp(left,right,n);
        if(!same||!n)break;
    }
    if(b!=INVALID_HANDLE_VALUE)CloseHandle(b);
    CloseHandle(a);return same;
}
BOOL run16s_uwp_calculator_image(PCWSTR image)
{
    if(!image||!*image)return FALSE;
    PCWSTR name=wcsrchr(image,L'\\');
    if(!name||_wcsicmp(name+1,L"calc.exe"))return FALSE;
    WCHAR path[MAX_PATH];
    for(unsigned i=0;i<2;++i){
        UINT n=i?GetSystemWow64DirectoryW(path,ARRAYSIZE(path)):GetSystemDirectoryW(path,ARRAYSIZE(path));
        if(!n||n+10>=ARRAYSIZE(path))continue;
        wcscpy(path+n,L"\\calc.exe");
        if(!_wcsicmp(image,path)||same_image(image,path))return TRUE;
    }
    return FALSE;
}
typedef struct app_window {DWORD pid;HWND core,frame;} app_window;
static BOOL class_is(HWND w,PCWSTR expected)
{WCHAR name[80];return GetClassNameW(w,name,ARRAYSIZE(name))&&!wcscmp(name,expected);}
static BOOL CALLBACK core_window(HWND w,LPARAM data)
{
    app_window *app=(app_window *)data;DWORD pid=0;GetWindowThreadProcessId(w,&pid);
    if(pid==app->pid&&class_is(w,L"Windows.UI.Core.CoreWindow")){
        HWND root=GetAncestor(w,GA_ROOT);DWORD cloaked=1;
        if(root!=w&&class_is(root,L"ApplicationFrameWindow")&&IsWindowVisible(root)&&
           SUCCEEDED(DwmGetWindowAttribute(root,DWMWA_CLOAKED,&cloaked,sizeof(cloaked)))&&!cloaked){app->core=w;app->frame=root;return FALSE;}
    }
    return TRUE;
}
static BOOL CALLBACK top_window(HWND w,LPARAM data)
{core_window(w,data);if(!((app_window *)data)->frame)EnumChildWindows(w,core_window,data);return !((app_window *)data)->frame;}
static DWORD install(app_window *app)
{
    DWORD host_pid=0,thread=GetWindowThreadProcessId(app->frame,&host_pid),error;
    HANDLE host=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,host_pid);
    if(!host)return GetLastError();
    WORD machine=0;error=win32_process_image_machine(host,&machine);CloseHandle(host);
    if(error)return error;
    if(machine!=IMAGE_FILE_MACHINE_AMD64)return ERROR_NOT_SUPPORTED;
    HWND auxiliary=NULL,title=ntw16s_uwp_title(app->frame,&auxiliary);
    if(!title)return ERROR_NOT_SUPPORTED;
    HRESULT capability=ntw16s_uwp_fullscreen_available(title);
    if(FAILED(capability))return (DWORD)capability;
    WPARAM action=capability==S_OK?NTW16S_UWP_APPLY_FULLSCREEN:NTW16S_UWP_APPLY;
    WCHAR path[32768];DWORD length=GetModuleFileNameW(NULL,path,ARRAYSIZE(path));
    if(!length||length>=ARRAYSIZE(path))return ERROR_INSUFFICIENT_BUFFER;
    WCHAR *tail=wcsrchr(path,L'\\');if(!tail || (size_t)(tail-path)+20>=ARRAYSIZE(path))return ERROR_BAD_PATHNAME;
    wcscpy(tail+1,L"ntw16s64.dll");
    error=win32_process_image_validate(path,machine,TRUE);if(error)return error;
    HMODULE module=LoadLibraryExW(path,NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    if(!module)return GetLastError();
    HOOKPROC callback=(HOOKPROC)(uintptr_t)GetProcAddress(module,"Ntw16sUwpFrameHook");
    HHOOK hook=callback?SetWindowsHookExW(WH_CALLWNDPROC,callback,module,thread):NULL;
    error=hook?0:GetLastError();
    if(hook){
        RemovePropW(app->frame,NTW16S_UWP_RESULT);DWORD_PTR ignored=0;
        UINT message=RegisterWindowMessageW(NTW16S_UWP_MESSAGE);
        if(!SendMessageTimeoutW(app->frame,message,action,(LPARAM)app->core,
            SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,3000,&ignored))error=GetLastError()?GetLastError():ERROR_TIMEOUT;
        else {uintptr_t result=(uintptr_t)GetPropW(app->frame,NTW16S_UWP_RESULT);error=result?(DWORD)(result-1):ERROR_INVALID_DATA;}
        RemovePropW(app->frame,NTW16S_UWP_RESULT);
        if(!UnhookWindowsHookEx(hook)&&!error)error=GetLastError();
    }
    FreeLibrary(module);return error;
}
static DWORD activate(PCWSTR app_id,PCWSTR arguments,BOOL wait,DWORD *created_pid)
{
    if(created_pid)*created_pid=0;
    if(!app_id||!wcschr(app_id,L'!'))return ERROR_INVALID_PARAMETER;
    HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(hr))return (DWORD)hr;
    IApplicationActivationManager *manager=NULL;app_window app={0};FILETIME start;
    GetSystemTimeAsFileTime(&start);
    hr=CoCreateInstance(&CLSID_ApplicationActivationManager,NULL,CLSCTX_LOCAL_SERVER,
        &IID_IApplicationActivationManager,(void **)&manager);
    if(SUCCEEDED(hr)){
        CoAllowSetForegroundWindow((IUnknown *)manager,NULL);
        hr=IApplicationActivationManager_ActivateApplication(manager,app_id,arguments,AO_NOERRORUI,&app.pid);
        IApplicationActivationManager_Release(manager);
    }
    DWORD error=FAILED(hr)?(DWORD)hr:0;HANDLE process=NULL;
    if(!error){process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,app.pid);if(!process)error=GetLastError();}
    if(!error){
        FILETIME created,exited,kernel,user;
        if(!GetProcessTimes(process,&created,&exited,&kernel,&user))error=GetLastError();
        else if(created_pid&&CompareFileTime(&created,&start)>=0)*created_pid=app.pid;
    }
    if(!error){
        ULONGLONG deadline=GetTickCount64()+10000;
        do{
            EnumWindows(top_window,(LPARAM)&app);if(app.frame)break;
            DWORD wait=MsgWaitForMultipleObjects(1,&process,FALSE,50,QS_ALLINPUT);
            if(wait==WAIT_OBJECT_0){error=ERROR_PROCESS_ABORTED;break;}
            if(wait==WAIT_FAILED){error=GetLastError();break;}
            MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
        }while(GetTickCount64()<deadline);
        if(!error)error=app.frame?install(&app):ERROR_TIMEOUT;
    }
    if(!error&&wait&&WaitForSingleObject(process,INFINITE)==WAIT_FAILED)error=GetLastError();
    if(process)CloseHandle(process);
    CoUninitialize();return error;
}
DWORD run16s_uwp_activate(PCWSTR app_id,DWORD *created_pid)
{return activate(app_id,NULL,FALSE,created_pid);}
DWORD run16s_uwp_command(PCWSTR image,PCWSTR arguments,BOOL wait,BOOL *handled)
{
    if(!handled)return ERROR_INVALID_PARAMETER;
    *handled=FALSE;
    if(!run16s_uwp_calculator_image(image))return ERROR_SUCCESS;
    UINT32 count=0,length=0;
    LONG error=GetPackagesByPackageFamily(L"Microsoft.WindowsCalculator_8wekyb3d8bbwe",&count,NULL,&length,NULL);
    if(error!=ERROR_SUCCESS&&error!=ERROR_INSUFFICIENT_BUFFER)return (DWORD)error;
    if(!count)return ERROR_SUCCESS; /* Older Windows can still have native calc. */
    *handled=TRUE;
    return activate(L"Microsoft.WindowsCalculator_8wekyb3d8bbwe!App",arguments,wait,NULL);
}
