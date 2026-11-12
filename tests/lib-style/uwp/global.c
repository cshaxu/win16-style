#include "lib-style/uwp/global.h"
#include <stdio.h>
#include <wchar.h>
static const WCHAR frame_class[]=L"ApplicationFrameWindow",core_class[]=L"Windows.UI.Core.CoreWindow";
static volatile LONG applies,removes,fail_apply,fail_remove,block_apply;
static HANDLE entered,gate;
static BOOL host_allowed(PCWSTR image,PCWSTR windows){(void)windows;return image && *image;}
static BOOL active(void *context){return context!=NULL;}
static DWORD change(const style_uwp_target *target,PCWSTR dll,BOOL apply,void *context)
{
    if(!target||!dll||!context)return ERROR_INVALID_PARAMETER;
    if(apply){
        InterlockedIncrement(&applies);
        if(InterlockedExchange(&fail_apply,0))return ERROR_ACCESS_DENIED;
        if(InterlockedCompareExchange(&block_apply,0,0)){
            SetEvent(entered);if(WaitForSingleObject(gate,5000)!=WAIT_OBJECT_0)return ERROR_TIMEOUT;
        }
    }else{
        InterlockedIncrement(&removes);
        if(InterlockedExchange(&fail_remove,0))return ERROR_ACCESS_DENIED;
    }
    return ERROR_SUCCESS;
}
static LRESULT CALLBACK proc(HWND w,UINT m,WPARAM a,LPARAM b)
{if(m==WM_CLOSE){DestroyWindow(w);PostQuitMessage(0);return 0;}return DefWindowProcW(w,m,a,b);}
static int child(PCWSTR title)
{
    HINSTANCE instance=GetModuleHandleW(NULL);WNDCLASSW frame={.lpfnWndProc=proc,.hInstance=instance,.lpszClassName=frame_class};
    WNDCLASSW core={.lpfnWndProc=DefWindowProcW,.hInstance=instance,.lpszClassName=core_class};
    if(!RegisterClassW(&frame)||!RegisterClassW(&core))return 1;
    HWND w=CreateWindowW(frame_class,title,WS_POPUP|WS_VISIBLE,-30000,-30000,100,80,NULL,NULL,instance,NULL);
    if(!w || !CreateWindowW(core_class,L"",WS_CHILD|WS_VISIBLE,0,0,20,20,w,NULL,instance,NULL))return 2;
    MSG msg;while(GetMessageW(&msg,NULL,0,0)>0){TranslateMessage(&msg);DispatchMessageW(&msg);}return 0;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==3&&!lstrcmpW(argv[1],L"--child"))return child(argv[2]);
    WCHAR image[MAX_PATH],title[80],command[MAX_PATH*2];STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION process={0};
    if(!GetModuleFileNameW(NULL,image,MAX_PATH))return 1;
    swprintf(title,ARRAYSIZE(title),L"StyleUwpGlobal-%lu",GetCurrentProcessId());
    if(swprintf(command,ARRAYSIZE(command),L"\"%ls\" --child %ls",image,title)<0)return 2;
    if(!CreateProcessW(image,command,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&startup,&process))return 3;
    HWND window=NULL;style_uwp_target observed={0};
    for(unsigned i=0;i<250;++i){
        if(WaitForSingleObject(process.hProcess,0)!=WAIT_TIMEOUT)break;
        window=FindWindowW(frame_class,title);
        if(window && style_uwp_target_window(window,&observed))break;
        window=NULL;Sleep(20);
    }
    DWORD pid=0;if(window)GetWindowThreadProcessId(window,&pid);
    win32_process_global_policy policy;style_uwp_global *manager=NULL;HANDLE quit=NULL,stop=NULL;int result=4;
    DWORD policy_error=window && pid==process.dwProcessId?win32_process_global_policy_init(&policy):ERROR_INVALID_WINDOW_HANDLE;
    if(policy_error){printf("UWP_GLOBAL_SETUP window=%d pid_match=%d policy=%lu\n",window!=NULL,pid==process.dwProcessId,(unsigned long)policy_error);goto done;}
    quit=CreateEventW(NULL,TRUE,FALSE,NULL);stop=CreateEventW(NULL,TRUE,FALSE,NULL);
    entered=CreateEventW(NULL,TRUE,FALSE,NULL);gate=CreateEventW(NULL,TRUE,FALSE,NULL);
    if(!quit||!stop||!entered||!gate)goto done;
    WORD machine=sizeof(void *)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386;
    DWORD host_pid=0,host_thread=GetWindowThreadProcessId(window,&host_pid);
    win32_process_global_identity identity;
    DWORD admission=win32_process_global_policy_inspect(&policy,window,host_pid,host_thread,machine,&identity,host_allowed);
    if(admission)printf("UWP_GLOBAL_ADMISSION error=%lu\n",(unsigned long)admission);
    const style_uwp_global_services services={machine,host_allowed,active,change,NULL,&policy};
    DWORD error=style_uwp_global_start(&policy,image,GetThreadDesktop(GetCurrentThreadId()),quit,stop,&services,&manager);
    fail_apply=1;
    if(error){printf("UWP_GLOBAL_START error=%lu\n",(unsigned long)error);goto done;}
    if(!style_uwp_global_observe(manager,window)){printf("UWP_GLOBAL_OBSERVE error=%lu\n",(unsigned long)GetLastError());goto done;}
    error=style_uwp_global_wait_idle(manager,5000);
    if(error){printf("UWP_GLOBAL_WAIT error=%lu\n",(unsigned long)error);goto done;}
    LONG queued=0,ready=0,failed=0;style_uwp_global_stats_read(manager,&queued,&ready,&failed);
    printf("UWP_GLOBAL_FIRST queued=%ld ready=%ld failed=%ld applies=%ld\n",queued,ready,failed,applies);
    if(queued||ready||failed!=1||applies!=1)goto done;
    if(!style_uwp_global_observe(manager,window)||style_uwp_global_wait_idle(manager,5000))goto done;
    queued=ready=failed=0;style_uwp_global_stats_read(manager,&queued,&ready,&failed);
    if(queued||ready!=1||failed||applies!=2)goto done;
    if(!style_uwp_global_observe(manager,window)||style_uwp_global_wait_idle(manager,5000)||applies!=2)goto done;
    fail_remove=1;SetEvent(stop);
    if(style_uwp_global_stop(manager,5000)!=ERROR_ACCESS_DENIED || removes!=1)goto done;
    if(style_uwp_global_stop(manager,5000))goto done;
    manager=NULL;
    if(removes!=2)goto done;
    ResetEvent(stop);block_apply=1;
    if(style_uwp_global_start(&policy,image,GetThreadDesktop(GetCurrentThreadId()),quit,stop,&services,&manager) ||
       !style_uwp_global_observe(manager,window) || WaitForSingleObject(entered,5000)!=WAIT_OBJECT_0)goto done;
    if(style_uwp_global_wait_idle(manager,10)!=ERROR_TIMEOUT)goto done;
    SetEvent(stop);if(style_uwp_global_stop(manager,10)!=ERROR_TIMEOUT)goto done;
    SetEvent(gate);if(style_uwp_global_stop(manager,5000))goto done;
    manager=NULL;block_apply=0;
    if(applies!=3 || removes!=3)goto done;
    ResetEvent(stop);
    if(style_uwp_global_start(&policy,image,GetThreadDesktop(GetCurrentThreadId()),quit,stop,&services,&manager) ||
       !style_uwp_global_observe(manager,window) || style_uwp_global_wait_idle(manager,5000))goto done;
    if(applies!=4)goto done;
    PostMessageW(window,WM_CLOSE,0,0);
    if(WaitForSingleObject(process.hProcess,5000)!=WAIT_OBJECT_0)goto done;
    window=NULL;SetEvent(stop);if(style_uwp_global_stop(manager,5000))goto done;
    manager=NULL;
    if(removes!=3)goto done;
    result=0;
done:
    if(gate)SetEvent(gate);
    if(manager){if(stop)SetEvent(stop);if(style_uwp_global_stop(manager,5000))result=5;}
    if(window)PostMessageW(window,WM_CLOSE,0,0);
    if(WaitForSingleObject(process.hProcess,5000)!=WAIT_OBJECT_0){TerminateProcess(process.hProcess,1);WaitForSingleObject(process.hProcess,1000);result=6;}
    if(quit)CloseHandle(quit);
    if(stop)CloseHandle(stop);
    if(entered)CloseHandle(entered);
    if(gate)CloseHandle(gate);
    CloseHandle(process.hThread);CloseHandle(process.hProcess);
    printf("UWP_GLOBAL_SHARED result=%d apply=%ld remove=%ld full_sequence=%d\n",result,applies,removes,result==0);return result;
}
