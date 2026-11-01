#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <sddl.h>
#include <tlhelp32.h>
#include "ntw16s-dll/global_session.h"

static BOOL launch(const WCHAR *image,WCHAR *command,WCHAR *desktop,PROCESS_INFORMATION *p)
{STARTUPINFOW si={.cb=sizeof(si),.lpDesktop=desktop};return CreateProcessW(image,command,NULL,NULL,TRUE,0,NULL,NULL,&si,p);}
static void timeout_diagnostic(DWORD pid,HANDLE process)
{
    WCHAR path[32768];DWORD length=ARRAYSIZE(path),exitcode=0;unsigned threads=0;BOOL module=FALSE;
    BOOL image=QueryFullProcessImageNameW(process,0,path,&length);(void)GetExitCodeProcess(process,&exitcode);
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);THREADENTRY32 thread={.dwSize=sizeof(thread)};
    if(snapshot!=INVALID_HANDLE_VALUE){if(Thread32First(snapshot,&thread))do{if(thread.th32OwnerProcessID==pid)++threads;}while(Thread32Next(snapshot,&thread));CloseHandle(snapshot);}
    snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);MODULEENTRY32W entry={.dwSize=sizeof(entry)};
    if(snapshot!=INVALID_HANDLE_VALUE){if(Module32FirstW(snapshot,&entry))do{if(!lstrcmpiW(entry.szModule,L"ntw16s32.dll") || !lstrcmpiW(entry.szModule,L"ntw16s64.dll"))module=TRUE;}while(Module32NextW(snapshot,&entry));CloseHandle(snapshot);}
    wprintf(L"CROSS TARGET DIAG pid=%lu exit=%lu threads=%u module=%d image=%ls\n",pid,exitcode,threads,module,image?path:L"unavailable");
}
static BOOL launch_low(const WCHAR *image,WCHAR *command,WCHAR *desktop,PROCESS_INFORMATION *p)
{
    HANDLE token=NULL,lower=NULL;PSID sid=NULL;DWORD error=ERROR_SUCCESS;BOOL made=FALSE;
    STARTUPINFOW si={.cb=sizeof(si),.lpDesktop=desktop,.dwFlags=STARTF_USESTDHANDLES};
    si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);si.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);si.hStdError=GetStdHandle(STD_ERROR_HANDLE);
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY|TOKEN_DUPLICATE|TOKEN_ADJUST_DEFAULT|TOKEN_ASSIGN_PRIMARY,&token) ||
       !CreateRestrictedToken(token,DISABLE_MAX_PRIVILEGE,0,NULL,0,NULL,0,NULL,&lower) ||
       !ConvertStringSidToSidW(L"S-1-16-4096",&sid)){error=GetLastError();goto done;}
    TOKEN_MANDATORY_LABEL label={0};label.Label.Attributes=SE_GROUP_INTEGRITY;label.Label.Sid=sid;
    if(!SetTokenInformation(lower,TokenIntegrityLevel,&label,sizeof(label)+GetLengthSid(sid))){error=GetLastError();goto done;}
    made=CreateProcessAsUserW(lower,image,command,NULL,NULL,TRUE,0,NULL,NULL,&si,p);
    if(!made)error=GetLastError();
done:
    if(sid)LocalFree(sid);
    if(lower)CloseHandle(lower);
    if(token)CloseHandle(token);
    printf("LOW CREATE made=%d error=%lu\n",made,error);SetLastError(error);return made;
}
/* Called only by the optional elevated compatibility test. The linked token
 * represents this same user's ordinary, filtered logon, not another user. */
static BOOL launch_medium(const WCHAR *image,WCHAR *command,WCHAR *desktop,DWORD source_pid,PROCESS_INFORMATION *p)
{
    HANDLE source=NULL,token=NULL,primary=NULL;DWORD bytes=0,error=ERROR_SUCCESS;BOOL made=FALSE;TOKEN_ELEVATION elevation={0};
    STARTUPINFOW si={.cb=sizeof(si),.lpDesktop=desktop,.dwFlags=STARTF_USESTDHANDLES};
    si.hStdInput=GetStdHandle(STD_INPUT_HANDLE);si.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);si.hStdError=GetStdHandle(STD_ERROR_HANDLE);
    source=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,source_pid);
    if(!source || !OpenProcessToken(source,TOKEN_QUERY|TOKEN_DUPLICATE,&token)){error=GetLastError();printf("MEDIUM TOKEN stage=open error=%lu\n",error);goto done;}
    if(!GetTokenInformation(token,TokenElevation,&elevation,sizeof(elevation),&bytes) || elevation.TokenIsElevated){error=ERROR_INVALID_TOKEN;printf("MEDIUM TOKEN stage=elevation error=%lu\n",error);goto done;}
    if(!DuplicateTokenEx(token,TOKEN_ALL_ACCESS,NULL,SecurityImpersonation,TokenPrimary,&primary)){error=GetLastError();printf("MEDIUM TOKEN stage=duplicate error=%lu\n",error);goto done;}
    made=CreateProcessWithTokenW(primary,0,image,command,0,NULL,NULL,&si,p);
    if(!made){error=GetLastError();printf("MEDIUM TOKEN stage=create error=%lu\n",error);}
done:
    if(primary)CloseHandle(primary);
    if(token)CloseHandle(token);
    if(source)CloseHandle(source);
    printf("MEDIUM CREATE made=%d error=%lu\n",made,error);SetLastError(error);return made;
}
int wmain(int argc,WCHAR **argv)
{
    if((argc==6 || argc==7 || argc==8) && !wcscmp(argv[1],L"broker")){
        HANDLE go=OpenEventW(SYNCHRONIZE,FALSE,argv[3]);if(!go)return 2;
        if(WaitForSingleObject(go,10000)!=WAIT_OBJECT_0){CloseHandle(go);return 3;}CloseHandle(go);
        BOOL loaded=GetModuleHandleW(L"ntw16s64.dll")!=NULL;WCHAR cmd[32768];PROCESS_INFORMATION target={0};
        BOOL low=argc==7 && !wcscmp(argv[6],L"low"),medium=argc==8 && !wcscmp(argv[6],L"medium");
        if(argc>6 && !low && !medium)return 4;
        DWORD source_pid=medium?wcstoul(argv[7],NULL,10):0;
        BOOL native_control=!wcscmp(argv[5],L"native-control");
        const WCHAR *target_mode=native_control?L"headless-child":argv[5];
        if(swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" %ls",argv[4],target_mode)<0 ||
           !(low?launch_low(argv[4],cmd,argv[2],&target):medium?launch_medium(argv[4],cmd,argv[2],source_pid,&target):launch(argv[4],cmd,argv[2],&target)))return 4;
        DWORD code=99,wait=WaitForSingleObject(target.hProcess,8000);
        if(wait==WAIT_OBJECT_0)GetExitCodeProcess(target.hProcess,&code);
        else{printf("CROSS TARGET timeout pid=%lu wait=%lu error=%lu\n",target.dwProcessId,wait,GetLastError());timeout_diagnostic(target.dwProcessId,target.hProcess);TerminateProcess(target.hProcess,ERROR_TIMEOUT);WaitForSingleObject(target.hProcess,1000);}
        CloseHandle(target.hThread);CloseHandle(target.hProcess);
        if(native_control){printf("CROSS NATIVE loaded=%d target_exit=%lu\n",loaded,code);return !loaded && (code==0 || code==41)?0:1;}
        printf("CROSS BROKER loaded=%d target_exit=%lu\n",loaded,code);return !loaded && !code?0:1;
    }
    if(argc!=4 && !(argc==5 && !wcscmp(argv[4],L"--low")) && !(argc==6 && !wcscmp(argv[4],L"--medium")))return 5;
    WCHAR desktop[128],other[128],event_name[128],name[NTW16S_GLOBAL_NAME_CAP],request[NTW16S_GLOBAL_NAME_CAP+16],self[32768],cmd[32768];DWORD needed=0;
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&needed) ||
       wcsncmp(desktop,L"Win16S-Dialog-",14) || !ntw16s_global_name(name,ARRAYSIZE(name)))return 6;
    swprintf(other,ARRAYSIZE(other),L"Win16S-Origin-%lu",GetCurrentProcessId());
    swprintf(event_name,ARRAYSIZE(event_name),L"Local\\Win16S-Origin-Go-%lu",GetCurrentProcessId());
    swprintf(request,ARRAYSIZE(request),L"%ls.request",name);
    HDESK sibling=CreateDesktopW(other,NULL,NULL,0,GENERIC_ALL,NULL);
    HANDLE go=CreateEventW(NULL,TRUE,FALSE,event_name),mapping=NULL,stop=NULL;ntw16s_global_session *session=NULL;
    PROCESS_INFORMATION broker={0},manager={0};BOOL pass=FALSE;DWORD code=99;
    if(!sibling || !go || !GetModuleFileNameW(NULL,self,ARRAYSIZE(self)))goto done;
    WCHAR token[64]=L"";
    if(argc==5)wcscpy(token,L" low");
    if(argc==6 && swprintf(token,ARRAYSIZE(token),L" medium %ls",argv[5])<0)goto done;
    swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" broker \"%ls\" %ls \"%ls\" %ls%ls",self,desktop,event_name,argv[2],argv[3],token);
    if(!launch(self,cmd,other,&broker))goto done;
    swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" --global",argv[1]);if(!launch(argv[1],cmd,NULL,&manager))goto done;
    for(unsigned i=0;i<250;i++){
        if(!mapping)mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
        if(mapping && !session)session=MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*session));
        if(session && session->owner_pid==manager.dwProcessId && session->phase==NTW16S_GLOBAL_ACTIVE && session->ready)break;
        if(WaitForSingleObject(manager.hProcess,0)!=WAIT_TIMEOUT)break;
        Sleep(20);
    }
    if(!session || session->owner_pid!=manager.dwProcessId || !session->ready || !SetEvent(go))goto done;
    if(WaitForSingleObject(broker.hProcess,10000)==WAIT_OBJECT_0 && GetExitCodeProcess(broker.hProcess,&code) && !code)pass=TRUE;
    printf("CROSS RESULT mode=%ls broker_exit=%lu pass=%d\n",argv[3],code,pass);
done:
    if(manager.hProcess && WaitForSingleObject(manager.hProcess,0)==WAIT_TIMEOUT){stop=OpenEventW(EVENT_MODIFY_STATE,FALSE,request);if(stop)SetEvent(stop);if(WaitForSingleObject(manager.hProcess,6000)!=WAIT_OBJECT_0){TerminateProcess(manager.hProcess,ERROR_TIMEOUT);WaitForSingleObject(manager.hProcess,1000);}}
    if(broker.hProcess && WaitForSingleObject(broker.hProcess,0)==WAIT_TIMEOUT){TerminateProcess(broker.hProcess,ERROR_TIMEOUT);WaitForSingleObject(broker.hProcess,1000);}
    if(session)UnmapViewOfFile(session);
    if(mapping)CloseHandle(mapping);
    if(stop)CloseHandle(stop);
    if(go)CloseHandle(go);
    if(broker.hThread)CloseHandle(broker.hThread);
    if(broker.hProcess)CloseHandle(broker.hProcess);
    if(manager.hThread)CloseHandle(manager.hThread);
    if(manager.hProcess)CloseHandle(manager.hProcess);
    if(sibling)CloseDesktop(sibling);
    return pass?0:1;
}
