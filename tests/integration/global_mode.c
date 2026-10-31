#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global_session.h"
#define QUERY_STYLE (WM_APP+71)
#define CREATE_EXTRA (WM_APP+72)
#define CREATE_PROCESS (WM_APP+73)
#define RESET_CLICKS (WM_APP+76)
#define QUERY_CLICKS (WM_APP+77)
#define QUERY_DPI (WM_APP+78)
#define TARGET_COUNT 9
static const WCHAR cls[]=L"Win16GlobalE2ETarget";
typedef struct target {PROCESS_INFORMATION process;HWND window;} target;
static BOOL launch(target *t,const WCHAR *image,const WCHAR *launcher,const WCHAR *title)
{
    WCHAR command[8192];STARTUPINFOW startup={.cb=sizeof(startup)};
    if(launcher){if(swprintf(command,ARRAYSIZE(command),L"\"%ls\" --wait \"%ls\" %ls",launcher,image,title)<0)return FALSE;}
    else if(swprintf(command,ARRAYSIZE(command),L"\"%ls\" %ls",image,title)<0)return FALSE;
    if(!CreateProcessW(launcher?launcher:image,command,NULL,NULL,TRUE,0,NULL,NULL,&startup,&t->process))return FALSE;
    for(unsigned i=0;i<160;++i){t->window=FindWindowW(cls,title);if(t->window && IsWindowVisible(t->window))return TRUE;
        if(WaitForSingleObject(t->process.hProcess,0)==WAIT_OBJECT_0)break;
        Sleep(30);}
    return FALSE;
}
static int query(HWND w)
{
    DWORD_PTR answer=0;if(!SendMessageTimeoutW(w,QUERY_STYLE,0,0,SMTO_ABORTIFHUNG,500,&answer))return -1;
    return (int)answer;
}
static BOOL wait_style(HWND w,int expected)
{
    for(unsigned i=0;i<160;++i){if(query(w)==expected)return TRUE;Sleep(30);}return FALSE;
}
static BOOL check_input(HWND w)
{
    HWND edit=GetDlgItem(w,2),button=GetDlgItem(w,1);WCHAR text[64];DWORD_PTR value=0;
    if(!edit || !button || !SendMessageTimeoutW(w,RESET_CLICKS,0,0,SMTO_ABORTIFHUNG,1000,&value))return FALSE;
    if(!SendMessageTimeoutW(edit,WM_SETTEXT,0,(LPARAM)L"global input",SMTO_ABORTIFHUNG,1000,&value) || !value){printf("GLOBAL INPUT set_failed result=%llu error=%lu\n",(unsigned long long)value,GetLastError());return FALSE;}
    SendMessageW(edit,EM_SETSEL,12,12);SendMessageW(edit,WM_CHAR,L'!',1);
    SendMessageW(button,BM_CLICK,0,0);
    ZeroMemory(text,sizeof(text));
    BOOL got=SendMessageTimeoutW(edit,WM_GETTEXT,ARRAYSIZE(text),(LPARAM)text,SMTO_ABORTIFHUNG,1000,&value)!=0;
    int length=got?(int)value:0;
    BOOL counted=SendMessageTimeoutW(w,QUERY_CLICKS,0,0,SMTO_ABORTIFHUNG,1000,&value)!=0;
    if(!length || lstrcmpW(text,L"global input!") || !counted || value!=1){
        printf("GLOBAL INPUT mismatch length=%d text=%ls clicks=%llu counted=%d\n",length,text,(unsigned long long)value,counted);return FALSE;
    }
    return TRUE;
}
static void cleanup(target *t)
{
    if(t->window && IsWindow(t->window))PostMessageW(t->window,WM_CLOSE,0,0);
    if(t->process.hProcess){if(WaitForSingleObject(t->process.hProcess,3000)!=WAIT_OBJECT_0){TerminateProcess(t->process.hProcess,ERROR_TIMEOUT);WaitForSingleObject(t->process.hProcess,1000);}
        CloseHandle(t->process.hThread);CloseHandle(t->process.hProcess);}
}
int wmain(int argc,WCHAR **argv)
{
    if(argc!=4)return 2;
    WCHAR desk[128],name[NTW16S_GLOBAL_NAME_CAP],request[NTW16S_GLOBAL_NAME_CAP+16],command[4096];DWORD needed=0;
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desk,sizeof(desk),&needed) || wcsncmp(desk,L"Win16S-Dialog-",14))return 3;
    if(!ntw16s_global_name(name,ARRAYSIZE(name)) || swprintf(request,ARRAYSIZE(request),L"%ls.request",name)<0)return 4;
    target targets[TARGET_COUNT]={0};PROCESS_INFORMATION manager={0};HANDLE mapping=NULL,event=NULL;ntw16s_global_session *session=NULL;BOOL passed=FALSE,active=FALSE;DWORD exitcode=99;
    const WCHAR *titles[]={L"T21PlainOld32",L"T21PlainOld64",L"T21RunOld64",L"T21RunOld32",L"T21PlainNew32",L"T21PlainNew64",L"T21RunNew64",L"T21RunNew32",L"T21AfterStop"};
    if(!launch(&targets[0],argv[2],NULL,titles[0]) || !launch(&targets[1],argv[3],NULL,titles[1]) ||
       !launch(&targets[2],argv[3],argv[1],titles[2]) || !launch(&targets[3],argv[2],argv[1],titles[3]))goto done;
    if(!wait_style(targets[0].window,0)||!wait_style(targets[1].window,0)||!wait_style(targets[2].window,1)||!wait_style(targets[3].window,1))goto done;
    if(!check_input(targets[0].window)||!check_input(targets[1].window))goto done;
    if(swprintf(command,ARRAYSIZE(command),L"\"%ls\" --global",argv[1])<0)goto done;
    STARTUPINFOW startup={.cb=sizeof(startup)};
    if(!CreateProcessW(argv[1],command,NULL,NULL,TRUE,0,NULL,NULL,&startup,&manager))goto done;
    for(unsigned i=0;i<200;++i){mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);if(mapping){session=MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*session));
        if(session && session->owner_pid==manager.dwProcessId && session->phase==NTW16S_GLOBAL_ACTIVE){active=TRUE;break;}
        if(session)UnmapViewOfFile(session);
        session=NULL;CloseHandle(mapping);mapping=NULL;}
        if(WaitForSingleObject(manager.hProcess,0)==WAIT_OBJECT_0)break;
        Sleep(50);}
    if(!active){printf("GLOBAL_E2E manager_not_active error=%lu\n",GetLastError());goto done;}
    for(unsigned i=0;i<4;++i)if(!wait_style(targets[i].window,1)){printf("GLOBAL_E2E existing_failed index=%u state=%d\n",i,query(targets[i].window));goto done;}
    if(!launch(&targets[4],argv[2],NULL,titles[4]) || !launch(&targets[5],argv[3],NULL,titles[5]) ||
       !launch(&targets[6],argv[3],argv[1],titles[6]) || !launch(&targets[7],argv[2],argv[1],titles[7]))goto done;
    for(unsigned i=4;i<8;++i)if(!wait_style(targets[i].window,1)){printf("GLOBAL_E2E new_failed index=%u state=%d\n",i,query(targets[i].window));goto done;}
    for(unsigned i=0;i<8;++i)if(!check_input(targets[i].window)){printf("GLOBAL_E2E active_input_failed index=%u\n",i);goto done;}
    DWORD_PTR dpi32=0,dpi64=0;
    if(!SendMessageTimeoutW(targets[0].window,QUERY_DPI,0,0,SMTO_ABORTIFHUNG,1000,&dpi32) || !dpi32 ||
       !SendMessageTimeoutW(targets[1].window,QUERY_DPI,0,0,SMTO_ABORTIFHUNG,1000,&dpi64) || !dpi64)goto done;
    event=OpenEventW(EVENT_MODIFY_STATE,FALSE,request);if(!event || !SetEvent(event))goto done;
    if(WaitForSingleObject(manager.hProcess,12000)!=WAIT_OBJECT_0 || !GetExitCodeProcess(manager.hProcess,&exitcode) || exitcode)goto done;
    for(unsigned i=0;i<8;++i){int expected=(i==2 || i==3 || i==6 || i==7)?1:0;if(!wait_style(targets[i].window,expected)){printf("GLOBAL_E2E restore_failed index=%u state=%d expected=%d\n",i,query(targets[i].window),expected);goto done;}}
    for(unsigned i=0;i<8;++i)if(!check_input(targets[i].window)){printf("GLOBAL_E2E stopped_input_failed index=%u\n",i);goto done;}
    const unsigned normal[]={2,3,6,7};
    for(unsigned item=0;item<ARRAYSIZE(normal);++item){unsigned i=normal[item];DWORD process=0;DWORD_PTR created=0;WCHAR title[80];
        GetWindowThreadProcessId(targets[i].window,&process);
        if(!SendMessageTimeoutW(targets[i].window,CREATE_EXTRA,0,0,SMTO_ABORTIFHUNG,2000,&created) || !created)goto done;
        swprintf(title,ARRAYSIZE(title),L"Win16GlobalExtra-%lu",process);
        HWND extra=FindWindowW(cls,title);if(!extra || !wait_style(extra,1)){
            printf("GLOBAL_E2E post_stop_window_failed index=%u state=%d\n",i,extra?query(extra):-1);goto done;}
        if(!SendMessageTimeoutW(targets[i].window,CREATE_PROCESS,0,0,SMTO_ABORTIFHUNG,5000,&created) || !created)goto done;
        swprintf(title,ARRAYSIZE(title),L"Win16GlobalChild-%lu",process);
        HWND child=NULL;for(unsigned retry=0;retry<160;++retry){child=FindWindowW(cls,title);if(child && IsWindowVisible(child))break;Sleep(30);}
        if(!child || !wait_style(child,1)){printf("GLOBAL_E2E post_stop_child_failed index=%u state=%d\n",i,child?query(child):-1);goto done;}
        PostMessageW(child,WM_CLOSE,0,0);
    }
    if(!launch(&targets[8],argv[3],NULL,titles[8]) || !wait_style(targets[8].window,0))goto done;
    printf("GLOBAL E2E PASS old=4 new=4 restore=4 normal_retained=4 post_stop_windows=4 post_stop_children=4 stopped=1 host=%lu\n",session->host_pid);
    printf("GLOBAL INPUT PASS native=2 active=8 stopped=8 dpi32=%llu dpi64=%llu\n",(unsigned long long)dpi32,(unsigned long long)dpi64);passed=TRUE;
done:
    if(!passed)printf("GLOBAL E2E FAIL active=%d exit=%lu error=%lu\n",active,exitcode,GetLastError());
    if(manager.hProcess && WaitForSingleObject(manager.hProcess,0)!=WAIT_OBJECT_0){if(!event)event=OpenEventW(EVENT_MODIFY_STATE,FALSE,request);if(event)SetEvent(event);
        if(WaitForSingleObject(manager.hProcess,5000)!=WAIT_OBJECT_0){TerminateProcess(manager.hProcess,ERROR_TIMEOUT);WaitForSingleObject(manager.hProcess,1000);}}
    if(event)CloseHandle(event);
    if(session)UnmapViewOfFile(session);
    if(mapping)CloseHandle(mapping);
    if(manager.hThread)CloseHandle(manager.hThread);
    if(manager.hProcess)CloseHandle(manager.hProcess);
    for(unsigned i=0;i<TARGET_COUNT;++i)cleanup(&targets[i]);
    return passed?0:1;
}
