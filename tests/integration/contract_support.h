#ifndef CONTRACT_SUPPORT_H
#define CONTRACT_SUPPORT_H
#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>
static WCHAR contract_report[MAX_PATH];
static void contract_fail(int line,const char *expression)
{
    FILE *file=NULL;DWORD error=GetLastError();fprintf(stderr,"CONTRACT FAIL %d error=%lu: %s\n",line,error,expression);
    if(contract_report[0] && !_wfopen_s(&file,contract_report,L"at")){fprintf(file,"FAIL %d error=%lu %s\n",line,error,expression);fclose(file);}ExitProcess(1);
}
#define CHECK(x) do{if(!(x))contract_fail(__LINE__,#x);}while(0)
static void contract_log(const char *phase,DWORD value)
{FILE *file=NULL;CHECK(!_wfopen_s(&file,contract_report,L"at"));fprintf(file,"%s %lu\n",phase,value);CHECK(!fclose(file));}
static DWORD contract_wait(HANDLE object,DWORD timeout)
{
    ULONGLONG deadline=GetTickCount64()+timeout;
    for(;;){ULONGLONG now=GetTickCount64();MSG message;if(now>=deadline)return WAIT_TIMEOUT;
        DWORD result=MsgWaitForMultipleObjectsEx(1,&object,(DWORD)(deadline-now),QS_ALLINPUT,MWMO_INPUTAVAILABLE);
        if(result!=WAIT_OBJECT_0+1)return result;
        while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){CHECK(message.message!=WM_QUIT);TranslateMessage(&message);DispatchMessageW(&message);}}
}
static LRESULT CALLBACK contract_owner(HWND w,UINT m,WPARAM a,LPARAM b)
{return DefWindowProcW(w,m,a,b);}
static void contract_window(BOOL styled,PCWSTR package)
{
    WNDCLASSW cls={0};HWND root,button;HMODULE dll;
    typedef BOOL (__cdecl *attached_fn)(HWND);attached_fn attached=NULL;
    dll=GetModuleHandleW(sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll");CHECK(!!dll==!!styled);
    if(styled){WCHAR actual[MAX_PATH],wanted[MAX_PATH];
        CHECK(swprintf_s(wanted,MAX_PATH,L"%ls\\ntw16s%u.dll",package,(unsigned)(sizeof(void *)*8))>0);
        CHECK(GetModuleFileNameW(dll,actual,MAX_PATH) && !_wcsicmp(actual,wanted));
        attached=(attached_fn)(void *)GetProcAddress(dll,"Ntw16sWindowAttached");CHECK(attached);}
    cls.lpfnWndProc=contract_owner;cls.lpszClassName=L"NativeCreationContract";cls.hInstance=GetModuleHandleW(NULL);
    CHECK(RegisterClassW(&cls) || GetLastError()==ERROR_CLASS_ALREADY_EXISTS);
    root=CreateWindowW(cls.lpszClassName,L"Native contract",WS_OVERLAPPEDWINDOW,0,0,360,240,NULL,NULL,cls.hInstance,NULL);CHECK(root);
    button=CreateWindowW(L"BUTTON",L"Native state",WS_CHILD|BS_AUTOCHECKBOX,10,10,160,30,root,NULL,cls.hInstance,NULL);CHECK(button);
    if(styled)CHECK(attached(root) && attached(button));
    SendMessageW(button,BM_SETCHECK,BST_CHECKED,0);CHECK(SendMessageW(button,BM_GETCHECK,0,0)==BST_CHECKED);
    CHECK(DestroyWindow(root));contract_log(styled?"window-attached":"window-native",GetCurrentProcessId());
}
#endif
