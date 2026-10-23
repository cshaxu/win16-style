#include "contract_support.h"
static int standby(int argc,WCHAR **argv)
{
    CHECK(argc==7 && !wcscpy_s(contract_report,MAX_PATH,argv[2]));contract_log("entry",GetCurrentProcessId());
    CHECK(sizeof(void *)*8==wcstoul(argv[6],NULL,10));
    HANDLE ready=(HANDLE)(ULONG_PTR)_wcstoui64(argv[4],NULL,10),release=(HANDLE)(ULONG_PTR)_wcstoui64(argv[5],NULL,10);
    CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));contract_window(TRUE,argv[3]);
    contract_log("ready",GetCurrentProcessId());CHECK(SetEvent(ready));CHECK(contract_wait(release,60000)==WAIT_OBJECT_0);
    contract_window(TRUE,argv[3]);contract_log("complete",GetCurrentProcessId());return 0;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==3 && !wcscmp(argv[1],L"--entry")){
        CHECK(!wcscpy_s(contract_report,MAX_PATH,argv[2]));contract_log("unexpected-entry",GetCurrentProcessId());return 42;}
    if(argc>1 && !wcscmp(argv[1],L"--standby"))return standby(argc,argv);
    CHECK(argc==8 && !wcscmp(argv[1],L"--parent"));
    CHECK(swprintf_s(contract_report,MAX_PATH,L"%ls\\parent.txt",argv[5])>0);contract_log("entry",GetCurrentProcessId());
    CHECK(sizeof(void *)*8==wcstoul(argv[7],NULL,10));BOOL ansi=_wtoi(argv[6]);
    CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));contract_window(TRUE,argv[4]);
    HANDLE job=CreateJobObjectW(NULL,NULL),port=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,1);CHECK(job && port);
    JOBOBJECT_ASSOCIATE_COMPLETION_PORT association={(void *)0x7711,port};
    CHECK(SetInformationJobObject(job,JobObjectAssociateCompletionPortInformation,&association,sizeof(association)) && AssignProcessToJobObject(job,GetCurrentProcess()));
    SECURITY_ATTRIBUTES sa={sizeof(sa),NULL,TRUE};HANDLE ready=CreateEventW(&sa,TRUE,FALSE,NULL),release=CreateEventW(&sa,TRUE,FALSE,NULL);CHECK(ready && release);
    WCHAR sibling_marker[MAX_PATH],failed_marker[MAX_PATH],missing[MAX_PATH],command[32768];
    CHECK(swprintf_s(sibling_marker,MAX_PATH,L"%ls\\sibling.txt",argv[5])>0 && swprintf_s(failed_marker,MAX_PATH,L"%ls\\failed-entry.txt",argv[5])>0);
    STARTUPINFOW startup={sizeof(startup)};PROCESS_INFORMATION sibling={0};
    CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" --standby \"%ls\" \"%ls\" %llu %llu %u",argv[3],sibling_marker,argv[4],
        (unsigned long long)(ULONG_PTR)ready,(unsigned long long)(ULONG_PTR)release,(unsigned)(sizeof(void *)*8))>0);
    CHECK(CreateProcessW(argv[3],command,NULL,NULL,TRUE,0,NULL,NULL,&startup,&sibling) && contract_wait(ready,30000)==WAIT_OBJECT_0);
    JOBOBJECT_BASIC_ACCOUNTING_INFORMATION before,after;
    CHECK(QueryInformationJobObject(job,JobObjectBasicAccountingInformation,&before,sizeof(before),NULL) && before.ActiveProcesses>=2);
    DWORD message;ULONG_PTR key;LPOVERLAPPED data;
    while(GetQueuedCompletionStatus(port,&message,&key,&data,0)){}CHECK(GetLastError()==WAIT_TIMEOUT);
    CHECK(swprintf_s(missing,MAX_PATH,L"%ls\\ntw16s%u.dll",argv[4],sizeof(void *)==8?32:64)>0 && DeleteFileW(missing));
    CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" --entry \"%ls\"",argv[2],failed_marker)>0);
    PROCESS_INFORMATION failed;memset(&failed,0x5a,sizeof(failed));BOOL created;
    SetLastError(0);
    if(ansi){char app[4096],line[32768];STARTUPINFOA a={sizeof(a)};
        CHECK(WideCharToMultiByte(CP_ACP,0,argv[2],-1,app,sizeof(app),NULL,NULL) && WideCharToMultiByte(CP_ACP,0,command,-1,line,sizeof(line),NULL,NULL));
        created=CreateProcessA(app,line,NULL,NULL,FALSE,0,NULL,NULL,&a,&failed);
    }else created=CreateProcessW(argv[2],command,NULL,NULL,FALSE,0,NULL,NULL,&startup,&failed);
    DWORD error=GetLastError();CHECK(!created && error==ERROR_FILE_NOT_FOUND && !failed.hProcess && !failed.hThread && !failed.dwProcessId && !failed.dwThreadId);
    DWORD born=0,exited=0;ULONGLONG deadline=GetTickCount64()+5000;
    while(!born || !exited){ULONGLONG now=GetTickCount64();CHECK(now<deadline);
        CHECK(GetQueuedCompletionStatus(port,&message,&key,&data,(DWORD)(deadline-now)) && key==0x7711);
        if(message==JOB_OBJECT_MSG_NEW_PROCESS)born=(DWORD)(ULONG_PTR)data;
        if(message==JOB_OBJECT_MSG_EXIT_PROCESS || message==JOB_OBJECT_MSG_ABNORMAL_EXIT_PROCESS)exited=(DWORD)(ULONG_PTR)data;
    }
    CHECK(born==exited && born!=GetCurrentProcessId() && born!=sibling.dwProcessId);
    CHECK(QueryInformationJobObject(job,JobObjectBasicAccountingInformation,&after,sizeof(after),NULL) && after.ActiveProcesses==before.ActiveProcesses);
    CHECK(GetFileAttributesW(failed_marker)==INVALID_FILE_ATTRIBUTES);
    DWORD code;CHECK(GetExitCodeProcess(sibling.hProcess,&code) && code==STILL_ACTIVE);contract_window(TRUE,argv[4]);
    /* A failed A/W call must not leave the TLS guard disabling later launches. */
    WCHAR next_marker[MAX_PATH];PROCESS_INFORMATION next={0};HANDLE next_ready=CreateEventW(&sa,TRUE,FALSE,NULL);CHECK(next_ready);
    CHECK(swprintf_s(next_marker,MAX_PATH,L"%ls\\next.txt",argv[5])>0);
    CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" --standby \"%ls\" \"%ls\" %llu %llu %u",argv[3],next_marker,argv[4],
        (unsigned long long)(ULONG_PTR)next_ready,(unsigned long long)(ULONG_PTR)release,(unsigned)(sizeof(void *)*8))>0);
    CHECK(CreateProcessW(argv[3],command,NULL,NULL,TRUE,0,NULL,NULL,&startup,&next) && contract_wait(next_ready,30000)==WAIT_OBJECT_0);
    CHECK(SetEvent(release) && contract_wait(sibling.hProcess,30000)==WAIT_OBJECT_0 && GetExitCodeProcess(sibling.hProcess,&code) && !code);
    CHECK(contract_wait(next.hProcess,30000)==WAIT_OBJECT_0 && GetExitCodeProcess(next.hProcess,&code) && !code);
    CHECK(CloseHandle(next.hThread) && CloseHandle(next.hProcess) && CloseHandle(next_ready));
    CHECK(CloseHandle(sibling.hThread) && CloseHandle(sibling.hProcess) && CloseHandle(ready) && CloseHandle(release) && CloseHandle(port) && CloseHandle(job));
    contract_log("rollback-created",born);contract_log("rollback-exited",exited);contract_log("complete",GetCurrentProcessId());return 0;
}
