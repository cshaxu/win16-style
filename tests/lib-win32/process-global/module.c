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
    puts("PROCESS_GLOBAL_MODULE_PASS matched=1 mismatch=1 fallback=1");return 0;
}
