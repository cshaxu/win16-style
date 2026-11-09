#include <windows.h>
#include <tlhelp32.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

typedef struct test_state { HANDLE job; HWND window; DWORD client,host; } test_state;
static BOOL in_job(HANDLE job,DWORD pid)
{
    HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid);BOOL own=FALSE;
    if(process){IsProcessInJob(process,job,&own);CloseHandle(process);}return own;
}
static BOOL CALLBACK find_window(HWND window,LPARAM context)
{
    test_state *state=(test_state *)context;WCHAR name[40];DWORD pid=0;
    if(!IsWindowVisible(window) || !GetClassNameW(window,name,ARRAYSIZE(name)) ||
       wcscmp(name,L"ConsoleWindowClass"))return TRUE;
    GetWindowThreadProcessId(window,&pid);
    if(in_job(state->job,pid)){state->window=window;state->client=pid;return FALSE;}
    return TRUE;
}
static DWORD find_host(HANDLE job)
{
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);DWORD pid=0;
    if(snapshot==INVALID_HANDLE_VALUE)return 0;
    PROCESSENTRY32W entry={.dwSize=sizeof(entry)};
    if(Process32FirstW(snapshot,&entry))do{
        if(!_wcsicmp(entry.szExeFile,L"conhost.exe") && in_job(job,entry.th32ProcessID)){
            pid=entry.th32ProcessID;break;
        }
    }while(Process32NextW(snapshot,&entry));
    CloseHandle(snapshot);return pid;
}
static BOOL query(HWND window,WPARAM operation)
{
    DWORD_PTR ignored=0;RemovePropW(window,L"Run16s.Console.Test.Result.v1");
    BOOL sent=SendMessageTimeoutW(window,RegisterWindowMessageW(L"Run16s.Console.Test.v1"),
        operation,0,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,1500,&ignored)!=0;
    BOOL result=sent && (uintptr_t)GetPropW(window,L"Run16s.Console.Test.Result.v1")==2;
    RemovePropW(window,L"Run16s.Console.Test.Result.v1");return result;
}
static BOOL console_io(DWORD client)
{
    BOOL found=FALSE;FreeConsole();if(!AttachConsole(client))return FALSE;
    HANDLE input=CreateFileW(L"CONIN$",GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
    HANDLE output=CreateFileW(L"CONOUT$",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
    const WCHAR command[]=L"echo RUN16S_CONSOLE_TEST\r";INPUT_RECORD events[ARRAYSIZE(command)]={0};
    for(unsigned i=0;i<ARRAYSIZE(command)-1;i++){
        events[i].EventType=KEY_EVENT;events[i].Event.KeyEvent.bKeyDown=TRUE;
        events[i].Event.KeyEvent.wRepeatCount=1;events[i].Event.KeyEvent.uChar.UnicodeChar=command[i];
        if(command[i]==L'\r')events[i].Event.KeyEvent.wVirtualKeyCode=VK_RETURN;
    }
    DWORD written=0;BOOL sent=input!=INVALID_HANDLE_VALUE &&
        WriteConsoleInputW(input,events,ARRAYSIZE(command)-1,&written) && written==ARRAYSIZE(command)-1;
    if(sent && output!=INVALID_HANDLE_VALUE){
        ULONGLONG deadline=GetTickCount64()+2000;
        do{
            CONSOLE_SCREEN_BUFFER_INFO info;
            if(GetConsoleScreenBufferInfo(output,&info)){
                DWORD size=(DWORD)info.dwSize.X*(DWORD)info.dwSize.Y;if(size>262144)size=262144;
                WCHAR *text=calloc((size_t)size+1,sizeof(*text));DWORD read=0;
                if(text){COORD origin={0,0};
                    if(ReadConsoleOutputCharacterW(output,text,size,origin,&read)){
                        WCHAR *first=wcsstr(text,L"RUN16S_CONSOLE_TEST");
                        found=first && wcsstr(first+18,L"RUN16S_CONSOLE_TEST")!=NULL;
                    }
                    free(text);
                }
            }
            if(!found)Sleep(50);
        }while(!found && GetTickCount64()<deadline);
    }
    if(input!=INVALID_HANDLE_VALUE)CloseHandle(input);
    if(output!=INVALID_HANDLE_VALUE)CloseHandle(output);
    FreeConsole();return found;
}

int wmain(int argc,WCHAR **argv)
{
    if(argc!=3 && (argc!=4 || wcscmp(argv[3],L"inherited")))return 2;
    int result=1;HHOOK hooks[128]={0};unsigned count=0;PROCESS_INFORMATION launch={0};
    HDESK old=GetThreadDesktop(GetCurrentThreadId()),desktop=NULL;HMODULE observer=NULL;
    test_state state={0};WCHAR name[80],command[32768],system[MAX_PATH];
    swprintf(name,ARRAYSIZE(name),L"Run16sConsoleTest-%lu",GetCurrentProcessId());
    desktop=CreateDesktopW(name,NULL,NULL,0,GENERIC_ALL,NULL);
    if(!desktop || !SetThreadDesktop(desktop))goto done;
    state.job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION limits={0};
    limits.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
    if(!state.job || !SetInformationJobObject(state.job,JobObjectExtendedLimitInformation,&limits,sizeof(limits)))goto done;
    UINT n=GetSystemDirectoryW(system,ARRAYSIZE(system));if(!n || n+13>=ARRAYSIZE(system))goto done;
    wcscat(system,L"\\conhost.exe");
    PCWSTR application=argv[1];
    if(argc==4){
        WCHAR cmd[MAX_PATH];UINT length=GetSystemDirectoryW(cmd,ARRAYSIZE(cmd));
        if(!length || length+9>=ARRAYSIZE(cmd))goto done;
        wcscat(cmd,L"\\cmd.exe");application=system;
        if(swprintf(command,ARRAYSIZE(command),L"\"%ls\" \"%ls\" /d /k \"\"%ls\" cmd\"",system,cmd,argv[1])<0)goto done;
    }else if(swprintf(command,ARRAYSIZE(command),L"\"%ls\" \"%ls\"",argv[1],system)<0)goto done;
    STARTUPINFOW startup={.cb=sizeof(startup),.lpDesktop=name};
    if(!CreateProcessW(application,command,NULL,NULL,FALSE,CREATE_SUSPENDED,NULL,NULL,&startup,&launch))goto done;
    if(!AssignProcessToJobObject(state.job,launch.hProcess)){TerminateProcess(launch.hProcess,1);goto done;}
    ResumeThread(launch.hThread);
    ULONGLONG deadline=GetTickCount64()+5000;
    do{
        state.host=find_host(state.job);EnumDesktopWindows(desktop,find_window,(LPARAM)&state);
        if(state.host && state.window)break;
        Sleep(50);
    }while(GetTickCount64()<deadline);
    if(!state.host || !state.window)goto done;
    observer=LoadLibraryW(argv[2]);HOOKPROC callback=observer?(HOOKPROC)(uintptr_t)GetProcAddress(observer,"ObserveConsole"):NULL;
    HANDLE threads=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
    if(!callback || threads==INVALID_HANDLE_VALUE)goto done;
    THREADENTRY32 entry={.dwSize=sizeof(entry)};
    if(Thread32First(threads,&entry))do{
        if(entry.th32OwnerProcessID==state.host && count<ARRAYSIZE(hooks)){
            HHOOK hook=SetWindowsHookExW(WH_CALLWNDPROC,callback,observer,entry.th32ThreadID);
            if(hook)hooks[count++]=hook;
        }
    }while(Thread32Next(threads,&entry));
    CloseHandle(threads);
    if(!count)goto done;
    deadline=GetTickCount64()+5000;
    while(!query(state.window,1) && GetTickCount64()<deadline)Sleep(50);
    if(!query(state.window,1) || !console_io(state.client) || !query(state.window,2))goto done;
    result=0;
done:
    if(result)fprintf(stderr,"console integration failed error=%lu\n",GetLastError());
    while(count)UnhookWindowsHookEx(hooks[--count]);
    if(observer)FreeLibrary(observer);
    if(launch.hThread)CloseHandle(launch.hThread);
    if(launch.hProcess)CloseHandle(launch.hProcess);
    if(state.job)CloseHandle(state.job);
    if(desktop){SetThreadDesktop(old);CloseDesktop(desktop);}
    return result;
}
