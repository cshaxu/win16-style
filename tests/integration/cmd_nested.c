#define _WIN32_WINNT 0x0A00
#include "cmd_support.h"
#include <rpc.h>
#define NTW16S_TEST_PROCESS_TIMEOUT 300000
#include "../ntw16s-dll/isolated_process.h"

typedef struct output {char bytes[131072];SIZE_T used;HANDLE pipe;FILE *transcript;} output;
static BOOL send_line(HANDLE input,const char *line)
{DWORD written;return WriteFile(input,line,(DWORD)strlen(line),&written,NULL)&&written==strlen(line);}
static BOOL pump(output *out)
{
    DWORD available,got;
    if(!PeekNamedPipe(out->pipe,NULL,0,NULL,&available,NULL))return GetLastError()==ERROR_BROKEN_PIPE;
    while(available){char buffer[4096];DWORD want=available<sizeof(buffer)?available:sizeof(buffer);
        if(!ReadFile(out->pipe,buffer,want,&got,NULL)||!got)return FALSE;
        fwrite(buffer,1,got,out->transcript);fflush(out->transcript);
        if(got>=sizeof(out->bytes)-out->used)return FALSE;
        memcpy(out->bytes+out->used,buffer,got);out->used+=got;out->bytes[out->used]=0;
        if(!PeekNamedPipe(out->pipe,NULL,0,NULL,&available,NULL))break;
    }
    return TRUE;
}
static BOOL wait_text(output *out,const char *marker,DWORD milliseconds,cmd_inventory *inventory)
{
    ULONGLONG deadline=GetTickCount64()+milliseconds;
    do{if(!pump(out)||!cmd_snapshot(inventory))return FALSE;
        if(strstr(out->bytes,marker))return TRUE;
        HANDLE timer=CreateWaitableTimerW(NULL,TRUE,NULL);if(!timer)return FALSE;
        LARGE_INTEGER due={.QuadPart=-200000};BOOL ok=SetWaitableTimer(timer,&due,0,NULL,NULL,FALSE);
        if(ok)ok=WaitForSingleObject(timer,50)==WAIT_OBJECT_0;
        CloseHandle(timer);if(!ok)return FALSE;
    }while(GetTickCount64()<deadline);
    return FALSE;
}
static BOOL write_wide(HANDLE pipe,PCWSTR text)
{
    char line[8192];BOOL replaced=FALSE;
    int n=WideCharToMultiByte(CP_ACP,WC_NO_BEST_FIT_CHARS,text,-1,line,sizeof(line),NULL,&replaced);
    return n>0&&!replaced&&send_line(pipe,line);
}
static cmd_process *cmd_find(cmd_inventory *inventory,DWORD parent,DWORD excluded)
{
    for(unsigned i=0;i<inventory->count;++i){cmd_process *p=&inventory->processes[i];
        if(p->is_cmd&&p->pid!=excluded&&(!parent||p->parent==parent))return p;
    }
    return NULL;
}
static BOOL CALLBACK dialog_text(HWND window,LPARAM data)
{
    FILE *report=(FILE *)data;WCHAR cls[80],text[1024]={0};DWORD_PTR ignored;
    GetClassNameW(window,cls,ARRAYSIZE(cls));
    SendMessageTimeoutW(window,WM_GETTEXT,ARRAYSIZE(text),(LPARAM)text,SMTO_ABORTIFHUNG,200,&ignored);
    if(text[0])fprintf(report,"OWNED DIALOG CONTROL class=%ls text=%ls\n",cls,text);
    return TRUE;
}
static BOOL CALLBACK failed_dialog(HWND window,LPARAM data)
{
    cmd_inventory *inventory=(cmd_inventory *)data;WCHAR cls[80],title[256]={0};DWORD pid;
    GetClassNameW(window,cls,ARRAYSIZE(cls));if(wcscmp(cls,L"#32770"))return TRUE;
    GetWindowThreadProcessId(window,&pid);BOOL owned=FALSE;
    for(unsigned i=0;i<inventory->count;++i)if(inventory->processes[i].pid==pid)owned=TRUE;
    if(!owned)return TRUE;
    GetWindowTextW(window,title,ARRAYSIZE(title));fprintf(inventory->report,"OWNED DIALOG after test failure pid=%lu title=%ls\n",pid,title);
    EnumChildWindows(window,dialog_text,(LPARAM)inventory->report);
    /* Only release an owned failure dialog after the case has already failed,
     * to obtain cmd's ERRORLEVEL diagnostic. Never turn this into a pass. */
    if(GetDlgItem(window,IDOK))PostMessageW(window,WM_COMMAND,IDOK,0);
    else PostMessageW(window,WM_CLOSE,0,0);
    return TRUE;
}
static int run_case(PCWSTR package,PCWSTR scratch,PCWSTR backend,unsigned mode,unsigned root_bits,unsigned child_bits)
{
    int result=1;HANDLE in_read=NULL,in_write=NULL,out_read=NULL,out_write=NULL,job=NULL;
    PROCESS_INFORMATION root={0};FILE *report=NULL,*transcript=NULL;cmd_inventory *inventory=NULL;
    output *out=NULL;LPPROC_THREAD_ATTRIBUTE_LIST attributes=NULL;
    WCHAR windows[512],root_cmd[1024],child_cmd[1024],launcher[1024],command[4096],folder[1024]={0},path[1200];
    char id[100],outer_ready[140],inner_ready[140],returned[160],variable[140],line[1024];
    SECURITY_ATTRIBUTES security={sizeof(security),NULL,TRUE};STARTUPINFOEXW startup={0};SIZE_T bytes=0;
    cmd_process *outer=NULL,*inner=NULL;DWORD exit_code=999;BOOL attributes_ready=FALSE,assigned=FALSE;
#define REQUIRE(x) do{if(!(x)){if(report)fprintf(report,"CMD FAIL line=%d win32=%lu expression=%s\n",__LINE__,GetLastError(),#x);goto done;}}while(0)
    REQUIRE(GetWindowsDirectoryW(windows,ARRAYSIZE(windows)));
    REQUIRE(swprintf_s(root_cmd,ARRAYSIZE(root_cmd),L"%ls\\%ls\\cmd.exe",windows,root_bits==64?L"System32":L"SysWOW64")>0);
    REQUIRE(swprintf_s(child_cmd,ARRAYSIZE(child_cmd),L"%ls\\%ls\\cmd.exe",windows,
        child_bits==32?L"SysWOW64":root_bits==32?L"Sysnative":L"System32")>0);
    REQUIRE(cmd_machine(root_cmd)==(root_bits==64?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386));
    REQUIRE(swprintf_s(folder,ARRAYSIZE(folder),L"%ls\\cmd-m%u-r%u-c%u-%lu-%llu",scratch,mode,root_bits,child_bits,GetCurrentProcessId(),GetTickCount64())>0);
    REQUIRE(CreateDirectoryW(folder,NULL));
    REQUIRE(swprintf_s(path,ARRAYSIZE(path),L"%ls\\case.log",folder)>0&&!_wfopen_s(&report,path,L"wt"));
    REQUIRE(swprintf_s(path,ARRAYSIZE(path),L"%ls\\transcript.txt",folder)>0&&!_wfopen_s(&transcript,path,L"wb"));
    setvbuf(report,NULL,_IONBF,0);fprintf(report,"CASE mode=%u root=%u child=%u; real interactive cmd, no /c\n",mode,root_bits,child_bits);
    inventory=calloc(1,sizeof(*inventory));out=calloc(1,sizeof(*out));REQUIRE(inventory&&out);
    inventory->report=report;inventory->package=package;out->transcript=transcript;
    sprintf(id,"M%u_R%u_C%u_%lu_%llu",mode,root_bits,child_bits,GetCurrentProcessId(),GetTickCount64());
    sprintf(variable,"NTW_CHILD_%s",id);sprintf(outer_ready,"__OUTER_READY_%s__",id);
    sprintf(inner_ready,"__INNER_%s__",id);sprintf(returned,"__OUTER_BACK_%s__:37",id);
    job=CreateJobObjectW(NULL,NULL);REQUIRE(job);inventory->job=job;
    JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    REQUIRE(SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits)));
    REQUIRE(CreatePipe(&in_read,&in_write,&security,0)&&SetHandleInformation(in_write,HANDLE_FLAG_INHERIT,0));
    REQUIRE(CreatePipe(&out_read,&out_write,&security,0)&&SetHandleInformation(out_read,HANDLE_FLAG_INHERIT,0));out->pipe=out_read;
    InitializeProcThreadAttributeList(NULL,1,0,&bytes);attributes=malloc(bytes);REQUIRE(attributes);
    REQUIRE(InitializeProcThreadAttributeList(attributes,1,0,&bytes));attributes_ready=TRUE;HANDLE inherited[]={in_read,out_write};
    REQUIRE(UpdateProcThreadAttribute(attributes,0,PROC_THREAD_ATTRIBUTE_HANDLE_LIST,inherited,sizeof(inherited),NULL,NULL));
    startup.StartupInfo.cb=sizeof(startup);startup.StartupInfo.dwFlags=STARTF_USESTDHANDLES;
    startup.StartupInfo.hStdInput=in_read;startup.StartupInfo.hStdOutput=startup.StartupInfo.hStdError=out_write;startup.lpAttributeList=attributes;
    REQUIRE(swprintf_s(launcher,ARRAYSIZE(launcher),L"%ls\\run16s.exe",package)>0);
    if(mode==0)REQUIRE(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" --wait \"%ls\" /d /q",launcher,root_cmd)>0);
    else if(mode==1)REQUIRE(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" --wait \"%ls\" /d /q",backend,root_cmd)>0);
    else REQUIRE(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" --wait \"%ls\" --wait \"%ls\" /d /q",launcher,backend,root_cmd)>0);
    DWORD creation_flags=CREATE_SUSPENDED|EXTENDED_STARTUPINFO_PRESENT|(mode?0:CREATE_NO_WINDOW);
    SetLastError(0);BOOL created=CreateProcessW(NULL,command,NULL,NULL,TRUE,creation_flags,NULL,NULL,&startup.StartupInfo,&root);
    DWORD create_error=created?0:GetLastError();fprintf(report,"ROOT CREATE ok=%d win32_error=%lu pid=%lu command=%ls\n",created,create_error,root.dwProcessId,command);
    REQUIRE(created);REQUIRE(AssignProcessToJobObject(job,root.hProcess));assigned=TRUE;REQUIRE(ResumeThread(root.hThread)==1);
    CloseHandle(in_read);in_read=NULL;CloseHandle(out_write);out_write=NULL;
    REQUIRE(send_line(in_write,"@echo off\r\n"));sprintf(line,"echo %s\r\n",outer_ready);REQUIRE(send_line(in_write,line));
    REQUIRE(wait_text(out,outer_ready,15000,inventory));outer=cmd_find(inventory,0,0);REQUIRE(outer);
    REQUIRE(outer->machine==(root_bits==64?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386));
    REQUIRE(!_wcsicmp(outer->path,root_cmd));
    if(mode!=1)REQUIRE(outer->ntw&&outer->ntw_initialized);
    if(mode)REQUIRE(outer->nthook);
    WCHAR child_line[4096],wide_variable[140],wide_id[100];
    REQUIRE(!strstr(out->bytes,inner_ready));
    REQUIRE(MultiByteToWideChar(CP_ACP,0,variable,-1,wide_variable,ARRAYSIZE(wide_variable)));
    REQUIRE(MultiByteToWideChar(CP_ACP,0,id,-1,wide_id,ARRAYSIZE(wide_id)));
    REQUIRE(swprintf_s(child_line,ARRAYSIZE(child_line),L"\"%ls\" /d /q /k \"set %ls=%ls\"\r\n",child_cmd,wide_variable,wide_id)>0);
    REQUIRE(write_wide(in_write,child_line));
    /* The complete expected marker is never sent to stdin. It is assembled
     * by the child cmd from a variable set only by its own /k initialization.
     * A failed CreateProcess leaves the outer cmd unable to print that marker. */
    sprintf(line,"echo __INNER_%%%s%%__\r\n",variable);REQUIRE(send_line(in_write,line));
    if(!wait_text(out,inner_ready,15000,inventory)){
        EnumDesktopWindows(GetThreadDesktop(GetCurrentThreadId()),failed_dialog,(LPARAM)inventory);
        sprintf(line,"echo __CREATE_ERROR_%s__:%%errorlevel%%\r\n",id);send_line(in_write,line);
        char error_marker[160];sprintf(error_marker,"__CREATE_ERROR_%s__:",id);wait_text(out,error_marker,2000,inventory);
        const char *reported=strstr(out->bytes,error_marker);
        if(reported)fprintf(report,"CMD NESTED COMMAND ERRORLEVEL=%lu (cmd diagnostic; not a fabricated GetLastError sample)\n",strtoul(reported+strlen(error_marker),NULL,10));
        else fprintf(report,"CMD NESTED COMMAND ERRORLEVEL unavailable; retain actual native child exit and failed marker evidence\n");
        fprintf(report,"NESTED CREATE FAILED: inspect cmd ERRORLEVEL/transcript; a warning alone is not PE classification\n");goto done;
    }
    inner=cmd_find(inventory,outer->pid,outer->pid);REQUIRE(inner);
    REQUIRE(inner->machine==(child_bits==64?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386));
    WCHAR expected_child[1024];REQUIRE(swprintf_s(expected_child,ARRAYSIZE(expected_child),L"%ls\\%ls\\cmd.exe",windows,child_bits==64?L"System32":L"SysWOW64")>0);
    REQUIRE(!_wcsicmp(inner->path,expected_child));
    REQUIRE(inner->pid!=outer->pid);
    if(mode!=1)REQUIRE(inner->ntw&&inner->ntw_initialized);
    if(mode)REQUIRE(inner->nthook);
    if(mode)fprintf(report,"NTHOOK STATE loaded_in_parent_and_child=1 initialization_proved_by_actual_native_descendant_inheritance=1\n");
    fprintf(report,"CHILD MARKER verified target_pid=%lu creator=%lu; native PE; propagation success distinct from module presence\n",inner->pid,outer->pid);
    REQUIRE(send_line(in_write,"exit /b 37\r\n"));
    sprintf(line,"echo __OUTER_BACK_%s__:%%errorlevel%%\r\n",id);REQUIRE(send_line(in_write,line));
    REQUIRE(wait_text(out,returned,10000,inventory));
    REQUIRE(WaitForSingleObject(inner->handle,5000)==WAIT_OBJECT_0&&GetExitCodeProcess(inner->handle,&exit_code)&&exit_code==37);
    fprintf(report,"CHILD EXIT pid=%lu code=%lu outer_restored=1\n",inner->pid,exit_code);
    REQUIRE(send_line(in_write,"exit /b 23\r\n"));CloseHandle(in_write);in_write=NULL;
    REQUIRE(WaitForSingleObject(root.hProcess,15000)==WAIT_OBJECT_0&&GetExitCodeProcess(root.hProcess,&exit_code)&&exit_code==23);
    REQUIRE(WaitForSingleObject(outer->handle,5000)==WAIT_OBJECT_0&&GetExitCodeProcess(outer->handle,&exit_code)&&exit_code==23);
    REQUIRE(pump(out));fprintf(report,"OUTER EXIT pid=%lu code=%lu wrapper_code=23\n",outer->pid,exit_code);
    const char *only_marker=strstr(out->bytes,inner_ready);REQUIRE(only_marker&&!strstr(only_marker+strlen(inner_ready),inner_ready));
    fprintf(report,"CMD NESTED PASS mode=%u root=%u child=%u marker=%s inner_exit=37 outer_exit=23\n",mode,root_bits,child_bits,inner_ready);result=0;
done:
    if(report&&root.hProcess){DWORD status=0;GetExitCodeProcess(root.hProcess,&status);fprintf(report,"ROOT PRE-CLEANUP exit_or_STILL_ACTIVE=%lu\n",status);}
    if(report&&inventory)for(unsigned i=0;i<inventory->count;++i){DWORD status=0;cmd_process *p=&inventory->processes[i];GetExitCodeProcess(p->handle,&status);
        fprintf(report,"PROCESS PRE-CLEANUP pid=%lu exit_or_STILL_ACTIVE=%lu ntw_loaded=%d ntw_propagation_initialized=%d nthook_loaded=%d\n",p->pid,status,p->ntw,p->ntw_initialized,p->nthook);}
    if(report)fflush(report);
    if(in_read)CloseHandle(in_read);
    if(in_write)CloseHandle(in_write);
    if(out_write)CloseHandle(out_write);
    if(root.hProcess&&!assigned)TerminateProcess(root.hProcess,ERROR_CANCELLED);
    if(job){
        TerminateJobObject(job,result?ERROR_CANCELLED:0);
        ULONGLONG deadline=GetTickCount64()+5000;JOBOBJECT_BASIC_ACCOUNTING_INFORMATION accounting={0};BOOL cleanup_known=FALSE;
        do{if(!QueryInformationJobObject(job,JobObjectBasicAccountingInformation,&accounting,sizeof(accounting),NULL)){cleanup_known=FALSE;break;}
            cleanup_known=TRUE;
            if(!accounting.ActiveProcesses)break;
            Sleep(10);
        }while(GetTickCount64()<deadline);
        if(report)fprintf(report,"CLEANUP verified=%d owned_active_processes=%lu\n",cleanup_known,accounting.ActiveProcesses);
        if(!cleanup_known||accounting.ActiveProcesses)result=1;
        CloseHandle(job);
    }
    if(root.hProcess){WaitForSingleObject(root.hProcess,2000);CloseHandle(root.hProcess);}if(root.hThread)CloseHandle(root.hThread);
    if(inventory){cmd_inventory_close(inventory);free(inventory);}if(out_read)CloseHandle(out_read);
    if(attributes){if(attributes_ready)DeleteProcThreadAttributeList(attributes);free(attributes);}free(out);
    if(transcript)fclose(transcript);
    if(report)fclose(report);
    printf("CMD CASE mode=%u root=%u child=%u result=%d report=%ls\n",mode,root_bits,child_bits,result,folder);
    return result;
#undef REQUIRE
}
static BOOL empty_rpc_scope(void)
{
    HANDLE token;TOKEN_STATISTICS stats;DWORD bytes,session;
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token))return FALSE;
    BOOL ok=GetTokenInformation(token,TokenStatistics,&stats,sizeof(stats),&bytes)&&GetTokenInformation(token,TokenSessionId,&session,sizeof(session),&bytes);
    CloseHandle(token);if(!ok)return FALSE;
    WCHAR text[200];swprintf_s(text,ARRAYSIZE(text),L"ncalrpc:[ntvdm-basesrv-%lu-%08lx-%08lx]",session,stats.AuthenticationId.HighPart,stats.AuthenticationId.LowPart);
    RPC_BINDING_HANDLE binding=NULL;if(RpcBindingFromStringBindingW((RPC_WSTR)text,&binding))return FALSE;
    RPC_STATUS status=RpcMgmtIsServerListening(binding);RpcBindingFree(&binding);
    return status==RPC_S_SERVER_UNAVAILABLE||status==RPC_S_NOT_LISTENING;
}
static BOOL fixture_package(PCWSTR scratch,PCWSTR run,PCWSTR dll32,PCWSTR dll64,WCHAR package[1024])
{
    if(swprintf_s(package,1024,L"%ls\\cmd-package-%lu-%llu",scratch,GetCurrentProcessId(),GetTickCount64())<0||!CreateDirectoryW(package,NULL))return FALSE;
    const WCHAR *sources[]={run,dll32,dll64},*names[]={L"run16s.exe",L"ntw16s32.dll",L"ntw16s64.dll"};
    for(unsigned i=0;i<3;++i){WCHAR target[1200];char before[65],after[65];
        if(swprintf_s(target,ARRAYSIZE(target),L"%ls\\%ls",package,names[i])<0||!cmd_hash(sources[i],before)||
           !CopyFileW(sources[i],target,TRUE)||!cmd_hash(target,after)||strcmp(before,after))return FALSE;
    }
    return TRUE;
}
static int matrix(int argc,WCHAR **argv)
{
    WCHAR scratch[1024];if(argc<3||!GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",scratch,ARRAYSIZE(scratch)))return 2;
    WCHAR copied[1024];PCWSTR package=argv[2];
    if(argc==5&&!wcscmp(argv[1],L"--build")){
        if(!fixture_package(scratch,argv[2],argv[3],argv[4],copied))return 1;
        package=copied;
    }
    if(!wcscmp(argv[1],L"--core")||!wcscmp(argv[1],L"--build")){
        for(unsigned r=0;r<2;++r)for(unsigned c=0;c<2;++c)if(run_case(package,scratch,NULL,0,r?64:32,c?64:32))return 1;
        puts("CMD NESTED PASS core four native PE edges, interactive stdin/stdout, child markers and both exit codes");return 0;
    }
    return 2;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==6&&!wcscmp(argv[1],L"--compat-build")){
        if(GetFileAttributesW(argv[2])==INVALID_FILE_ATTRIBUTES){puts("CMD OPTIONAL SKIP installed run16 absent");return 77;}
        WCHAR scratch[1024],package[1024];
        if(!GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",scratch,ARRAYSIZE(scratch))||!fixture_package(scratch,argv[3],argv[4],argv[5],package))return 1;
        WCHAR *arguments[]={argv[0],L"--compat",package,argv[2]};return wmain(4,arguments);
    }
    if(argc==9&&!wcscmp(argv[1],L"--compat-case")){
        HANDLE token;TOKEN_STATISTICS stats;DWORD bytes;unsigned long high,low;
        if(swscanf_s(argv[8],L"%8lx%8lx",&high,&low)!=2||!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token))return 2;
        BOOL fresh=GetTokenInformation(token,TokenStatistics,&stats,sizeof(stats),&bytes)&&
            ((DWORD)stats.AuthenticationId.HighPart!=high||stats.AuthenticationId.LowPart!=low);
        CloseHandle(token);if(!fresh)return 2;
        if(!empty_rpc_scope()){puts("CMD OPTIONAL SKIP isolated endpoint not empty/unavailable");return 77;}
        /* The installed character frontend expects an actual caller Console.
         * It is created only on our private desktop. cmd's own standard I/O
         * remains the explicit pipes used by the interactive protocol. */
        /* CREATE_NO_WINDOW gave this freshly created worker its own headless
         * Console. Replace only that owned Console with a window-backed one;
         * an inherited user Console is never used by this worker. */
        FreeConsole();
        BOOL allocated=FALSE;
        {
            if(!AllocConsole()){
                DWORD console_error=GetLastError();
                WCHAR status[1200];swprintf_s(status,ARRAYSIZE(status),L"%ls\\cmd-compat-skip-%lu.txt",argv[3],GetCurrentProcessId());
                FILE *f=NULL;if(!_wfopen_s(&f,status,L"wt")){fprintf(f,"CMD OPTIONAL SKIP owned Console unavailable win32=%lu\n",console_error);fclose(f);}
                return 77;
            }
            allocated=TRUE;
        }
        int result=run_case(argv[2],argv[3],argv[4],(unsigned)_wtoi(argv[5]),(unsigned)_wtoi(argv[6]),(unsigned)_wtoi(argv[7]));
        if(allocated)FreeConsole();
        return result;
    }
    if(argc==4&&!wcscmp(argv[1],L"--compat")){
        if(GetFileAttributesW(argv[3])==INVALID_FILE_ATTRIBUTES){puts("CMD OPTIONAL SKIP installed run16 absent");return 77;}
        WCHAR image[1024],scratch[1024],desktop_name[100];GetModuleFileNameW(NULL,image,ARRAYSIZE(image));
        if(!GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",scratch,ARRAYSIZE(scratch)))return 2;
        swprintf_s(desktop_name,ARRAYSIZE(desktop_name),L"CmdNestedCompat-%lu",GetCurrentProcessId());
        HDESK desktop=CreateDesktopW(desktop_name,NULL,NULL,0,GENERIC_ALL,NULL);if(!desktop)return 77;
        int result=0;HANDLE token;TOKEN_STATISTICS original;DWORD bytes;
        if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token)){CloseDesktop(desktop);return 77;}
        BOOL captured=GetTokenInformation(token,TokenStatistics,&original,sizeof(original),&bytes);CloseHandle(token);
        if(!captured){CloseDesktop(desktop);return 77;}
        for(unsigned mode=1;mode<=2&&!result;++mode)for(unsigned r=0;r<2&&!result;++r)for(unsigned c=0;c<2&&!result;++c){
            WCHAR command[4096];swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" --compat-case \"%ls\" \"%ls\" \"%ls\" %u %u %u %08lx%08lx",image,argv[2],scratch,argv[3],mode,r?64:32,c?64:32,(DWORD)original.AuthenticationId.HighPart,original.AuthenticationId.LowPart);
            STARTUPINFOW startup={.cb=sizeof(startup),.lpDesktop=desktop_name};PROCESS_INFORMATION child={0};
            HANDLE job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
            if(!job||!SetInformationJobObject(job,JobObjectExtendedLimitInformation,&limits,sizeof(limits))){result=1;if(job)CloseHandle(job);break;}
            if(!CreateProcessWithLogonW(L"win16style-owned-probe",L".",L"local-only-unused",LOGON_NETCREDENTIALS_ONLY,image,command,CREATE_NO_WINDOW|CREATE_SUSPENDED,NULL,NULL,&startup,&child)){
                printf("CMD OPTIONAL SKIP separate logon unavailable win32=%lu\n",GetLastError());CloseHandle(job);result=77;break;}
            if(!AssignProcessToJobObject(job,child.hProcess)){TerminateProcess(child.hProcess,1);result=1;}
            else{ResumeThread(child.hThread);DWORD wait=WaitForSingleObject(child.hProcess,65000),code=1;
                if(wait==WAIT_OBJECT_0)GetExitCodeProcess(child.hProcess,&code);
                result=(int)code;}
            printf("CMD COMPAT CASE mode=%u root=%u child=%u exit=%d\n",mode,r?64:32,c?64:32,result);fflush(stdout);
            TerminateJobObject(job,ERROR_CANCELLED);
            ULONGLONG cleanup_deadline=GetTickCount64()+5000;JOBOBJECT_BASIC_ACCOUNTING_INFORMATION accounting={0};BOOL cleanup_known=FALSE;
            do{cleanup_known=QueryInformationJobObject(job,JobObjectBasicAccountingInformation,&accounting,sizeof(accounting),NULL);
                if(!cleanup_known||!accounting.ActiveProcesses)break;
                Sleep(10);
            }while(GetTickCount64()<cleanup_deadline);
            printf("CMD COMPAT CLEANUP mode=%u root=%u child=%u verified=%d owned_active_processes=%lu\n",mode,r?64:32,c?64:32,cleanup_known,accounting.ActiveProcesses);fflush(stdout);
            if(!cleanup_known||accounting.ActiveProcesses)result=1;
            CloseHandle(job);WaitForSingleObject(child.hProcess,2000);CloseHandle(child.hThread);CloseHandle(child.hProcess);
        }
        CloseDesktop(desktop);if(!result)puts("CMD NESTED PASS optional nthook/dual-hook eight native PE cases");return result;
    }
    return isolated_process(argc,argv,matrix);
}
