#include <windows.h>
#include <stdio.h>
int main(void)
{
    if(GetModuleHandleW(L"ntw16s32.dll") || GetModuleHandleW(L"ntw16s64.dll"))return 42;
    puts("STYLE ONLY CHILD PASS no_style_dll=1");return 31;
}
