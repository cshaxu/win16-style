#include "lib-win32/process-global/module.h"
#include <stdio.h>
#include <wchar.h>
#include <tlhelp32.h>
static unsigned snapshot_calls,snapshot_mode;
static HANDLE WINAPI test_snapshot(DWORD flags,DWORD pid)
{
    if(flags!=(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32) || pid!=42){SetLastError(ERROR_INVALID_PARAMETER);return INVALID_HANDLE_VALUE;}
    ++snapshot_calls;
    if(snapshot_mode==1 && snapshot_calls==8)return (HANDLE)(ULONG_PTR)1;
    SetLastError(snapshot_mode==0?ERROR_ACCESS_DENIED:ERROR_BAD_LENGTH);return INVALID_HANDLE_VALUE;
}
#define CreateToolhelp32Snapshot test_snapshot
#include "lib-win32/process-global/module_snapshot.h"
#undef CreateToolhelp32Snapshot
static BOOL allow;static unsigned equals,checks;
static BOOL same(const WCHAR *a,const WCHAR *b)
{(void)a;(void)b;++equals;return allow;}
static DWORD check(HANDLE p,DWORD id,HMODULE m,const WCHAR *name,const WCHAR *path)
{(void)p;(void)id;(void)m;(void)name;(void)path;++checks;return ERROR_SUCCESS;}
typedef struct check_context {DWORD error;unsigned calls;} check_context;
static DWORD check_using(HANDLE process,DWORD pid,HMODULE module,const WCHAR *name,const WCHAR *path,void *value)
{
    check_context *context=value;WCHAR nested[MAX_PATH];(void)module;(void)name;(void)path;
    ++context->calls;
    /* A nested path selection must not overwrite the outer callback context. */
    DWORD error=win32_process_global_select_path_using(process,pid,L"C:\\nonexistent\\nested-absent-module.dll",
        nested,ARRAYSIZE(nested),same,check_using,NULL);
    return error?error:context->error;
}
static DWORD resolve_using(HANDLE process,DWORD pid,const WCHAR *path,void **address,void *value)
{
    check_context *context=value;(void)process;(void)pid;(void)path;++context->calls;
    *address=(void *)GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"GetTickCount");
    return context->error;
}
int main(void)
{
    WCHAR selected[MAX_PATH];HANDLE p=GetCurrentProcess();DWORD id=GetCurrentProcessId();
    snapshot_mode=0;snapshot_calls=0;
    if(win32_process_global_module_snapshot(42)!=INVALID_HANDLE_VALUE || snapshot_calls!=1 || GetLastError()!=ERROR_ACCESS_DENIED)return 10;
    snapshot_mode=1;snapshot_calls=0;
    if(win32_process_global_module_snapshot(42)!=(HANDLE)(ULONG_PTR)1 || snapshot_calls!=8)return 11;
    snapshot_mode=2;snapshot_calls=0;
    if(win32_process_global_module_snapshot(42)!=INVALID_HANDLE_VALUE || snapshot_calls!=8 || GetLastError()!=ERROR_BAD_LENGTH)return 12;
    DWORD e=win32_process_global_select_path(p,id,L"C:\\nonexistent\\kernel32.dll",selected,ARRAYSIZE(selected),same,check);
    if(e!=ERROR_REVISION_MISMATCH || equals!=1 || checks)return 1;
    allow=TRUE;e=win32_process_global_select_path(p,id,L"C:\\nonexistent\\kernel32.dll",selected,ARRAYSIZE(selected),same,check);
    if(e || equals!=2 || checks!=1 || !wcsrchr(selected,L'\\') || lstrcmpiW(wcsrchr(selected,L'\\')+1,L"kernel32.dll"))return 2;
    e=win32_process_global_select_path(p,id,L"C:\\nonexistent\\no-such-module-123456.dll",selected,ARRAYSIZE(selected),same,check);
    if(e || lstrcmpW(selected,L"C:\\nonexistent\\no-such-module-123456.dll"))return 3;
    if(win32_process_global_select_path(p,id,L"badname",selected,ARRAYSIZE(selected),same,check)!=ERROR_INVALID_PARAMETER)return 4;
    check_context context={ERROR_ACCESS_DENIED,0};
    e=win32_process_global_select_path_using(p,id,L"C:\\nonexistent\\kernel32.dll",selected,ARRAYSIZE(selected),same,check_using,&context);
    if(e!=ERROR_ACCESS_DENIED || context.calls!=1)return 13;
    context.error=0;
    e=win32_process_global_select_path_using(p,id,L"C:\\nonexistent\\kernel32.dll",selected,ARRAYSIZE(selected),same,check_using,&context);
    if(e || context.calls!=2)return 14;
    HMODULE module=GetModuleHandleW(L"kernel32.dll");
    e=win32_process_global_mapped_identity_using(p,id,module,L"kernel32.dll",L"owned-path","GetTickCount",resolve_using,&context);
    if(e || context.calls!=3)return 15;
    context.error=ERROR_ACCESS_DENIED;
    e=win32_process_global_mapped_identity_using(p,id,module,L"kernel32.dll",L"owned-path","GetTickCount",resolve_using,&context);
    if(e!=ERROR_ACCESS_DENIED || context.calls!=4)return 16;
    WCHAR temp[MAX_PATH],left[MAX_PATH]={0},right[MAX_PATH]={0};DWORD wrote=0;BOOL equal=FALSE,written=FALSE,different=FALSE,missing=FALSE;
    HANDLE one=INVALID_HANDLE_VALUE,two=INVALID_HANDLE_VALUE;
    if(!GetTempPathW(ARRAYSIZE(temp),temp) || !GetTempFileNameW(temp,L"gm1",0,left) || !GetTempFileNameW(temp,L"gm2",0,right))goto file_done;
    one=CreateFileW(left,GENERIC_WRITE,0,NULL,TRUNCATE_EXISTING,0,NULL);
    two=CreateFileW(right,GENERIC_WRITE,0,NULL,TRUNCATE_EXISTING,0,NULL);
    if(one==INVALID_HANDLE_VALUE || two==INVALID_HANDLE_VALUE)goto file_done;
    written=WriteFile(one,"abc",3,&wrote,NULL) && wrote==3 && WriteFile(two,"abc",3,&wrote,NULL) && wrote==3;
    CloseHandle(one);one=INVALID_HANDLE_VALUE;CloseHandle(two);two=INVALID_HANDLE_VALUE;
    if(!written)goto file_done;
    equal=win32_process_global_files_equal(left,right);
    two=CreateFileW(right,GENERIC_WRITE,0,NULL,TRUNCATE_EXISTING,0,NULL);
    if(two==INVALID_HANDLE_VALUE)goto file_done;
    written=WriteFile(two,"abd",3,&wrote,NULL) && wrote==3;CloseHandle(two);two=INVALID_HANDLE_VALUE;
    different=!win32_process_global_files_equal(left,right);
    missing=!win32_process_global_files_equal(left,L"C:\\nonexistent\\missing-module.bin");
file_done:
    if(one!=INVALID_HANDLE_VALUE)CloseHandle(one);
    if(two!=INVALID_HANDLE_VALUE)CloseHandle(two);
    if(left[0])DeleteFileW(left);
    if(right[0])DeleteFileW(right);
    if(!equal || !written || !different || !missing)return 9;
    puts("PROCESS_GLOBAL_MODULE_PASS matched=1 mismatch=1 fallback=1 file_equal=1 file_different=1 missing=1 context=1 nested=1 mapped=1 error=1");return 0;
}
