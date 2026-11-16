#include "ntw16s-dll/global/policy.h"
#include <wchar.h>

BOOL ntw16s_global_image_allowed(const WCHAR *image,const WCHAR *windows)
{
    if(!image || !*image || !windows || !*windows)return FALSE;
    size_t length=wcslen(windows);
    if(!_wcsnicmp(image,windows,length) && image[length]==L'\\'){
        const WCHAR *base=wcsrchr(image,L'\\');base=base?base+1:image;
        static const WCHAR *const shared[]={L"explorer.exe",L"ApplicationFrameHost.exe",L"ShellExperienceHost.exe",
            L"StartMenuExperienceHost.exe",L"SearchHost.exe",L"SearchApp.exe",L"sihost.exe",L"taskhostw.exe",
            L"RuntimeBroker.exe",L"dllhost.exe"};
        for(unsigned i=0;i<ARRAYSIZE(shared);++i)if(!lstrcmpiW(base,shared[i]))return FALSE;
    }
    return TRUE;
}
