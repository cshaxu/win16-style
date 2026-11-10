#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global/attach.h"
#include "ntw16s-dll/global/session.h"
#include "global_session_fixture.h"

#include "global_target_fixture.h"
static LRESULT CALLBACK window_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_APP+54){
        WCHAR entered_name[128],release_name[128];
        if(GetEnvironmentVariableW(L"WIN16_ATTACH_STALL_ENTERED",entered_name,ARRAYSIZE(entered_name)) &&
           GetEnvironmentVariableW(L"WIN16_ATTACH_STALL_RELEASE",release_name,ARRAYSIZE(release_name))){
            HANDLE entered=OpenEventW(EVENT_MODIFY_STATE,FALSE,entered_name),release=OpenEventW(SYNCHRONIZE,FALSE,release_name);
            if(entered && release){SetEvent(entered);(void)WaitForSingleObject(release,3000);}
            if(entered)CloseHandle(entered);
            if(release)CloseHandle(release);
        }
        return 0;
    }
    return global_fixture_message(w,m,a,b);
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !lstrcmpW(argv[1],L"target"))return target(0);
    WCHAR name[NTW16S_GLOBAL_NAME_CAP],stop_name[NTW16S_GLOBAL_NAME_CAP+16],ready_name[128],entered_name[128],release_name[128],self[MAX_PATH],cmd[MAX_PATH+32],dll[MAX_PATH];
    HANDLE map=NULL;ntw16s_global_session *session=NULL;
    int session_error=global_test_session(name,stop_name,&map,&session,0x3579);if(session_error)return session_error;
    swprintf(ready_name,ARRAYSIZE(ready_name),L"Local\\Win16AttachReady-%lu",GetCurrentProcessId());SetEnvironmentVariableW(L"WIN16_ATTACH_TEST_READY",ready_name);
    swprintf(entered_name,ARRAYSIZE(entered_name),L"Local\\Win16AttachStallEntered-%lu",GetCurrentProcessId());
    swprintf(release_name,ARRAYSIZE(release_name),L"Local\\Win16AttachStallRelease-%lu",GetCurrentProcessId());
    SetEnvironmentVariableW(L"WIN16_ATTACH_STALL_ENTERED",entered_name);SetEnvironmentVariableW(L"WIN16_ATTACH_STALL_RELEASE",release_name);
    HANDLE ready=CreateEventW(NULL,TRUE,FALSE,ready_name),stop=CreateEventW(NULL,TRUE,FALSE,stop_name);
    HANDLE entered=CreateEventW(NULL,TRUE,FALSE,entered_name),release=CreateEventW(NULL,TRUE,FALSE,release_name);
    GetModuleFileNameW(NULL,self,ARRAYSIZE(self));wcscpy(dll,self);WCHAR *slash=wcsrchr(dll,L'\\');if(!slash)return 12;
    wcscpy(slash+1,sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" target",self);
    STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};
    if(!ready || !stop || !entered || !release || !CreateProcessW(self,cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))return 13;
    BOOL started=WaitForSingleObject(ready,3000)==WAIT_OBJECT_0;HWND window=FindWindowW(L"Win16OwnedAttachTest",L"owned");
    win32_process_global_policy policy;DWORD init=win32_process_global_policy_init(&policy);ntw16s_global_attach ticket={0};
    int before=window?query(window,WM_APP+52):-1;
    DWORD begin=started && !init?ntw16s_global_attach_begin(&policy,window,pi.dwProcessId,pi.dwThreadId,
        sizeof(void*)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386,session->cookie,dll,&ticket):ERROR_INVALID_STATE;
    DWORD advance=begin;
    if(!begin && WaitForSingleObject(ticket.load.thread,3000)==WAIT_OBJECT_0){
        advance=ntw16s_global_attach_advance(&ticket,NULL);
        if(advance==ERROR_IO_PENDING && ticket.start.ready){
            HANDLE waits[]={ticket.start.ready,ticket.start.thread};
            if(WaitForMultipleObjects(ARRAYSIZE(waits),waits,FALSE,3000)<WAIT_OBJECT_0+ARRAYSIZE(waits))advance=ntw16s_global_attach_advance(&ticket,NULL);
        }
    }
    int attached=window?query(window,WM_APP+52):-1;
    HWND child=window?GetDlgItem(window,101):NULL;if(child)SendMessageW(child,BM_CLICK,0,0);
    int clicks=window?query(window,WM_APP+53):-1;
    BOOL blocked=FALSE;DWORD bounded=ERROR_INVALID_STATE;ULONGLONG elapsed=0;
    if(!advance && window && PostMessageW(window,WM_APP+54,0,0) && WaitForSingleObject(entered,2000)==WAIT_OBJECT_0){
        blocked=TRUE;ULONGLONG start=GetTickCount64();bounded=ntw16s_global_attach_apply(&ticket);elapsed=GetTickCount64()-start;
    }
    SetEvent(release);
    int after_block=window?query(window,WM_APP+52):-1;
    session->phase=NTW16S_GLOBAL_STOPPING;
    DWORD removed=advance?advance:ntw16s_global_attach_remove(&ticket);int after=window?query(window,WM_APP+52):-1;
    SetEvent(stop);DWORD worker_wait=ticket.start.thread?WaitForSingleObject(ticket.start.thread,3000):WAIT_FAILED;
    DWORD closed=ntw16s_global_attach_close(&ticket);
    if(window)PostMessageW(window,WM_CLOSE,0,0);
    DWORD child_wait=WaitForSingleObject(pi.hProcess,3000),code=99;if(child_wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);else TerminateProcess(pi.hProcess,ERROR_TIMEOUT);
    CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(ready);CloseHandle(stop);CloseHandle(entered);CloseHandle(release);UnmapViewOfFile(session);CloseHandle(map);
    if(!started || init || before || begin || advance || attached!=3 || clicks!=1 || !blocked || bounded!=ERROR_TIMEOUT || elapsed>1500 || after_block!=3 || removed || after || worker_wait!=WAIT_OBJECT_0 || closed || child_wait!=WAIT_OBJECT_0 || code){
        printf("ATTACH BLOCK blocked=%d error=%lu elapsed=%llu attached=%d\n",blocked,bounded,(unsigned long long)elapsed,after_block);
        printf("ATTACH FAIL started=%d init=%lu before=%d begin=%lu advance=%lu attached=%d clicks=%d remove=%lu after=%d worker_wait=%lu closed=%lu child_wait=%lu code=%lu\n",started,init,before,begin,advance,attached,clicks,removed,after,worker_wait,closed,child_wait,code);return 14;
    }
    printf("ATTACH BOUNDED PASS bits=%u timeout=%lu elapsed=%llu\n",(unsigned)(sizeof(void*)*8),bounded,(unsigned long long)elapsed);
    printf("ATTACH PASS bits=%u unloaded=1 loaded=1 parent=1 child=1 click=1 removed=1 cleanup=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
