#include <windows.h>
#include <shellapi.h>
#include <objbase.h>
#include <stdio.h>
#include <wchar.h>
#include "../ntw16s-dll/isolated_process.h"
static HANDLE job,stop;
static DWORD observed_pid;
static HDESK desktop;
static LONG seen;
static WCHAR actual_class[80],actual_title[256];
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SYSTEM SHELL FAIL line=%d error=%lu\n",__LINE__,GetLastError());if(job)CloseHandle(job);return 1;}}while(0)
static BOOL CALLBACK dialog(HWND w,LPARAM unused)
{
    (void)unused;DWORD pid;GetWindowThreadProcessId(w,&pid);
    if(pid!=observed_pid||seen||!IsWindowVisible(w))return TRUE;
    WCHAR cls[80];GetClassNameW(w,cls,80);
    if(wcscmp(cls,L"Shell_Dialog")&&wcscmp(cls,L"#32770"))return TRUE;
    wcscpy_s(actual_class,80,cls);GetWindowTextW(w,actual_title,256);
    InterlockedIncrement(&seen);PostMessageW(w,WM_CLOSE,0,0);return TRUE;
}
static DWORD WINAPI observe(void *unused)
{
    (void)unused;
    while(WaitForSingleObject(stop,30)==WAIT_TIMEOUT)EnumDesktopWindows(desktop,dialog,0);
    return 0;
}
static int matrix(int argc,WCHAR **argv)
{
    CHECK(argc==2);WCHAR directory[MAX_PATH],path[MAX_PATH],command[32768];
    CHECK(GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",directory,MAX_PATH));
    desktop=GetThreadDesktop(GetCurrentThreadId());HRESULT apartment=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE);CHECK(SUCCEEDED(apartment));
    for(unsigned kind=0;kind<2;++kind){
        CHECK(swprintf_s(path,MAX_PATH,L"%ls\\legacy-%u.exe",directory,kind)>0);
        BYTE bytes[256]={0};IMAGE_DOS_HEADER *dos=(void *)bytes;dos->e_magic=IMAGE_DOS_SIGNATURE;dos->e_lfanew=kind?64:0;
        bytes[64]='N';bytes[65]='E';HANDLE f=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL);DWORD wrote;
        CHECK(f!=INVALID_HANDLE_VALUE&&WriteFile(f,bytes,sizeof(bytes),&wrote,NULL)&&CloseHandle(f));
        WCHAR expected_class[80],expected_title[256];
        for(unsigned via_cli=0;via_cli<2;++via_cli){
            seen=0;actual_class[0]=actual_title[0]=0;stop=CreateEventW(NULL,TRUE,FALSE,NULL);CHECK(stop);
            PROCESS_INFORMATION process={0};HANDLE completion=NULL;
            if(via_cli){
                job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
                CHECK(job&&SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits)));
                completion=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,1);CHECK(completion);
                JOBOBJECT_ASSOCIATE_COMPLETION_PORT associate={.CompletionKey=job,.CompletionPort=completion};
                CHECK(SetInformationJobObject(job,JobObjectAssociateCompletionPortInformation,&associate,sizeof(associate)));
                CHECK(swprintf_s(command,32768,L"\"%ls\" \"%ls\"",argv[1],path)>0);
                STARTUPINFOW startup={.cb=sizeof(startup)};
                CHECK(CreateProcessW(argv[1],command,NULL,NULL,FALSE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&startup,&process));
                if(!AssignProcessToJobObject(job,process.hProcess)){TerminateProcess(process.hProcess,1);CHECK(FALSE);}
                observed_pid=process.dwProcessId;
            }else observed_pid=GetCurrentProcessId();
            HANDLE observer=CreateThread(NULL,0,observe,NULL,0,NULL);CHECK(observer);
            DWORD error;
            if(via_cli){
                CHECK(ResumeThread(process.hThread)==1);CloseHandle(process.hThread);
                CHECK(WaitForSingleObject(process.hProcess,15000)==WAIT_OBJECT_0&&GetExitCodeProcess(process.hProcess,&error));CloseHandle(process.hProcess);
            }else{
                SHELLEXECUTEINFOW info={.cbSize=sizeof(info),.fMask=SEE_MASK_NOASYNC|SEE_MASK_NOCLOSEPROCESS,.lpFile=path,.nShow=SW_SHOWNORMAL};
                BOOL ok=ShellExecuteExW(&info);error=GetLastError();CHECK(!ok&&!info.hProcess);
            }
            CHECK(SetEvent(stop)&&WaitForSingleObject(observer,2000)==WAIT_OBJECT_0);CloseHandle(observer);CloseHandle(stop);
            if(!seen){fprintf(stderr,"SYSTEM SHELL MISSING kind=%u via_cli=%u exit=%lu\n",kind,via_cli,error);CHECK(seen);}
            CHECK(actual_title[0]);
            if(via_cli){CHECK(error==4&&!wcscmp(expected_class,actual_class)&&!wcscmp(expected_title,actual_title));
                /* Process exit can be signaled before Job accounting is updated. Join
                 * the kernel's whole-job completion event, then require zero. */
                ULONGLONG deadline=GetTickCount64()+10000;BOOL empty=FALSE;
                while(!empty){ULONGLONG now=GetTickCount64();if(now>=deadline)break;
                    DWORD message;ULONG_PTR key;LPOVERLAPPED value;
                    CHECK(GetQueuedCompletionStatus(completion,&message,&key,&value,(DWORD)(deadline-now)));
                    CHECK(key==(ULONG_PTR)job);empty=message==JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO;}
                JOBOBJECT_BASIC_ACCOUNTING_INFORMATION count;
                CHECK(empty&&QueryInformationJobObject(job,JobObjectBasicAccountingInformation,&count,sizeof(count),NULL)&&!count.ActiveProcesses);
                CloseHandle(completion);CloseHandle(job);job=NULL;
            }else{CHECK(error==ERROR_CANCELLED);wcscpy_s(expected_class,80,actual_class);wcscpy_s(expected_title,256,actual_title);}
            wprintf(L"SYSTEM ERROR kind=%ls via=%ls class=%ls title=%ls exit=%lu\n",kind?L"NE":L"MZ",via_cli?L"run16s":L"Windows Shell",actual_class,actual_title,error);
        }
        CHECK(DeleteFileW(path));
    }
    CoUninitialize();puts("SYSTEM SHELL PASS 2 formats direct-vs-launcher native UI matched exit4 owned_jobs_empty");return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,matrix);}
