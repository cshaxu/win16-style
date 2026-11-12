#include <windows.h>
#include <stdio.h>
int main(void)
{
    if(GetModuleHandleW(L"user32.dll") || GetModuleHandleW(L"comctl32.dll")){
        puts("LAZY GUI SKIP gui modules already loaded");return 77;
    }
    HMODULE module=LoadLibraryW(sizeof(void *)==8?L"ntwshk64.dll":L"ntwshk32.dll");
    if(!module)return 1;
    BOOL clean=!GetModuleHandleW(L"user32.dll") && !GetModuleHandleW(L"comctl32.dll");
    FreeLibrary(module);
    printf("LAZY GUI bits=%u helper_without_gui=%d\n",(unsigned)(sizeof(void *)*8),clean);
    return clean?0:2;
}
