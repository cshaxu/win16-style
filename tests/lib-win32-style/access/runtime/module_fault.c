#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <stdio.h>
#include <string.h>

static unsigned scenario,closed;
static unsigned send_mode,removed,hook_mode,unhooked,hook_calls;
static HANDLE WINAPI test_remove_prop(HWND window,LPCWSTR name)
{(void)window;(void)name;++removed;SetLastError(ERROR_INVALID_HANDLE);return NULL;}
static HANDLE WINAPI test_get_prop(HWND window,LPCWSTR name)
{(void)window;(void)name;return send_mode==1?NULL:(HANDLE)(UINT_PTR)(send_mode==4?1:6);}
static LRESULT WINAPI test_send(HWND window,UINT message,WPARAM wparam,LPARAM lparam,UINT flags,UINT timeout,PDWORD_PTR result)
{
    (void)window;(void)message;(void)wparam;(void)lparam;(void)flags;(void)timeout;(void)result;
    if(send_mode==2 || send_mode==3){if(send_mode==2)SetLastError(ERROR_ACCESS_DENIED);return 0;}
    return 1;
}
static HANDLE WINAPI test_snapshot(DWORD flags,DWORD pid)
{(void)flags;(void)pid;return (HANDLE)1;}
static BOOL WINAPI test_first(HANDLE list,LPMODULEENTRY32W entry)
{
    (void)list;
    lstrcpyW(entry->szModule,scenario>=3?L"sample64.dll":L"other.dll");
    lstrcpyW(entry->szExePath,scenario==3?L"C:\\old\\sample64.dll":L"C:\\new\\sample64.dll");
    entry->modBaseAddr=(BYTE *)(UINT_PTR)0x1000;entry->modBaseSize=1024;return TRUE;
}
static BOOL WINAPI test_next(HANDLE list,LPMODULEENTRY32W entry)
{
    (void)list;(void)entry;
    SetLastError(scenario==0?ERROR_NO_MORE_FILES:scenario==1?ERROR_ACCESS_DENIED:ERROR_SUCCESS);
    return FALSE;
}
static BOOL WINAPI test_close(HANDLE handle)
{(void)handle;++closed;SetLastError(ERROR_INVALID_HANDLE);return TRUE;}
static HANDLE WINAPI test_open(DWORD access,BOOL inherit,DWORD pid)
{(void)access;(void)inherit;(void)pid;return (HANDLE)2;}
static DWORD WINAPI test_mapping(HANDLE p,LPVOID a,LPWSTR text,DWORD capacity)
{(void)p;(void)a;lstrcpynW(text,L"same-mapping",(int)capacity);return 12;}
static BOOL WINAPI test_read(HANDLE p,LPCVOID source,LPVOID out,SIZE_T size,SIZE_T *read)
{
    (void)p;(void)source;
    if(scenario==4){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    memset(out,scenario==7?1:0,size);*read=scenario==5?size-1:size;return TRUE;
}
static HHOOK WINAPI test_hook(int kind,HOOKPROC callback,HINSTANCE instance,DWORD thread)
{
    (void)kind;(void)callback;(void)instance;(void)thread;++hook_calls;
    if(hook_mode==1 || hook_mode==2){SetLastError(hook_mode==1?0:ERROR_ACCESS_DENIED);return NULL;}
    return (HHOOK)(UINT_PTR)1;
}
static BOOL WINAPI test_unhook(HHOOK hook)
{(void)hook;++unhooked;if(hook_mode==4){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}return TRUE;}
static DWORD WINAPI test_thread(HWND window,LPDWORD process)
{(void)window;*process=42;return hook_mode==3?10:9;}
static UINT WINAPI test_message(LPCWSTR name)
{(void)name;return hook_mode==5?0:1;}
#define SetWindowsHookExW test_hook
#define UnhookWindowsHookEx test_unhook
#define GetWindowThreadProcessId test_thread
#define RegisterWindowMessageW test_message
#define CreateToolhelp32Snapshot test_snapshot
#define Module32FirstW test_first
#define Module32NextW test_next
#define CloseHandle test_close
#define OpenProcess test_open
#undef GetMappedFileNameW
#define GetMappedFileNameW test_mapping
#define ReadProcessMemory test_read
#define RemovePropW test_remove_prop
#define GetPropW test_get_prop
#define SendMessageTimeoutW test_send
#include "../../../../src/lib-win32-style/access/runtime/module.c"
#include "../../../../src/lib-win32-style/access/runtime/window_dispatch.c"
int main(void)
{
    const DWORD expected[]={ERROR_SUCCESS,ERROR_ACCESS_DENIED,ERROR_GEN_FAILURE,ERROR_REVISION_MISMATCH,
        ERROR_ACCESS_DENIED,ERROR_PARTIAL_COPY,ERROR_SUCCESS,ERROR_REVISION_MISMATCH};
    BYTE local[64]={0};
    for(scenario=0;scenario<ARRAYSIZE(expected);++scenario){
        closed=0;
        DWORD result=win32_style_module_compatible(42,(HMODULE)local,(HOOKPROC)local,L"C:\\new\\sample64.dll");
        if(result!=expected[scenario] || closed!=(scenario>=4?2u:1u))return (int)scenario+1;
    }
    const DWORD send_expected[]={5,ERROR_INVALID_DATA,ERROR_ACCESS_DENIED,ERROR_TIMEOUT};
    for(send_mode=0;send_mode<ARRAYSIZE(send_expected);++send_mode){
        removed=0;
        if(win32_style_window_request((HWND)(UINT_PTR)1,1,L"result",1,0,3000)!=send_expected[send_mode] || removed!=2)return 20+(int)send_mode;
    }
    if(win32_style_window_dispatch_loaded((HWND)(UINT_PTR)1,42,0,NULL,NULL,L"message",L"result",1,0,3000)!=ERROR_INVALID_WINDOW_HANDLE)return 29;
    if(hook_calls)return 28;
    const DWORD dispatch_expected[]={0,ERROR_PROC_NOT_FOUND,ERROR_ACCESS_DENIED,ERROR_INVALID_WINDOW_HANDLE,ERROR_NOT_SUPPORTED,ERROR_INVALID_WINDOW_HANDLE};
    for(hook_mode=0;hook_mode<ARRAYSIZE(dispatch_expected);++hook_mode){
        send_mode=4;unhooked=0;
        DWORD error=win32_style_window_dispatch_loaded((HWND)(UINT_PTR)1,42,9,NULL,NULL,L"message",L"result",1,0,3000);
        if(error!=dispatch_expected[hook_mode] || unhooked!=(hook_mode==1 || hook_mode==2?0u:1u))return 30+(int)hook_mode;
    }
    hook_mode=4;send_mode=0;
    if(win32_style_window_dispatch_loaded((HWND)(UINT_PTR)1,42,9,NULL,NULL,L"message",L"result",1,0,3000)!=ERROR_ACCESS_DENIED)return 40;
    puts("WINDOW_DISPATCH_ERRORS_OK hook=1 no_error=1 identity=1 registration=1 unhook=1 original_error=1");
    puts("WINDOW_MESSAGE_OK result=1 missing=1 error=1 timeout=1 cleanup=1");
    puts("WINDOW_MODULE_ENUM_OK exhausted=1 failure=1 no_error=1 mismatch=1 cleanup=1 read=1 short=1 equal=1");return 0;
}
