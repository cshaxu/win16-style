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

static inline BOOL copy_package_themes(PCWSTR package,PCWSTR launcher)
{
    WCHAR source[MAX_PATH],directory[MAX_PATH],destination[MAX_PATH];
    WCHAR *filename=NULL;
    DWORD count=GetFullPathNameW(launcher,ARRAYSIZE(source),source,&filename);
    if(!count || count>=ARRAYSIZE(source) || !filename)return FALSE;
    size_t capacity=ARRAYSIZE(source)-(size_t)(filename-source);
    if(swprintf_s(directory,ARRAYSIZE(directory),L"%ls\\themes",package)<0 ||
       !CreateDirectoryW(directory,NULL))return FALSE;
    static const WCHAR *const families[]={L"Win16",L"Win2K"};
    static const WCHAR *const extensions[]={L"theme",L"msstyles"};
    for(unsigned family=0;family<ARRAYSIZE(families);++family){
        PCWSTR name=families[family];
        if(swprintf_s(directory,ARRAYSIZE(directory),L"%ls\\themes\\%ls",package,name)<0 ||
           !CreateDirectoryW(directory,NULL))return FALSE;
        for(unsigned i=0;i<ARRAYSIZE(extensions);++i)
            if(swprintf_s(filename,capacity,L"themes\\%ls\\%ls.%ls",name,name,extensions[i])<0 ||
               swprintf_s(destination,ARRAYSIZE(destination),L"%ls\\%ls.%ls",directory,name,extensions[i])<0 ||
               !CopyFileW(source,destination,TRUE))return FALSE;
    }
    return TRUE;
}
#endif
