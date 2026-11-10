#include "console_host.h"
#include "window_target.h"
#include "ntw16s-dll/window_thread.h"
#include <stdint.h>
#include <tlhelp32.h>
#include <wchar.h>

static DWORD attach_current_conhost(HWND console,PCWSTR directory);

BOOL run16s_console_host_image(PCWSTR image)
{
    WCHAR system[MAX_PATH];UINT n=GetSystemDirectoryW(system,ARRAYSIZE(system));
    if(!image || !n || n+13>=ARRAYSIZE(system))return FALSE;
    wcscat(system,L"\\conhost.exe");return !_wcsicmp(image,system);
}

static DWORD query_process_image(HANDLE process,WCHAR *image,DWORD *length)
{
    if(QueryFullProcessImageNameW(process,0,image,length))return ERROR_SUCCESS;
    DWORD error=GetLastError();
    return WaitForSingleObject(process,0)==WAIT_OBJECT_0?ERROR_NOT_FOUND:error;
}

DWORD run16s_console_host_attach_current(PCWSTR directory)
{
    if(!directory)return ERROR_INVALID_PARAMETER;
    HWND console=GetConsoleWindow();WCHAR name[64];
    if(!console || !GetClassNameW(console,name,ARRAYSIZE(name)))
        return ERROR_NOT_FOUND;
    if(!wcscmp(name,L"ConsoleWindowClass"))return attach_current_conhost(console,directory);
    if(wcscmp(name,L"PseudoConsoleWindow"))return ERROR_NOT_FOUND;
    HWND root=GetAncestor(console,GA_ROOTOWNER);DWORD pid=0;
    DWORD thread=root?GetWindowThreadProcessId(root,&pid):0;
    if(!thread || !pid || !GetClassNameW(root,name,ARRAYSIZE(name)) ||
       wcscmp(name,L"CASCADIA_HOSTING_WINDOW_CLASS"))return ERROR_NOT_FOUND;
    HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,pid);
    if(!process)return GetLastError();
    WCHAR image[32768];DWORD length=ARRAYSIZE(image);
    DWORD image_error=query_process_image(process,image,&length);
    if(image_error){CloseHandle(process);return image_error;}
    WCHAR *base=wcsrchr(image,L'\\');
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
    BOOL next=Process32FirstW(snapshot,&item);
    if(!next && GetLastError()!=ERROR_NO_MORE_FILES)error=GetLastError();
    while(next){
        if(item.th32ParentProcessID==host){
            console_search found={.client=item.th32ProcessID};
            SetLastError(ERROR_SUCCESS);
            if(!EnumWindows(find_console,(LPARAM)&found)){error=GetLastError();if(!error)error=ERROR_GEN_FAILURE;break;}
            if(found.count){
                if(found.count!=1 || error==ERROR_SUCCESS){error=ERROR_MORE_DATA;break;}
                *s=found;error=ERROR_SUCCESS;
            }
        }
        next=Process32NextW(snapshot,&item);
        if(!next && GetLastError()!=ERROR_NO_MORE_FILES)error=GetLastError();
    }
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
        UINT message=RegisterWindowMessageW(NTW16S_WINDOW_THREAD_MESSAGE);
        error=run16s_window_send(window,message);
    }
done:
    CloseHandle(snapshot);
    while(count)UnhookWindowsHookEx(hooks[--count]);
    return error;
}

/* Windows exposes no public API for the server PID of a classic console.
 * Query only this launcher's console, validate the system image, and fall
 * back to no attachment if the internal query is unavailable. */
static DWORD attach_current_conhost(HWND console,PCWSTR directory)
{
    typedef LONG (NTAPI *query_fn)(HANDLE,ULONG,void *,ULONG,ULONG *);
    HMODULE ntdll=GetModuleHandleW(L"ntdll.dll");
    query_fn query=ntdll?(query_fn)(uintptr_t)GetProcAddress(ntdll,"NtQueryInformationProcess"):NULL;
    ULONG_PTR value=0;ULONG bytes=0;
    if(!query || query(GetCurrentProcess(),49,&value,sizeof(value),&bytes)<0 ||
       bytes!=sizeof(value) || !(value&~(ULONG_PTR)3) || (value&~(ULONG_PTR)3)>MAXDWORD)
        return ERROR_NOT_FOUND;
    DWORD host=(DWORD)(value&~(ULONG_PTR)3);HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,host);
    if(!process)return GetLastError();
    WCHAR image[32768];DWORD length=ARRAYSIZE(image);
    DWORD image_error=query_process_image(process,image,&length);
    if(image_error){CloseHandle(process);return image_error;}
    if(!run16s_console_host_image(image) || WaitForSingleObject(process,0)!=WAIT_TIMEOUT || GetConsoleWindow()!=console){CloseHandle(process);return ERROR_NOT_FOUND;}
    HMODULE module;HOOKPROC callback;
    DWORD error=run16s_window_module_open(host,directory,&module,&callback);
    if(error){CloseHandle(process);return error;}
    error=send_to_host(host,console,module,callback);
    FreeLibrary(module);CloseHandle(process);return error;
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
       !GetProcessTimes(client,&client_created,&exit_time,&kernel_time,&user_time)){
        error=GetLastError();CloseHandle(client);return error;
    }
    if(CompareFileTime(&client_created,&host_created)<0){CloseHandle(client);return ERROR_INVALID_DATA;}
    DWORD pid=0;GetWindowThreadProcessId(target.window,&pid);
    if(pid!=target.client || !child_of(target.client,host) || WaitForSingleObject(process,0)!=WAIT_TIMEOUT ||
       WaitForSingleObject(client,0)!=WAIT_TIMEOUT){CloseHandle(client);return ERROR_INVALID_WINDOW_HANDLE;}
    HMODULE module;HOOKPROC callback;
    error=run16s_window_module_open(host,directory,&module,&callback);
    if(error){CloseHandle(client);return error;}
    error=send_to_host(host,target.window,module,callback);
    FreeLibrary(module);CloseHandle(client);return error;
}
