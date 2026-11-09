#include "lib-win32/win16-control-xaml/runtime_find.h"
#include <stdio.h>
#include <wchar.h>

#define CHECK(expr) do { if (!(expr)) { fwprintf(stderr,L"line %d: %hs\n",__LINE__,#expr); return 1; } } while (0)
int wmain(int argc,wchar_t **argv)
{
    if (argc!=3) return 2;
    WCHAR loader[32752],xaml[32752],actual[32752];
    HANDLE process=GetCurrentProcess();
    CHECK(win16_xaml_runtime_find(process,loader,ARRAYSIZE(loader),xaml,ARRAYSIZE(xaml))==ERROR_MOD_NOT_FOUND);
    CHECK(!*loader && !*xaml);
    HMODULE first=LoadLibraryW(argv[1]); CHECK(first!=NULL);
    CHECK(win16_xaml_runtime_find(process,loader,ARRAYSIZE(loader),xaml,ARRAYSIZE(xaml))==ERROR_MOD_NOT_FOUND);
    CHECK(!*loader && !*xaml);
    HMODULE second=LoadLibraryW(argv[2]); CHECK(second!=NULL);
    CHECK(win16_xaml_runtime_find(process,loader,ARRAYSIZE(loader),xaml,ARRAYSIZE(xaml))==ERROR_SUCCESS);
    CHECK(GetModuleFileNameW(first,actual,ARRAYSIZE(actual)) && !_wcsicmp(loader,actual));
    CHECK(GetModuleFileNameW(second,actual,ARRAYSIZE(actual)) && !_wcsicmp(xaml,actual));
    CHECK(win16_xaml_runtime_find(process,loader,2,xaml,ARRAYSIZE(xaml))==ERROR_INSUFFICIENT_BUFFER);
    CHECK(!*loader && !*xaml);
    CHECK(win16_xaml_runtime_find(process,loader,ARRAYSIZE(loader),xaml,2)==ERROR_INSUFFICIENT_BUFFER);
    CHECK(!*loader && !*xaml);
    CHECK(FreeLibrary(second)); CHECK(FreeLibrary(first));
    return 0;
}
