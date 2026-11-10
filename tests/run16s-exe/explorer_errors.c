#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
static unsigned scenario;
static void *test_calloc(size_t n,size_t size)
{return scenario==0?NULL:calloc(n,size);}
static int WINAPI test_class(HWND w,LPWSTR name,int capacity)
{(void)w;return lstrcpynW(name,L"CabinetWClass",capacity)?13:0;}
static DWORD WINAPI test_thread(HWND w,LPDWORD pid)
{(void)w;*pid=42;return 43;}
static BOOL WINAPI test_enum(WNDENUMPROC callback,LPARAM data)
{
    if(scenario==1){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    if(scenario==2)return FALSE;
    if(scenario==3)for(unsigned i=0;i<257;++i)if(!callback((HWND)(UINT_PTR)(i+1),data))return FALSE;
    return TRUE;
}
#define calloc test_calloc
#define GetClassNameW test_class
#define GetWindowThreadProcessId test_thread
#define EnumWindows test_enum
#include "../../src/run16s-exe/explorer.c"
DWORD run16s_window_attach(HWND w,DWORD pid,DWORD tid,PCWSTR directory)
{(void)w;(void)pid;(void)tid;(void)directory;return ERROR_SUCCESS;}
int main(void)
{
    const DWORD expected[]={ERROR_NOT_ENOUGH_MEMORY,ERROR_ACCESS_DENIED,ERROR_GEN_FAILURE,ERROR_MORE_DATA};
    for(scenario=0;scenario<ARRAYSIZE(expected);++scenario){
        if(run16s_explorer_snapshot_create() || GetLastError()!=expected[scenario])return (int)scenario+1;
    }
    scenario=4;run16s_explorer_snapshot *snapshot=run16s_explorer_snapshot_create();
    if(!snapshot)return 5;
    for(scenario=1;scenario<=3;++scenario)
        if(run16s_explorer_attach_new(snapshot,L"test.exe",L"C:\\")!=expected[scenario])return (int)scenario+5;
    run16s_explorer_snapshot_free(snapshot);
    puts("EXPLORER_SNAPSHOT_ERRORS_OK allocation=1 enumeration=1 overflow=1 success=1 followup=1");return 0;
}
