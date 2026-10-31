#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <tlhelp32.h>
#include "ntw16s-dll/global_session.h"
#include "ntw16s-dll/global_load.h"
#include "ntw16s-dll/global_start.h"
#include "ntw16s-dll/global_attach.h"

static BOOL launch(const WCHAR *image, const WCHAR *args, PROCESS_INFORMATION *p)
{
    WCHAR cmd[32768]; STARTUPINFOW si={.cb=sizeof(si)};
    return swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" %ls",image,args)>=0 &&
        CreateProcessW(image,cmd,NULL,NULL,TRUE,0,NULL,NULL,&si,p);
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==4 && !wcscmp(argv[1],L"target")){
        WCHAR started_name[160];swprintf(started_name,ARRAYSIZE(started_name),L"%ls.ready",argv[2]);
        HANDLE started=OpenEventW(EVENT_MODIFY_STATE,FALSE,started_name);
        if(!started || !SetEvent(started))return 3;
        CloseHandle(started);
        HANDLE go=OpenEventW(SYNCHRONIZE,FALSE,argv[2]);
        if(!go || WaitForSingleObject(go,12000)!=WAIT_OBJECT_0)return 3;
        CloseHandle(go);
        HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");
        WCHAR self[32768];GetModuleFileNameW(NULL,self,ARRAYSIZE(self));PROCESS_INFORMATION child={0};
        BOOL made=launch(self,L"child",&child);DWORD code=99;
        if(made){if(WaitForSingleObject(child.hProcess,5000)==WAIT_OBJECT_0)GetExitCodeProcess(child.hProcess,&code);
          else {TerminateProcess(child.hProcess,ERROR_TIMEOUT);WaitForSingleObject(child.hProcess,1000);}
          CloseHandle(child.hThread);CloseHandle(child.hProcess);}
        printf("EXISTING ROOT bits=%u dll=%d child=%d child_exit=%lu\n",(unsigned)(sizeof(void*)*8),dll!=NULL,made,code);
        WCHAR readyname[160];swprintf(readyname,ARRAYSIZE(readyname),L"%ls.ready",argv[3]);
        HANDLE ready=OpenEventW(EVENT_MODIFY_STATE,FALSE,readyname);if(ready){SetEvent(ready);CloseHandle(ready);}
        HANDLE stopped=OpenEventW(SYNCHRONIZE,FALSE,argv[3]);
        BOOL signaled=stopped && WaitForSingleObject(stopped,12000)==WAIT_OBJECT_0;
        if(stopped)CloseHandle(stopped);
        PROCESS_INFORMATION after={0};DWORD after_code=99;BOOL after_made=signaled && launch(self,L"child",&after);
        if(after_made){if(WaitForSingleObject(after.hProcess,5000)==WAIT_OBJECT_0)GetExitCodeProcess(after.hProcess,&after_code);
          else {TerminateProcess(after.hProcess,ERROR_TIMEOUT);WaitForSingleObject(after.hProcess,1000);}
          CloseHandle(after.hThread);CloseHandle(after.hProcess);}
        printf("EXISTING STOPPED child=%d child_exit=%lu\n",after_made,after_code);
        return dll && made && code==0 && after_made && after_code==41?0:1;
    }
    if(argc==2 && !wcscmp(argv[1],L"child")){
        BOOL loaded=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll")!=NULL;
        printf("EXISTING CHILD bits=%u dll=%d\n",(unsigned)(sizeof(void*)*8),loaded);return loaded?0:41;
    }
    if(argc!=3 && argc!=4)return 2;
    WCHAR desk[128],name[NTW16S_GLOBAL_NAME_CAP],stopname[NTW16S_GLOBAL_NAME_CAP+16],goname[128],aftername[128],readyname[160],started_name[160],self[32768],cmd[512];DWORD n=0;
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desk,sizeof(desk),&n)||wcsncmp(desk,L"Win16S-Dialog-",14))return 4;
    if(!ntw16s_global_name(name,ARRAYSIZE(name)))return 5;
    swprintf(stopname,ARRAYSIZE(stopname),L"%ls.request",name);
    swprintf(goname,ARRAYSIZE(goname),L"Local\\Win16S.HeadlessProbe.%lu",GetCurrentProcessId());
    swprintf(started_name,ARRAYSIZE(started_name),L"%ls.ready",goname);
    swprintf(aftername,ARRAYSIZE(aftername),L"Local\\Win16S.HeadlessAfter.%lu",GetCurrentProcessId());
    swprintf(readyname,ARRAYSIZE(readyname),L"%ls.ready",aftername);
    HANDLE go=CreateEventW(NULL,TRUE,FALSE,goname),after=CreateEventW(NULL,TRUE,FALSE,aftername),ready=CreateEventW(NULL,TRUE,FALSE,readyname),started=CreateEventW(NULL,TRUE,FALSE,started_name),map=NULL,stop=NULL;ntw16s_global_session *s=NULL;
    PROCESS_INFORMATION target={0},manager={0};ntw16s_global_attach ticket={0};ntw16s_global_policy policy;
    DWORD error=0,code=99;BOOL ok=FALSE;
    if(!go || !after || !ready || !started || !GetModuleFileNameW(NULL,self,ARRAYSIZE(self)))goto done;
    swprintf(cmd,ARRAYSIZE(cmd),L"target %ls %ls",goname,aftername);
    if(!launch(self,cmd,&target) || WaitForSingleObject(started,3000)!=WAIT_OBJECT_0 ||
       !launch(argv[1],L"--global",&manager))goto done;
    printf("EXISTING before_global_ready=1 pid=%lu\n",target.dwProcessId);
    for(unsigned i=0;i<200;i++){
        map=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
        if(map){s=MapViewOfFile(map,FILE_MAP_READ,0,0,sizeof(*s));if(s && s->owner_pid==manager.dwProcessId && s->phase==NTW16S_GLOBAL_ACTIVE && s->ready)break;
          if(s)UnmapViewOfFile(s);
          s=NULL;CloseHandle(map);map=NULL;}
        Sleep(30);
    }
    if(!s)goto done;
    if(argc==4){
        BOOL loaded=FALSE;ULONGLONG deadline=GetTickCount64()+5000;
        while(!loaded && GetTickCount64()<deadline){
            HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,target.dwProcessId);
            MODULEENTRY32W entry={.dwSize=sizeof(entry)};
            if(snap!=INVALID_HANDLE_VALUE){if(Module32FirstW(snap,&entry))do{
                if(!lstrcmpiW(entry.szModule,sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll")){loaded=TRUE;break;}
            }while(Module32NextW(snap,&entry));CloseHandle(snap);}
            if(!loaded)Sleep(20);
        }
        printf("EXISTING auto_load=%d\n",loaded);if(!loaded)goto done;
        Sleep(250);
    }else{
    error=ntw16s_global_policy_init(&policy);
    if(!error)error=ntw16s_global_attach_begin_thread(&policy,target.dwProcessId,target.dwThreadId,
        sizeof(void*)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386,s->cookie,argv[2],&ticket);
    if(!error && WaitForSingleObject(ticket.load.thread,5000)==WAIT_OBJECT_0){
        error=ntw16s_global_attach_advance(&ticket,NULL);
        if(error==ERROR_IO_PENDING && ticket.start.ready){HANDLE waits[]={ticket.start.ready,ticket.start.thread};
            if(WaitForMultipleObjects(ARRAYSIZE(waits),waits,FALSE,5000)<WAIT_OBJECT_0+ARRAYSIZE(waits))error=ntw16s_global_attach_advance(&ticket,NULL);}
    }
    printf("EXISTING attach=%lu\n",error);
    if(error)goto done;
    }
    SetEvent(go);
    if(WaitForSingleObject(ready,6000)!=WAIT_OBJECT_0)goto done;
    stop=OpenEventW(EVENT_MODIFY_STATE,FALSE,stopname);
    if(!stop || !SetEvent(stop) || WaitForSingleObject(manager.hProcess,6000)!=WAIT_OBJECT_0)goto done;
    SetEvent(after);
    if(WaitForSingleObject(target.hProcess,6000)==WAIT_OBJECT_0 && GetExitCodeProcess(target.hProcess,&code))ok=code==0;
    printf("EXISTING RESULT target_exit=%lu\n",code);
done:
    if(manager.hProcess && WaitForSingleObject(manager.hProcess,0)==WAIT_TIMEOUT){stop=OpenEventW(EVENT_MODIFY_STATE,FALSE,stopname);if(stop)SetEvent(stop);if(WaitForSingleObject(manager.hProcess,6000)!=WAIT_OBJECT_0){TerminateProcess(manager.hProcess,ERROR_TIMEOUT);WaitForSingleObject(manager.hProcess,1000);}}
    if(target.hProcess && WaitForSingleObject(target.hProcess,0)==WAIT_TIMEOUT){TerminateProcess(target.hProcess,ERROR_TIMEOUT);WaitForSingleObject(target.hProcess,1000);}
    (void)ntw16s_global_attach_close(&ticket);
    if(s)UnmapViewOfFile(s);
    if(map)CloseHandle(map);
    if(stop)CloseHandle(stop);
    if(go)CloseHandle(go);
    if(after)CloseHandle(after);
    if(ready)CloseHandle(ready);
    if(started)CloseHandle(started);
    if(target.hThread)CloseHandle(target.hThread);
    if(target.hProcess)CloseHandle(target.hProcess);
    if(manager.hThread)CloseHandle(manager.hThread);
    if(manager.hProcess)CloseHandle(manager.hProcess);
    return ok?0:1;
}
