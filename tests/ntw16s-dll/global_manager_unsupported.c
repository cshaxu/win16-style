#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global/manager.h"
#include "ntw16s-dll/global/session.h"

static HWND button,later;static unsigned clicks;
static LRESULT CALLBACK window_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_APP+52){
        HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");
        BOOL (__cdecl *attached)(HWND)=dll?(void *)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;
        return attached?(attached(w)?1:0)|(attached(button)?2:0):0;
    }
    if(m==WM_APP+53)return clicks;
    if(m==WM_APP+54){
        later=CreateWindowW(L"Win16OwnedAttachTest",L"later eligible",WS_OVERLAPPEDWINDOW,200,0,180,120,NULL,NULL,GetModuleHandleW(NULL),NULL);
        if(later){ShowWindow(later,SW_SHOW);SetWindowPos(later,HWND_BOTTOM,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);}return later!=NULL;
    }
    if(m==WM_APP+55){
        HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");
        BOOL (__cdecl *attached)(HWND)=dll?(void *)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;
        return attached && later?attached(later):0;
    }
    if(m==WM_COMMAND && HIWORD(a)==BN_CLICKED){++clicks;return 0;}
    if(m==WM_CLOSE){DestroyWindow(w);return 0;}
    if(m==WM_DESTROY){PostQuitMessage(0);return 0;}
    return DefWindowProcW(w,m,a,b);
}
static int target(void)
{
    WCHAR name[128];if(!GetEnvironmentVariableW(L"WIN16_ATTACH_TEST_READY",name,ARRAYSIZE(name)))return 2;
    HANDLE ready=OpenEventW(EVENT_MODIFY_STATE,FALSE,name);if(!ready)return 3;
    WNDCLASSW c={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=window_proc,.lpszClassName=L"Win16OwnedAttachTest"};
    if(!RegisterClassW(&c))return 4;
    HWND window=CreateWindowExW(WS_EX_LAYERED,c.lpszClassName,L"owned",WS_OVERLAPPEDWINDOW,0,0,180,120,NULL,NULL,c.hInstance,NULL);if(!window)return 5;
    button=CreateWindowW(L"BUTTON",L"Click",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,10,10,80,30,window,(HMENU)(UINT_PTR)101,c.hInstance,NULL);
    if(!button)return 6;
    ShowWindow(window,SW_SHOW);SetEvent(ready);CloseHandle(ready);
    MSG message;while(GetMessageW(&message,NULL,0,0)>0){TranslateMessage(&message);DispatchMessageW(&message);}return 0;
}
static int query(HWND window,UINT message)
{DWORD_PTR result=0;return SendMessageTimeoutW(window,message,0,0,SMTO_ABORTIFHUNG,1000,&result)?(int)result:-1;}
int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !lstrcmpW(argv[1],L"target"))return target();
    BOOL preexisting=argc==2 && !lstrcmpW(argv[1],L"preexisting");if(argc!=1 && !preexisting)return 2;
    WCHAR desktop[128],name[NTW16S_GLOBAL_NAME_CAP],stop_name[NTW16S_GLOBAL_NAME_CAP+16],ready_name[128],self[MAX_PATH],cmd[MAX_PATH+32],dll[MAX_PATH];DWORD needed=0;
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&needed) ||
       wcsncmp(desktop,L"Win16S-Dialog-",14))return 7;
    if(!ntw16s_global_name(name,ARRAYSIZE(name)) || swprintf(stop_name,ARRAYSIZE(stop_name),L"%ls.stop",name)<0)return 8;
    HANDLE map=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(ntw16s_global_session),name);
    if(!map || GetLastError()==ERROR_ALREADY_EXISTS)return 9;
    ntw16s_global_session *session=(ntw16s_global_session *)MapViewOfFile(map,FILE_MAP_ALL_ACCESS,0,0,sizeof(*session));if(!session)return 10;
    ZeroMemory(session,sizeof(*session));session->magic=NTW16S_GLOBAL_MAGIC;session->version=NTW16S_GLOBAL_VERSION;
    session->owner_pid=GetCurrentProcessId();session->cookie=0x3579;session->phase=NTW16S_GLOBAL_ACTIVE;
    if(!win32_process_global_birth(GetCurrentProcess(),&session->owner_birth_low,&session->owner_birth_high))return 11;
    swprintf(ready_name,ARRAYSIZE(ready_name),L"Local\\Win16AttachReady-%lu",GetCurrentProcessId());SetEnvironmentVariableW(L"WIN16_ATTACH_TEST_READY",ready_name);
    HANDLE ready=CreateEventW(NULL,TRUE,FALSE,ready_name),stop=CreateEventW(NULL,TRUE,FALSE,stop_name);
    GetModuleFileNameW(NULL,self,ARRAYSIZE(self));wcscpy(dll,self);WCHAR *slash=wcsrchr(dll,L'\\');if(!slash)return 12;
    wcscpy(slash+1,sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" target",self);
    STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};
    if(!ready || !stop || !CreateProcessW(self,cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))return 13;
    BOOL started=WaitForSingleObject(ready,3000)==WAIT_OBJECT_0;HWND window=FindWindowW(L"Win16OwnedAttachTest",L"owned");
    int precreated=preexisting && window?query(window,WM_APP+54):0;
    ntw16s_global_manager *manager=NULL;DWORD start=started?ntw16s_global_manager_start(dll,sizeof(void*)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386,&manager):ERROR_INVALID_STATE;
    ntw16s_global_manager_stats stats={0};ULONGLONG deadline=GetTickCount64()+5000;MSG msg;
    while(manager && GetTickCount64()<deadline){
        while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);}
        ntw16s_global_manager_stats_read(manager,&stats);if(stats.failed && stats.active)break;
        MsgWaitForMultipleObjects(0,NULL,FALSE,10,QS_ALLINPUT);
    }
    int before=window?query(window,WM_APP+52):-1,made=preexisting?precreated:(window?query(window,WM_APP+54):0),later=0;
    deadline=GetTickCount64()+3000;
    while(manager && GetTickCount64()<deadline && !later){
        while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);}
        later=query(window,WM_APP+55);if(!later)Sleep(10);
    }
    HWND late_window=FindWindowW(L"Win16OwnedAttachTest",L"later eligible");
    DWORD_PTR ignored=0,ack=0,wrong=0,inactive=0;UINT control=RegisterWindowMessageW(NTW16S_GLOBAL_CONTROL);
    BOOL ack_sent=late_window && control && SendMessageTimeoutW(late_window,control,session->cookie,NTW16S_GLOBAL_QUERY,SMTO_ABORTIFHUNG,1000,&ack);
    BOOL wrong_sent=late_window && control && SendMessageTimeoutW(late_window,control,session->cookie^1u,NTW16S_GLOBAL_QUERY,SMTO_ABORTIFHUNG,1000,&wrong);
    session->phase=NTW16S_GLOBAL_STOPPING;
    BOOL inactive_sent=late_window && control && SendMessageTimeoutW(late_window,control,session->cookie,NTW16S_GLOBAL_QUERY,SMTO_ABORTIFHUNG,1000,&inactive);
    BOOL sent=late_window && control && SendMessageTimeoutW(late_window,control,session->cookie,NTW16S_GLOBAL_REMOVE,SMTO_ABORTIFHUNG,1000,&ignored);
    int after=window?query(window,WM_APP+55):-1;SetEvent(stop);
    DWORD stopped=manager?ntw16s_global_manager_stop(manager,5000):ERROR_INVALID_STATE;
    if(window)PostMessageW(window,WM_CLOSE,0,0);
    DWORD wait=WaitForSingleObject(pi.hProcess,3000),code=99;if(wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);else TerminateProcess(pi.hProcess,ERROR_TIMEOUT);
    CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(ready);CloseHandle(stop);UnmapViewOfFile(session);CloseHandle(map);
    printf("MANAGER_UNSUPPORTED bits=%u mode=%ls start=%lu failed=%ld active=%ld before=%d made=%d later=%d sent=%d after=%d stopped=%lu wait=%lu code=%lu ack=%d wrong=%d inactive=%d\n",(unsigned)(sizeof(void*)*8),preexisting?L"preexisting":L"later",start,stats.failed,stats.active,before,made,later,sent,after,stopped,wait,code,ack_sent && ack==NTW16S_GLOBAL_ATTACHED,wrong_sent && !wrong,inactive_sent && !inactive);
    return started && !start && stats.failed>0 && stats.active>=1 && !(before&1) && made && later==1 && sent && !after && !stopped && wait==WAIT_OBJECT_0 && !code && ack_sent && ack==NTW16S_GLOBAL_ATTACHED && wrong_sent && !wrong && inactive_sent && !inactive?0:1;
}

