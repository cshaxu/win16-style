#include <windows.h>
#include <shellapi.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#define REQUIRE(x) do{if(!(x)){fwprintf(stderr,L"UAC DRIVER FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static BOOL elevated(void)
{HANDLE token;TOKEN_ELEVATION e={0};DWORD n=0;if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token))return FALSE;BOOL ok=GetTokenInformation(token,TokenElevation,&e,sizeof(e),&n);CloseHandle(token);return ok&&e.TokenIsElevated;}
int wmain(int argc,WCHAR **argv)
{
    if(argc==3&&!wcscmp(argv[1],L"--baseline")){
        STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION p={0};WCHAR command[32768];
        swprintf(command,32768,L"\"%ls\"",argv[2]);
        BOOL ok=CreateProcessW(argv[2],command,NULL,NULL,FALSE,CREATE_SUSPENDED,NULL,NULL,&si,&p);DWORD error=ok?0:GetLastError();
        if(ok){TerminateProcess(p.hProcess,0);WaitForSingleObject(p.hProcess,3000);CloseHandle(p.hThread);CloseHandle(p.hProcess);}
        wprintf(L"UAC BASELINE elevated=%d created=%d error=%lu\n",elevated(),ok,error);
        return !elevated()&&error==ERROR_ELEVATION_REQUIRED?0:77;
    }
    if(argc==8&&!wcscmp(argv[1],L"--batch")){
        REQUIRE(elevated());WCHAR self[32768];REQUIRE(GetModuleFileNameW(NULL,self,32768));
        for(int i=0;i<6;i++){
            WCHAR command[32768],report[32768];swprintf(report,32768,L"%ls\\case-%d.txt",argv[7],i);
            PCWSTR mode=i<4?L"--once":L"--gui-async";PCWSTR target=argv[3+(i%4)];
            if(i>=4)target=argv[5+(i%2)];
            REQUIRE(swprintf(command,32768,L"\"%ls\" %ls \"%ls\" \"%ls\" \"%ls\" batch-%d",self,mode,argv[2],target,report,i)>0);
            WCHAR log[32768];swprintf(log,32768,L"%ls.driver.log",report);
            SECURITY_ATTRIBUTES sa={.nLength=sizeof(sa),.bInheritHandle=TRUE};
            HANDLE output=CreateFileW(log,GENERIC_WRITE,FILE_SHARE_READ,&sa,CREATE_NEW,0,NULL);REQUIRE(output!=INVALID_HANDLE_VALUE);
            STARTUPINFOW si={.cb=sizeof(si),.dwFlags=STARTF_USESTDHANDLES,.hStdOutput=output,.hStdError=output};PROCESS_INFORMATION p={0};
            BOOL started=CreateProcessW(self,command,NULL,NULL,TRUE,CREATE_NO_WINDOW,NULL,NULL,&si,&p);CloseHandle(output);REQUIRE(started);CloseHandle(p.hThread);
            DWORD waited=WaitForSingleObject(p.hProcess,15000),code=STILL_ACTIVE;GetExitCodeProcess(p.hProcess,&code);
            if(waited!=WAIT_OBJECT_0){TerminateProcess(p.hProcess,ERROR_TIMEOUT);WaitForSingleObject(p.hProcess,3000);}
            CloseHandle(p.hProcess);REQUIRE(waited==WAIT_OBJECT_0&&code==0);
        }
        WCHAR marker[32768];swprintf(marker,32768,L"%ls\\batch-pass.txt",argv[7]);FILE *file=_wfopen(marker,L"wb");REQUIRE(file);fputs("UAC HIGH BATCH PASS cases=6\n",file);fclose(file);return 0;
    }
    REQUIRE(argc==6&&(!wcscmp(argv[1],L"--once")||!wcscmp(argv[1],L"--gui-async")));
    BOOL async=!wcscmp(argv[1],L"--gui-async");
    REQUIRE(GetFileAttributesW(argv[4])==INVALID_FILE_ATTRIBUTES);
    SetEnvironmentVariableW(L"WIN16STYLE_UAC_MARKER",elevated()?L"BATCH_HIGH":L"MEDIUM_NOT_FORWARDED");
    /* --once launcher target report nonce. The only user operation is the
     * genuine OS consent/credentials dialog; all application checks are native. */
    WCHAR command[32768];REQUIRE(swprintf(command,32768,L"\"%ls\" %ls\"%ls\" \"%ls\" \"%ls\" \"\" \"a b\" tail\\",argv[2],async?L"":L"--wait ",argv[3],argv[4],argv[5])>0);
    STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION p={0};
    REQUIRE(CreateProcessW(argv[2],command,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&p));CloseHandle(p.hThread);
    DWORD waited=WaitForSingleObject(p.hProcess,90000),code=STILL_ACTIVE;
    if(waited!=WAIT_OBJECT_0){TerminateProcess(p.hProcess,ERROR_TIMEOUT);WaitForSingleObject(p.hProcess,3000);}
    REQUIRE(GetExitCodeProcess(p.hProcess,&code));CloseHandle(p.hProcess);
    wprintf(L"UAC WRAPPER wait=%lu exit=%lu\n",waited,code);fflush(stdout);
    if(code==ERROR_CANCELLED){puts("UAC CANCEL PASS error=1223");return 77;}
    REQUIRE(waited==WAIT_OBJECT_0&&code==(async?0:73));
    WCHAR done[32768];swprintf(done,32768,L"%ls.done",argv[4]);
    if(async){for(unsigned i=0;i<400&&GetFileAttributesW(done)==INVALID_FILE_ATTRIBUTES;i++)Sleep(20);}
    FILE *completed=_wfopen(done,L"rb");REQUIRE(completed);char finished[8]={0};fread(finished,1,5,completed);fclose(completed);REQUIRE(!strcmp(finished,"PASS\n"));
    FILE *file=_wfopen(argv[4],L"r, ccs=UTF-8");REQUIRE(file);
    WCHAR line[32768],cwd[32768];GetCurrentDirectoryW(32768,cwd);
    BOOL marker=FALSE,nonce=FALSE,empty=FALSE,spaced=FALSE,slash=FALSE,token=FALSE,il=FALSE,working=FALSE,environment=FALSE,dll=FALSE;
    WCHAR package[32768];REQUIRE(GetFullPathNameW(argv[2],32768,package,NULL));WCHAR *separator=wcsrchr(package,L'\\');REQUIRE(separator);*separator=0;
    while(fgetws(line,32768,file)){
        if(!wcscmp(line,L"UAC TARGET PASS\n"))marker=TRUE;
        if(!wcsncmp(line,L"nonce=",6)){size_t n=wcslen(line);if(n&&line[n-1]==L'\n')line[n-1]=0;nonce=!wcscmp(line+6,argv[5]);}
        if(!wcscmp(line,L"arg0=\n"))empty=TRUE;
        if(!wcscmp(line,L"arg1=a b\n"))spaced=TRUE;
        if(!wcscmp(line,L"arg2=tail\\\n"))slash=TRUE;
        if(!wcscmp(line,L"elevated=1\n"))token=TRUE;
        if(!wcsncmp(line,L"integrity=",10))il=wcstoul(line+10,NULL,10)>=SECURITY_MANDATORY_HIGH_RID;
        if(!wcsncmp(line,L"cwd=",4)){size_t n=wcslen(line);if(n&&line[n-1]==L'\n')line[n-1]=0;working=!_wcsicmp(line+4,cwd);}
        if(!wcsncmp(line,L"environment_marker=",19))environment=!wcscmp(line+19,elevated()?L"BATCH_HIGH\n":L"\n");
        if(!wcsncmp(line,L"dll=",4)){
            size_t n=wcslen(line);if(n&&line[n-1]==L'\n')line[n-1]=0;
            WCHAR expected32[32768],expected64[32768];swprintf(expected32,32768,L"%ls\\ntw16s32.dll",package);swprintf(expected64,32768,L"%ls\\ntw16s64.dll",package);
            dll=!_wcsicmp(line+4,expected32)||!_wcsicmp(line+4,expected64);
        }
    }fclose(file);REQUIRE(marker&&nonce&&empty&&spaced&&slash);
    REQUIRE(token&&il&&working&&environment&&dll);
    WCHAR capture[32768];swprintf(capture,32768,L"%ls.bmp",argv[4]);REQUIRE(GetFileAttributesW(capture)!=INVALID_FILE_ATTRIBUTES);
    puts("UAC PACKAGE PASS elevated=verified attached=verified args=verified cwd=verified environment=verified module_path=verified");return 0;
}
