#ifndef TEST_GLOBAL_DIALOG_THREAD_H
#define TEST_GLOBAL_DIALOG_THREAD_H
#include <windows.h>
static inline BOOL initialize_late_receiver(PCWSTR path)
{
    HMODULE module=LoadLibraryW(path);
    HOOKPROC receiver=module?(HOOKPROC)(void *)GetProcAddress(module,"Ntw16sGlobalHook"):NULL;
    HHOOK hook=receiver?SetWindowsHookExW(WH_CALLWNDPROC,receiver,NULL,GetCurrentThreadId()):NULL;
    if(!hook)return FALSE;
    HWND owner=CreateWindowExW(0,L"STATIC",L"Owned late dialog fixture",WS_OVERLAPPEDWINDOW,0,0,240,120,
        NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!owner)return FALSE;
    ShowWindow(owner,SW_SHOWNOACTIVATE);
    ULONGLONG deadline=GetTickCount64()+5000;
    while(!GetPropW(owner,L"Win2kStyle.Frame") && GetTickCount64()<deadline){
        if(MsgWaitForMultipleObjects(0,NULL,FALSE,50,QS_ALLINPUT)==WAIT_OBJECT_0){
            MSG msg;
            while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE))DispatchMessageW(&msg);
        }
    }
    BOOL ready=GetPropW(owner,L"Win2kStyle.Frame")!=NULL;
    DestroyWindow(owner);
    return ready;
}

#endif
