#include "lib-win32/process-global/module.h"
#include <stdio.h>
#include <wchar.h>
static BOOL allow;static unsigned equals,checks;
static BOOL same(const WCHAR *a,const WCHAR *b)
{(void)a;(void)b;++equals;return allow;}
static DWORD check(HANDLE p,DWORD id,HMODULE m,const WCHAR *name,const WCHAR *path)
{(void)p;(void)id;(void)m;(void)name;(void)path;++checks;return ERROR_SUCCESS;}
int main(void)
{
    WCHAR selected[MAX_PATH];HANDLE p=GetCurrentProcess();DWORD id=GetCurrentProcessId();
    DWORD e=win32_process_global_select_path(p,id,L"C:\\nonexistent\\kernel32.dll",selected,ARRAYSIZE(selected),same,check);
    if(e!=ERROR_REVISION_MISMATCH || equals!=1 || checks)return 1;
    allow=TRUE;e=win32_process_global_select_path(p,id,L"C:\\nonexistent\\kernel32.dll",selected,ARRAYSIZE(selected),same,check);
    if(e || equals!=2 || checks!=1 || !wcsrchr(selected,L'\\') || lstrcmpiW(wcsrchr(selected,L'\\')+1,L"kernel32.dll"))return 2;
    e=win32_process_global_select_path(p,id,L"C:\\nonexistent\\no-such-module-123456.dll",selected,ARRAYSIZE(selected),same,check);
    if(e || lstrcmpW(selected,L"C:\\nonexistent\\no-such-module-123456.dll"))return 3;
    if(win32_process_global_select_path(p,id,L"badname",selected,ARRAYSIZE(selected),same,check)!=ERROR_INVALID_PARAMETER)return 4;
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
    puts("PROCESS_GLOBAL_MODULE_PASS matched=1 mismatch=1 fallback=1 file_equal=1 file_different=1 missing=1");return 0;
}
