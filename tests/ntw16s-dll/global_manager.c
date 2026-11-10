#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global/manager.h"
#include "ntw16s-dll/global/policy.h"
#include "ntw16s-dll/global/session.h"
#include "global_session_fixture.h"

static HWND button;
static unsigned clicks;
static BOOL delayed;
static HANDLE delay_started;
static DWORD rendered_button(void)
{
    RECT r;if(!button || !GetWindowRect(button,&r) || r.right<=r.left || r.bottom<=r.top)return 0;
    int width=r.right-r.left,height=r.bottom-r.top;BITMAPINFO info={0};
    info.bmiHeader.biSize=sizeof(info.bmiHeader);info.bmiHeader.biWidth=width;info.bmiHeader.biHeight=-height;
    info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;info.bmiHeader.biCompression=BI_RGB;
    void *bits=NULL;HDC dc=CreateCompatibleDC(NULL);if(!dc)return 0;
    HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,NULL,0);if(!bitmap || !bits){if(bitmap)DeleteObject(bitmap);DeleteDC(dc);return 0;}
    HGDIOBJ old=SelectObject(dc,bitmap);ZeroMemory(bits,(SIZE_T)width*(SIZE_T)height*4);
    SendMessageW(button,WM_PRINT,(WPARAM)dc,PRF_NONCLIENT|PRF_CLIENT|PRF_ERASEBKGND);GdiFlush();
    DWORD hash=2166136261u;const BYTE *p=bits;
    for(SIZE_T i=0;i<(SIZE_T)width*(SIZE_T)height*4;++i){hash^=p[i];hash*=16777619u;}
    SelectObject(dc,old);DeleteObject(bitmap);DeleteDC(dc);return hash?hash:1;
}
static LRESULT CALLBACK window_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==RegisterWindowMessageW(NTW16S_GLOBAL_CONTROL) && b==NTW16S_GLOBAL_APPLY && !delayed){
        delayed=TRUE;if(delay_started)SetEvent(delay_started);Sleep(650);
    }
    if(m==WM_APP+52){
        HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");
        BOOL (__cdecl *attached)(HWND)=dll?(void *)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;
        return attached?(attached(w)?1:0)|(attached(button)?2:0):0;
    }
    if(m==WM_APP+53)return clicks;
    if(m==WM_APP+55)return (LRESULT)(DWORD)rendered_button();
    if(m==WM_APP+56){LOGFONTW font={0};HFONT value=(HFONT)SendMessageW(button,WM_GETFONT,0,0);if(!value || !GetObjectW(value,sizeof(font),&font))return 0;return (LRESULT)(DWORD)(((unsigned)font.lfHeight&0xffffu)|(((unsigned)font.lfWeight&0xffffu)<<16));}
    if(m==WM_APP+57)return GetWindowLongW(button,GWL_STYLE);
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
    if(GetEnvironmentVariableW(L"WIN16_MANAGER_DELAY_STARTED",name,ARRAYSIZE(name)))delay_started=OpenEventW(EVENT_MODIFY_STATE,FALSE,name);
    WNDCLASSW c={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=window_proc,.lpszClassName=L"Win16OwnedManagerTest"};
    if(!RegisterClassW(&c))return 4;
    HWND w=CreateWindowW(c.lpszClassName,title,WS_OVERLAPPEDWINDOW,0,0,180,120,NULL,NULL,c.hInstance,NULL);
    if(!w)return 5;
    button=CreateWindowW(L"BUTTON",L"Click",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,10,10,80,30,w,(HMENU)(UINT_PTR)101,c.hInstance,NULL);
    if(!button)return 6;
    ShowWindow(w,SW_SHOW);SetEvent(ready);CloseHandle(ready);
    MSG msg;while(GetMessageW(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}
    if(delay_started)CloseHandle(delay_started);
    return 0;
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
static int stop_race_child(void)
{
    MSG msg;PeekMessageW(&msg,NULL,WM_USER,WM_USER,PM_NOREMOVE);
    Sleep(600);
    return GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll")?41:0;
}
static int stop_race(void)
{
    WCHAR name[NTW16S_GLOBAL_NAME_CAP],stop_name[NTW16S_GLOBAL_NAME_CAP+16],self[MAX_PATH],dll[MAX_PATH],cmd[MAX_PATH+32];
    HANDLE map=NULL;ntw16s_global_session *session=NULL;
    int session_error=global_test_session(name,stop_name,&map,&session,0x7894);if(session_error)return session_error;
    HANDLE stop=CreateEventW(NULL,TRUE,FALSE,stop_name);if(!stop)return 12;
    GetModuleFileNameW(NULL,self,ARRAYSIZE(self));wcscpy(dll,self);WCHAR *slash=wcsrchr(dll,L'\\');if(!slash)return 13;
    wcscpy(slash+1,sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    HWND sentinel=CreateWindowW(L"STATIC",L"test sentinel",WS_OVERLAPPED,0,0,10,10,NULL,NULL,GetModuleHandleW(NULL),NULL);
    ntw16s_global_manager *manager=NULL;DWORD start=sentinel?ntw16s_global_manager_start(dll,sizeof(void*)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386,&manager):GetLastError();
    PROCESS_INFORMATION pi={0};STARTUPINFOW si={.cb=sizeof(si)};swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" race-child",self);
    BOOL created=!start && CreateProcessW(self,cmd,NULL,NULL,FALSE,CREATE_SUSPENDED,NULL,NULL,&si,&pi);
    win32_process_global_policy policy={0};win32_process_global_identity identity;DWORD initial=ERROR_INVALID_STATE;
    if(created && !win32_process_global_policy_init(&policy)){
        initial=ntw16s_global_policy_inspect_thread(&policy,pi.dwProcessId,pi.dwThreadId,sizeof(void*)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386,&identity);
        ntw16s_global_manager_test_process(manager,pi.dwProcessId);
    }
    BOOL tracked=created && ntw16s_global_manager_tracks(manager,pi.dwProcessId,pi.dwThreadId);
    DWORD pending=tracked?ntw16s_global_manager_wait_idle(manager,40):ERROR_INVALID_STATE;
    session->phase=NTW16S_GLOBAL_STOPPING;
    BOOL resumed=created && ResumeThread(pi.hThread)!=(DWORD)-1;
    BOOL exited=resumed && WaitForSingleObject(pi.hProcess,3000)==WAIT_OBJECT_0;DWORD child=999;
    if(exited)GetExitCodeProcess(pi.hProcess,&child);
    SetEvent(stop);DWORD stopped=manager?ntw16s_global_manager_stop(manager,5000):ERROR_INVALID_STATE;
    if(created){if(!exited)TerminateProcess(pi.hProcess,ERROR_TIMEOUT);CloseHandle(pi.hThread);CloseHandle(pi.hProcess);}
    if(sentinel)DestroyWindow(sentinel);
    CloseHandle(stop);UnmapViewOfFile(session);CloseHandle(map);
    printf("MANAGER STOP RACE bits=%u start=%lu created=%d initial=%lu tracked=%d pending=%lu resumed=%d exited=%d child=%lu stopped=%lu\n",(unsigned)(sizeof(void*)*8),start,created,initial,tracked,pending,resumed,exited,child,stopped);
    return !start && created && initial==ERROR_NOT_READY && tracked && pending==ERROR_TIMEOUT && resumed && exited && !child && !stopped?0:15;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !lstrcmpW(argv[1],L"target"))return target();
    if(argc==2 && !lstrcmpW(argv[1],L"race-child"))return stop_race_child();
    if(argc==2 && !lstrcmpW(argv[1],L"stop-race"))return stop_race();
    WCHAR name[NTW16S_GLOBAL_NAME_CAP],stop_name[NTW16S_GLOBAL_NAME_CAP+16],self[MAX_PATH],dll[MAX_PATH];
    HANDLE map=NULL;ntw16s_global_session *session=NULL;
    int session_error=global_test_session(name,stop_name,&map,&session,0x7893);if(session_error)return session_error;
    HANDLE stop=CreateEventW(NULL,TRUE,FALSE,stop_name);if(!stop)return 12;
    GetModuleFileNameW(NULL,self,ARRAYSIZE(self));wcscpy(dll,self);WCHAR *slash=wcsrchr(dll,L'\\');if(!slash)return 13;
    wcscpy(slash+1,sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    PROCESS_INFORMATION pi[2]={{0}};HWND windows[2]={0};ntw16s_global_manager *manager=NULL;
    WCHAR delay_name[128];swprintf(delay_name,ARRAYSIZE(delay_name),L"Local\\Win16ManagerDelay-%lu",GetCurrentProcessId());
    HANDLE delay_event=CreateEventW(NULL,TRUE,FALSE,delay_name);SetEnvironmentVariableW(L"WIN16_MANAGER_DELAY_STARTED",delay_name);
    BOOL before=spawn(self,0,&pi[0],&windows[0]);
    DWORD native_draw=before?(DWORD)query(windows[0],WM_APP+55):0;
    DWORD native_font=before?(DWORD)query(windows[0],WM_APP+56):0,native_style=before?(DWORD)query(windows[0],WM_APP+57):0;
    DWORD start=before?ntw16s_global_manager_start(dll,sizeof(void*)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386,&manager):ERROR_INVALID_STATE;
    BOOL delayed_started=delay_event && WaitForSingleObject(delay_event,3000)==WAIT_OBJECT_0;
    DWORD idle_timeout=manager && delayed_started?ntw16s_global_manager_wait_idle(manager,20):ERROR_INVALID_STATE;
    DWORD idle_done=manager?ntw16s_global_manager_wait_idle(manager,3000):ERROR_INVALID_STATE;
    BOOL first=!start && wait_attached(manager,windows[0]);
    DWORD styled_draw=first?(DWORD)query(windows[0],WM_APP+55):0;
    BOOL created=first && spawn(self,1,&pi[1],&windows[1]);
    BOOL second_attached=created && wait_attached(manager,windows[1]);
    BOOL second_active=second_attached && wait_active(manager,2);
    BOOL second=second_attached && second_active;
    ntw16s_global_manager_stats stats={0};if(manager)ntw16s_global_manager_stats_read(manager,&stats);
    int clicked[2]={-1,-1};
    for(unsigned i=0;i<2;++i)if(windows[i]){HWND child=GetDlgItem(windows[i],101);if(child)SendMessageW(child,BM_CLICK,0,0);clicked[i]=query(windows[i],WM_APP+53);}
    DWORD old_pid=pi[1].dwProcessId,old_tid=pi[1].dwThreadId;
    BOOL tracked=manager && ntw16s_global_manager_tracks(manager,old_pid,old_tid),exited=FALSE,retired=FALSE;
    if(second && windows[1] && pi[1].hProcess){
        PostMessageW(windows[1],WM_CLOSE,0,0);exited=WaitForSingleObject(pi[1].hProcess,3000)==WAIT_OBJECT_0;
        if(exited){CloseHandle(pi[1].hThread);CloseHandle(pi[1].hProcess);ZeroMemory(&pi[1],sizeof(pi[1]));windows[1]=NULL;
            for(unsigned i=0;i<40;++i)query(windows[0],WM_APP+54);
            retired=wait_retired(manager,old_pid,old_tid);}
    }
    int survivor=windows[0]?query(windows[0],WM_APP+52):-1;
    if(windows[0]){HWND child=GetDlgItem(windows[0],101);if(child)SendMessageW(child,BM_CLICK,0,0);}
    int survivor_clicks=windows[0]?query(windows[0],WM_APP+53):-1;
    BOOL replacement=retired && spawn(self,2,&pi[1],&windows[1]) && wait_attached(manager,windows[1]);
    if(replacement){HWND child=GetDlgItem(windows[1],101);if(child)SendMessageW(child,BM_CLICK,0,0);}
    int replacement_clicks=replacement?query(windows[1],WM_APP+53):-1;
    session->phase=NTW16S_GLOBAL_STOPPING;UINT control=RegisterWindowMessageW(NTW16S_GLOBAL_CONTROL);
    for(unsigned i=0;i<2;++i)if(windows[i]){DWORD_PTR ignored;SendMessageTimeoutW(windows[i],control,session->cookie,NTW16S_GLOBAL_REMOVE,SMTO_ABORTIFHUNG,500,&ignored);}
    int removed[2]={windows[0]?query(windows[0],WM_APP+52):0,windows[1]?query(windows[1],WM_APP+52):-1};
    DWORD restored_draw=windows[0]?(DWORD)query(windows[0],WM_APP+55):0;
    DWORD restored_font=windows[0]?(DWORD)query(windows[0],WM_APP+56):0,restored_style=windows[0]?(DWORD)query(windows[0],WM_APP+57):0;
    SetEvent(stop);DWORD stopped=manager?ntw16s_global_manager_stop(manager,5000):ERROR_INVALID_STATE;
    BOOL cleaned=TRUE;
    for(unsigned i=0;i<2;++i)if(pi[i].hProcess){
        if(windows[i])PostMessageW(windows[i],WM_CLOSE,0,0);
        if(WaitForSingleObject(pi[i].hProcess,3000)!=WAIT_OBJECT_0){TerminateProcess(pi[i].hProcess,ERROR_TIMEOUT);cleaned=FALSE;}
        CloseHandle(pi[i].hThread);CloseHandle(pi[i].hProcess);
    }
    if(delay_event)CloseHandle(delay_event);
    CloseHandle(stop);UnmapViewOfFile(session);CloseHandle(map);
    printf("MANAGER DRAW bits=%u changed=%d restored_equal=%d native=%lu styled=%lu restored=%lu font=%lu/%lu style=%lu/%lu\n",(unsigned)(sizeof(void*)*8),native_draw!=styled_draw,native_draw==restored_draw,native_draw,styled_draw,restored_draw,native_font,restored_font,native_style,restored_style);
    if(!native_draw || !styled_draw || !restored_draw || native_draw==styled_draw ||
       !before || start || !delayed_started || idle_timeout!=ERROR_TIMEOUT || idle_done || !first || !created || !second || !tracked || !exited || !retired || !replacement || replacement_clicks!=1 || survivor!=3 || survivor_clicks!=2 || clicked[0]!=1 || clicked[1]!=1 || removed[0] || removed[1] || stopped || !cleaned){
        printf("MANAGER FAIL before=%d start=%lu delay=%d idle_timeout=%lu idle_done=%lu first=%d created=%d second=%d attached=%d active=%d exited=%d retired=%d replacement=%d replacement_clicks=%d survivor=%d survivor_clicks=%d stats=%ld,%ld,%ld,%ld clicks=%d,%d removed=%d,%d stop=%lu cleanup=%d\n",before,start,delayed_started,idle_timeout,idle_done,first,created,second,second_attached,second_active,exited,retired,replacement,replacement_clicks,survivor,survivor_clicks,stats.queued,stats.active,stats.failed,stats.overflow,clicked[0],clicked[1],removed[0],removed[1],stopped,cleaned);return 14;
    }
    printf("MANAGER IDLE bits=%u timeout=%lu completed=%lu\n",(unsigned)(sizeof(void*)*8),idle_timeout,idle_done);
    printf("MANAGER PASS bits=%u existing=1 new=1 clicks=2 removed=2 cleanup=1 retired=1 survivor=1 replacement=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
