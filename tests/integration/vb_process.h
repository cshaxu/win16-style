#ifndef TEST_VB_PROCESS_H
#define TEST_VB_PROCESS_H

#include "vb_pump.h"

static void vb_process_file_path(const WCHAR *path,const char *prefix)
{
    HANDLE file=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,NULL,OPEN_EXISTING,0,NULL);
    CHECK(file!=INVALID_HANDLE_VALUE);
    WCHAR actual[32768];
    CHECK(GetFinalPathNameByHandleW(file,actual,ARRAYSIZE(actual),FILE_NAME_NORMALIZED));
    printf("%s_MODULE_FILE actual=%ls\n",prefix,actual);
    CloseHandle(file);
}

static DWORD vb_process_member(HWND window)
{
    DWORD pid=0;GetWindowThreadProcessId(window,&pid);
    HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);BOOL own=FALSE;
    if(process){IsProcessInJob(process,job,&own);CloseHandle(process);}
    return own?pid:0;
}

static void vb_process_identity(HANDLE target,DWORD pid,DWORD creator,DWORD initial,HWND form,const char *prefix,WCHAR path[MAX_PATH])
{
    DWORD size=MAX_PATH;
    CHECK(QueryFullProcessImageNameW(target,0,path,&size));
    BOOL wow=FALSE;CHECK(IsWow64Process(target,&wow) && wow);
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);CHECK(snapshot!=INVALID_HANDLE_VALUE);
    PROCESSENTRY32W entry={.dwSize=sizeof(entry)};DWORD parent=0;CHECK(Process32FirstW(snapshot,&entry));
    do{if(entry.th32ProcessID==pid){parent=entry.th32ParentProcessID;break;}}while(Process32NextW(snapshot,&entry));CloseHandle(snapshot);
    CHECK(parent==creator);
    printf("%s_TARGET pid=%lu creator=%lu initial_pid=%lu bits=32 path=%ls dpi=%u\n",prefix,pid,parent,initial,path,GetDpiForWindow(form));
}

static HWINEVENTHOOK vb_process_start(WCHAR **argv,BOOL styled,WINEVENTPROC shown,PROCESS_INFORMATION *process)
{
    WCHAR command[2048];
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limit={0};
    limit.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    job=CreateJobObjectW(NULL,NULL);
    CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limit,sizeof(limit)));
    port=CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,0,1);
    JOBOBJECT_ASSOCIATE_COMPLETION_PORT association={.CompletionKey=job,.CompletionPort=port};
    CHECK(port && SetInformationJobObject(job,JobObjectAssociateCompletionPortInformation,&association,sizeof(association)));
    ready=CreateEventW(NULL,TRUE,FALSE,NULL);
    HWINEVENTHOOK hook=SetWinEventHook(EVENT_OBJECT_SHOW,EVENT_OBJECT_SHOW,NULL,shown,0,0,WINEVENT_OUTOFCONTEXT|WINEVENT_SKIPOWNPROCESS);
    CHECK(ready && hook);
    STARTUPINFOW si={.cb=sizeof(si)};
    if(styled)CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" --wait \"%ls\"",argv[1],argv[2])>0);
    else CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\"",argv[2])>0);
    CHECK(CreateProcessW(styled?argv[1]:argv[2],command,NULL,NULL,FALSE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&si,process));
    CHECK(AssignProcessToJobObject(job,process->hProcess) && ResumeThread(process->hThread)==1 && pump(ready,12000)==WAIT_OBJECT_0);
    return hook;
}

static void vb_process_finish(HANDLE target,PROCESS_INFORMATION *process,HWINEVENTHOOK hook)
{
    CHECK(WaitForSingleObject(target,5000)==WAIT_OBJECT_0 && WaitForSingleObject(process->hProcess,5000)==WAIT_OBJECT_0);
    DWORD code;
    CHECK(GetExitCodeProcess(target,&code) && !code);
    BOOL zero=FALSE;
    ULONGLONG deadline=GetTickCount64()+3000;
    while(!zero){
        ULONGLONG now=GetTickCount64();if(now>=deadline)break;
        DWORD message;
        ULONG_PTR key;
        OVERLAPPED *overlap;
        CHECK(GetQueuedCompletionStatus(port,&message,&key,&overlap,(DWORD)(deadline-now)));
        if(key==(ULONG_PTR)job && message==JOB_OBJECT_MSG_ACTIVE_PROCESS_ZERO)zero=TRUE;
    }
    CHECK(zero);
    JOBOBJECT_BASIC_ACCOUNTING_INFORMATION active;
    CHECK(QueryInformationJobObject(job,JobObjectBasicAccountingInformation,&active,sizeof(active),NULL) && !active.ActiveProcesses);
    CloseHandle(target);
    CloseHandle(process->hProcess);
    CloseHandle(process->hThread);
    UnhookWinEvent(hook);
    CloseHandle(ready);
    CloseHandle(job);
    CloseHandle(port);
}

#endif
