#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <tlhelp32.h>
#include "ntw16s-dll/global/session.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"

typedef struct owned_window {DWORD pid;HWND window;} owned_window;
static BOOL CALLBACK find_owned_window(HWND window,LPARAM value)
{
    owned_window *owned=(owned_window *)value;
    DWORD pid=0;WCHAR name[64];
    GetWindowThreadProcessId(window,&pid);
    if(pid!=owned->pid || !GetClassNameW(window,name,ARRAYSIZE(name)) ||
       wcscmp(name,L"GlobalPropagationProbe") || !GetPropW(window,L"Owned.Propagation.TestReady"))return TRUE;
    owned->window=window;return FALSE;
}
static DWORD launched_pid(DWORD launcher)
{
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if(snapshot==INVALID_HANDLE_VALUE)return 0;
    PROCESSENTRY32W entry={.dwSize=sizeof(entry)};DWORD pid=0;
    if(Process32FirstW(snapshot,&entry))do{
        if(entry.th32ParentProcessID==launcher && !wcscmp(entry.szExeFile,L"global-propagation-probe.exe")){
            pid=entry.th32ProcessID;break;
        }
    }while(Process32NextW(snapshot,&entry));
    CloseHandle(snapshot);return pid;
}
static BOOL switch_styles(DWORD pid,ntw16s_global_session *session,unsigned initial)
{
    owned_window owned={.pid=initial?0:pid};
    ULONGLONG deadline=GetTickCount64()+4000;
    do {
        if(!owned.pid)owned.pid=launched_pid(pid);
        if(owned.pid)EnumWindows(find_owned_window,(LPARAM)&owned);
        if(!owned.window)Sleep(20);
    } while(!owned.window && GetTickCount64()<deadline);
    if(!owned.window)return FALSE;
    UINT control=RegisterWindowMessageW(NTW16S_GLOBAL_CONTROL);
    const unsigned expected[]={1,2,0,2,1,0};
    const unsigned themes[]={
        classic_theme_encode_selection((classic_theme_selection){classic_theme_folder_id(L"Win16"),CLASSIC_THEME_WIN16}),
        classic_theme_encode_selection((classic_theme_selection){classic_theme_folder_id(L"Win2K"),CLASSIC_THEME_WIN2K})
    };
    DWORD_PTR current=MAXDWORD;
    BOOL passed=control && SendMessageTimeoutW(owned.window,WM_APP+113,0,0,SMTO_ABORTIFHUNG,300,&current) &&
        current==initial;
    printf("OWNED GLOBAL INITIAL expected=%u observed=%llu\n",initial,(unsigned long long)current);
    for(unsigned i=0;passed && i<ARRAYSIZE(expected);++i){
        BOOL apply=expected[i]!=0;
        if(apply && i && !expected[i-1])++session->cookie;
        if(apply)session->style=themes[expected[i]-1];
        InterlockedExchange(&session->phase,apply?NTW16S_GLOBAL_ACTIVE:NTW16S_GLOBAL_STOPPING);
        ULONGLONG until=GetTickCount64()+1000;
        DWORD_PTR observed=MAXDWORD;
        UINT_PTR selection=0,wanted=apply?(UINT_PTR)session->style+1u:0;
        do {
            DWORD_PTR ignored;DWORD current_pid=0;
            GetWindowThreadProcessId(owned.window,&current_pid);
            if(current_pid!=owned.pid || !SendMessageTimeoutW(owned.window,control,session->cookie,
                apply?NTW16S_GLOBAL_APPLY:NTW16S_GLOBAL_REMOVE,SMTO_ABORTIFHUNG,300,&ignored) ||
               !SendMessageTimeoutW(owned.window,WM_APP+113,0,0,SMTO_ABORTIFHUNG,300,&observed))break;
            selection=(UINT_PTR)GetPropW(owned.window,L"Win16Style.Global.Profile");
            if(observed==expected[i] && selection==wanted)break;
            Sleep(20);
        } while(GetTickCount64()<until);
        passed=observed==expected[i] && selection==wanted;
        printf("OWNED GLOBAL SWITCH step=%u expected=%u observed=%llu selection=%llu wanted=%llu\n",
            i+1,expected[i],(unsigned long long)observed,(unsigned long long)selection,(unsigned long long)wanted);
    }
    DWORD current_pid=0;
    if(GetWindowThreadProcessId(owned.window,&current_pid) && current_pid==owned.pid)
        PostMessageW(owned.window,WM_CLOSE,0,0);
    if(passed)puts("OWNED GLOBAL THEME ID PASS");
    return passed;
}

int wmain(int argc,WCHAR **argv)
{
    BOOL switching=(argc==4 || argc==6) && !wcscmp(argv[3],L"--owned-switch");
    BOOL owned=switching || (argc==4 && !wcscmp(argv[3],L"--owned-thread"));
    unsigned initial=0;
    if(argc==6){
        if(!switching)return 2;
        if(!wcscmp(argv[5],L"16"))initial=1;
        else if(!wcscmp(argv[5],L"2k"))initial=2;
        else return 2;
    }
    if(argc!=3 && !owned)return 2;
    WCHAR desktop[128],name[NTW16S_GLOBAL_NAME_CAP],command[32768];DWORD needed=0;
    if(!owned && (!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&needed)||wcsncmp(desktop,L"Win16S-Dialog-",14)))return 3;
    /* The owned mode uses the child's localhook path, which observes only its own UI thread. */
    if(owned && !SetEnvironmentVariableW(L"WIN16S_TEST_NEUTRAL_RUNTIME",L"1"))return 3;
    if(switching && !SetEnvironmentVariableW(L"WIN16S_TEST_OWNED_SWITCH",L"1"))return 3;
    if(!style_global_session_name(&ntw16s_global_namespace,name,ARRAYSIZE(name)))return 4;
    HANDLE mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(ntw16s_global_session),name);
    if(!mapping || GetLastError()==ERROR_ALREADY_EXISTS)return 5;
    ntw16s_global_session *s=MapViewOfFile(mapping,FILE_MAP_WRITE,0,0,sizeof(*s));if(!s)return 6;
    s->magic=NTW16S_GLOBAL_MAGIC;s->version=NTW16S_GLOBAL_VERSION;s->owner_pid=GetCurrentProcessId();s->cookie=0x672ad713;
    if(!win32_process_global_birth(GetCurrentProcess(),&s->owner_birth_low,&s->owner_birth_high))return 7;
    s->phase=switching?NTW16S_GLOBAL_STOPPING:NTW16S_GLOBAL_ACTIVE;STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION child={0};
    int length=initial?swprintf(command,ARRAYSIZE(command),L"\"%ls\" %ls\"%ls\" localhook \"%ls\"",
        argv[4],initial==2?L"Win2K ":L"Win16 ",argv[1],argv[2]):
        swprintf(command,ARRAYSIZE(command),L"\"%ls\" localhook \"%ls\"",argv[1],argv[2]);
    if(length<0 || !CreateProcessW(initial?argv[4]:argv[1],command,NULL,NULL,TRUE,0,NULL,NULL,&si,&child))return 8;
    BOOL switched=!switching || switch_styles(child.dwProcessId,s,initial);
    DWORD code=99;if(WaitForSingleObject(child.hProcess,12000)==WAIT_OBJECT_0)GetExitCodeProcess(child.hProcess,&code);
    else{TerminateProcess(child.hProcess,ERROR_TIMEOUT);WaitForSingleObject(child.hProcess,1000);}
    CloseHandle(child.hThread);CloseHandle(child.hProcess);s->phase=NTW16S_GLOBAL_STOPPING;UnmapViewOfFile(s);CloseHandle(mapping);
    printf("LOCAL GLOBAL HOOK target_exit=%lu\n",code);return code || !switched?1:0;
}
