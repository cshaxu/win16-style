#define COBJMACROS
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <oleacc.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global/session.h"

/* Opt-in interactive test. Only the newly created application/controller are
   closed; enabling the real controller uses its normal desktop-wide policy. */
typedef struct application { DWORD pid; HANDLE process; HWND frame,surface; } application;
static HWND tray,menu_window;
static DWORD controller_pid;
static BOOL stop_owned_controller(void)
{
    WCHAR name[NTW16S_GLOBAL_NAME_CAP],request[NTW16S_GLOBAL_NAME_CAP+16];
    if(!ntw16s_global_name(name,ARRAYSIZE(name))||swprintf(request,ARRAYSIZE(request),L"%ls.request",name)<0)return FALSE;
    HANDLE mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
    if(!mapping)return FALSE;
    ntw16s_global_session *session=MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*session));
    BOOL own=session&&session->magic==NTW16S_GLOBAL_MAGIC&&session->version==NTW16S_GLOBAL_VERSION&&
        session->owner_pid==controller_pid&&ntw16s_global_owner_alive(session);
    if(session)UnmapViewOfFile(session);
    CloseHandle(mapping);if(!own)return FALSE;
    HANDLE event=OpenEventW(EVENT_MODIFY_STATE,FALSE,request);if(!event)return FALSE;
    BOOL sent=SetEvent(event);CloseHandle(event);if(!sent)return FALSE;
    ULONGLONG end=GetTickCount64()+20000;
    do{
        event=OpenEventW(SYNCHRONIZE,FALSE,request);
        if(!event)return GetLastError()==ERROR_FILE_NOT_FOUND;
        CloseHandle(event);Sleep(50);
    }while(GetTickCount64()<end);
    return FALSE;
}
static void pump(DWORD milliseconds)
{
    MsgWaitForMultipleObjects(0,NULL,FALSE,milliseconds,QS_ALLINPUT);
    MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){
        TranslateMessage(&message);DispatchMessageW(&message);
    }
}
static BOOL image_present(PCWSTR image)
{
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);
    if(snapshot==INVALID_HANDLE_VALUE)return TRUE;
    PROCESSENTRY32W entry={.dwSize=sizeof(entry)};BOOL found=FALSE;
    for(BOOL n=Process32FirstW(snapshot,&entry);n;n=Process32NextW(snapshot,&entry))
        if(!_wcsicmp(entry.szExeFile,image))found=TRUE;
    CloseHandle(snapshot);return found;
}
static BOOL CALLBACK core(HWND window,LPARAM value)
{
    application *app=(application *)value;DWORD pid=0;WCHAR name[80];
    GetWindowThreadProcessId(window,&pid);GetClassNameW(window,name,ARRAYSIZE(name));
    if(pid==app->pid&&!wcscmp(name,L"Windows.UI.Core.CoreWindow"))app->frame=GetAncestor(window,GA_ROOT);
    return TRUE;
}
static BOOL CALLBACK frames(HWND window,LPARAM value)
{
    WCHAR name[80];GetClassNameW(window,name,ARRAYSIZE(name));
    if(!wcscmp(name,L"ApplicationFrameWindow")&&IsWindowVisible(window))EnumChildWindows(window,core,value);
    return TRUE;
}
static BOOL CALLBACK surfaces(HWND window,LPARAM value)
{
    application *app=(application *)value;
    if(GetWindow(window,GW_OWNER)==app->frame&&(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_LAYERED)&&IsWindowVisible(window))app->surface=window;
    return TRUE;
}
static BOOL styled(application *app)
{app->surface=NULL;EnumWindows(surfaces,(LPARAM)app);return app->surface!=NULL;}
static BOOL wait_style(application *app,BOOL wanted)
{
    ULONGLONG end=GetTickCount64()+15000;
    do{if(styled(app)==wanted)return TRUE;pump(30);}while(GetTickCount64()<end);
    return FALSE;
}
static BOOL activate(application *app,PCWSTR aumid)
{
    FILETIME start,birth,a,b,c;GetSystemTimeAsFileTime(&start);
    IApplicationActivationManager *manager=NULL;
    HRESULT hr=CoCreateInstance(&CLSID_ApplicationActivationManager,NULL,CLSCTX_LOCAL_SERVER,&IID_IApplicationActivationManager,(void **)&manager);
    if(SUCCEEDED(hr)){
        hr=IApplicationActivationManager_ActivateApplication(manager,aumid,NULL,AO_NOERRORUI,&app->pid);
        IApplicationActivationManager_Release(manager);
    }
    printf("ACTIVATE hr=%lx pid=%lu\n",(unsigned long)hr,app->pid);
    if(FAILED(hr))return FALSE;
    app->process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE|PROCESS_TERMINATE,FALSE,app->pid);
    if(!app->process)return FALSE;
    if(!GetProcessTimes(app->process,&birth,&a,&b,&c)||CompareFileTime(&birth,&start)<0){CloseHandle(app->process);app->process=NULL;return FALSE;}
    ULONGLONG end=GetTickCount64()+10000;
    do{EnumWindows(frames,(LPARAM)app);if(app->frame)break;pump(30);}while(GetTickCount64()<end);
    printf("FRAME hwnd=%p dpi=%u\n",(void *)app->frame,app->frame?GetDpiForWindow(app->frame):0);
    return app->frame!=NULL;
}
static BOOL CALLBACK find_menu(HWND window,LPARAM unused)
{
    (void)unused;DWORD pid=0;WCHAR name[40];GetWindowThreadProcessId(window,&pid);
    GetClassNameW(window,name,ARRAYSIZE(name));
    if(pid==controller_pid&&IsWindowVisible(window)&&!wcscmp(name,L"#32768"))menu_window=window;
    return TRUE;
}
static BOOL choose(BOOL expected_checked,BOOL exit_menu)
{
    menu_window=NULL;
    if(!PostMessageW(tray,WM_APP+53,1,WM_RBUTTONUP))return FALSE;
    ULONGLONG end=GetTickCount64()+3000;
    do{EnumWindows(find_menu,0);if(menu_window)break;pump(20);}while(GetTickCount64()<end);
    if(!menu_window)return FALSE;
    DWORD_PTR result=0;
    if(!SendMessageTimeoutW(menu_window,0x01e1,0,0,SMTO_ABORTIFHUNG,500,&result))return FALSE;
    HMENU menu=(HMENU)result;UINT state=GetMenuState(menu,1001,MF_BYCOMMAND);
    printf("MENU checked=%d disabled=%d expected=%d exit=%d\n",!!(state&MF_CHECKED),!!(state&(MF_GRAYED|MF_DISABLED)),expected_checked,exit_menu);
    if(state==0xffffffffu||GetMenuItemCount(menu)!=2||!!(state&MF_CHECKED)!=expected_checked||(!exit_menu&&(state&(MF_GRAYED|MF_DISABLED)))){
        PostMessageW(menu_window,WM_CANCELMODE,0,0);return FALSE;
    }
    IAccessible *accessible=NULL;
    HRESULT hr=AccessibleObjectFromWindow(menu_window,OBJID_CLIENT,&IID_IAccessible,(void **)&accessible);
    if(SUCCEEDED(hr)){
        VARIANT child;VariantInit(&child);V_VT(&child)=VT_I4;V_I4(&child)=exit_menu?2:1;
        hr=IAccessible_accDoDefaultAction(accessible,child);IAccessible_Release(accessible);
    }
    printf("MENU_ACCESSIBLE hr=%lx\n",(unsigned long)hr);return SUCCEEDED(hr);
}
static void cleanup(application *app)
{
    if(!app->process)return;
    if(app->frame)PostMessageW(app->frame,WM_CLOSE,0,0);
    if(WaitForSingleObject(app->process,3000)!=WAIT_OBJECT_0){TerminateProcess(app->process,ERROR_TIMEOUT);WaitForSingleObject(app->process,1000);}
    CloseHandle(app->process);
}
int wmain(int argc,WCHAR **argv)
{
    BOOL dpi_set=SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    BOOL menu_only=argc==4&&!wcscmp(argv[3],L"--menu-only");
    if(argc!=3&&!menu_only){puts("SKIP opt-in executable and log path required");return 77;}
    if(!_wfreopen(argv[2],L"w",stdout))return 1;
    setvbuf(stdout,NULL,_IONBF,0);
    printf("DPI_SET ok=%d awareness=%d\n",dpi_set,(int)GetAwarenessFromDpiAwarenessContext(GetThreadDpiAwarenessContext()));
    HANDLE token=NULL;TOKEN_ELEVATION elevation={0};DWORD bytes=0;
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token))return 1;
    BOOL elevated=GetTokenInformation(token,TokenElevation,&elevation,sizeof(elevation),&bytes)&&elevation.TokenIsElevated;
    CloseHandle(token);if(!elevated&&!menu_only){puts("SKIP elevated probe required");return 77;}
    HDESK input=OpenInputDesktop(0,FALSE,DESKTOP_READOBJECTS);WCHAR desktop[80]={0};
    if(input){GetUserObjectInformationW(input,UOI_NAME,desktop,sizeof(desktop),&bytes);CloseDesktop(input);}
    if(wcscmp(desktop,L"Default")||!GetForegroundWindow()){puts("SKIP normal input desktop required");return 77;}
    if(FindWindowW(L"LibTrayWindow",L"Win16 Style")||image_present(L"CalculatorApp.exe")||image_present(L"ScanApp.exe")){
        puts("SKIP existing controller or target");return 77;
    }
    HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(hr))return 1;
    POINT pointer;BOOL restore_pointer=GetCursorPos(&pointer);
    application apps[2]={0};PROCESS_INFORMATION process={0};BOOL ok=FALSE,enabled=FALSE;
    HANDLE output=INVALID_HANDLE_VALUE,input_handle=INVALID_HANDLE_VALUE;
    if(!menu_only&&(!activate(&apps[0],L"Microsoft.WindowsCalculator_8wekyb3d8bbwe!App")||styled(&apps[0])))goto done;
    WCHAR command[33000];if(swprintf(command,ARRAYSIZE(command),L"\"%ls\" --global",argv[1])<0)goto done;
    WCHAR log_path[32768];if(swprintf(log_path,ARRAYSIZE(log_path),L"%ls.controller.log",argv[2])<0)goto done;
    SECURITY_ATTRIBUTES security={.nLength=sizeof(security),.bInheritHandle=TRUE};
    output=CreateFileW(log_path,GENERIC_WRITE,FILE_SHARE_READ,&security,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
    input_handle=CreateFileW(L"NUL",GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,&security,OPEN_EXISTING,0,NULL);
    if(output==INVALID_HANDLE_VALUE||input_handle==INVALID_HANDLE_VALUE)goto done;
    STARTUPINFOW startup={.cb=sizeof(startup),.dwFlags=STARTF_USESTDHANDLES,
        .hStdInput=input_handle,.hStdOutput=output,.hStdError=output};
    if(!CreateProcessW(argv[1],command,NULL,NULL,TRUE,CREATE_NO_WINDOW,NULL,NULL,&startup,&process))goto done;
    controller_pid=process.dwProcessId;ULONGLONG end=GetTickCount64()+10000;
    do{HWND candidate=FindWindowW(L"LibTrayWindow",L"Win16 Style");DWORD pid=0;
        if(candidate)GetWindowThreadProcessId(candidate,&pid);
        if(pid==controller_pid){tray=candidate;break;}pump(30);
    }while(GetTickCount64()<end&&WaitForSingleObject(process.hProcess,0)==WAIT_TIMEOUT);
    if(!tray)goto done;
    NOTIFYICONIDENTIFIER icon={.cbSize=sizeof(icon),.hWnd=tray,.uID=1};RECT rect={0};
    hr=Shell_NotifyIconGetRect(&icon,&rect);
    printf("TRAY pid=%lu hwnd=%p icon_hr=%lx rect=%ld,%ld,%ld,%ld\n",controller_pid,(void *)tray,(unsigned long)hr,rect.left,rect.top,rect.right,rect.bottom);
    if(menu_only)goto exit_tray;
    if(styled(&apps[0])||!choose(FALSE,FALSE))goto done;
    enabled=TRUE;if(!wait_style(&apps[0],TRUE))goto done;
    puts("EXISTING_UWP_STYLED");
    if(!activate(&apps[1],L"Microsoft.WindowsScan_8wekyb3d8bbwe!App")||!wait_style(&apps[1],TRUE))goto done;
    puts("NEW_UWP_STYLED");
    HWND first=apps[0].surface,second=apps[1].surface;
    /* The controller may still be finishing discovery when the first surface
       appears. Allow its UI to process the ready notification before the menu. */
    pump(250);
    /* Exercise the exact stop event used by the menu action. MSAA invocation
       of the checked item is separately unverified (DISP_E_MEMBERNOTFOUND). */
    BOOL stopped=stop_owned_controller();printf("STOP_REQUEST completed=%d\n",stopped);
    if(!stopped||!wait_style(&apps[0],FALSE)||!wait_style(&apps[1],FALSE))goto done;
    enabled=FALSE;
    printf("REMOVE first_exists=%d second_exists=%d\n",IsWindow(first),IsWindow(second));
    if(IsWindow(first)||IsWindow(second))goto done;
    /* An external stop request does not set the tray's private stopping flag:
       the worker exits normally and the controller follows it. This is a
       shutdown test, not proof of the Uncheck-and-remain-resident UI action. */
    DWORD shutdown_code=MAXDWORD;
    if(WaitForSingleObject(process.hProcess,5000)!=WAIT_OBJECT_0||
       !GetExitCodeProcess(process.hProcess,&shutdown_code)||shutdown_code)goto done;
    puts("GLOBAL_SHUTDOWN_PASS");ok=TRUE;goto done;
exit_tray:
    pump(250);if(!choose(FALSE,TRUE))goto done;
    DWORD code=MAXDWORD;if(WaitForSingleObject(process.hProcess,5000)!=WAIT_OBJECT_0||!GetExitCodeProcess(process.hProcess,&code)||code)goto done;
    ok=TRUE;
done:
    if(process.hProcess){DWORD code=MAXDWORD;GetExitCodeProcess(process.hProcess,&code);printf("CONTROLLER_EXIT before_cleanup=%lu\n",code);}
    if(process.hProcess&&WaitForSingleObject(process.hProcess,0)==WAIT_TIMEOUT){
        if(enabled&&tray){if(stop_owned_controller()){wait_style(&apps[0],FALSE);if(apps[1].frame)wait_style(&apps[1],FALSE);enabled=FALSE;}}
        if(tray)choose(enabled,TRUE);
        if(WaitForSingleObject(process.hProcess,5000)!=WAIT_OBJECT_0){TerminateProcess(process.hProcess,ERROR_TIMEOUT);WaitForSingleObject(process.hProcess,1000);}
    }
    if(process.hThread)CloseHandle(process.hThread);
    if(process.hProcess)CloseHandle(process.hProcess);
    if(output!=INVALID_HANDLE_VALUE)CloseHandle(output);
    if(input_handle!=INVALID_HANDLE_VALUE)CloseHandle(input_handle);
    cleanup(&apps[1]);cleanup(&apps[0]);CoUninitialize();
    if(restore_pointer)SetCursorPos(pointer.x,pointer.y);
    printf("UWP_TRAY_%s mode=%s enabled_at_cleanup=%d\n",ok?"PASS":"FAIL",menu_only?"menu-exit":"enable-shutdown",enabled);return ok?0:1;
}
