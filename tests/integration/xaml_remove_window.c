#include <windows.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "ntw16s-dll/window_thread.h"

typedef struct probe { DWORD pid; HMODULE module,property_module; HOOKPROC callback,property_callback; uintptr_t adapter; BOOL removed,property_ok; } probe;
static BOOL CALLBACK visit(HWND window,LPARAM parameter)
{
    probe *p=(probe *)parameter; DWORD pid=0;
    DWORD thread=GetWindowThreadProcessId(window,&pid);
    if (pid!=p->pid || !thread || GetAncestor(window,GA_ROOT)!=window) return TRUE;
    WCHAR name[120]={0}; GetClassNameW(window,name,ARRAYSIZE(name));
    wprintf(L"window_class=%ls visible=%d\n",name,IsWindowVisible(window));
    HHOOK hook=SetWindowsHookExW(WH_CALLWNDPROC,p->callback,p->module,thread);
    if (!hook) return TRUE;
    HHOOK property_hook=p->property_callback ? SetWindowsHookExW(WH_CALLWNDPROC,p->property_callback,p->property_module,thread) : NULL;
    UINT property_message=RegisterWindowMessageW(L"Win16Xaml.Test.Property");
    DWORD_PTR ignored=0;
    uintptr_t before=0,after=0;
    if (property_hook && property_message) {
        RemovePropW(window,L"Win16Xaml.Test.Property.Result");
        if (SendMessageTimeoutW(window,property_message,1,(LPARAM)p->adapter,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,2000,&ignored))
            before=(uintptr_t)GetPropW(window,L"Win16Xaml.Test.Property.Result");
        RemovePropW(window,L"Win16Xaml.Test.Property.Result");
    }
    UINT message=RegisterWindowMessageW(NTW16S_WINDOW_THREAD_MESSAGE);
    RemovePropW(window,NTW16S_WINDOW_THREAD_RESULT);
    if (message && SendMessageTimeoutW(window,message,NTW16S_WINDOW_THREAD_REMOVE,(LPARAM)window,
                                       SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,2000,&ignored)) {
        uintptr_t result=(uintptr_t)GetPropW(window,NTW16S_WINDOW_THREAD_RESULT);
        wprintf(L"remove_result=%llu\n",(unsigned long long)result);
        if (result==1) p->removed=TRUE;
    }
    if (p->removed && property_hook && property_message) {
        if (SendMessageTimeoutW(window,property_message,2,0,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,2000,&ignored))
            after=(uintptr_t)GetPropW(window,L"Win16Xaml.Test.Property.Result");
        wprintf(L"property_source_before=%llu after=%llu (encoded source+1)\n",(unsigned long long)before,(unsigned long long)after);
        p->property_ok=before==5 && after==2;
    }
    RemovePropW(window,L"Win16Xaml.Test.Property.Result");
    RemovePropW(window,NTW16S_WINDOW_THREAD_RESULT);
    UnhookWindowsHookEx(hook);
    if (property_hook) UnhookWindowsHookEx(property_hook);
    return !p->removed;
}
int wmain(int argc,WCHAR **argv)
{
    if (argc!=3 && argc!=5) return 2;
    probe p={0}; p.pid=wcstoul(argv[1],NULL,10); p.module=LoadLibraryW(argv[2]);
    if (!p.pid || !p.module) return 3;
    p.callback=(HOOKPROC)(uintptr_t)GetProcAddress(p.module,"Ntw16sWindowThreadHook");
    if (!p.callback) { FreeLibrary(p.module); return 4; }
    if (argc==5) {
        p.adapter=(uintptr_t)wcstoull(argv[4],NULL,10);
        p.property_module=LoadLibraryW(argv[3]);
        if (p.property_module) p.property_callback=(HOOKPROC)(uintptr_t)GetProcAddress(p.property_module,"XamlPropertyProbe");
        if (!p.adapter || !p.property_callback) { if (p.property_module) FreeLibrary(p.property_module); FreeLibrary(p.module); return 5; }
    }
    EnumWindows(visit,(LPARAM)&p);
    if (p.property_module) FreeLibrary(p.property_module);
    FreeLibrary(p.module);
    return !p.removed ? 77 : argc==5 && !p.property_ok ? 6 : 0;
}
