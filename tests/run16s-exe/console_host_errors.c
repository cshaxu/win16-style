#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

static unsigned mode,closes,time_calls;
static HANDLE WINAPI test_snapshot(DWORD flags,DWORD pid)
{(void)flags;(void)pid;return (HANDLE)(ULONG_PTR)1;}
static BOOL WINAPI test_first(HANDLE snapshot,LPPROCESSENTRY32W entry)
{
    (void)snapshot;
    if(mode<2){SetLastError(mode?ERROR_ACCESS_DENIED:ERROR_NO_MORE_FILES);return FALSE;}
    entry->th32ParentProcessID=mode>=6?99:10;entry->th32ProcessID=20;return TRUE;
}
static BOOL WINAPI test_next(HANDLE snapshot,LPPROCESSENTRY32W entry)
{(void)snapshot;(void)entry;SetLastError(mode==2 || mode==7?ERROR_PARTIAL_COPY:ERROR_NO_MORE_FILES);return FALSE;}
static BOOL WINAPI test_close(HANDLE handle)
{(void)handle;++closes;SetLastError(ERROR_INVALID_HANDLE);return TRUE;}
static HWND WINAPI test_console(void){return (HWND)(ULONG_PTR)1;}
static int WINAPI test_class(HWND window,LPWSTR name,int size)
{
    const WCHAR *value=window==(HWND)(ULONG_PTR)1?L"PseudoConsoleWindow":
        window==(HWND)(ULONG_PTR)3?L"ConsoleWindowClass":L"CASCADIA_HOSTING_WINDOW_CLASS";
    return lstrcpynW(name,value,size)?lstrlenW(name):0;
}
static HWND WINAPI test_ancestor(HWND window,UINT flags)
{(void)window;(void)flags;return (HWND)(ULONG_PTR)2;}
static DWORD WINAPI test_thread(HWND window,LPDWORD pid)
{*pid=window==(HWND)(ULONG_PTR)3?20:123;return 456;}
static HANDLE WINAPI test_open(DWORD access,BOOL inherit,DWORD pid)
{(void)access;(void)inherit;(void)pid;return (HANDLE)(ULONG_PTR)2;}
static BOOL WINAPI test_image(HANDLE process,DWORD flags,LPWSTR image,PDWORD size)
{(void)process;(void)flags;(void)image;(void)size;SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
static DWORD WINAPI test_wait(HANDLE object,DWORD timeout)
{(void)object;(void)timeout;return mode==5?WAIT_OBJECT_0:WAIT_TIMEOUT;}
static BOOL WINAPI test_visible(HWND window){(void)window;return TRUE;}
static BOOL WINAPI test_enum(WNDENUMPROC callback,LPARAM context)
{if(mode==11)return FALSE;if(mode==12){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}return callback((HWND)(ULONG_PTR)3,context);}
static DWORD WINAPI test_process_id(HANDLE process)
{(void)process;return 99;}
static BOOL WINAPI test_times(HANDLE process,LPFILETIME creation,LPFILETIME exit_time,LPFILETIME kernel,LPFILETIME user)
{
    (void)process;(void)exit_time;(void)kernel;(void)user;
    ++time_calls;
    if((mode==8 && time_calls==1) || (mode==9 && time_calls==2)){
        SetLastError(ERROR_ACCESS_DENIED);return FALSE;
    }
    creation->dwHighDateTime=0;creation->dwLowDateTime=time_calls==1?10:5;return TRUE;
}
#define IsWindowVisible test_visible
#define EnumWindows test_enum
#define GetConsoleWindow test_console
#define GetClassNameW test_class
#define GetAncestor test_ancestor
#define GetWindowThreadProcessId test_thread
#define OpenProcess test_open
#define QueryFullProcessImageNameW test_image
#define WaitForSingleObject test_wait
#define CreateToolhelp32Snapshot test_snapshot
#define Process32FirstW test_first
#define Process32NextW test_next
#define CloseHandle test_close
#define GetProcessId test_process_id
#define GetProcessTimes test_times
#include "../../src/run16s-exe/console_host.c"
#undef CreateToolhelp32Snapshot
#undef Process32FirstW
#undef Process32NextW
#undef CloseHandle

int main(void)
{
    const DWORD expected[]={ERROR_NOT_FOUND,ERROR_ACCESS_DENIED,ERROR_PARTIAL_COPY,ERROR_NOT_FOUND};
    for(mode=0;mode<ARRAYSIZE(expected);++mode){
        console_search result={0};closes=0;
        DWORD error=find_client(99,&result);
        if(error!=expected[mode] || closes!=1){fprintf(stderr,"console scan mode=%u error=%lu closes=%u\n",mode,error,closes);return 1;}
    }
    puts("CONSOLE_SCAN_ERRORS_OK empty=1 first=1 next=1 exhausted=1 cleanup=1");
    for(mode=4;mode<=5;++mode){
        closes=0;DWORD error=run16s_console_host_attach_current(L".");
        if(error!=(mode==4?ERROR_ACCESS_DENIED:ERROR_NOT_FOUND) || closes!=1){
            fprintf(stderr,"console image mode=%u error=%lu closes=%u\n",mode,error,closes);return 1;
        }
    }
    puts("CONSOLE_IMAGE_ERROR_OK denied=1 exited=1 cleanup=1");
    for(mode=6;mode<=7;++mode){
        console_search result={0};closes=0;DWORD error=find_client(99,&result);
        if(error!=(mode==6?ERROR_SUCCESS:ERROR_PARTIAL_COPY) || result.client!=20 ||
           result.count!=1 || closes!=1){
            fprintf(stderr,"console found mode=%u error=%lu count=%u closes=%u\n",mode,error,result.count,closes);return 1;
        }
    }
    puts("CONSOLE_SCAN_FOUND_OK success=1 later_failure=1 cleanup=1");
    for(mode=8;mode<=10;++mode){
        closes=0;time_calls=0;
        DWORD error=run16s_console_host_attach((HANDLE)(ULONG_PTR)9,L".");
        if(error!=(mode==10?ERROR_INVALID_DATA:ERROR_ACCESS_DENIED) ||
           time_calls!=(mode==8?1u:2u) || closes!=2){
            fprintf(stderr,"console times mode=%u error=%lu calls=%u closes=%u\n",mode,error,time_calls,closes);return 1;
        }
    }
    puts("CONSOLE_TIME_ERRORS_OK host=1 client=1 order=1 cleanup=1");
    for(mode=11;mode<=12;++mode){
        console_search result={0};closes=0;SetLastError(ERROR_INVALID_HANDLE);
        DWORD error=find_client(99,&result);
        if(error!=(mode==11?ERROR_GEN_FAILURE:ERROR_ACCESS_DENIED) || closes!=1){
            fprintf(stderr,"console enum mode=%u error=%lu closes=%u\n",mode,error,closes);return 1;
        }
    }
    puts("CONSOLE_ENUM_ERRORS_OK missing=1 denied=1 cleanup=1");
    return 0;
}
