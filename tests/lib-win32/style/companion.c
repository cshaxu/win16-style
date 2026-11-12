#include <windows.h>
#include <stdio.h>
#include "lib-win32/style/companion.h"
int main(void)
{
    const char *missing[]={"fixture_first","fixture_missing"};
    const char *valid[]={"fixture_first","fixture_second"};
    FARPROC entries[2]={(FARPROC)(void *)1,(FARPROC)(void *)1};
    if(win32_style_companion((const void *)&main,L"core-style-peer-fixture.dll",missing,entries,2,1) ||
       GetLastError()!=ERROR_PROC_NOT_FOUND || entries[0] || entries[1])return 1;
    if(win32_style_companion((const void *)&main,L"..\\foreign.dll",valid,entries,2,1) ||
       GetLastError()!=ERROR_INVALID_PARAMETER)return 2;
    if(!win32_style_companion((const void *)&main,L"core-style-peer-fixture.dll",valid,entries,2,1))return 3;
    int (__cdecl *first)(void)=(void *)entries[0],(__cdecl *second)(void)=(void *)entries[1];
    if(!first || !second || first()!=17 || second()!=29 || !GetModuleHandleW(L"core-style-peer-fixture.dll"))return 4;
    puts("STYLE COMPANION PASS resolve=1 missing=1 pin=1 path=1");return 0;
}
