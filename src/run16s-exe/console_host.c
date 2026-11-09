#include "console_host.h"
#include "window_target.h"
#include "ntw16s-dll/window_thread.h"
#include <stdint.h>
#include <tlhelp32.h>
#include <wchar.h>

BOOL run16s_console_host_image(PCWSTR image)
{
    WCHAR system[MAX_PATH];UINT n=GetSystemDirectoryW(system,ARRAYSIZE(system));
    if(!image || !n || n+13>=ARRAYSIZE(system))return FALSE;
    wcscat(system,L"\\conhost.exe");return !_wcsicmp(image,system);
}

DWORD run16s_console_host_attach_current(PCWSTR directory)
{
    HWND console=GetConsoleWindow();WCHAR name[64];
    if(!console || !GetClassNameW(console,name,ARRAYSIZE(name)) || wcscmp(name,L"PseudoConsoleWindow"))
        return ERROR_NOT_FOUND;
    HWND root=GetAncestor(console,GA_ROOTOWNER);DWORD pid=0;
    DWORD thread=root?GetWindowThreadProcessId(root,&pid):0;
    if(!thread || !pid || !GetClassNameW(root,name,ARRAYSIZE(name)) ||
       wcscmp(name,L"CASCADIA_HOSTING_WINDOW_CLASS"))return ERROR_NOT_FOUND;
    HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,pid);
    if(!process)return GetLastError();
    WCHAR image[32768];DWORD length=ARRAYSIZE(image);
    BOOL ok=QueryFullProcessImageNameW(process,0,image,&length);
    WCHAR *base=ok?wcsrchr(image,L'\\'):NULL;
    if(!base || _wcsicmp(base+1,L"WindowsTerminal.exe") ||
       WaitForSingleObject(process,0)!=WAIT_TIMEOUT || GetAncestor(console,GA_ROOTOWNER)!=root){
        CloseHandle(process);return ERROR_NOT_FOUND;
    }
    DWORD error=run16s_window_attach(root,pid,thread,directory);
    CloseHandle(process);return error;
}

typedef struct console_search { DWORD client; HWND window; unsigned count; } console_search;
static BOOL CALLBACK find_console(HWND w,LPARAM context)
{
    console_search *s=(console_search *)context;WCHAR name[32];DWORD pid=0;
    if(!IsWindowVisible(w) || !GetClassNameW(w,name,ARRAYSIZE(name)) || wcscmp(name,L"ConsoleWindowClass"))return TRUE;
    GetWindowThreadProcessId(w,&pid);
    if(pid==s->client){s->window=w;++s->count;}
    return TRUE;
}

static DWORD find_client(DWORD host,console_search *s)
{
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if(snapshot==INVALID_HANDLE_VALUE)return GetLastError();
    PROCESSENTRY32W item={.dwSize=sizeof(item)};DWORD error=ERROR_NOT_FOUND;
    if(Process32FirstW(snapshot,&item))do{
        if(item.th32ParentProcessID!=host)continue;
        console_search found={.client=item.th32ProcessID};
        if(!EnumWindows(find_console,(LPARAM)&found)){error=GetLastError()?GetLastError():ERROR_GEN_FAILURE;break;}
        if(found.count){
            if(found.count!=1 || error==ERROR_SUCCESS){error=ERROR_MORE_DATA;break;}
            *s=found;error=ERROR_SUCCESS;
        }
    }while(Process32NextW(snapshot,&item));
    CloseHandle(snapshot);return error;
}

static BOOL child_of(DWORD child,DWORD host)
{
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if(snapshot==INVALID_HANDLE_VALUE)return FALSE;
    PROCESSENTRY32W item={.dwSize=sizeof(item)};BOOL found=FALSE;
    if(Process32FirstW(snapshot,&item))do{
        if(item.th32ProcessID==child){found=item.th32ParentProcessID==host;break;}
    }while(Process32NextW(snapshot,&item));
    CloseHandle(snapshot);return found;
}

static DWORD send_to_host(DWORD host,HWND window,HMODULE module,HOOKPROC callback)
{
    HHOOK hooks[128]={0};unsigned count=0;DWORD error=ERROR_NOT_FOUND;
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
    if(snapshot==INVALID_HANDLE_VALUE)return GetLastError();
    THREADENTRY32 item={.dwSize=sizeof(item)};
    if(Thread32First(snapshot,&item))do{
        if(item.th32OwnerProcessID!=host)continue;
        if(count==ARRAYSIZE(hooks)){error=ERROR_MORE_DATA;goto done;}
        HHOOK hook=SetWindowsHookExW(WH_CALLWNDPROC,callback,module,item.th32ThreadID);
        if(hook)hooks[count++]=hook;
    }while(Thread32Next(snapshot,&item));
    if(count){
        DWORD_PTR ignored=0;UINT message=RegisterWindowMessageW(NTW16S_WINDOW_THREAD_MESSAGE);
        RemovePropW(window,NTW16S_WINDOW_THREAD_RESULT);
        if(!message || !SendMessageTimeoutW(window,message,NTW16S_WINDOW_THREAD_ATTACH,(LPARAM)window,
           SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,3000,&ignored))error=GetLastError()?GetLastError():ERROR_TIMEOUT;
        else {uintptr_t result=(uintptr_t)GetPropW(window,NTW16S_WINDOW_THREAD_RESULT);
            error=result?(DWORD)(result-1):ERROR_INVALID_DATA;}
        RemovePropW(window,NTW16S_WINDOW_THREAD_RESULT);
    }
done:
    CloseHandle(snapshot);
    while(count)UnhookWindowsHookEx(hooks[--count]);
    return error;
}

DWORD run16s_console_host_attach(HANDLE process,PCWSTR directory)
{
    DWORD host=GetProcessId(process),error=ERROR_NOT_FOUND;console_search target={0};
    if(!host || !directory)return ERROR_INVALID_PARAMETER;
    ULONGLONG deadline=GetTickCount64()+5000;
    do{
        if(WaitForSingleObject(process,0)!=WAIT_TIMEOUT)return ERROR_PROCESS_ABORTED;
        error=find_client(host,&target);
        if(error!=ERROR_NOT_FOUND)break;
        Sleep(50);
    }while(GetTickCount64()<deadline);
    if(error)return error;
    HANDLE client=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,target.client);
    if(!client)return GetLastError();
    FILETIME host_created,client_created,exit_time,kernel_time,user_time;
    if(!GetProcessTimes(process,&host_created,&exit_time,&kernel_time,&user_time) ||
       !GetProcessTimes(client,&client_created,&exit_time,&kernel_time,&user_time) ||
       CompareFileTime(&client_created,&host_created)<0){CloseHandle(client);return ERROR_INVALID_DATA;}
    DWORD pid=0;GetWindowThreadProcessId(target.window,&pid);
    if(pid!=target.client || !child_of(target.client,host) || WaitForSingleObject(process,0)!=WAIT_TIMEOUT ||
       WaitForSingleObject(client,0)!=WAIT_TIMEOUT){CloseHandle(client);return ERROR_INVALID_WINDOW_HANDLE;}
    WCHAR path[32768];size_t length=wcslen(directory);
    if(length+14>=ARRAYSIZE(path)){CloseHandle(client);return ERROR_FILENAME_EXCED_RANGE;}
    wcscpy(path,directory);wcscat(path,L"ntw16s64.dll");
    HMODULE module=LoadLibraryExW(path,NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    if(!module){error=GetLastError();CloseHandle(client);return error;}
    HOOKPROC callback=(HOOKPROC)(uintptr_t)GetProcAddress(module,"Ntw16sWindowThreadHook");
    error=callback?run16s_window_module_compatible(host,module,callback,path):ERROR_PROC_NOT_FOUND;
    if(!error)error=send_to_host(host,target.window,module,callback);
    FreeLibrary(module);CloseHandle(client);return error;
}
