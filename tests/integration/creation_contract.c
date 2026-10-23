#include "contract_support.h"
typedef struct contract_case {BOOL ansi,empty_application,extended,inherit,suspended,unicode_environment;DWORD priority,extra;} contract_case;
static const contract_case cases[]={
    {FALSE,FALSE,FALSE,FALSE,FALSE,TRUE,0,0},
    {TRUE,FALSE,FALSE,FALSE,TRUE,FALSE,0,0},
    {FALSE,TRUE,FALSE,TRUE,TRUE,TRUE,0,0},
    {TRUE,TRUE,FALSE,TRUE,FALSE,TRUE,0,0},
    {FALSE,FALSE,TRUE,TRUE,TRUE,TRUE,BELOW_NORMAL_PRIORITY_CLASS,0},
    {TRUE,FALSE,TRUE,TRUE,FALSE,FALSE,0,0},
    {FALSE,TRUE,FALSE,FALSE,TRUE,TRUE,0,CREATE_NEW_PROCESS_GROUP},
    {TRUE,TRUE,TRUE,TRUE,TRUE,TRUE,0,0}
};
static int child(int argc,WCHAR **argv)
{
    CHECK(argc>=3 && !wcscpy_s(contract_report,MAX_PATH,argv[2]));contract_log("entry",GetCurrentProcessId());
    CHECK(argc==16);unsigned variant=(unsigned)_wtoi(argv[4]);CHECK(variant<8);const contract_case *c=&cases[variant];
    BOOL styled=_wtoi(argv[3]);DWORD flags=0;WCHAR value[1024],expected[1024],actual[1024],package[MAX_PATH];
    HANDLE event=(HANDLE)(ULONG_PTR)_wcstoui64(argv[5],NULL,10),deny=(HANDLE)(ULONG_PTR)_wcstoui64(argv[6],NULL,10);
    BOOL has_event=GetHandleInformation(event,&flags);CHECK(!!has_event==!!_wtoi(argv[7]));if(has_event)CHECK(SetEvent(event));
    BOOL has_deny=GetHandleInformation(deny,&flags);CHECK(!!has_deny==!!_wtoi(argv[8]));if(has_deny)CHECK(SetEvent(deny));
    CHECK(GetPriorityClass(GetCurrentProcess())==wcstoul(argv[9],NULL,10) && sizeof(void *)*8==wcstoul(argv[10],NULL,10));
    CHECK(!wcscmp(argv[11],L"two words") && !wcscmp(argv[12],L"quote\"inside") && !wcscmp(argv[13],L"trailing\\") && !wcscmp(argv[14],L""));
    CHECK(!wcscmp(argv[15],c->ansi?L"ansi-ascii":L"Unicode 参数 Ω"));
    CHECK(GetEnvironmentVariableW(L"WIN16STYLE_VALUE",value,ARRAYSIZE(value)));
    CHECK(!wcscmp(value,c->unicode_environment?L"unicode-value 中文 Ω":L"ansi-value-123"));
    CHECK(!GetEnvironmentVariableW(L"WIN16STYLE_PARENT_ONLY_SENTINEL",value,ARRAYSIZE(value)));
    CHECK(!GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",value,ARRAYSIZE(value)));
    CHECK(GetEnvironmentVariableW(L"WIN16STYLE_EXPECT_CWD",expected,ARRAYSIZE(expected)) && GetCurrentDirectoryW(ARRAYSIZE(actual),actual));
    CHECK(!_wcsicmp(expected,actual));
    CHECK(GetEnvironmentVariableW(L"WIN16STYLE_EXPECT_DESKTOP",expected,ARRAYSIZE(expected)));
    DWORD needed;CHECK(GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,actual,sizeof(actual),&needed) && !wcscmp(expected,actual));
    STARTUPINFOW startup={sizeof(startup)};GetStartupInfoW(&startup);
    CHECK((startup.dwFlags&STARTF_USESHOWWINDOW) && startup.wShowWindow==SW_HIDE && startup.lpTitle && !wcscmp(startup.lpTitle,L"Contract startup title"));
    if(c->inherit){char input[80];CHECK(startup.dwFlags&STARTF_USESTDHANDLES);CHECK(fgets(input,sizeof(input),stdin) && !strcmp(input,"contract-input\n"));
        CHECK(fputs("child-stdout\n",stdout)!=EOF && fputs("child-stderr\n",stderr)!=EOF && !fflush(stdout) && !fflush(stderr));}
    CHECK(GetEnvironmentVariableW(L"WIN16STYLE_EXPECT_PACKAGE",package,MAX_PATH));
    CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));contract_window(styled,package);
    contract_log("complete",GetCurrentProcessId());return 0;
}
static void append_environment(WCHAR *block,size_t capacity,size_t *used,PCWSTR key,PCWSTR value)
{int count=swprintf_s(block+*used,capacity-*used,L"%ls=%ls",key,value);CHECK(count>0);*used+=(size_t)count+1;}
static void create_case(unsigned variant,PCWSTR executable,PCWSTR unicode_executable,unsigned child_bits,BOOL styled,PCWSTR package,PCWSTR folder)
{
    const contract_case *c=&cases[variant];WCHAR path[MAX_PATH],cwd[MAX_PATH],desktop[MAX_PATH],windows[MAX_PATH];
    WCHAR command[32768],block[4096];char command_a[32768],application_a[4096],directory_a[4096],environment_a[8192],desktop_a[MAX_PATH];
    HANDLE filler[512],event,deny,input=NULL,output=NULL;SECURITY_ATTRIBUTES sa={sizeof(sa),NULL,TRUE};DWORD bytes,needed;
    for(unsigned i=0;i<512;++i){filler[i]=CreateEventW(NULL,TRUE,FALSE,NULL);CHECK(filler[i]);}
    event=CreateEventW(&sa,TRUE,FALSE,NULL);deny=CreateEventW(&sa,TRUE,FALSE,NULL);CHECK(event && deny);
    for(unsigned i=0;i<512;++i)CHECK(CloseHandle(filler[i]));
    CHECK(swprintf_s(path,MAX_PATH,L"%ls\\v%u.txt",folder,variant)>0 && GetFileAttributesW(path)==INVALID_FILE_ATTRIBUTES);
    CHECK(swprintf_s(cwd,MAX_PATH,L"%ls\\%ls",folder,c->ansi?L"cwd ascii":L"工作 目录")>0);
    CHECK(CreateDirectoryW(cwd,NULL) || GetLastError()==ERROR_ALREADY_EXISTS);
    CHECK(GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&needed));
    CHECK(GetWindowsDirectoryW(windows,MAX_PATH));size_t used=0;
    append_environment(block,ARRAYSIZE(block),&used,L"WIN16STYLE_VALUE",c->unicode_environment?L"unicode-value 中文 Ω":L"ansi-value-123");
    append_environment(block,ARRAYSIZE(block),&used,L"WIN16STYLE_EXPECT_CWD",cwd);
    append_environment(block,ARRAYSIZE(block),&used,L"WIN16STYLE_EXPECT_DESKTOP",desktop);
    append_environment(block,ARRAYSIZE(block),&used,L"WIN16STYLE_EXPECT_PACKAGE",package);
    append_environment(block,ARRAYSIZE(block),&used,L"SystemRoot",windows);block[used++]=0;
    CHECK(WideCharToMultiByte(CP_ACP,0,block,(int)used,environment_a,sizeof(environment_a),NULL,NULL));
    DWORD priority=c->priority?c->priority:GetPriorityClass(GetCurrentProcess());if(!c->priority && priority!=IDLE_PRIORITY_CLASS && priority!=BELOW_NORMAL_PRIORITY_CLASS)priority=NORMAL_PRIORITY_CLASS;
    PCWSTR app=c->ansi?executable:unicode_executable;
    CHECK(swprintf_s(command,ARRAYSIZE(command),L"\"%ls\" --child \"%ls\" %d %u %llu %llu %d %d %lu %u \"two words\" \"quote\\\"inside\" \"trailing\\\\\" \"\" \"%ls\"",
        app,path,styled,variant,(unsigned long long)(ULONG_PTR)event,(unsigned long long)(ULONG_PTR)deny,c->inherit,c->inherit&&!c->extended,priority,child_bits,c->ansi?L"ansi-ascii":L"Unicode 参数 Ω")>0);
    STARTUPINFOEXW w={0};STARTUPINFOEXA a={0};PROCESS_INFORMATION process={0};
    w.StartupInfo.cb=c->extended?sizeof(w):sizeof(w.StartupInfo);w.StartupInfo.lpTitle=(LPWSTR)L"Contract startup title";
    w.StartupInfo.lpDesktop=desktop;w.StartupInfo.dwFlags=STARTF_USESHOWWINDOW;w.StartupInfo.wShowWindow=SW_HIDE;
    if(c->inherit){WCHAR io[MAX_PATH];CHECK(swprintf_s(io,MAX_PATH,L"%ls\\v%u.in",folder,variant)>0);
        input=CreateFileW(io,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,&sa,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);CHECK(input!=INVALID_HANDLE_VALUE);
        CHECK(WriteFile(input,"contract-input\n",15,&bytes,NULL) && bytes==15 && SetFilePointer(input,0,NULL,FILE_BEGIN)==0);
        CHECK(swprintf_s(io,MAX_PATH,L"%ls\\v%u.out",folder,variant)>0);
        output=CreateFileW(io,GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,&sa,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);CHECK(output!=INVALID_HANDLE_VALUE);
        w.StartupInfo.dwFlags|=STARTF_USESTDHANDLES;w.StartupInfo.hStdInput=input;w.StartupInfo.hStdOutput=output;w.StartupInfo.hStdError=output;}
    SIZE_T attribute_bytes=0;HANDLE allowed[3]={event,input,output};
    if(c->extended){InitializeProcThreadAttributeList(NULL,1,0,&attribute_bytes);w.lpAttributeList=HeapAlloc(GetProcessHeap(),0,attribute_bytes);CHECK(w.lpAttributeList);
        CHECK(InitializeProcThreadAttributeList(w.lpAttributeList,1,0,&attribute_bytes));
        CHECK(UpdateProcThreadAttribute(w.lpAttributeList,0,PROC_THREAD_ATTRIBUTE_HANDLE_LIST,allowed,sizeof(allowed),NULL,NULL));}
    DWORD flags=c->extra|c->priority|(c->suspended?CREATE_SUSPENDED:0)|(c->unicode_environment?CREATE_UNICODE_ENVIRONMENT:0)|(c->extended?EXTENDED_STARTUPINFO_PRESENT:0);
    DWORD event_flags_before;CHECK(GetHandleInformation(event,&event_flags_before));
    if(c->ansi){
        CHECK(WideCharToMultiByte(CP_ACP,0,app,-1,application_a,sizeof(application_a),NULL,NULL));
        CHECK(WideCharToMultiByte(CP_ACP,0,command,-1,command_a,sizeof(command_a),NULL,NULL));
        CHECK(WideCharToMultiByte(CP_ACP,0,cwd,-1,directory_a,sizeof(directory_a),NULL,NULL));
        CHECK(WideCharToMultiByte(CP_ACP,0,desktop,-1,desktop_a,sizeof(desktop_a),NULL,NULL));
        a.StartupInfo.cb=c->extended?sizeof(a):sizeof(a.StartupInfo);a.StartupInfo.lpTitle=(LPSTR)"Contract startup title";
        a.StartupInfo.lpDesktop=desktop_a;a.StartupInfo.dwFlags=w.StartupInfo.dwFlags;a.StartupInfo.wShowWindow=SW_HIDE;
        a.StartupInfo.hStdInput=input;a.StartupInfo.hStdOutput=output;a.StartupInfo.hStdError=output;a.lpAttributeList=w.lpAttributeList;
        CHECK(CreateProcessA(c->empty_application?NULL:application_a,command_a,(variant&1)?&sa:NULL,(variant&1)?&sa:NULL,c->inherit,flags,c->unicode_environment?(void *)block:environment_a,directory_a,&a.StartupInfo,&process));
    }else CHECK(CreateProcessW(c->empty_application?NULL:app,command,(variant&1)?&sa:NULL,(variant&1)?&sa:NULL,c->inherit,flags,c->unicode_environment?(void *)block:environment_a,cwd,&w.StartupInfo,&process));
    DWORD handle_flags;CHECK(GetHandleInformation(process.hProcess,&handle_flags) && !!(handle_flags&HANDLE_FLAG_INHERIT)==!!(variant&1));
    CHECK(GetHandleInformation(process.hThread,&handle_flags) && !!(handle_flags&HANDLE_FLAG_INHERIT)==!!(variant&1));
    CHECK(GetHandleInformation(event,&handle_flags) && handle_flags==event_flags_before);
    if(c->suspended){CHECK(GetFileAttributesW(path)==INVALID_FILE_ATTRIBUTES && SuspendThread(process.hThread)==1 && ResumeThread(process.hThread)==2 && ResumeThread(process.hThread)==1);}
    DWORD wait=contract_wait(process.hProcess,30000),code=1;
    if(wait!=WAIT_OBJECT_0){TerminateProcess(process.hProcess,ERROR_TIMEOUT);WaitForSingleObject(process.hProcess,1000);}
    CHECK(wait==WAIT_OBJECT_0 && GetExitCodeProcess(process.hProcess,&code));CloseHandle(process.hThread);CloseHandle(process.hProcess);CHECK(!code);
    CHECK(WaitForSingleObject(event,0)==(c->inherit?WAIT_OBJECT_0:WAIT_TIMEOUT));
    CHECK(WaitForSingleObject(deny,0)==(c->inherit&&!c->extended?WAIT_OBJECT_0:WAIT_TIMEOUT));
    CHECK(CloseHandle(event) && CloseHandle(deny));if(input)CHECK(CloseHandle(input));if(output)CHECK(CloseHandle(output));
    if(c->extended){DeleteProcThreadAttributeList(w.lpAttributeList);HeapFree(GetProcessHeap(),0,w.lpAttributeList);}
    if(c->inherit){WCHAR io[MAX_PATH];char text[200]={0};FILE *file=NULL;CHECK(swprintf_s(io,MAX_PATH,L"%ls\\v%u.out",folder,variant)>0 && !_wfopen_s(&file,io,L"rb"));
        CHECK(fread(text,1,sizeof(text)-1,file)>0 && !fclose(file));CHECK(strstr(text,"child-stdout") && strstr(text,"child-stderr"));}
    contract_log("case-complete",variant);
}
int wmain(int argc,WCHAR **argv)
{
    if(argc>1 && !wcscmp(argv[1],L"--child"))return child(argc,argv);
    CHECK(argc==9 && !wcscmp(argv[1],L"--parent"));
    CHECK(swprintf_s(contract_report,MAX_PATH,L"%ls\\parent.txt",argv[7])>0);contract_log("entry",GetCurrentProcessId());
    CHECK(sizeof(void *)*8==wcstoul(argv[8],NULL,10));BOOL styled=_wtoi(argv[5]);
    CHECK(SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2));contract_window(styled,argv[6]);
    CHECK(SetEnvironmentVariableW(L"WIN16STYLE_PARENT_ONLY_SENTINEL",L"not inherited in custom block"));
    WCHAR before[MAX_PATH],after[MAX_PATH];CHECK(GetCurrentDirectoryW(MAX_PATH,before));
    {WCHAR missing[MAX_PATH],line[32768];char missing_a[4096],line_a[2]="";PROCESS_INFORMATION process={0};
        STARTUPINFOA a={sizeof(a)};STARTUPINFOW w={sizeof(w)};
        CHECK(swprintf_s(missing,MAX_PATH,L"%ls\\native-missing.exe",argv[7])>0 && WideCharToMultiByte(CP_ACP,0,missing,-1,missing_a,sizeof(missing_a),NULL,NULL));
        SetLastError(0);CHECK(!CreateProcessA(missing_a,line_a,NULL,NULL,FALSE,0,NULL,NULL,&a,&process) && GetLastError()==ERROR_FILE_NOT_FOUND);
        CHECK(swprintf_s(line,ARRAYSIZE(line),L"\"%ls\"",argv[2])>0);
        SetLastError(0);CHECK(!CreateProcessW(argv[2],line,NULL,NULL,FALSE,CREATE_NEW_CONSOLE|DETACHED_PROCESS,NULL,NULL,&w,&process) && GetLastError()==ERROR_INVALID_PARAMETER);
        contract_log("native-errors",2);}
    for(unsigned variant=0;variant<8;++variant)create_case(variant,argv[2],argv[3],(unsigned)_wtoi(argv[4]),styled,argv[6],argv[7]);
    CHECK(GetCurrentDirectoryW(MAX_PATH,after) && !wcscmp(before,after));contract_log("complete",GetCurrentProcessId());return 0;
}
