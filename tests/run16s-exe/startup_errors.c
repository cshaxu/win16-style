#include <windows.h>
#include <shellapi.h>
#include <objbase.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "../ntw16s-dll/isolated_process.h"
static HANDLE job,stop_event,port,empty;
static HDESK desktop;
static DWORD observed_pid;
static HWND dialogs[8];
static unsigned seen,launchers,cmds,consoles,unknown;
static WCHAR actual_class[80],actual_title[256],launcher[32768],shell[MAX_PATH],host[MAX_PATH];
#define CHECK(x) do{if(!(x)){fprintf(stderr,"STARTUP ERRORS FAIL line=%d error=%lu\n",__LINE__,GetLastError());if(job)CloseHandle(job);return 1;}}while(0)
static BOOL CALLBACK window(HWND w,LPARAM unused)
{
    (void)unused;DWORD pid;WCHAR cls[80];GetWindowThreadProcessId(w,&pid);
    BOOL owned=pid==observed_pid;
    if(job&&!owned){HANDLE p=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);if(p){IsProcessInJob(p,job,&owned);CloseHandle(p);}}
    if(!owned||!IsWindowVisible(w)||!GetClassNameW(w,cls,80)||
        (wcscmp(cls,L"Shell_Dialog")&&wcscmp(cls,L"#32770")))return TRUE;
    for(unsigned i=0;i<seen&&i<8;++i)if(dialogs[i]==w)return TRUE;
    if(seen<8)dialogs[seen]=w;
    ++seen;wcscpy_s(actual_class,80,cls);GetWindowTextW(w,actual_title,256);
    printf("NATIVE UI pid=%lu hwnd=%p class=%ls title=%ls ordinal=%u\n",pid,(void *)w,cls,actual_title,seen);fflush(stdout);
    PostMessageW(w,WM_CLOSE,0,0);return TRUE;
}
static DWORD WINAPI observe(void *unused)
{
    (void)unused;
    while(WaitForSingleObject(stop_event,0)==WAIT_TIMEOUT){
        if(port){DWORD message;ULONG_PTR key;LPOVERLAPPED value;
            if(GetQueuedCompletionStatus(port,&message,&key,&value,10)){
                if(message==JOB_OBJECT_MSG_NEW_PROCESS){DWORD pid=(DWORD)(ULONG_PTR)value,cap=32768;WCHAR path[32768]={0};
                    HANDLE p=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);
                    if(p){QueryFullProcessImageNameW(p,0,path,&cap);CloseHandle(p);}
                    if(!_wcsicmp(path,launcher))++launchers;
                    else if(!_wcsicmp(path,shell))++cmds;
                    else if(!_wcsicmp(path,host))++consoles;
                    else ++unknown;
                    printf("OWNED IMAGE pid=%lu image=%ls\n",pid,path);fflush(stdout);}
                if(message==JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO)SetEvent(empty);
            }
        }else WaitForSingleObject(stop_event,10);
        EnumDesktopWindows(desktop,window,0);
    }
    return 0;
}
static int matrix(int argc,WCHAR **argv)
{
    CHECK(argc==2);WCHAR dir[MAX_PATH],image[MAX_PATH],report[MAX_PATH],system[MAX_PATH],command[32768];
    CHECK(GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",dir,MAX_PATH));
    CHECK(GetFullPathNameW(argv[1],32768,launcher,NULL)&&GetSystemDirectoryW(system,MAX_PATH));
    CHECK(swprintf_s(shell,MAX_PATH,L"%ls\\cmd.exe",system)>0&&swprintf_s(host,MAX_PATH,L"%ls\\conhost.exe",system)>0);
    desktop=GetThreadDesktop(GetCurrentThreadId());CHECK(SUCCEEDED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE)));
    for(unsigned kind=0;kind<2;++kind){
        CHECK(swprintf_s(image,MAX_PATH,L"%ls\\legacy-%u.exe",dir,kind)>0);
        BYTE bytes[256]={0};IMAGE_DOS_HEADER *dos=(void *)bytes;dos->e_magic=IMAGE_DOS_SIGNATURE;dos->e_lfanew=kind?64:0;bytes[64]='N';bytes[65]='E';
        HANDLE f=CreateFileW(image,GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL);DWORD wrote;
        CHECK(f!=INVALID_HANDLE_VALUE&&WriteFile(f,bytes,sizeof(bytes),&wrote,NULL)&&wrote==sizeof(bytes)&&CloseHandle(f));
        WCHAR expected_class[80]={0},expected_title[256]={0};
        /* Direct Shell supplies the language/OS-specific UI baseline. */
        for(unsigned mode=0;mode<4;++mode){
            seen=launchers=cmds=consoles=unknown=0;actual_class[0]=actual_title[0]=0;
            stop_event=CreateEventW(NULL,TRUE,FALSE,NULL);CHECK(stop_event);
            PROCESS_INFORMATION p={0};HANDLE output=NULL,null=NULL;DWORD code;
            if(mode){
                CHECK(swprintf_s(report,MAX_PATH,L"%ls\\stderr-%u-%u.txt",dir,kind,mode)>0);
                SECURITY_ATTRIBUTES inherit={.nLength=sizeof(inherit),.bInheritHandle=TRUE};
                output=CreateFileW(report,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,&inherit,CREATE_NEW,0,NULL);
                null=CreateFileW(L"NUL",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,&inherit,OPEN_EXISTING,0,NULL);
                CHECK(output!=INVALID_HANDLE_VALUE&&null!=INVALID_HANDLE_VALUE);
                job=CreateJobObjectW(NULL,NULL);port=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,1);empty=CreateEventW(NULL,TRUE,FALSE,NULL);
                JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
                JOBOBJECT_ASSOCIATE_COMPLETION_PORT association={.CompletionKey=job,.CompletionPort=port};
                CHECK(job&&port&&empty&&SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits))&&
                    SetInformationJobObject(job,JobObjectAssociateCompletionPortInformation,&association,sizeof(association)));
                if(mode==2)CHECK(swprintf_s(command,32768,L"\"%ls\" /d /s /c \"\"%ls\" \"%ls\"\"",shell,launcher,image)>0);
                else CHECK(swprintf_s(command,32768,L"\"%ls\" \"%ls\"",launcher,image)>0);
                STARTUPINFOW startup={.cb=sizeof(startup),.dwFlags=STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW,.wShowWindow=mode==3?SW_HIDE:SW_SHOWNORMAL,.hStdInput=null,.hStdOutput=null,.hStdError=output};
                CHECK(CreateProcessW(mode==2?shell:launcher,command,NULL,NULL,TRUE,CREATE_SUSPENDED|CREATE_NEW_CONSOLE,NULL,NULL,&startup,&p));
                if(!AssignProcessToJobObject(job,p.hProcess)){TerminateProcess(p.hProcess,1);CHECK(FALSE);}
                observed_pid=p.dwProcessId;
                /* For shared cmd, match any launcher UI via its Job below: a
                 * duplicate error from the nested launcher must also fail. */
            }else observed_pid=GetCurrentProcessId();
            HANDLE observer=CreateThread(NULL,0,observe,NULL,0,NULL);CHECK(observer);
            if(mode){CHECK(ResumeThread(p.hThread)==1);CloseHandle(p.hThread);
                CHECK(WaitForSingleObject(p.hProcess,15000)==WAIT_OBJECT_0&&GetExitCodeProcess(p.hProcess,&code));
                CHECK(WaitForSingleObject(empty,10000)==WAIT_OBJECT_0);
            }else{
                SHELLEXECUTEINFOW info={.cbSize=sizeof(info),.fMask=SEE_MASK_NOCLOSEPROCESS|SEE_MASK_NOASYNC,.lpFile=image,.nShow=SW_SHOWNORMAL};
                CHECK(!ShellExecuteExW(&info));code=GetLastError();CHECK(!info.hProcess);
            }
            CHECK(SetEvent(stop_event)&&WaitForSingleObject(observer,2000)==WAIT_OBJECT_0);CloseHandle(observer);CloseHandle(stop_event);
            if(!mode){printf("SHELL BASELINE kind=%u ui=%u error=%lu class=%ls title=%ls\n",kind,seen,code,actual_class,actual_title);fflush(stdout);CHECK(seen==1&&code==ERROR_CANCELLED&&actual_title[0]);wcscpy_s(expected_class,80,actual_class);wcscpy_s(expected_title,256,actual_title);}
            else{
                JOBOBJECT_BASIC_ACCOUNTING_INFORMATION count;
                CHECK(QueryInformationJobObject(job,JobObjectBasicAccountingInformation,&count,sizeof(count),NULL)&&!count.ActiveProcesses);
                CHECK(SetFilePointer(output,0,NULL,FILE_BEGIN)!=INVALID_SET_FILE_POINTER);char text[8192];DWORD read;
                CHECK(ReadFile(output,text,sizeof(text)-1,&read,NULL));text[read]=0;
                printf("ROUTE kind=%s mode=%s exit=%lu ui=%u stderr_bytes=%lu active=%lu total=%lu launcher=%u cmd=%u conhost=%u unknown=%u\n",
                    kind?"NE":"MZ",mode==1?"private-console":mode==2?"shared-cmd":"hidden-console",code,seen,read,count.ActiveProcesses,count.TotalProcesses,launchers,cmds,consoles,unknown);fflush(stdout);
                CHECK(code==4&&launchers==1&&cmds==(mode==2)&&!unknown&&count.TotalProcesses==launchers+cmds+consoles);
                if(mode==1)CHECK(seen==1&&!read&&!wcscmp(expected_class,actual_class)&&!wcscmp(expected_title,actual_title));
                else{const char *match=strstr(text,"native target validation failed (193)");CHECK(!seen&&match&&!strstr(match+1,"native target validation failed (193)"));}
                CloseHandle(p.hProcess);CloseHandle(empty);CloseHandle(port);port=NULL;CloseHandle(job);job=NULL;CloseHandle(output);CloseHandle(null);CHECK(DeleteFileW(report));
            }
        }
        CHECK(DeleteFileW(image));
    }
    CoUninitialize();puts("STARTUP ERRORS PASS 2 formats Windows-Shell/private-console/shared-cmd/hidden-console one_error_channel exit4 jobs_empty");return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,matrix);}
