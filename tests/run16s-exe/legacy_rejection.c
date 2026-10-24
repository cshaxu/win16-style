#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "../ntw16s-dll/isolated_process.h"
static HANDLE job,stop_event,job_port,job_empty;
static HDESK desktop;
static LONG extra_ui;
static unsigned launchers,cmds,consoles,unknown;
static WCHAR launcher_path[32768],cmd_path[MAX_PATH],console_path[MAX_PATH];
#define CHECK(x) do{if(!(x)){fprintf(stderr,"LEGACY REJECTION FAIL line=%d error=%lu\n",__LINE__,GetLastError());if(job)CloseHandle(job);return 1;}}while(0)
static BOOL CALLBACK window(HWND w,LPARAM unused)
{
    (void)unused;WCHAR cls[64];DWORD pid;BOOL member=FALSE;
    if(!IsWindowVisible(w)||!GetClassNameW(w,cls,64)||
        (wcscmp(cls,L"Shell_Dialog")&&wcscmp(cls,L"#32770")))return TRUE;
    GetWindowThreadProcessId(w,&pid);HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);
    if(process){IsProcessInJob(process,job,&member);CloseHandle(process);}
    if(member){InterlockedIncrement(&extra_ui);PostMessageW(w,WM_CLOSE,0,0);}
    return TRUE;
}
static DWORD WINAPI observe(void *unused)
{
    (void)unused;
    while(WaitForSingleObject(stop_event,0)==WAIT_TIMEOUT){
        DWORD message;ULONG_PTR key;LPOVERLAPPED value;
        if(GetQueuedCompletionStatus(job_port,&message,&key,&value,10)){
            DWORD pid=(DWORD)(ULONG_PTR)value;
            if(message==JOB_OBJECT_MSG_NEW_PROCESS){WCHAR image[32768]={0};DWORD cap=32768;
                HANDLE p=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);
                if(p){QueryFullProcessImageNameW(p,0,image,&cap);CloseHandle(p);}
                if(!_wcsicmp(image,launcher_path))++launchers;
                else if(!_wcsicmp(image,cmd_path))++cmds;
                else if(!_wcsicmp(image,console_path))++consoles;
                else ++unknown;
                printf("OWNED IMAGE pid=%lu image=%ls\n",pid,image);fflush(stdout);}
            if(message==JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO)SetEvent(job_empty);
        }
        EnumDesktopWindows(desktop,window,0);
    }
    return 0;
}
static int matrix(int argc,WCHAR **argv)
{
    CHECK(argc==2);WCHAR directory[MAX_PATH],image[MAX_PATH],report[MAX_PATH],system[MAX_PATH],shell[MAX_PATH],command[32768];
    CHECK(GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",directory,MAX_PATH));
    CHECK(GetSystemDirectoryW(system,MAX_PATH)&&swprintf_s(shell,MAX_PATH,L"%ls\\cmd.exe",system)>0);
    CHECK(GetFullPathNameW(argv[1],32768,launcher_path,NULL));
    CHECK(wcscpy_s(cmd_path,MAX_PATH,shell)==0&&swprintf_s(console_path,MAX_PATH,L"%ls\\conhost.exe",system)>0);
    desktop=GetThreadDesktop(GetCurrentThreadId());
    for(unsigned kind=0;kind<2;++kind){
        CHECK(swprintf_s(image,MAX_PATH,L"%ls\\reject-%u.exe",directory,kind)>0);
        BYTE bytes[256]={0};IMAGE_DOS_HEADER *dos=(void *)bytes;dos->e_magic=IMAGE_DOS_SIGNATURE;dos->e_lfanew=kind?64:0;bytes[64]='N';bytes[65]='E';
        HANDLE file=CreateFileW(image,GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL);DWORD written;
        CHECK(file!=INVALID_HANDLE_VALUE&&WriteFile(file,bytes,sizeof(bytes),&written,NULL)&&written==sizeof(bytes)&&CloseHandle(file));
        for(unsigned via_cmd=0;via_cmd<2;++via_cmd){
            CHECK(swprintf_s(report,MAX_PATH,L"%ls\\stderr-%u-%u.txt",directory,kind,via_cmd)>0);
            SECURITY_ATTRIBUTES inherit={.nLength=sizeof(inherit),.bInheritHandle=TRUE};
            HANDLE output=CreateFileW(report,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ,&inherit,CREATE_NEW,0,NULL);
            HANDLE null=CreateFileW(L"NUL",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,&inherit,OPEN_EXISTING,0,NULL);
            CHECK(output!=INVALID_HANDLE_VALUE&&null!=INVALID_HANDLE_VALUE);
            job=CreateJobObjectW(NULL,NULL);HANDLE completion=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,1);
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
            JOBOBJECT_ASSOCIATE_COMPLETION_PORT association={.CompletionKey=job,.CompletionPort=completion};
            CHECK(job&&completion&&SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits))&&
                SetInformationJobObject(job,JobObjectAssociateCompletionPortInformation,&association,sizeof(association)));
            if(via_cmd)CHECK(swprintf_s(command,32768,L"\"%ls\" /d /s /c \"\"%ls\" \"%ls\"\"",shell,argv[1],image)>0);
            else CHECK(swprintf_s(command,32768,L"\"%ls\" \"%ls\"",argv[1],image)>0);
            STARTUPINFOW startup={.cb=sizeof(startup),.dwFlags=STARTF_USESTDHANDLES,.hStdInput=null,.hStdOutput=null,.hStdError=output};
            PROCESS_INFORMATION process={0};
            CHECK(CreateProcessW(via_cmd?shell:argv[1],command,NULL,NULL,TRUE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&startup,&process));
            if(!AssignProcessToJobObject(job,process.hProcess)){TerminateProcess(process.hProcess,1);CHECK(FALSE);}
            job_port=completion;job_empty=CreateEventW(NULL,TRUE,FALSE,NULL);CHECK(job_empty);stop_event=CreateEventW(NULL,TRUE,FALSE,NULL);extra_ui=0;launchers=cmds=consoles=unknown=0;
            HANDLE observer=CreateThread(NULL,0,observe,NULL,0,NULL);CHECK(stop_event&&observer);
            CHECK(ResumeThread(process.hThread)==1);CloseHandle(process.hThread);
            CHECK(WaitForSingleObject(process.hProcess,10000)==WAIT_OBJECT_0);DWORD code;
            CHECK(GetExitCodeProcess(process.hProcess,&code));
            BOOL empty=WaitForSingleObject(job_empty,10000)==WAIT_OBJECT_0;
            CHECK(SetEvent(stop_event)&&WaitForSingleObject(observer,2000)==WAIT_OBJECT_0);CloseHandle(process.hProcess);CloseHandle(observer);CloseHandle(stop_event);
            JOBOBJECT_BASIC_ACCOUNTING_INFORMATION count;
            CHECK(empty&&QueryInformationJobObject(job,JobObjectBasicAccountingInformation,&count,sizeof(count),NULL)&&!count.ActiveProcesses);
            CloseHandle(job_empty);CloseHandle(completion);CloseHandle(job);job=NULL;CloseHandle(null);
            CHECK(SetFilePointer(output,0,NULL,FILE_BEGIN)!=INVALID_SET_FILE_POINTER);char diagnostic[8192];DWORD read;
            CHECK(ReadFile(output,diagnostic,sizeof(diagnostic)-1,&read,NULL)&&read>0&&CloseHandle(output));diagnostic[read]=0;
            const char *match=strstr(diagnostic,"native target validation failed (193)");
            printf("REJECTION kind=%s via=%s exit=%lu ui=%ld total_processes=%lu stderr193=%u\n",kind?"NE":"MZ",via_cmd?"cmd":"direct",code,extra_ui,count.TotalProcesses,match!=NULL);
            CHECK(code==4&&match&&!strstr(match+1,"native target validation failed (193)"));
            printf("OWNED COUNTS launcher=%u cmd=%u console=%u unknown=%u\n",launchers,cmds,consoles,unknown);
            CHECK(!extra_ui&&launchers==1&&cmds==via_cmd&&!unknown&&count.TotalProcesses==launchers+cmds+consoles);
            CHECK(DeleteFileW(report));
        }
        CHECK(DeleteFileW(image));
    }
    puts("LEGACY REJECTION PASS 4 cases stderr193 exit4 no_retry_ui exact_owned_processes");return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,matrix);}
