#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global_attach.h"
#include "ntw16s-dll/global_session.h"

static HWND button;static unsigned clicks;
static LRESULT CALLBACK window_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_APP+52){
        HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");
        BOOL (__cdecl *attached)(HWND)=dll?(void *)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;
        return attached?(attached(w)?1:0)|(attached(button)?2:0):0;
    }
    if(m==WM_APP+53)return clicks;
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
    HWND window=CreateWindowW(c.lpszClassName,L"owned",WS_OVERLAPPEDWINDOW,0,0,180,120,NULL,NULL,c.hInstance,NULL);if(!window)return 5;
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
    WCHAR desktop[128],name[NTW16S_GLOBAL_NAME_CAP],stop_name[NTW16S_GLOBAL_NAME_CAP+16],ready_name[128],self[MAX_PATH],cmd[MAX_PATH+32],dll[MAX_PATH];DWORD needed=0;
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&needed) ||
       wcsncmp(desktop,L"Win16S-Dialog-",14))return 7;
    if(!ntw16s_global_name(name,ARRAYSIZE(name)) || swprintf(stop_name,ARRAYSIZE(stop_name),L"%ls.stop",name)<0)return 8;
    HANDLE map=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(ntw16s_global_session),name);
    if(!map || GetLastError()==ERROR_ALREADY_EXISTS)return 9;
    ntw16s_global_session *session=(ntw16s_global_session *)MapViewOfFile(map,FILE_MAP_ALL_ACCESS,0,0,sizeof(*session));if(!session)return 10;
    ZeroMemory(session,sizeof(*session));session->magic=NTW16S_GLOBAL_MAGIC;session->version=NTW16S_GLOBAL_VERSION;
    session->owner_pid=GetCurrentProcessId();session->cookie=0x3579;session->phase=NTW16S_GLOBAL_ACTIVE;
    if(!ntw16s_global_birth(GetCurrentProcess(),&session->owner_birth_low,&session->owner_birth_high))return 11;
    swprintf(ready_name,ARRAYSIZE(ready_name),L"Local\\Win16AttachReady-%lu",GetCurrentProcessId());SetEnvironmentVariableW(L"WIN16_ATTACH_TEST_READY",ready_name);
    HANDLE ready=CreateEventW(NULL,TRUE,FALSE,ready_name),stop=CreateEventW(NULL,TRUE,FALSE,stop_name);
    GetModuleFileNameW(NULL,self,ARRAYSIZE(self));wcscpy(dll,self);WCHAR *slash=wcsrchr(dll,L'\\');if(!slash)return 12;
    wcscpy(slash+1,sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" target",self);
    STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};
    if(!ready || !stop || !CreateProcessW(self,cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,&pi))return 13;
    BOOL started=WaitForSingleObject(ready,3000)==WAIT_OBJECT_0;HWND window=FindWindowW(L"Win16OwnedAttachTest",L"owned");
    ntw16s_global_policy policy;DWORD init=ntw16s_global_policy_init(&policy);ntw16s_global_attach ticket={0};
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
    session->phase=NTW16S_GLOBAL_STOPPING;
    DWORD removed=advance?advance:ntw16s_global_attach_remove(&ticket);int after=window?query(window,WM_APP+52):-1;
    SetEvent(stop);DWORD worker_wait=ticket.start.thread?WaitForSingleObject(ticket.start.thread,3000):WAIT_FAILED;
    DWORD closed=ntw16s_global_attach_close(&ticket);
    if(window)PostMessageW(window,WM_CLOSE,0,0);
    DWORD child_wait=WaitForSingleObject(pi.hProcess,3000),code=99;if(child_wait==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);else TerminateProcess(pi.hProcess,ERROR_TIMEOUT);
    CloseHandle(pi.hThread);CloseHandle(pi.hProcess);CloseHandle(ready);CloseHandle(stop);UnmapViewOfFile(session);CloseHandle(map);
    if(!started || init || before || begin || advance || attached!=3 || clicks!=1 || removed || after || worker_wait!=WAIT_OBJECT_0 || closed || child_wait!=WAIT_OBJECT_0 || code){
        printf("ATTACH FAIL started=%d init=%lu before=%d begin=%lu advance=%lu attached=%d clicks=%d remove=%lu after=%d worker_wait=%lu closed=%lu child_wait=%lu code=%lu\n",started,init,before,begin,advance,attached,clicks,removed,after,worker_wait,closed,child_wait,code);return 14;
    }
    printf("ATTACH PASS bits=%u unloaded=1 loaded=1 parent=1 child=1 click=1 removed=1 cleanup=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
