#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <io.h>

#define TEST_COMMAND (WM_APP+54)
#define QUERY_STYLE (WM_APP+71)
#define QUERY_STYLE_2K (WM_APP+81)
typedef struct target {PROCESS_INFORMATION process;HWND window;} target;
static BOOL launch(target *t,const WCHAR *image,unsigned index)
{
    WCHAR title[80],command[32768];STARTUPINFOW startup={.cb=sizeof(startup)};
    if(swprintf(title,ARRAYSIZE(title),L"TrayStyles-%lu-%u",GetCurrentProcessId(),index)<0 ||
       swprintf(command,ARRAYSIZE(command),L"\"%ls\" %ls",image,title)<0 ||
       !CreateProcessW(image,command,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&startup,&t->process))return FALSE;
    ULONGLONG end=GetTickCount64()+6000;
    do{t->window=FindWindowW(L"Win16GlobalE2ETarget",title);if(t->window)return TRUE;
       if(WaitForSingleObject(t->process.hProcess,0)==WAIT_OBJECT_0)break;
       Sleep(30);}while(GetTickCount64()<end);
    return FALSE;
}
static int query(HWND window,UINT message,WPARAM value)
{
    DWORD_PTR result=0;
    return SendMessageTimeoutW(window,message,value,0,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,500,&result)?(int)result:-1;
}
static BOOL wait_value(HWND window,UINT message,WPARAM value,int expected,DWORD timeout)
{
    ULONGLONG end=GetTickCount64()+timeout;
    do{if(query(window,message,value)==expected)return TRUE;Sleep(30);}while(GetTickCount64()<end);
    return FALSE;
}
static void cleanup(target *t)
{
    if(t->window && IsWindow(t->window))PostMessageW(t->window,WM_CLOSE,0,0);
    if(t->process.hProcess){
        if(WaitForSingleObject(t->process.hProcess,3000)!=WAIT_OBJECT_0){TerminateProcess(t->process.hProcess,ERROR_TIMEOUT);WaitForSingleObject(t->process.hProcess,1000);}
        CloseHandle(t->process.hThread);CloseHandle(t->process.hProcess);
    }
}
int wmain(int argc,WCHAR **argv)
{
    BOOL startup_only=argc==5 && !wcscmp(argv[4],L"--startup-only");
    BOOL logging=argc==6 && !wcscmp(argv[4],L"--log");
    if(argc!=4 && !startup_only && !logging)return 2;
    if(logging){
        if(!_wfreopen(argv[5],L"w",stdout))return 2;
        setvbuf(stdout,NULL,_IONBF,0);
        HANDLE output=(HANDLE)_get_osfhandle(_fileno(stdout));
        if(output==INVALID_HANDLE_VALUE || !SetHandleInformation(output,HANDLE_FLAG_INHERIT,HANDLE_FLAG_INHERIT) ||
           !SetStdHandle(STD_OUTPUT_HANDLE,output) || !SetStdHandle(STD_ERROR_HANDLE,output))return 2;
    }
    target targets[3]={0};PROCESS_INFORMATION manager={0};HWND tray=NULL;BOOL passed=FALSE;
    WCHAR command[32768];STARTUPINFOW startup={.cb=sizeof(startup),.dwFlags=STARTF_USESTDHANDLES,
        .hStdInput=GetStdHandle(STD_INPUT_HANDLE),.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE),.hStdError=GetStdHandle(STD_ERROR_HANDLE)};
    if(FindWindowW(L"LibTrayWindow",L"Win16 Style")){puts("SKIP existing tray controller");return 77;}
    if(!launch(&targets[0],argv[2],0) || !launch(&targets[1],argv[3],1))goto done;
    if(swprintf(command,ARRAYSIZE(command),L"\"%ls\" --global",argv[1])<0 ||
       !CreateProcessW(argv[1],command,NULL,NULL,TRUE,CREATE_NO_WINDOW,NULL,NULL,&startup,&manager))goto done;
    ULONGLONG end=GetTickCount64()+12000;
    do{DWORD pid=0;tray=FindWindowW(L"LibTrayWindow",L"Win16 Style");
       if(tray)GetWindowThreadProcessId(tray,&pid);
       if(pid==manager.dwProcessId)break;
       tray=NULL;
       if(WaitForSingleObject(manager.hProcess,0)==WAIT_OBJECT_0)break;
       Sleep(30);}while(GetTickCount64()<end);
    if(!tray || !wait_value(tray,TEST_COMMAND,3,0,3000)){
        DWORD code=STILL_ACTIVE;GetExitCodeProcess(manager.hProcess,&code);
        printf("FAIL tray startup window=%d manager_exit=%lu\n",tray!=NULL,code);goto done;
    }
    if(startup_only){puts("PASS tray startup");passed=TRUE;goto done;}
    if(query(tray,TEST_COMMAND,1)<0 || !wait_value(tray,TEST_COMMAND,3,1,18000) ||
       !wait_value(targets[0].window,QUERY_STYLE,0,1,10000) || !wait_value(targets[1].window,QUERY_STYLE,0,1,10000)){
        puts("FAIL enable16");goto done;}
    puts("PASS enable16 both existing");
    if(query(tray,TEST_COMMAND,6)<0 || !wait_value(tray,TEST_COMMAND,3,5,20000) ||
       !wait_value(targets[0].window,QUERY_STYLE_2K,0,1,10000) || !wait_value(targets[1].window,QUERY_STYLE_2K,0,1,10000) ||
       query(targets[0].window,QUERY_STYLE,0)!=0 || query(targets[1].window,QUERY_STYLE,0)!=0){
        DWORD code=STILL_ACTIVE;GetExitCodeProcess(manager.hProcess,&code);
        printf("FAIL switch2k state=%d win16=%d,%d win2k=%d,%d manager=%lu\n",query(tray,TEST_COMMAND,3),
               query(targets[0].window,QUERY_STYLE,0),query(targets[1].window,QUERY_STYLE,0),
               query(targets[0].window,QUERY_STYLE_2K,0),query(targets[1].window,QUERY_STYLE_2K,0),code);goto done;}
    puts("PASS switch2k both existing");
    if(!launch(&targets[2],argv[2],2) || !wait_value(targets[2].window,QUERY_STYLE_2K,0,1,10000)){puts("FAIL new2k");goto done;}
    puts("PASS new window 2k");
    if(query(tray,TEST_COMMAND,1)<0 || !wait_value(tray,TEST_COMMAND,3,1,20000) ||
       !wait_value(targets[0].window,QUERY_STYLE,0,1,10000) || !wait_value(targets[1].window,QUERY_STYLE,0,1,10000) ||
       !wait_value(targets[2].window,QUERY_STYLE,0,1,10000)){puts("FAIL switch16");goto done;}
    puts("PASS switch16 three windows");
    if(query(tray,TEST_COMMAND,1)<0 || !wait_value(tray,TEST_COMMAND,3,0,20000) ||
       !wait_value(targets[0].window,QUERY_STYLE,0,0,5000) || !wait_value(targets[1].window,QUERY_STYLE,0,0,5000) ||
       !wait_value(targets[2].window,QUERY_STYLE,0,0,5000)){puts("FAIL disable");goto done;}
    puts("PASS disable three windows");passed=TRUE;
done:
    if(tray && IsWindow(tray)){
        int state=query(tray,TEST_COMMAND,3);
        if(state==1 || state==5){query(tray,TEST_COMMAND,state==5?6:1);(void)wait_value(tray,TEST_COMMAND,3,0,20000);}
        PostMessageW(tray,TEST_COMMAND,2,0);
    }
    if(manager.hProcess){if(WaitForSingleObject(manager.hProcess,5000)!=WAIT_OBJECT_0){TerminateProcess(manager.hProcess,ERROR_TIMEOUT);WaitForSingleObject(manager.hProcess,1000);}
        CloseHandle(manager.hThread);CloseHandle(manager.hProcess);}
    for(unsigned i=0;i<ARRAYSIZE(targets);++i)cleanup(&targets[i]);
    return passed?0:1;
}
