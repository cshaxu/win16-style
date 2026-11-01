#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <wchar.h>

#define TEST_COMMAND (WM_APP+54)
#define QUERY_STYLE (WM_APP+71)
typedef struct target {PROCESS_INFORMATION process;HWND window;} target;
static BOOL target_start(target *item,const WCHAR *image,unsigned index)
{
    WCHAR title[80],command[8192];swprintf(title,ARRAYSIZE(title),L"TrayProbe-%lu-%u",GetCurrentProcessId(),index);
    if(FindWindowW(L"Win16GlobalE2ETarget",title))return FALSE;
    if(swprintf(command,ARRAYSIZE(command),L"\"%ls\" %ls",image,title)<0)return FALSE;
    STARTUPINFOW si={.cb=sizeof(si)};if(!CreateProcessW(image,command,NULL,NULL,TRUE,0,NULL,NULL,&si,&item->process))return FALSE;
    for(unsigned i=0;i<160;++i){item->window=FindWindowW(L"Win16GlobalE2ETarget",title);if(item->window)return TRUE;
        if(WaitForSingleObject(item->process.hProcess,0)==WAIT_OBJECT_0)break;
        Sleep(30);}
    return FALSE;
}
static int query(HWND window,UINT message,WPARAM value)
{
    DWORD_PTR result=0;if(!SendMessageTimeoutW(window,message,value,0,SMTO_ABORTIFHUNG,500,&result))return -1;
    return (int)result;
}
static BOOL wait_value(HWND window,UINT message,WPARAM value,int expected)
{for(unsigned i=0;i<200;++i){if(query(window,message,value)==expected)return TRUE;Sleep(30);}return FALSE;}
static BOOL propagation(const WCHAR *target,const WCHAR *argument,DWORD expected)
{
    WCHAR image[32768],command[32816];if(wcslen(target)>=ARRAYSIZE(image))return FALSE;
    wcscpy(image,target);WCHAR *slash=wcsrchr(image,L'\\');if(!slash)return FALSE;
    if((size_t)(slash-image)+32>=ARRAYSIZE(image))return FALSE;
    wcscpy(slash+1,L"global-propagation-probe.exe");
    if(swprintf(command,ARRAYSIZE(command),L"\"%ls\" %ls",image,argument)<0)return FALSE;
    STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION process={0};DWORD code=999;
    if(!CreateProcessW(image,command,NULL,NULL,TRUE,0,NULL,NULL,&si,&process))return FALSE;
    if(WaitForSingleObject(process.hProcess,10000)==WAIT_OBJECT_0)GetExitCodeProcess(process.hProcess,&code);
    else{TerminateProcess(process.hProcess,ERROR_TIMEOUT);WaitForSingleObject(process.hProcess,1000);}
    CloseHandle(process.hThread);CloseHandle(process.hProcess);return code==expected;
}
static void cleanup(target *item)
{
    if(item->window && IsWindow(item->window))PostMessageW(item->window,WM_CLOSE,0,0);
    if(item->process.hProcess){if(WaitForSingleObject(item->process.hProcess,3000)!=WAIT_OBJECT_0){TerminateProcess(item->process.hProcess,ERROR_TIMEOUT);WaitForSingleObject(item->process.hProcess,1000);}
        CloseHandle(item->process.hThread);CloseHandle(item->process.hProcess);}
}
int wmain(int argc,WCHAR **argv)
{
    if(argc!=4)return 2;
    WCHAR desk[128],command[4096];DWORD needed=0;
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desk,sizeof(desk),&needed) ||
       wcsncmp(desk,L"Win16S-Dialog-",14))return 3;
    target targets[3]={0};PROCESS_INFORMATION manager={0};HWND tray=NULL;BOOL passed=FALSE;DWORD code=99;
    if(!target_start(&targets[0],argv[2],0) || !target_start(&targets[1],argv[3],1) ||
       !wait_value(targets[0].window,QUERY_STYLE,0,0) || !wait_value(targets[1].window,QUERY_STYLE,0,0))goto done;
    if(swprintf(command,ARRAYSIZE(command),L"\"%ls\" --global",argv[1])<0)goto done;
    STARTUPINFOW startup={.cb=sizeof(startup)};
    if(!CreateProcessW(argv[1],command,NULL,NULL,TRUE,0,NULL,NULL,&startup,&manager))goto done;
    for(unsigned i=0;i<200;++i){DWORD pid=0;tray=FindWindowW(L"Run16sTrayWindow",L"Win16 Style");
        if(tray)GetWindowThreadProcessId(tray,&pid);
        if(pid==manager.dwProcessId)break;
        tray=NULL;
        if(WaitForSingleObject(manager.hProcess,0)==WAIT_OBJECT_0)break;
        Sleep(30);}
    if(!tray || !wait_value(tray,TEST_COMMAND,3,0) || query(tray,TEST_COMMAND,4)!=1 || !wait_value(targets[0].window,QUERY_STYLE,0,0) ||
       !wait_value(targets[1].window,QUERY_STYLE,0,0)){printf("TRAY TEST default off failed\n");goto done;}
    if(query(tray,TEST_COMMAND,1)<0 || !wait_value(tray,TEST_COMMAND,3,1) || query(tray,TEST_COMMAND,4)!=1 || !wait_value(targets[0].window,QUERY_STYLE,0,1) ||
       !wait_value(targets[1].window,QUERY_STYLE,0,1)){printf("TRAY TEST initial failed\n");goto done;}
    if(query(tray,TEST_COMMAND,1)<0 || !wait_value(tray,TEST_COMMAND,3,0) || query(tray,TEST_COMMAND,4)!=1 ||
       !wait_value(targets[0].window,QUERY_STYLE,0,0) || !wait_value(targets[1].window,QUERY_STYLE,0,0) ||
       !target_start(&targets[2],argv[2],2) || !wait_value(targets[2].window,QUERY_STYLE,0,0) ||
       !propagation(argv[2],L"headless-child",41) || !propagation(argv[3],L"headless-child",41)){printf("TRAY TEST disable failed\n");goto done;}
    if(query(tray,TEST_COMMAND,1)<0 || !wait_value(tray,TEST_COMMAND,3,1) || query(tray,TEST_COMMAND,4)!=1 ||
       !wait_value(targets[0].window,QUERY_STYLE,0,1) || !wait_value(targets[1].window,QUERY_STYLE,0,1) ||
       !wait_value(targets[2].window,QUERY_STYLE,0,1) || !propagation(argv[2],L"normal",0) ||
       !propagation(argv[3],L"normal",0)){printf("TRAY TEST reenable failed\n");goto done;}
    PostMessageW(tray,TEST_COMMAND,2,0);
    if(WaitForSingleObject(manager.hProcess,5000)!=WAIT_OBJECT_0 || !GetExitCodeProcess(manager.hProcess,&code) || code ||
       !wait_value(targets[0].window,QUERY_STYLE,0,1) || !wait_value(targets[1].window,QUERY_STYLE,0,1)){
        printf("TRAY TEST exit failed code=%lu\n",code);goto done;}
    printf("TRAY GLOBAL PASS default_off=2 existing=2 off=3 on=3 propagation=2 exit_preserved=2\n");passed=TRUE;
done:
    if(manager.hProcess){if(WaitForSingleObject(manager.hProcess,0)!=WAIT_OBJECT_0){if(tray)PostMessageW(tray,TEST_COMMAND,2,0);
        if(WaitForSingleObject(manager.hProcess,5000)!=WAIT_OBJECT_0){TerminateProcess(manager.hProcess,ERROR_TIMEOUT);WaitForSingleObject(manager.hProcess,1000);}}
        CloseHandle(manager.hThread);CloseHandle(manager.hProcess);}
    for(unsigned i=0;i<ARRAYSIZE(targets);++i)cleanup(&targets[i]);
    return passed?0:1;
}
