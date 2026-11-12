#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <uxtheme.h>
#include "ntw16s-dll/injection/intercept.h"
#include "ntw16s-dll/injection/late_process.h"
#include "ntw16s-dll/global/session.h"
#include "lib-win32/process-hook/late_batch.h"
#include "lib-win32/process-hook/propagate.h"
ntwshk_context ntw16s_process_context;
static volatile LONG running=1,ticks,style_registrations;
static BOOL chain_titles,fail_titles;
static volatile LONG first_windows,second_windows;
static volatile LONG first_themes,second_themes;
static __typeof__(&CreateWindowExW) first_create_window,second_create_window;
static __typeof__(&SetWindowTheme) first_set_theme,second_set_theme;
#define WINDOW_ARGUMENTS DWORD ex,LPCWSTR cls,LPCWSTR title,DWORD style,int x,int y,int width,int height,HWND parent,HMENU menu,HINSTANCE instance,LPVOID param
#define WINDOW_VALUES ex,cls,title,style,x,y,width,height,parent,menu,instance,param
static HWND WINAPI first_window(WINDOW_ARGUMENTS)
{InterlockedIncrement(&first_windows);return first_create_window(WINDOW_VALUES);}
static HWND WINAPI second_window(WINDOW_ARGUMENTS)
{InterlockedIncrement(&second_windows);return second_create_window(WINDOW_VALUES);}
static HRESULT WINAPI first_theme(HWND window,LPCWSTR application,LPCWSTR ids)
{InterlockedIncrement(&first_themes);return first_set_theme(window,application,ids);}
static HRESULT WINAPI second_theme(HWND window,LPCWSTR application,LPCWSTR ids)
{InterlockedIncrement(&second_themes);return second_set_theme(window,application,ids);}
/* The ordinary-startup backend is not linked into this isolated fixture. */
DWORD process_hook_startup_attach(void *slot,void *replacement)
{(void)slot;(void)replacement;return ERROR_NOT_SUPPORTED;}
/* This fixture isolates process propagation. Real style registrations are
 * exercised by the actual DLL integration tests. */
LONG ntw16s_attach_titles_using(DWORD (*attach)(void *,void *))
{
    if(!attach)return ERROR_INVALID_PARAMETER;
    InterlockedIncrement(&style_registrations);
    if(fail_titles)return ERROR_ACCESS_DENIED;
    if(!chain_titles)return NO_ERROR;
    second_create_window=CreateWindowExW;LONG error=(LONG)attach(&second_create_window,(void *)second_window);
    if(!error){second_set_theme=SetWindowTheme;error=(LONG)attach(&second_set_theme,(void *)second_theme);}
    return error;
}
LONG ntw16s_attach_file_dialogs_using(DWORD (*attach)(void *,void *)){if(attach){InterlockedIncrement(&style_registrations);return NO_ERROR;}return ERROR_INVALID_PARAMETER;}
LONG ntw16s_attach_com_dialogs_using(DWORD (*attach)(void *,void *)){if(attach){InterlockedIncrement(&style_registrations);return NO_ERROR;}return ERROR_INVALID_PARAMETER;}
LONG ntw16s_attach_folder_dialogs_using(DWORD (*attach)(void *,void *)){if(attach){InterlockedIncrement(&style_registrations);return NO_ERROR;}return ERROR_INVALID_PARAMETER;}
static DWORD WINAPI background(void *p)
{(void)p;while(InterlockedCompareExchange(&running,0,0)){InterlockedIncrement(&ticks);Sleep(1);}return 0;}
static process_hook_late_patch first_hooks[5];
static process_hook_late_patch *first_items[5];
static process_hook_late_batch first_batch;
static DWORD first_prepare(void *slot,void *replacement)
{return process_hook_late_batch_prepare(&first_batch,slot,replacement);}
static volatile LONG first_installs;
static BOOL first_active(void *context){(void)context;return TRUE;}
static BOOL first_accept_w(LPCWSTR application,LPWSTR command,void *context)
{(void)application;(void)command;(void)context;return TRUE;}
static BOOL first_accept_a(LPCSTR application,LPSTR command,void *context)
{(void)application;(void)command;(void)context;return TRUE;}
static DWORD first_install(HANDLE process,process_hook_create_w create,void *context)
{(void)process;(void)create;(void)context;InterlockedIncrement(&first_installs);return ERROR_SUCCESS;}
static DWORD WINAPI probe_create(void *context)
{
    (void)context;WCHAR image[32768],command[32790];STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child={0};
    if(!GetModuleFileNameW(NULL,image,ARRAYSIZE(image)) || swprintf(command,ARRAYSIZE(command),L"\"%ls\" probe-child",image)<0)return 1;
    if(!CreateProcessW(image,command,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&startup,&child))return 2;
    DWORD result=3;if(WaitForSingleObject(child.hProcess,3000)==WAIT_OBJECT_0)GetExitCodeProcess(child.hProcess,&result);
    else{TerminateProcess(child.hProcess,ERROR_TIMEOUT);WaitForSingleObject(child.hProcess,1000);}
    CloseHandle(child.hThread);CloseHandle(child.hProcess);return result;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !wcscmp(argv[1],L"probe-child"))return 0;
    if(argc==2 && !wcscmp(argv[1],L"retry-after-failure")){
        fail_titles=TRUE;
        DWORD first=ntw16s_late_process_init(L"C:\\fixture",123);
        BOOL clean=!process_hook_propagation_registered() && !ntw16s_process_context.flags;
        fail_titles=FALSE;
        DWORD second=ntw16s_late_process_init(L"C:\\fixture",123);
        BOOL installed=process_hook_propagation_registered();
        printf("LATE RETRY bits=%u first=%lu clean=%d second=%lu installed=%d\n",
               (unsigned)(sizeof(void *)*8),first,clean,second,installed);
        return first==ERROR_ACCESS_DENIED && clean && !second && installed?0:1;
    }
    if(argc==2 && (!wcscmp(argv[1],L"existing-propagation") || !wcscmp(argv[1],L"reuse-retry"))){
        BOOL retry=!wcscmp(argv[1],L"reuse-retry"),rollback=TRUE;
        SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
        const process_hook_propagation_ops first={first_active,first_accept_w,first_accept_a,first_install,NULL};
        DWORD error=process_hook_late_batch_begin(&first_batch,first_hooks,first_items,ARRAYSIZE(first_hooks));
        if(!error)error=process_hook_propagation_attach(first_prepare,&first);
        if(!error)error=process_hook_propagation_attach_as_user_w(first_prepare);
        first_create_window=CreateWindowExW;
        if(!error)error=first_prepare(&first_create_window,(void *)first_window);
        first_set_theme=SetWindowTheme;
        if(!error)error=first_prepare(&first_set_theme,(void *)first_theme);
        error=process_hook_late_batch_commit(&first_batch,error,100,1);
        if(error){printf("LATE PREEXISTING initial=%lu\n",error);return 2;}
        BYTE before_w[16],before_a[16];memcpy(before_w,(const void *)CreateProcessW,sizeof(before_w));
        memcpy(before_a,(const void *)CreateProcessA,sizeof(before_a));
        chain_titles=TRUE;
        if(retry){fail_titles=TRUE;DWORD rejected=ntw16s_late_process_init(L"C:\\fixture",123);
            rollback=rejected==ERROR_ACCESS_DENIED && process_hook_propagation_registered() && !ntw16s_process_context.flags;
            fail_titles=FALSE;style_registrations=0;}
        DWORD status=ntw16s_late_process_init(L"C:\\fixture",123);
        BOOL unchanged=!memcmp(before_w,(const void *)CreateProcessW,sizeof(before_w)) &&
                       !memcmp(before_a,(const void *)CreateProcessA,sizeof(before_a));
        HANDLE probe=CreateThread(NULL,0,probe_create,NULL,0,NULL);DWORD result=1;
        BOOL created=probe && WaitForSingleObject(probe,5000)==WAIT_OBJECT_0 && GetExitCodeThread(probe,&result) && !result;
        if(probe)CloseHandle(probe);
        HWND window=CreateWindowExW(0,L"STATIC",L"",WS_OVERLAPPED,0,0,100,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
        InterlockedExchange(&first_themes,0);InterlockedExchange(&second_themes,0);
        HRESULT theme=window?SetWindowTheme(window,L"Explorer",NULL):E_FAIL;
        BOOL theme_chained=SUCCEEDED(theme) && first_themes==1 && second_themes==1;
        if(window)DestroyWindow(window);
        BOOL chained=window && first_windows==1 && second_windows==1;
        printf("LATE PREEXISTING bits=%u initial=0 status=%lu unchanged=%d create=%d original_install=%ld style=%ld chained=%d theme_chain=%d rollback=%d\n",
               (unsigned)(sizeof(void *)*8),status,unchanged,created,first_installs,style_registrations,chained,theme_chained,rollback);
        return !status && unchanged && created && first_installs==1 && style_registrations==4 && chained && theme_chained && rollback?0:1;
    }
    if(argc==2 && !wcscmp(argv[1],L"existing")){
        BYTE before_w[16],before_a[16];memcpy(before_w,(const void *)CreateProcessW,sizeof(before_w));memcpy(before_a,(const void *)CreateProcessA,sizeof(before_a));
        ntw16s_process_context.flags=NTWSHK_POLICY_STYLE|NTWSHK_POLICY_PROPAGATE;
        DWORD status=ntw16s_late_process_init(L"C:\\fixture",123);
        BOOL unchanged=!memcmp(before_w,(const void *)CreateProcessW,sizeof(before_w)) && !memcmp(before_a,(const void *)CreateProcessA,sizeof(before_a));
        printf("LATE EXISTING bits=%u status=%lu unchanged=%d\n",(unsigned)(sizeof(void*)*8),status,unchanged);
        return !status && unchanged?0:1;
    }
    BOOL propagation_only=argc==5 && !wcscmp(argv[4],L"propagation-only");
    if((argc!=4 && !propagation_only) || (wcscmp(argv[3],L"A") && wcscmp(argv[3],L"W")))return 2;
    if(propagation_only && !SetEnvironmentVariableW(L"WIN16S_TEST_PROPAGATION_ONLY",L"1"))return 2;
    WCHAR directory[NTWSHK_PATH_CAP];DWORD length=GetFullPathNameW(argv[2],NTWSHK_PATH_CAP,directory,NULL);
    if(!length || length>=NTWSHK_PATH_CAP)return 3;
    WCHAR session_name[NTW16S_GLOBAL_NAME_CAP];if(!ntw16s_global_name(session_name,ARRAYSIZE(session_name)))return 9;
    HANDLE mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(ntw16s_global_session),session_name);
    if(!mapping || GetLastError()==ERROR_ALREADY_EXISTS)return 9;
    ntw16s_global_session *session=MapViewOfFile(mapping,FILE_MAP_WRITE,0,0,sizeof(*session));if(!session)return 9;
    session->magic=NTW16S_GLOBAL_MAGIC;session->version=NTW16S_GLOBAL_VERSION;session->owner_pid=GetCurrentProcessId();session->cookie=0x513a9187;
    if(!win32_process_global_birth(GetCurrentProcess(),&session->owner_birth_low,&session->owner_birth_high))return 9;
    session->phase=NTW16S_GLOBAL_ACTIVE;
    SetLastError(777);BOOL active=ntw16s_global_policy_active(session->cookie),wrong=ntw16s_global_policy_active(session->cookie^1u);
    if(!active || wrong || GetLastError()!=777)return 10;
    session->phase=NTW16S_GLOBAL_STOPPING;if(ntw16s_global_policy_active(session->cookie))return 10;
    session->phase=NTW16S_GLOBAL_ACTIVE;
    HANDLE thread=CreateThread(NULL,0,background,NULL,0,NULL);if(!thread)return 4;
    while(!InterlockedCompareExchange(&ticks,0,0))Sleep(1);
    DWORD error=propagation_only?ntw16s_late_process_init_mode(directory,session->cookie,NTWSHK_POLICY_PROPAGATE):ntw16s_late_process_init(directory,session->cookie);
    if(error){printf("LATE PROCESS install=%lu\n",error);return 6;}
    DWORD repeat=propagation_only?ntw16s_late_process_init_mode(directory,session->cookie,NTWSHK_POLICY_PROPAGATE):ntw16s_late_process_init(directory,session->cookie);if(repeat)return 5;
    if(propagation_only && (style_registrations || (ntw16s_process_context.flags&NTWSHK_POLICY_STYLE)))return 11;
    PROCESS_INFORMATION child={0};BOOL made=FALSE;
    if(!wcscmp(argv[3],L"W")){
        WCHAR command[32768];STARTUPINFOW si={.cb=sizeof(si)};
        if(swprintf(command,ARRAYSIZE(command),L"\"%ls\"",argv[1])<0)return 7;
        made=CreateProcessW(argv[1],command,NULL,NULL,TRUE,0,NULL,NULL,&si,&child);
    }else{
        char image[32768],command[32768];STARTUPINFOA si={.cb=sizeof(si)};BOOL replaced=FALSE,utf8=GetACP()==CP_UTF8;
        if(!WideCharToMultiByte(CP_ACP,utf8?0:WC_NO_BEST_FIT_CHARS,argv[1],-1,image,sizeof(image),NULL,utf8?NULL:&replaced)||replaced)return 8;
        if(snprintf(command,sizeof(command),"\"%s\"",image)<0)return 8;
        made=CreateProcessA(image,command,NULL,NULL,TRUE,0,NULL,NULL,&si,&child);
    }
    DWORD code=99;if(made){if(WaitForSingleObject(child.hProcess,10000)==WAIT_OBJECT_0)GetExitCodeProcess(child.hProcess,&code);else{TerminateProcess(child.hProcess,ERROR_TIMEOUT);WaitForSingleObject(child.hProcess,1000);}CloseHandle(child.hThread);CloseHandle(child.hProcess);}
    WCHAR ready_name[128],go_name[128],wait_command[32768];
    swprintf(ready_name,ARRAYSIZE(ready_name),L"Local\\Win16ProbeReady.%lu",GetCurrentProcessId());
    swprintf(go_name,ARRAYSIZE(go_name),L"Local\\Win16ProbeGo.%lu",GetCurrentProcessId());
    HANDLE ready=CreateEventW(NULL,TRUE,FALSE,ready_name),go=CreateEventW(NULL,TRUE,FALSE,go_name);
    PROCESS_INFORMATION waiting={0};STARTUPINFOW wait_si={.cb=sizeof(wait_si)};BOOL waiting_created=FALSE;
    if(ready && go && swprintf(wait_command,ARRAYSIZE(wait_command),L"\"%ls\" wait %ls %ls",argv[1],ready_name,go_name)>=0)
        waiting_created=CreateProcessW(argv[1],wait_command,NULL,NULL,TRUE,0,NULL,NULL,&wait_si,&waiting);
    BOOL waiting_ready=waiting_created && WaitForSingleObject(ready,5000)==WAIT_OBJECT_0;
    session->phase=NTW16S_GLOBAL_STOPPING;if(go)SetEvent(go);
    DWORD waiting_code=99;if(waiting_created){if(WaitForSingleObject(waiting.hProcess,7000)==WAIT_OBJECT_0)GetExitCodeProcess(waiting.hProcess,&waiting_code);else{TerminateProcess(waiting.hProcess,ERROR_TIMEOUT);WaitForSingleObject(waiting.hProcess,1000);}CloseHandle(waiting.hThread);CloseHandle(waiting.hProcess);}
    if(ready)CloseHandle(ready);
    if(go)CloseHandle(go);
    PROCESS_INFORMATION after={0};BOOL stopped_created=FALSE;
    if(!wcscmp(argv[3],L"W")){
        WCHAR command[32768];STARTUPINFOW si={.cb=sizeof(si)};
        if(swprintf(command,ARRAYSIZE(command),L"\"%ls\" child",argv[1])>=0)stopped_created=CreateProcessW(argv[1],command,NULL,NULL,TRUE,0,NULL,NULL,&si,&after);
    }else{
        char image[32768],command[32768];STARTUPINFOA si={.cb=sizeof(si)};BOOL replaced=FALSE,utf8=GetACP()==CP_UTF8;
        if(WideCharToMultiByte(CP_ACP,utf8?0:WC_NO_BEST_FIT_CHARS,argv[1],-1,image,sizeof(image),NULL,utf8?NULL:&replaced) && !replaced && snprintf(command,sizeof(command),"\"%s\" child",image)>0)
            stopped_created=CreateProcessA(image,command,NULL,NULL,TRUE,0,NULL,NULL,&si,&after);
    }
    DWORD stopped_code=99;if(stopped_created){if(WaitForSingleObject(after.hProcess,5000)==WAIT_OBJECT_0)GetExitCodeProcess(after.hProcess,&stopped_code);else{TerminateProcess(after.hProcess,ERROR_TIMEOUT);WaitForSingleObject(after.hProcess,1000);}CloseHandle(after.hThread);CloseHandle(after.hProcess);}
    session->cookie^=0x100u;session->phase=NTW16S_GLOBAL_ACTIVE;
    DWORD reinit=propagation_only?ntw16s_late_process_init_mode(directory,session->cookie,NTWSHK_POLICY_PROPAGATE):ntw16s_late_process_init(directory,session->cookie);PROCESS_INFORMATION again={0};STARTUPINFOW again_si={.cb=sizeof(again_si)};WCHAR again_command[32768];BOOL restarted=FALSE;
    if(!reinit && swprintf(again_command,ARRAYSIZE(again_command),L"\"%ls\" child",argv[1])>=0)
        restarted=CreateProcessW(argv[1],again_command,NULL,NULL,TRUE,0,NULL,NULL,&again_si,&again);
    DWORD again_code=99;if(restarted){if(WaitForSingleObject(again.hProcess,5000)==WAIT_OBJECT_0)GetExitCodeProcess(again.hProcess,&again_code);else{TerminateProcess(again.hProcess,ERROR_TIMEOUT);WaitForSingleObject(again.hProcess,1000);}CloseHandle(again.hThread);CloseHandle(again.hProcess);}
    InterlockedExchange(&running,0);DWORD joined=WaitForSingleObject(thread,2000);CloseHandle(thread);
    printf("LATE PROCESS bits=%u api=%ls init=1 repeat=1 created=%d exit=%lu stopped=%d stopped_exit=%lu descendant=%d restarted=%d joined=%d\n",(unsigned)(sizeof(void*)*8),argv[3],made,code,stopped_created,stopped_code,waiting_ready&&waiting_code==0,restarted&&again_code==0,joined==WAIT_OBJECT_0);
    if(propagation_only)printf("LATE PROPAGATION ONLY bits=%u style_registration=%ld style_flag=%u\n",(unsigned)(sizeof(void*)*8),style_registrations,(unsigned)!!(ntw16s_process_context.flags&NTWSHK_POLICY_STYLE));
    UnmapViewOfFile(session);CloseHandle(mapping);
    /* Test process exits with detours and trampoline storage still live. */
    return made && code==0 && stopped_created && stopped_code==41 && waiting_ready && waiting_code==0 && restarted && again_code==0 && joined==WAIT_OBJECT_0?0:1;
}
