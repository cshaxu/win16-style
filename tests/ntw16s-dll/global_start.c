#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global_session.h"
#include "ntw16s-dll/global_start.h"
#include "ntw16s-dll/global_load.h"

static LRESULT CALLBACK window_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_CLOSE){DestroyWindow(w);return 0;}
    if(m==WM_DESTROY){PostQuitMessage(0);return 0;}
    return DefWindowProcW(w,m,a,b);
}
static int target(void)
{
    WCHAR dll[32752],ready_name[128];
    WCHAR remote[2];BOOL remote_load=GetEnvironmentVariableW(L"WIN16_START_TEST_REMOTE",remote,ARRAYSIZE(remote))!=0;
    if(!GetEnvironmentVariableW(L"WIN16_START_TEST_DLL",dll,ARRAYSIZE(dll)) || (!remote_load && !LoadLibraryW(dll)) ||
       !GetEnvironmentVariableW(L"WIN16_START_TEST_READY",ready_name,ARRAYSIZE(ready_name)))return 2;
    HANDLE ready=OpenEventW(EVENT_MODIFY_STATE,FALSE,ready_name);if(!ready)return 3;
    WNDCLASSW c={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=window_proc,.lpszClassName=L"Win16OwnedStartTest"};
    if(!RegisterClassW(&c))return 4;
    HWND window=CreateWindowW(c.lpszClassName,L"owned",WS_OVERLAPPEDWINDOW,0,0,150,100,NULL,NULL,c.hInstance,NULL);
    if(!window)return 5;
    ShowWindow(window,SW_SHOW);SetEvent(ready);CloseHandle(ready);
    MSG message;while(GetMessageW(&message,NULL,0,0)>0){TranslateMessage(&message);DispatchMessageW(&message);}
    return 0;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !lstrcmpW(argv[1],L"target"))return target();
    BOOL remote_load=argc==3 && !lstrcmpW(argv[2],L"remote-load");
    if(argc!=2 && !remote_load)return 6;
    WCHAR desktop[128],name[NTW16S_GLOBAL_NAME_CAP],stop_name[NTW16S_GLOBAL_NAME_CAP+16],
          child_ready_name[128],self[MAX_PATH],cmd[MAX_PATH+32];DWORD needed=0;
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&needed) ||
       wcsncmp(desktop,L"Win16S-Dialog-",14))return 7;
    if(!ntw16s_global_name(name,ARRAYSIZE(name)) || swprintf(stop_name,ARRAYSIZE(stop_name),L"%ls.stop",name)<0)return 8;
    HANDLE map=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(ntw16s_global_session),name);
    if(!map || GetLastError()==ERROR_ALREADY_EXISTS)return 9;
    ntw16s_global_session *session=(ntw16s_global_session *)MapViewOfFile(map,FILE_MAP_ALL_ACCESS,0,0,sizeof(*session));if(!session)return 10;
    ZeroMemory(session,sizeof(*session));session->magic=NTW16S_GLOBAL_MAGIC;session->version=NTW16S_GLOBAL_VERSION;
    session->owner_pid=GetCurrentProcessId();session->cookie=0x2468;session->phase=NTW16S_GLOBAL_ACTIVE;
    if(!ntw16s_global_birth(GetCurrentProcess(),&session->owner_birth_low,&session->owner_birth_high))return 11;
    swprintf(child_ready_name,ARRAYSIZE(child_ready_name),L"Local\\Win16StartReady-%lu",GetCurrentProcessId());
    SetEnvironmentVariableW(L"WIN16_START_TEST_DLL",argv[1]);SetEnvironmentVariableW(L"WIN16_START_TEST_READY",child_ready_name);
    SetEnvironmentVariableW(L"WIN16_START_TEST_REMOTE",remote_load?L"1":NULL);
    HANDLE child_ready=CreateEventW(NULL,TRUE,FALSE,child_ready_name),stop=CreateEventW(NULL,TRUE,FALSE,stop_name);
    GetModuleFileNameW(NULL,self,ARRAYSIZE(self));swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" target",self);
    STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};
    if(!child_ready || !stop || !CreateProcessW(self,cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))return 12;
    BOOL started=WaitForSingleObject(child_ready,3000)==WAIT_OBJECT_0;
    ntw16s_global_load load={0};void *loaded_worker=NULL;
    DWORD load_begin=remote_load && started?ntw16s_global_load_begin(pi.hProcess,pi.dwProcessId,argv[1],&load):ERROR_SUCCESS;
    DWORD load_finish=remote_load && !load_begin?ntw16s_global_load_finish(&load,3000,&loaded_worker):load_begin;
    ntw16s_global_start bad={0},wrong={0},collision_ticket={0},ticket={0};
    DWORD bad_pid=ntw16s_global_start_begin(pi.hProcess,pi.dwProcessId+1,pi.dwThreadId,session->cookie,argv[1],NULL,&bad);
    DWORD bad_thread=ntw16s_global_start_begin(pi.hProcess,pi.dwProcessId,GetCurrentThreadId(),session->cookie,argv[1],NULL,&bad);
    WCHAR collision_name[NTW16S_GLOBAL_NAME_CAP+64];
    swprintf(collision_name,ARRAYSIZE(collision_name),L"%ls.thread.%lu.%lu.%lu",name,pi.dwProcessId,pi.dwThreadId,session->cookie);
    HANDLE collision=CreateEventW(NULL,TRUE,FALSE,collision_name);
    DWORD collision_error=collision?ntw16s_global_start_begin(pi.hProcess,pi.dwProcessId,pi.dwThreadId,session->cookie,argv[1],NULL,&collision_ticket):ERROR_INVALID_HANDLE;
    BOOL collision_unchanged=collision && WaitForSingleObject(collision,0)==WAIT_TIMEOUT;if(collision)CloseHandle(collision);
    DWORD wrong_begin=ntw16s_global_start_begin(pi.hProcess,pi.dwProcessId,pi.dwThreadId,session->cookie+1,argv[1],NULL,&wrong);
    DWORD wrong_ready=wrong_begin?wrong_begin:ntw16s_global_start_ready(&wrong,3000);
    DWORD wrong_wait=wrong.thread?WaitForSingleObject(wrong.thread,3000):WAIT_FAILED;
    DWORD wrong_close=wrong_begin?wrong_begin:ntw16s_global_start_close(&wrong);
    DWORD begin=started?ntw16s_global_start_begin(pi.hProcess,pi.dwProcessId,pi.dwThreadId,session->cookie,argv[1],NULL,&ticket):ERROR_TIMEOUT;
    DWORD ready=begin?begin:ntw16s_global_start_ready(&ticket,3000);
    BOOL packet_freed=!begin && !ready && !ticket.remote_packet;
    DWORD busy=begin?begin:ntw16s_global_start_close(&ticket);
    SetEvent(stop);DWORD worker_wait=ticket.thread?WaitForSingleObject(ticket.thread,3000):WAIT_FAILED;
    DWORD close_error=begin?begin:ntw16s_global_start_close(&ticket);
    HWND window=FindWindowW(L"Win16OwnedStartTest",L"owned");if(window)PostMessageW(window,WM_CLOSE,0,0);
    DWORD child_wait=WaitForSingleObject(pi.hProcess,3000),code=99;
    if(child_wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);else TerminateProcess(pi.hProcess,ERROR_TIMEOUT);
    CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(child_ready);CloseHandle(stop);UnmapViewOfFile(session);CloseHandle(map);
    if(!started || load_begin || load_finish || (remote_load && !loaded_worker) || bad_pid!=ERROR_INVALID_PARAMETER || bad_thread!=ERROR_INVALID_THREAD_ID || collision_error!=ERROR_ALREADY_EXISTS || !collision_unchanged ||
       wrong_begin || wrong_ready!=ERROR_ACCESS_DENIED || wrong_wait!=WAIT_OBJECT_0 || wrong_close || begin || ready || !packet_freed || busy!=ERROR_BUSY ||
       worker_wait!=WAIT_OBJECT_0 || close_error || child_wait!=WAIT_OBJECT_0 || code){
        printf("GLOBAL START FAIL started=%d load_begin=%lu load_finish=%lu bad_pid=%lu bad_thread=%lu collision=%lu unchanged=%d wrong_begin=%lu wrong_ready=%lu wrong_wait=%lu wrong_close=%lu begin=%lu ready=%lu packet=%d busy=%lu worker_wait=%lu close=%lu child_wait=%lu code=%lu\n",started,load_begin,load_finish,bad_pid,bad_thread,collision_error,collision_unchanged,wrong_begin,wrong_ready,wrong_wait,wrong_close,begin,ready,packet_freed,busy,worker_wait,close_error,child_wait,code);return 13;
    }
    printf("GLOBAL START PASS bits=%u real_dll=1 ready=1 packet_freed=1 busy=1 stop=1 bad_pid=1 bad_thread=1 bad_cookie=1 collision=1 cleanup=1 remote_load=%d\n",(unsigned)(sizeof(void*)*8),remote_load);return 0;
}
