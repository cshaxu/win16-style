#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global_manager.h"
#include "ntw16s-dll/global_session.h"

static HWND button;
static unsigned clicks;
static BOOL delayed;
static LRESULT CALLBACK window_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==RegisterWindowMessageW(NTW16S_GLOBAL_CONTROL) && b==NTW16S_GLOBAL_APPLY && !delayed){
        delayed=TRUE;Sleep(650);
    }
    if(m==WM_APP+52){
        HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");
        BOOL (__cdecl *attached)(HWND)=dll?(void *)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;
        return attached?(attached(w)?1:0)|(attached(button)?2:0):0;
    }
    if(m==WM_APP+53)return clicks;
    if(m==WM_APP+54){ShowWindow(button,SW_HIDE);ShowWindow(button,SW_SHOW);return 1;}
    if(m==WM_COMMAND && HIWORD(a)==BN_CLICKED){++clicks;return 0;}
    if(m==WM_CLOSE){DestroyWindow(w);return 0;}
    if(m==WM_DESTROY){PostQuitMessage(0);return 0;}
    return DefWindowProcW(w,m,a,b);
}
static int target(void)
{
    WCHAR name[128],title[128];
    if(!GetEnvironmentVariableW(L"WIN16_MANAGER_READY",name,ARRAYSIZE(name)) ||
       !GetEnvironmentVariableW(L"WIN16_MANAGER_TITLE",title,ARRAYSIZE(title)))return 2;
    HANDLE ready=OpenEventW(EVENT_MODIFY_STATE,FALSE,name);if(!ready)return 3;
    WNDCLASSW c={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=window_proc,.lpszClassName=L"Win16OwnedManagerTest"};
    if(!RegisterClassW(&c))return 4;
    HWND w=CreateWindowW(c.lpszClassName,title,WS_OVERLAPPEDWINDOW,0,0,180,120,NULL,NULL,c.hInstance,NULL);
    if(!w)return 5;
    button=CreateWindowW(L"BUTTON",L"Click",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,10,10,80,30,w,(HMENU)(UINT_PTR)101,c.hInstance,NULL);
    if(!button)return 6;
    ShowWindow(w,SW_SHOW);SetEvent(ready);CloseHandle(ready);
    MSG msg;while(GetMessageW(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}return 0;
}
static int query(HWND w,UINT message)
{DWORD_PTR value=0;return SendMessageTimeoutW(w,message,0,0,SMTO_ABORTIFHUNG,500,&value)?(int)value:-1;}
static BOOL wait_attached(ntw16s_global_manager *manager,HWND w)
{
    ULONGLONG deadline=GetTickCount64()+5000;HANDLE changed=ntw16s_global_manager_changed(manager);
    while(GetTickCount64()<deadline){
        MSG msg;while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);}
        if(query(w,WM_APP+52)==3)return TRUE;
        ULONGLONG now=GetTickCount64();DWORD left=now<deadline?(DWORD)(deadline-now):0;
        DWORD result=MsgWaitForMultipleObjects(1,&changed,FALSE,left,QS_ALLINPUT);
        if(result==WAIT_TIMEOUT || result==WAIT_FAILED)return FALSE;
    }
    return FALSE;
}
static BOOL wait_active(ntw16s_global_manager *manager,LONG count)
{
    ULONGLONG deadline=GetTickCount64()+5000;HANDLE changed=ntw16s_global_manager_changed(manager);
    while(GetTickCount64()<deadline){
        ntw16s_global_manager_stats stats;ntw16s_global_manager_stats_read(manager,&stats);
        if(stats.active>=count)return TRUE;
        MSG msg;while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);}
        ULONGLONG now=GetTickCount64();DWORD left=now<deadline?(DWORD)(deadline-now):0;
        DWORD result=MsgWaitForMultipleObjects(1,&changed,FALSE,left,QS_ALLINPUT);
        if(result==WAIT_TIMEOUT || result==WAIT_FAILED)return FALSE;
    }
    return FALSE;
}
static BOOL wait_retired(ntw16s_global_manager *manager,DWORD pid,DWORD tid)
{
    ULONGLONG deadline=GetTickCount64()+3000;HANDLE changed=ntw16s_global_manager_changed(manager);
    while(GetTickCount64()<deadline){
        MSG msg;while(PeekMessageW(&msg,NULL,0,0,PM_REMOVE)){TranslateMessage(&msg);DispatchMessageW(&msg);}
        if(!ntw16s_global_manager_tracks(manager,pid,tid))return TRUE;
        ULONGLONG now=GetTickCount64();DWORD left=now<deadline?(DWORD)(deadline-now):0;
        DWORD result=MsgWaitForMultipleObjects(1,&changed,FALSE,left,QS_ALLINPUT);
        if(result==WAIT_TIMEOUT || result==WAIT_FAILED)return FALSE;
    }
    return FALSE;
}
static BOOL spawn(const WCHAR *self,unsigned index,PROCESS_INFORMATION *pi,HWND *window)
{
    WCHAR ready_name[128],title[128],cmd[MAX_PATH+32];
    swprintf(ready_name,ARRAYSIZE(ready_name),L"Local\\Win16ManagerReady-%lu-%u",GetCurrentProcessId(),index);
    swprintf(title,ARRAYSIZE(title),L"owned-%lu-%u",GetCurrentProcessId(),index);
    HANDLE ready=CreateEventW(NULL,TRUE,FALSE,ready_name);if(!ready)return FALSE;
    SetEnvironmentVariableW(L"WIN16_MANAGER_READY",ready_name);SetEnvironmentVariableW(L"WIN16_MANAGER_TITLE",title);
    swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" target",self);
    STARTUPINFOW si={.cb=sizeof(si)};
    BOOL started=CreateProcessW(self,cmd,NULL,NULL,FALSE,0,NULL,NULL,&si,pi);
    BOOL ok=started && WaitForSingleObject(ready,3000)==WAIT_OBJECT_0;
    *window=ok?FindWindowW(L"Win16OwnedManagerTest",title):NULL;
    CloseHandle(ready);return ok && *window;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !lstrcmpW(argv[1],L"target"))return target();
    WCHAR desktop[128],name[NTW16S_GLOBAL_NAME_CAP],stop_name[NTW16S_GLOBAL_NAME_CAP+16],self[MAX_PATH],dll[MAX_PATH];DWORD needed=0;
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&needed) ||
       wcsncmp(desktop,L"Win16S-Dialog-",14))return 7;
    if(!ntw16s_global_name(name,ARRAYSIZE(name)) || swprintf(stop_name,ARRAYSIZE(stop_name),L"%ls.stop",name)<0)return 8;
    HANDLE map=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(ntw16s_global_session),name);
    if(!map || GetLastError()==ERROR_ALREADY_EXISTS)return 9;
    ntw16s_global_session *session=(ntw16s_global_session *)MapViewOfFile(map,FILE_MAP_ALL_ACCESS,0,0,sizeof(*session));if(!session)return 10;
    ZeroMemory(session,sizeof(*session));session->magic=NTW16S_GLOBAL_MAGIC;session->version=NTW16S_GLOBAL_VERSION;
    session->owner_pid=GetCurrentProcessId();session->cookie=0x7893;session->phase=NTW16S_GLOBAL_ACTIVE;
    if(!ntw16s_global_birth(GetCurrentProcess(),&session->owner_birth_low,&session->owner_birth_high))return 11;
    HANDLE stop=CreateEventW(NULL,TRUE,FALSE,stop_name);if(!stop)return 12;
    GetModuleFileNameW(NULL,self,ARRAYSIZE(self));wcscpy(dll,self);WCHAR *slash=wcsrchr(dll,L'\\');if(!slash)return 13;
    wcscpy(slash+1,sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    PROCESS_INFORMATION pi[2]={{0}};HWND windows[2]={0};ntw16s_global_manager *manager=NULL;
    BOOL before=spawn(self,0,&pi[0],&windows[0]);
    DWORD start=before?ntw16s_global_manager_start(dll,sizeof(void*)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386,&manager):ERROR_INVALID_STATE;
    BOOL first=!start && wait_attached(manager,windows[0]);
    BOOL created=first && spawn(self,1,&pi[1],&windows[1]);
    BOOL second_attached=created && wait_attached(manager,windows[1]);
    BOOL second_active=second_attached && wait_active(manager,2);
    BOOL second=second_attached && second_active;
    ntw16s_global_manager_stats stats={0};if(manager)ntw16s_global_manager_stats_read(manager,&stats);
    int clicked[2]={-1,-1};
    for(unsigned i=0;i<2;++i)if(windows[i]){HWND child=GetDlgItem(windows[i],101);if(child)SendMessageW(child,BM_CLICK,0,0);clicked[i]=query(windows[i],WM_APP+53);}
    DWORD old_pid=pi[0].dwProcessId,old_tid=pi[0].dwThreadId;
    BOOL tracked=manager && ntw16s_global_manager_tracks(manager,old_pid,old_tid),exited=FALSE,retired=FALSE;
    if(second && windows[0] && pi[0].hProcess){
        PostMessageW(windows[0],WM_CLOSE,0,0);exited=WaitForSingleObject(pi[0].hProcess,3000)==WAIT_OBJECT_0;
        if(exited){CloseHandle(pi[0].hThread);CloseHandle(pi[0].hProcess);ZeroMemory(&pi[0],sizeof(pi[0]));windows[0]=NULL;
            for(unsigned i=0;i<40;++i)query(windows[1],WM_APP+54);
            retired=wait_retired(manager,old_pid,old_tid);}
    }
    int survivor=windows[1]?query(windows[1],WM_APP+52):-1;
    if(windows[1]){HWND child=GetDlgItem(windows[1],101);if(child)SendMessageW(child,BM_CLICK,0,0);}
    int survivor_clicks=windows[1]?query(windows[1],WM_APP+53):-1;
    BOOL replacement=retired && spawn(self,2,&pi[0],&windows[0]) && wait_attached(manager,windows[0]);
    if(replacement){HWND child=GetDlgItem(windows[0],101);if(child)SendMessageW(child,BM_CLICK,0,0);}
    int replacement_clicks=replacement?query(windows[0],WM_APP+53):-1;
    session->phase=NTW16S_GLOBAL_STOPPING;UINT control=RegisterWindowMessageW(NTW16S_GLOBAL_CONTROL);
    for(unsigned i=0;i<2;++i)if(windows[i]){DWORD_PTR ignored;SendMessageTimeoutW(windows[i],control,session->cookie,NTW16S_GLOBAL_REMOVE,SMTO_ABORTIFHUNG,500,&ignored);}
    int removed[2]={windows[0]?query(windows[0],WM_APP+52):0,windows[1]?query(windows[1],WM_APP+52):-1};
    SetEvent(stop);DWORD stopped=manager?ntw16s_global_manager_stop(manager,5000):ERROR_INVALID_STATE;
    BOOL cleaned=TRUE;
    for(unsigned i=0;i<2;++i)if(pi[i].hProcess){
        if(windows[i])PostMessageW(windows[i],WM_CLOSE,0,0);
        if(WaitForSingleObject(pi[i].hProcess,3000)!=WAIT_OBJECT_0){TerminateProcess(pi[i].hProcess,ERROR_TIMEOUT);cleaned=FALSE;}
        CloseHandle(pi[i].hThread);CloseHandle(pi[i].hProcess);
    }
    CloseHandle(stop);UnmapViewOfFile(session);CloseHandle(map);
    if(!before || start || !first || !created || !second || !tracked || !exited || !retired || !replacement || replacement_clicks!=1 || survivor!=3 || survivor_clicks!=2 || clicked[0]!=1 || clicked[1]!=1 || removed[0] || removed[1] || stopped || !cleaned){
        printf("MANAGER FAIL before=%d start=%lu first=%d created=%d second=%d attached=%d active=%d exited=%d retired=%d replacement=%d replacement_clicks=%d survivor=%d survivor_clicks=%d stats=%ld,%ld,%ld,%ld clicks=%d,%d removed=%d,%d stop=%lu cleanup=%d\n",before,start,first,created,second,second_attached,second_active,exited,retired,replacement,replacement_clicks,survivor,survivor_clicks,stats.queued,stats.active,stats.failed,stats.overflow,clicked[0],clicked[1],removed[0],removed[1],stopped,cleaned);return 14;
    }
    printf("MANAGER PASS bits=%u existing=1 new=1 clicks=2 removed=2 cleanup=1 retired=1 survivor=1 replacement=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
