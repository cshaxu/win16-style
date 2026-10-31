#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "ntw16s-dll/global_policy.h"

static LRESULT CALLBACK window_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_CLOSE){DestroyWindow(w);return 0;}
    if(m==WM_DESTROY){PostQuitMessage(0);return 0;}
    return DefWindowProcW(w,m,a,b);
}
static int target(void)
{
    WCHAR name[128];if(!GetEnvironmentVariableW(L"WIN16_POLICY_READY",name,ARRAYSIZE(name)))return 2;
    HANDLE ready=OpenEventW(EVENT_MODIFY_STATE,FALSE,name);if(!ready)return 3;
    WNDCLASSW c={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=window_proc,.lpszClassName=L"Win16OwnedPolicyTest"};
    if(!RegisterClassW(&c))return 4;
    HWND w=CreateWindowW(c.lpszClassName,L"owned",WS_OVERLAPPEDWINDOW,0,0,150,100,NULL,NULL,c.hInstance,NULL);
    if(!w)return 5;
    ShowWindow(w,SW_SHOW);SetEvent(ready);CloseHandle(ready);
    MSG m;while(GetMessageW(&m,NULL,0,0)>0){TranslateMessage(&m);DispatchMessageW(&m);}return 0;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !lstrcmpW(argv[1],L"target"))return target();
    WCHAR desktop[128],name[128],self[MAX_PATH],cmd[MAX_PATH+32];DWORD needed=0;
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&needed) ||
       wcsncmp(desktop,L"Win16S-Dialog-",14))return 6;
    if(ntw16s_global_image_allowed(L"C:\\Windows\\explorer.exe",L"C:\\Windows") ||
       ntw16s_global_image_allowed(L"C:\\WINDOWS\\System32\\RuntimeBroker.EXE",L"C:\\Windows") ||
       !ntw16s_global_image_allowed(L"C:\\other\\explorer.exe",L"C:\\Windows") ||
       !ntw16s_global_image_allowed(L"C:\\Windows\\notepad.exe",L"C:\\Windows"))return 7;
    ntw16s_global_policy policy;DWORD init=ntw16s_global_policy_init(&policy);if(init)return 8;
    swprintf(name,ARRAYSIZE(name),L"Local\\Win16PolicyReady-%lu",GetCurrentProcessId());
    SetEnvironmentVariableW(L"WIN16_POLICY_READY",name);HANDLE ready=CreateEventW(NULL,TRUE,FALSE,name);
    GetModuleFileNameW(NULL,self,ARRAYSIZE(self));swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" target",self);
    STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};
    if(!ready || !CreateProcessW(self,cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))return 9;
    BOOL started=WaitForSingleObject(ready,3000)==WAIT_OBJECT_0;HWND window=FindWindowW(L"Win16OwnedPolicyTest",L"owned");
    WORD machine=sizeof(void*)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386;
    ntw16s_global_identity identity={0},invalid={0},zero={0};
    DWORD result=started?ntw16s_global_policy_inspect(&policy,window,pi.dwProcessId,pi.dwThreadId,machine,&identity):ERROR_TIMEOUT;
    DWORD wrong_pid=ntw16s_global_policy_inspect(&policy,window,pi.dwProcessId+1,pi.dwThreadId,machine,&invalid);
    BOOL cleared=!memcmp(&invalid,&zero,sizeof(zero));
    DWORD wrong_thread=ntw16s_global_policy_inspect(&policy,window,pi.dwProcessId,pi.dwThreadId+1,machine,&invalid);
    DWORD wrong_arch=ntw16s_global_policy_inspect(&policy,window,pi.dwProcessId,pi.dwThreadId,
        machine==IMAGE_FILE_MACHINE_I386?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386,&invalid);
    ntw16s_global_policy bad_policy=policy;bad_policy.sid_bytes=sizeof(bad_policy.sid)+1;
    DWORD wrong_sid=ntw16s_global_policy_inspect(&bad_policy,window,pi.dwProcessId,pi.dwThreadId,machine,&invalid);
    if(window)PostMessageW(window,WM_CLOSE,0,0);
    DWORD wait=WaitForSingleObject(pi.hProcess,3000),code=99;if(wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);else TerminateProcess(pi.hProcess,ERROR_TIMEOUT);
    CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(ready);
    if(!started || result || identity.pid!=pi.dwProcessId || identity.thread_id!=pi.dwThreadId || identity.machine!=machine ||
       (!identity.process_birth_low && !identity.process_birth_high) || (!identity.thread_birth_low && !identity.thread_birth_high) ||
       wrong_pid!=ERROR_INVALID_WINDOW_HANDLE || wrong_thread!=ERROR_INVALID_WINDOW_HANDLE || wrong_arch!=ERROR_BAD_EXE_FORMAT || wrong_sid!=ERROR_INVALID_SID || !cleared || wait!=WAIT_OBJECT_0 || code){
        printf("POLICY FAIL result=%lu pid=%lu thread=%lu arch=%lu sid=%lu cleared=%d wait=%lu code=%lu\n",result,wrong_pid,wrong_thread,wrong_arch,wrong_sid,cleared,wait,code);return 10;
    }
    printf("POLICY PASS bits=%u owned=1 birth=1 wrong_pid=1 wrong_thread=1 wrong_arch=1 wrong_sid=1 shared=1 cleanup=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
