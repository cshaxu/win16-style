#define COBJMACROS
#include "uwp.h"
#include "ntw16s-dll/uwp/hook.h"
#include "lib-win32/process-image/image.h"
#include <shobjidl.h>
#include <dwmapi.h>
#include <stdint.h>
#include <wchar.h>
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
        if(!SendMessageTimeoutW(app->frame,message,NTW16S_UWP_APPLY,(LPARAM)app->core,
            SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,3000,&ignored))error=GetLastError()?GetLastError():ERROR_TIMEOUT;
        else {uintptr_t result=(uintptr_t)GetPropW(app->frame,NTW16S_UWP_RESULT);error=result?(DWORD)(result-1):ERROR_INVALID_DATA;}
        RemovePropW(app->frame,NTW16S_UWP_RESULT);
        if(!UnhookWindowsHookEx(hook)&&!error)error=GetLastError();
    }
    FreeLibrary(module);return error;
}
DWORD run16s_uwp_activate(PCWSTR app_id,DWORD *created_pid)
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
        hr=IApplicationActivationManager_ActivateApplication(manager,app_id,NULL,AO_NOERRORUI,&app.pid);
        IApplicationActivationManager_Release(manager);
    }
    DWORD error=FAILED(hr)?(DWORD)hr:0;HANDLE process=NULL;
    if(!error){process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,app.pid);if(!process)error=GetLastError();}
    if(!error){
        FILETIME created,exited,kernel,user;
        if(!GetProcessTimes(process,&created,&exited,&kernel,&user))error=GetLastError();
        else if(CompareFileTime(&created,&start)<0)error=ERROR_ALREADY_EXISTS;
    }
    if(!error){
        if(created_pid)*created_pid=app.pid;
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
    if(process)CloseHandle(process);
    CoUninitialize();return error;
}
