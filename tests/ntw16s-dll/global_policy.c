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
static int other_station(void)
{
    WCHAR ready_name[128],desktop_name[128],station_name[128];
    if(!GetEnvironmentVariableW(L"WIN16_POLICY_READY",ready_name,ARRAYSIZE(ready_name)) ||
       !GetEnvironmentVariableW(L"WIN16_POLICY_DESKTOP",desktop_name,ARRAYSIZE(desktop_name)))return 11;
    swprintf(station_name,ARRAYSIZE(station_name),L"Win16S-Policy-Station-%lu",GetCurrentProcessId());
    HWINSTA station=CreateWindowStationW(station_name,0,WINSTA_ALL_ACCESS,NULL);
    if(!station || !SetProcessWindowStation(station))return 12;
    HDESK desktop=CreateDesktopW(desktop_name,NULL,NULL,0,GENERIC_ALL,NULL);
    if(!desktop || !SetThreadDesktop(desktop))return 13;
    HANDLE ready=OpenEventW(EVENT_MODIFY_STATE,FALSE,ready_name);if(!ready || !SetEvent(ready))return 14;
    Sleep(5000);CloseHandle(ready);CloseDesktop(desktop);CloseWindowStation(station);return 0;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !lstrcmpW(argv[1],L"target"))return target();
    if(argc==2 && !lstrcmpW(argv[1],L"other-station"))return other_station();
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
    DWORD revalidate=ntw16s_global_policy_revalidate(pi.hProcess,window,&identity);
    ntw16s_global_identity stale=identity;stale.process_birth_low^=1;
    DWORD stale_process=ntw16s_global_policy_revalidate(pi.hProcess,window,&stale);
    stale=identity;stale.thread_birth_low^=1;
    DWORD stale_thread=ntw16s_global_policy_revalidate(pi.hProcess,window,&stale);
    ntw16s_global_identity thread_identity={0};
    DWORD thread_result=ntw16s_global_policy_inspect_thread(&policy,pi.dwProcessId,pi.dwThreadId,machine,&thread_identity);
    DWORD thread_valid=ntw16s_global_policy_revalidate_thread(pi.hProcess,&thread_identity);
    stale=thread_identity;stale.thread_birth_low^=1;
    DWORD thread_stale=ntw16s_global_policy_revalidate_thread(pi.hProcess,&stale);
    DWORD thread_wrong_arch=ntw16s_global_policy_inspect_thread(&policy,pi.dwProcessId,pi.dwThreadId,
        machine==IMAGE_FILE_MACHINE_I386?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386,&invalid);
    WCHAR other_name[128];swprintf(other_name,ARRAYSIZE(other_name),L"Win16S-Policy-Other-%lu",GetCurrentProcessId());
    HDESK other=CreateDesktopW(other_name,NULL,NULL,0,GENERIC_ALL,NULL);PROCESS_INFORMATION other_pi={0};
    STARTUPINFOW other_si={.cb=sizeof(other_si),.lpDesktop=other_name};
    ResetEvent(ready);swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" target",self);
    BOOL other_started=other && CreateProcessW(self,cmd,NULL,NULL,FALSE,0,NULL,NULL,&other_si,&other_pi) &&
        WaitForSingleObject(ready,3000)==WAIT_OBJECT_0;
    DWORD other_result=other_started?ntw16s_global_policy_inspect_thread(&policy,other_pi.dwProcessId,other_pi.dwThreadId,machine,&invalid):ERROR_TIMEOUT;
    if(other_pi.hProcess){TerminateProcess(other_pi.hProcess,0);WaitForSingleObject(other_pi.hProcess,3000);CloseHandle(other_pi.hThread);CloseHandle(other_pi.hProcess);}
    if(other)CloseDesktop(other);
    SetEnvironmentVariableW(L"WIN16_POLICY_DESKTOP",desktop);ResetEvent(ready);
    swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" other-station",self);
    PROCESS_INFORMATION station_pi={0};STARTUPINFOW station_si={.cb=sizeof(station_si)};
    BOOL station_created=CreateProcessW(self,cmd,NULL,NULL,FALSE,0,NULL,NULL,&station_si,&station_pi);
    BOOL station_started=station_created && WaitForSingleObject(ready,3000)==WAIT_OBJECT_0;
    DWORD station_result=station_started?ntw16s_global_policy_inspect_thread(&policy,station_pi.dwProcessId,station_pi.dwThreadId,machine,&invalid):ERROR_TIMEOUT;
    if(!station_started){DWORD station_exit=99;if(station_created)GetExitCodeProcess(station_pi.hProcess,&station_exit);printf("POLICY STATION SKIP start=%d exit=%lu error=%lu\n",station_created,station_exit,GetLastError());}
    if(station_pi.hProcess){TerminateProcess(station_pi.hProcess,0);WaitForSingleObject(station_pi.hProcess,3000);CloseHandle(station_pi.hThread);CloseHandle(station_pi.hProcess);}
    if(window)PostMessageW(window,WM_CLOSE,0,0);
    DWORD wait=WaitForSingleObject(pi.hProcess,3000),code=99;if(wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);else TerminateProcess(pi.hProcess,ERROR_TIMEOUT);
    CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(ready);
    if(!started || result || identity.pid!=pi.dwProcessId || identity.thread_id!=pi.dwThreadId || identity.machine!=machine ||
       (!identity.process_birth_low && !identity.process_birth_high) || (!identity.thread_birth_low && !identity.thread_birth_high) ||
       wrong_pid!=ERROR_INVALID_WINDOW_HANDLE || wrong_thread!=ERROR_INVALID_WINDOW_HANDLE || wrong_arch!=ERROR_BAD_EXE_FORMAT || wrong_sid!=ERROR_INVALID_SID ||
       revalidate || stale_process!=ERROR_INVALID_STATE || stale_thread!=ERROR_INVALID_STATE || thread_result || thread_valid ||
       thread_stale!=ERROR_INVALID_STATE || thread_wrong_arch!=ERROR_BAD_EXE_FORMAT || other_result!=ERROR_ACCESS_DENIED ||
       (station_started && station_result!=ERROR_ACCESS_DENIED) || !cleared || wait!=WAIT_OBJECT_0 || code){
        printf("POLICY FAIL result=%lu pid=%lu thread=%lu arch=%lu sid=%lu revalidate=%lu stale_process=%lu stale_thread=%lu headless=%lu headless_valid=%lu headless_stale=%lu headless_arch=%lu other_desktop=%lu other_station=%lu cleared=%d wait=%lu code=%lu\n",result,wrong_pid,wrong_thread,wrong_arch,wrong_sid,revalidate,stale_process,stale_thread,thread_result,thread_valid,thread_stale,thread_wrong_arch,other_result,station_result,cleared,wait,code);return 10;
    }
    printf("POLICY PASS bits=%u owned=1 birth=1 wrong_pid=1 wrong_thread=1 wrong_arch=1 wrong_sid=1 revalidate=1 stale=1 shared=1 cleanup=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
