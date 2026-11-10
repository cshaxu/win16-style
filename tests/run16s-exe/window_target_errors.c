#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <stdio.h>
#include <string.h>

static unsigned scenario,closed;
static unsigned send_mode,removed;
static HANDLE WINAPI test_remove_prop(HWND window,LPCWSTR name)
{(void)window;(void)name;++removed;SetLastError(ERROR_INVALID_HANDLE);return NULL;}
static HANDLE WINAPI test_get_prop(HWND window,LPCWSTR name)
{(void)window;(void)name;return send_mode==1?NULL:(HANDLE)(UINT_PTR)6;}
static LRESULT WINAPI test_send(HWND window,UINT message,WPARAM wparam,LPARAM lparam,UINT flags,UINT timeout,PDWORD_PTR result)
{
    (void)window;(void)message;(void)wparam;(void)lparam;(void)flags;(void)timeout;(void)result;
    if(send_mode>=2){if(send_mode==2)SetLastError(ERROR_ACCESS_DENIED);return 0;}
    return 1;
}
static HANDLE WINAPI test_snapshot(DWORD flags,DWORD pid)
{(void)flags;(void)pid;return (HANDLE)1;}
static BOOL WINAPI test_first(HANDLE list,LPMODULEENTRY32W entry)
{
    (void)list;
    lstrcpyW(entry->szModule,scenario>=3?L"ntw16s64.dll":L"other.dll");
    lstrcpyW(entry->szExePath,scenario==3?L"C:\\old\\ntw16s64.dll":L"C:\\new\\ntw16s64.dll");
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
#include "../../src/run16s-exe/window_target.c"
int main(void)
{
    const DWORD expected[]={ERROR_SUCCESS,ERROR_ACCESS_DENIED,ERROR_GEN_FAILURE,ERROR_REVISION_MISMATCH,
        ERROR_ACCESS_DENIED,ERROR_PARTIAL_COPY,ERROR_SUCCESS,ERROR_REVISION_MISMATCH};
    BYTE local[64]={0};
    for(scenario=0;scenario<ARRAYSIZE(expected);++scenario){
        closed=0;
        DWORD result=module_compatible(42,(HMODULE)local,(HOOKPROC)local,L"C:\\new\\ntw16s64.dll");
        if(result!=expected[scenario] || closed!=(scenario>=4?2u:1u))return (int)scenario+1;
    }
    const DWORD send_expected[]={5,ERROR_INVALID_DATA,ERROR_ACCESS_DENIED,ERROR_TIMEOUT};
    for(send_mode=0;send_mode<ARRAYSIZE(send_expected);++send_mode){
        removed=0;
        if(run16s_window_send((HWND)(UINT_PTR)1,1)!=send_expected[send_mode] || removed!=2)return 20+(int)send_mode;
    }
    puts("WINDOW_MESSAGE_OK result=1 missing=1 error=1 timeout=1 cleanup=1");
    puts("WINDOW_MODULE_ENUM_OK exhausted=1 failure=1 no_error=1 mismatch=1 cleanup=1 read=1 short=1 equal=1");return 0;
}
