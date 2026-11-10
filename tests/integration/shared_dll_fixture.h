#ifndef TEST_SHARED_DLL_FIXTURE_H
#define TEST_SHARED_DLL_FIXTURE_H
#include <windows.h>
#include <wchar.h>

static BOOL copy_shared_dlls(PCWSTR package,PCWSTR style32,PCWSTR style64)
{
    const WCHAR *styles[]={style32,style64};
    for(unsigned i=0;i<2;++i){
        WCHAR source[32768],destination[32768];
        DWORD count=GetFullPathNameW(styles[i],ARRAYSIZE(source),source,NULL);
        if(!count || count>=ARRAYSIZE(source))return FALSE;
        WCHAR *slash=wcsrchr(source,L'\\');if(!slash)return FALSE;
        const WCHAR *name=i?L"ntwshk64.dll":L"ntwshk32.dll";
        if(wcscpy_s(slash+1,ARRAYSIZE(source)-(size_t)(slash+1-source),name) ||
           swprintf_s(destination,ARRAYSIZE(destination),L"%ls\\%ls",package,name)<0 ||
           !CopyFileW(source,destination,FALSE))return FALSE;
    }
    return TRUE;
}
#endif
