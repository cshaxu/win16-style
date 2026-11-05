#include "global_policy.h"
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

DWORD ntw16s_global_policy_init(ntw16s_global_policy *p)
{return win32_process_global_policy_init(p);}
DWORD ntw16s_global_policy_inspect(const ntw16s_global_policy *p,HWND w,DWORD pid,DWORD tid,WORD machine,ntw16s_global_identity *i)
{return win32_process_global_policy_inspect(p,w,pid,tid,machine,i,ntw16s_global_image_allowed);}
DWORD ntw16s_global_policy_inspect_thread(const ntw16s_global_policy *p,DWORD pid,DWORD tid,WORD machine,ntw16s_global_identity *i)
{return win32_process_global_policy_inspect_thread(p,pid,tid,machine,i,ntw16s_global_image_allowed);}
DWORD ntw16s_global_policy_revalidate(HANDLE p,HWND w,const ntw16s_global_identity *i)
{return win32_process_global_policy_revalidate(p,w,i);}
DWORD ntw16s_global_policy_revalidate_thread(HANDLE p,const ntw16s_global_identity *i)
{return win32_process_global_policy_revalidate_thread(p,i);}
