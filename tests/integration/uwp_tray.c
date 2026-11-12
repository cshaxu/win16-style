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
static BOOL mouse_menu;
static BOOL wait_ready(void)
{
    WCHAR name[NTW16S_GLOBAL_NAME_CAP];if(!ntw16s_global_name(name,ARRAYSIZE(name)))return FALSE;
    ULONGLONG end=GetTickCount64()+15000;
    do{
        HANDLE mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
        ntw16s_global_session *s=mapping?MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*s)):NULL;
        BOOL ready=s&&s->magic==NTW16S_GLOBAL_MAGIC&&s->version==NTW16S_GLOBAL_VERSION&&
            s->owner_pid==controller_pid&&s->phase==NTW16S_GLOBAL_ACTIVE&&s->ready;
        if(s)UnmapViewOfFile(s);
        if(mapping)CloseHandle(mapping);
        if(ready)return TRUE;
        Sleep(20);
    }while(GetTickCount64()<end);
    return FALSE;
}
static BOOL wait_ready_style(DWORD style)
{
    WCHAR name[NTW16S_GLOBAL_NAME_CAP];if(!ntw16s_global_name(name,ARRAYSIZE(name)))return FALSE;
    ULONGLONG started=GetTickCount64(),end=started+90000;
    do{HANDLE mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
        ntw16s_global_session *s=mapping?MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*s)):NULL;
        BOOL ready=s && s->magic==NTW16S_GLOBAL_MAGIC && s->version==NTW16S_GLOBAL_VERSION &&
            s->owner_pid==controller_pid && s->phase==NTW16S_GLOBAL_ACTIVE && s->ready && s->style==style;
        if(s)UnmapViewOfFile(s);
        if(mapping)CloseHandle(mapping);
        if(ready){printf("SESSION_READY style=%lu wait_ms=%llu\n",(unsigned long)style,(unsigned long long)(GetTickCount64()-started));return TRUE;}
        Sleep(50);
    }while(GetTickCount64()<end);
    return FALSE;
}
static BOOL wait_session_stopped(void)
{
    WCHAR name[NTW16S_GLOBAL_NAME_CAP],request[NTW16S_GLOBAL_NAME_CAP+16];
    if(!ntw16s_global_name(name,ARRAYSIZE(name))||swprintf(request,ARRAYSIZE(request),L"%ls.request",name)<0)return FALSE;
    ULONGLONG end=GetTickCount64()+20000;
    do{HANDLE event=OpenEventW(SYNCHRONIZE,FALSE,request);
        if(!event)return GetLastError()==ERROR_FILE_NOT_FOUND;
        CloseHandle(event);Sleep(50);
    }while(GetTickCount64()<end);
    return FALSE;
}
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
/* expected: 0 = neither, 1 = Win16, 2 = Win2000; item: 0..2. */
static BOOL choose_item(unsigned expected,unsigned item)
{
    menu_window=NULL;
    for(unsigned attempt=0;attempt<(mouse_menu?2u:1u);++attempt){
    if(mouse_menu){
        NOTIFYICONIDENTIFIER icon={.cbSize=sizeof(icon),.hWnd=tray,.uID=1};RECT rect;
        HRESULT status=E_FAIL;ULONGLONG deadline=GetTickCount64()+5000;
        do{status=Shell_NotifyIconGetRect(&icon,&rect);if(SUCCEEDED(status))break;pump(50);}while(GetTickCount64()<deadline);
        if(FAILED(status)){printf("TRAY_MOUSE_LOCATION_FAIL hr=%lx\n",(unsigned long)status);return FALSE;}
        if(!SetCursorPos((rect.left+rect.right)/2,(rect.top+rect.bottom)/2)){puts("TRAY_MOUSE_POSITION_FAIL");return FALSE;}
        printf("TRAY_MOUSE_RECT %ld,%ld,%ld,%ld\n",rect.left,rect.top,rect.right,rect.bottom);
        pump(150);INPUT input[2]={{0}};input[0].type=INPUT_MOUSE;input[0].mi.dwFlags=MOUSEEVENTF_LEFTDOWN;
        input[1].type=INPUT_MOUSE;input[1].mi.dwFlags=MOUSEEVENTF_LEFTUP;
        if(SendInput(ARRAYSIZE(input),input,sizeof(input[0]))!=ARRAYSIZE(input))return FALSE;
    }else if(!PostMessageW(tray,WM_APP+53,1,WM_RBUTTONUP))return FALSE;
    ULONGLONG end=GetTickCount64()+3000;
    do{EnumWindows(find_menu,0);if(menu_window)break;pump(20);}while(GetTickCount64()<end);
    if(menu_window)break;
    }
    if(!menu_window){puts("TRAY_MENU_NOT_FOUND");return FALSE;}
    DWORD_PTR result=0;
    if(!SendMessageTimeoutW(menu_window,0x01e1,0,0,SMTO_ABORTIFHUNG,500,&result))return FALSE;
    HMENU menu=(HMENU)result;UINT state=GetMenuState(menu,1001,MF_BYCOMMAND);
    UINT state2k=GetMenuState(menu,1003,MF_BYCOMMAND);
    WCHAR first[64]={0},second[64]={0},third[64]={0};
    printf("MENU win16_checked=%d win2k_checked=%d disabled=%d expected=%d exit=%d\n",
        !!(state&MF_CHECKED),!!(state2k&MF_CHECKED),!!(state&(MF_GRAYED|MF_DISABLED)),expected,item==2);
    if(state==0xffffffffu||state2k==0xffffffffu||GetMenuItemCount(menu)!=3||
       !GetMenuStringW(menu,0,first,ARRAYSIZE(first),MF_BYPOSITION)||
       !GetMenuStringW(menu,1,second,ARRAYSIZE(second),MF_BYPOSITION)||
       !GetMenuStringW(menu,2,third,ARRAYSIZE(third),MF_BYPOSITION)||
       lstrcmpW(first,L"Enable Win16 Style")||lstrcmpW(second,L"Enable Windows 2000 Style")||lstrcmpW(third,L"Exit")||
       !!(state&MF_CHECKED)!=(expected==1)||!!(state2k&MF_CHECKED)!=(expected==2)||
       (item!=2&&((state|state2k)&(MF_GRAYED|MF_DISABLED)))){
        PostMessageW(menu_window,WM_CANCELMODE,0,0);return FALSE;
    }
    MENUITEMINFOW info={.cbSize=sizeof(info),.fMask=MIIM_FTYPE|MIIM_STATE};
    GetMenuItemInfoW(menu,item,TRUE,&info);
    printf("MENU_TYPE type=%x state=%x\n",info.fType,info.fState);
    if(mouse_menu){
        RECT rect,bounds;if(!GetMenuItemRect(NULL,menu,item,&rect)||!GetWindowRect(menu_window,&bounds))return FALSE;
        POINT at={(rect.left+rect.right)/2,(rect.top+rect.bottom)/2},cursor={0};
        if(!PtInRect(&bounds,at)){UINT dpi=GetDpiForSystem();at.x=MulDiv(at.x,dpi,96);at.y=MulDiv(at.y,dpi,96);}
        if(!PtInRect(&bounds,at)||!SetCursorPos(at.x,at.y))return FALSE;
        printf("MENU_MOUSE_RECT %ld,%ld,%ld,%ld\n",rect.left,rect.top,rect.right,rect.bottom);
        pump(150);HWND hit=WindowFromPoint(at);
        GetCursorPos(&cursor);
        printf("MENU_MOUSE_HIT menu=%p hit=%p foreground=%p cursor=%ld,%ld bounds=%ld,%ld,%ld,%ld dpi=%u\n",(void *)menu_window,(void *)hit,(void *)GetForegroundWindow(),cursor.x,cursor.y,bounds.left,bounds.top,bounds.right,bounds.bottom,GetDpiForWindow(menu_window));
        if(hit!=menu_window || cursor.x!=at.x || cursor.y!=at.y)return FALSE;
        INPUT input[2]={{0}};input[0].type=INPUT_MOUSE;input[0].mi.dwFlags=MOUSEEVENTF_LEFTDOWN;
        input[1].type=INPUT_MOUSE;input[1].mi.dwFlags=MOUSEEVENTF_LEFTUP;
        UINT sent=SendInput(ARRAYSIZE(input),input,sizeof(input[0]));printf("MENU_MOUSE item=%u sent=%u\n",item,sent);return sent==ARRAYSIZE(input);
    }
    IAccessible *accessible=NULL;
    HRESULT hr=AccessibleObjectFromWindow(menu_window,OBJID_CLIENT,&IID_IAccessible,(void **)&accessible);
    if(SUCCEEDED(hr)){
        VARIANT child;VariantInit(&child);V_VT(&child)=VT_I4;V_I4(&child)=(LONG)item+1;
        BSTR name=NULL,action=NULL;VARIANT state;VariantInit(&state);LONG count=0;
        IAccessible_get_accChildCount(accessible,&count);
        HRESULT name_hr=IAccessible_get_accName(accessible,child,&name);
        HRESULT action_hr=IAccessible_get_accDefaultAction(accessible,child,&action);
        HRESULT state_hr=IAccessible_get_accState(accessible,child,&state);
        printf("MENU_PROVIDER children=%ld name_hr=%lx name=%ls action_hr=%lx action=%ls state_hr=%lx state=%ld\n",
            count,(unsigned long)name_hr,name?name:L"",(unsigned long)action_hr,action?action:L"",
            (unsigned long)state_hr,V_VT(&state)==VT_I4?V_I4(&state):-1L);
        SysFreeString(name);SysFreeString(action);VariantClear(&state);
        hr=IAccessible_accDoDefaultAction(accessible,child);IAccessible_Release(accessible);
    }
    printf("MENU_ACCESSIBLE hr=%lx\n",(unsigned long)hr);return SUCCEEDED(hr);
}
static BOOL choose(BOOL expected_checked,BOOL exit_menu)
{return choose_item(expected_checked?1u:0u,exit_menu?2u:0u);}
static void cleanup(application *app)
{
    if(!app->process)return;
    if(app->frame)PostMessageW(app->frame,WM_CLOSE,0,0);
    if(WaitForSingleObject(app->process,3000)!=WAIT_OBJECT_0){TerminateProcess(app->process,ERROR_TIMEOUT);WaitForSingleObject(app->process,1000);}
    CloseHandle(app->process);
}
typedef struct frame_target {PROCESS_INFORMATION process;HWND window;BOOL real_app,owned;} frame_target;
static BOOL CALLBACK find_real_frame(HWND window,LPARAM value)
{
    frame_target *target=(frame_target *)value;DWORD pid=0;GetWindowThreadProcessId(window,&pid);
    if(pid==target->process.dwProcessId && IsWindowVisible(window) && !GetWindow(window,GW_OWNER)){
        target->window=window;return FALSE;
    }
    return TRUE;
}
static BOOL frame_start(frame_target *target,PCWSTR image,unsigned index,BOOL real_app)
{
    WCHAR title[80],command[32768];STARTUPINFOW startup={.cb=sizeof(startup)};
    if(swprintf(title,ARRAYSIZE(title),L"PublishedTray-%lu-%u",GetCurrentProcessId(),index)<0 ||
       swprintf(command,ARRAYSIZE(command),real_app?L"\"%ls\"":L"\"%ls\" %ls",image,title)<0 ||
       !CreateProcessW(image,command,NULL,NULL,FALSE,0,NULL,NULL,&startup,&target->process))return FALSE;
    target->real_app=real_app;
    target->owned=TRUE;
    ULONGLONG end=GetTickCount64()+10000;
    do{if(real_app)EnumWindows(find_real_frame,(LPARAM)target);else target->window=FindWindowW(L"Win16GlobalE2ETarget",title);
       if(target->window)return TRUE;
       if(WaitForSingleObject(target->process.hProcess,0)==WAIT_OBJECT_0)break;
       pump(30);}while(GetTickCount64()<end);
    return FALSE;
}
static BOOL frame_bind_existing(frame_target *target,DWORD pid,PCWSTR image)
{
    WCHAR actual[32768];DWORD size=ARRAYSIZE(actual);
    target->process.hProcess=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,pid);
    if(!target->process.hProcess)return FALSE;
    target->process.dwProcessId=pid;target->real_app=TRUE;
    if(!QueryFullProcessImageNameW(target->process.hProcess,0,actual,&size)||lstrcmpiW(actual,image))return FALSE;
    EnumWindows(find_real_frame,(LPARAM)target);return target->window!=NULL;
}
static BOOL frame_wait(frame_target *target,UINT message,int expected)
{
    ULONGLONG end=GetTickCount64()+20000;DWORD_PTR last=0;BOOL received=FALSE;
    do{if(target->real_app){BOOL classic=GetPropW(target->window,L"Win2kStyle.Frame")!=NULL;
           BOOL managed=GetPropW(target->window,L"StyleCommon.GlobalWindow")!=NULL;
           last=message==WM_APP+81?classic:managed&&!classic;received=IsWindow(target->window);}
       else received=SendMessageTimeoutW(target->window,message,0,0,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,500,&last)!=0;
       if(received && (int)last==expected)return TRUE;
       pump(30);}while(GetTickCount64()<end);
    printf("FRAME_WAIT_FAIL pid=%lu message=%u expected=%d received=%d last=%llu error=%lu\n",target->process.dwProcessId,message,expected,received,(unsigned long long)last,GetLastError());
    return FALSE;
}
static BOOL frame_pixels(frame_target *targets,COLORREF pixels[2],const char *stage)
{
    for(unsigned i=0;i<2;++i){DWORD_PTR value=CLR_INVALID;
        if(targets[i].real_app){RECT rect;HDC dc=GetWindowDC(targets[i].window);
            if(!dc || !GetWindowRect(targets[i].window,&rect)){if(dc)ReleaseDC(targets[i].window,dc);return FALSE;}
            value=GetPixel(dc,(rect.right-rect.left)/2,10);ReleaseDC(targets[i].window,dc);
        }else if(!SendMessageTimeoutW(targets[i].window,WM_APP+82,0,0,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,1000,&value))return FALSE;
        if(value==CLR_INVALID)return FALSE;
        pixels[i]=(COLORREF)value;
    }
    printf("CAPTION_PIXELS %s first=%06lx second=%06lx\n",stage,(unsigned long)pixels[0],(unsigned long)pixels[1]);return TRUE;
}
static void frame_cleanup(frame_target *target)
{
    if(!target->owned){if(target->process.hProcess)CloseHandle(target->process.hProcess);return;}
    if(target->window && IsWindow(target->window))PostMessageW(target->window,WM_CLOSE,0,0);
    if(target->process.hProcess){if(WaitForSingleObject(target->process.hProcess,3000)!=WAIT_OBJECT_0){
        TerminateProcess(target->process.hProcess,ERROR_TIMEOUT);WaitForSingleObject(target->process.hProcess,1000);}
        CloseHandle(target->process.hThread);CloseHandle(target->process.hProcess);}
}
int wmain(int argc,WCHAR **argv)
{
    BOOL dpi_set=SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    BOOL menu_only=argc==4&&!wcscmp(argv[3],L"--menu-only");
    BOOL rapid=argc==6&&!wcscmp(argv[3],L"--rapid");
    BOOL early_switch=argc==6&&!wcscmp(argv[3],L"--early-switch");
    BOOL mouse_existing=argc==6&&!wcscmp(argv[3],L"--sol-mouse");
    BOOL mouse_new=argc==5&&!wcscmp(argv[3],L"--sol-mouse-new");
    mouse_menu=mouse_existing||mouse_new;
    BOOL existing=mouse_existing||(argc==6&&!wcscmp(argv[3],L"--sol-existing"));
    BOOL sol=existing||mouse_new||(argc==5&&!wcscmp(argv[3],L"--sol"));
    BOOL styles=sol||rapid||early_switch||(argc==6&&!wcscmp(argv[3],L"--styles"));
    BOOL window_events=argc==4&&!wcscmp(argv[3],L"--window-events");
    BOOL toggle=window_events||(argc==4&&!wcscmp(argv[3],L"--toggle"));
    if(argc!=3&&!menu_only&&!toggle&&!styles){puts("SKIP opt-in executable and log path required");return 77;}
    if(!_wfreopen(argv[2],L"w",stdout))return 1;
    setvbuf(stdout,NULL,_IONBF,0);
    printf("DPI_SET ok=%d awareness=%d\n",dpi_set,(int)GetAwarenessFromDpiAwarenessContext(GetThreadDpiAwarenessContext()));
    HANDLE token=NULL;TOKEN_ELEVATION elevation={0};DWORD bytes=0;
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token))return 1;
    BOOL elevated=GetTokenInformation(token,TokenElevation,&elevation,sizeof(elevation),&bytes)&&elevation.TokenIsElevated;
    CloseHandle(token);
    printf("TOKEN elevated=%d window_events_only=%d\n",elevated,window_events);
    if(!elevated&&!menu_only&&!window_events){puts("SKIP elevated probe required");return 77;}
    HDESK input=OpenInputDesktop(0,FALSE,DESKTOP_READOBJECTS);WCHAR desktop[80]={0};
    if(input){GetUserObjectInformationW(input,UOI_NAME,desktop,sizeof(desktop),&bytes);CloseDesktop(input);}
    if(wcscmp(desktop,L"Default")||!GetForegroundWindow()){puts("SKIP normal input desktop required");return 77;}
    if(FindWindowW(L"LibTrayWindow",L"Win16 Style")||
       (!menu_only&&!styles&&(image_present(L"CalculatorApp.exe")||image_present(L"ScanApp.exe")))){
        puts("SKIP existing controller or target");return 77;
    }
    HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(hr))return 1;
    POINT pointer;BOOL restore_pointer=GetCursorPos(&pointer);
    application apps[2]={0};frame_target frames_test[3]={0};COLORREF pixels16[2],pixels2k[2],pixels_return[2];PROCESS_INFORMATION process={0};BOOL ok=FALSE,enabled=FALSE;
    HANDLE output=INVALID_HANDLE_VALUE,input_handle=INVALID_HANDLE_VALUE;
    if(styles && (!(existing?frame_bind_existing(&frames_test[0],wcstoul(argv[5],NULL,10),argv[4]):frame_start(&frames_test[0],argv[4],0,sol))||
                  !frame_start(&frames_test[1],sol?argv[4]:argv[5],1,sol)))goto done;
    if(styles && (!frame_wait(&frames_test[0],WM_APP+71,0)||!frame_wait(&frames_test[1],WM_APP+71,0)||
                  !frame_wait(&frames_test[0],WM_APP+81,0)||!frame_wait(&frames_test[1],WM_APP+81,0)))goto done;
    if(!menu_only&&!styles&&(!activate(&apps[0],L"Microsoft.WindowsCalculator_8wekyb3d8bbwe!App")||styled(&apps[0])))goto done;
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
    if(FAILED(hr)||rect.right<=rect.left||rect.bottom<=rect.top)puts("TRAY_ICON_LOCATION_UNVERIFIED");
    if(menu_only)goto exit_tray;
    if(early_switch){
        if(!choose_item(0,0))goto done;
        enabled=TRUE;pump(250);
        if(!choose_item(1,1))goto done;
        ULONGLONG six=GetTickCount64()+6000;
        while(GetTickCount64()<six)pump(50);
        for(unsigned i=0;i<2;++i){DWORD_PTR value=0;
            BOOL sent=SendMessageTimeoutW(frames_test[i].window,WM_APP+81,0,0,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,500,&value)!=0;
            printf("EARLY_6S frame=%u sent=%d win2k=%llu\n",i,sent,(unsigned long long)value);
        }
        if(!wait_ready_style(NTW16S_GLOBAL_STYLE_2K)){puts("EARLY_SWITCH_SESSION_FAIL");goto done;}
        if(!frame_wait(&frames_test[0],WM_APP+81,1)||!frame_wait(&frames_test[1],WM_APP+81,1)||
           !frame_pixels(frames_test,pixels2k,"early-win2k"))goto done;
        puts("PUBLISHED_EARLY_SWITCH_2K_PASS");pump(500);
        if(!choose_item(2,1))goto done;
        if(!frame_wait(&frames_test[0],WM_APP+81,0)||!frame_wait(&frames_test[1],WM_APP+81,0)||!wait_session_stopped())goto done;
        enabled=FALSE;goto exit_tray;
    }
    if(rapid){
        if(!choose_item(0,0))goto done;
        enabled=TRUE;
        pump(250);if(!choose_item(1,1))goto done;
        pump(250);if(!choose_item(2,0)||!wait_ready_style(NTW16S_GLOBAL_STYLE_16))goto done;
        if(!frame_wait(&frames_test[0],WM_APP+71,1)||!frame_wait(&frames_test[1],WM_APP+71,1)||
           !frame_pixels(frames_test,pixels16,"rapid-win16"))goto done;
        puts("PUBLISHED_STYLES_RAPID_LAST_CHOICE_PASS");
        pump(500);if(!choose_item(1,0))goto done;
        if(!frame_wait(&frames_test[0],WM_APP+71,0)||!frame_wait(&frames_test[1],WM_APP+71,0)||!wait_session_stopped())goto done;
        enabled=FALSE;goto exit_tray;
    }
    if(styles){
        if(!choose_item(0,0))goto done;
        enabled=TRUE;
        if(!wait_ready()||!frame_wait(&frames_test[0],WM_APP+71,1)||!frame_wait(&frames_test[1],WM_APP+71,1)||
           !frame_pixels(frames_test,pixels16,"win16"))goto done;
        pump(500);if(!choose_item(1,1)||!wait_ready_style(NTW16S_GLOBAL_STYLE_2K))goto done;
        if(!frame_wait(&frames_test[0],WM_APP+81,1)||!frame_wait(&frames_test[1],WM_APP+81,1)||
           !frame_wait(&frames_test[0],WM_APP+71,0)||!frame_wait(&frames_test[1],WM_APP+71,0)||
           !frame_pixels(frames_test,pixels2k,"win2k")||pixels16[0]==pixels2k[0]||pixels16[1]==pixels2k[1])goto done;
        puts("PUBLISHED_STYLES_SWITCH_2K_PASS");
        if(!frame_start(&frames_test[2],sol?argv[4]:argv[5],2,sol)||!frame_wait(&frames_test[2],WM_APP+81,1))goto done;
        puts("PUBLISHED_STYLES_NEW_WINDOW_PASS");if(!wait_ready())goto done;
        pump(500);if(!choose_item(2,0))goto done;
        if(!wait_ready_style(NTW16S_GLOBAL_STYLE_16)){puts("WAIT_READY_16_FAIL");goto done;}
        for(unsigned i=0;i<ARRAYSIZE(frames_test);++i)
            if(!frame_wait(&frames_test[i],WM_APP+81,0)||!frame_wait(&frames_test[i],WM_APP+71,1))goto done;
        if(!frame_pixels(frames_test,pixels_return,"win16-after-win2k")||
           pixels_return[0]==pixels2k[0]||pixels_return[1]==pixels2k[1])goto done;
        puts("PUBLISHED_STYLES_SWITCH_16_PASS");if(!wait_ready())goto done;
        pump(500);if(!choose_item(1,0))goto done;
        for(unsigned i=0;i<ARRAYSIZE(frames_test);++i)if(!frame_wait(&frames_test[i],WM_APP+71,0))goto done;
        if(!wait_session_stopped())goto done;
        enabled=FALSE;puts("PUBLISHED_STYLES_DISABLE_16_PASS");pump(500);
        if(!choose_item(0,1))goto done;
        enabled=TRUE;
        if(!wait_ready_style(NTW16S_GLOBAL_STYLE_2K))goto done;
        for(unsigned i=0;i<ARRAYSIZE(frames_test);++i)if(!frame_wait(&frames_test[i],WM_APP+81,1))goto done;
        puts("PUBLISHED_STYLES_NORMAL_TO_2K_PASS");pump(500);if(!choose_item(2,1))goto done;
        for(unsigned i=0;i<ARRAYSIZE(frames_test);++i)if(!frame_wait(&frames_test[i],WM_APP+81,0))goto done;
        if(!wait_session_stopped())goto done;
        enabled=FALSE;puts("PUBLISHED_STYLES_DISABLE_2K_PASS");goto exit_tray;
    }
    if(styled(&apps[0])||!choose(FALSE,FALSE))goto done;
    enabled=TRUE;if(!wait_style(&apps[0],TRUE))goto done;
    puts("EXISTING_UWP_STYLED");
    if(!activate(&apps[1],L"Microsoft.WindowsScan_8wekyb3d8bbwe!App")||!wait_style(&apps[1],TRUE))goto done;
    puts("NEW_UWP_STYLED");
    HWND first=apps[0].surface,second=apps[1].surface;
    if(toggle&&!wait_ready()){puts("FAIL session not ready for Uncheck");goto done;}
    /* The controller may still be finishing discovery when the first surface
       appears. Allow its UI to process the ready notification before the menu. */
    pump(250);
    /* Toggle exercises the real menu action; the default mode separately
       exercises the owned controller shutdown event. */
    BOOL stopped=toggle?choose(TRUE,FALSE):stop_owned_controller();printf("STOP_REQUEST completed=%d\n",stopped);
    if(!stopped||!wait_style(&apps[0],FALSE)||!wait_style(&apps[1],FALSE))goto done;
    enabled=FALSE;
    printf("REMOVE first_exists=%d second_exists=%d\n",IsWindow(first),IsWindow(second));
    if(IsWindow(first)||IsWindow(second))goto done;
    if(toggle){
        if(WaitForSingleObject(process.hProcess,0)!=WAIT_TIMEOUT||!IsWindow(tray))goto done;
        puts("UNCHECK_RESIDENT_PASS");
        pump(250);
        if(!choose(FALSE,FALSE))goto done;
        enabled=TRUE;
        if(!wait_style(&apps[0],TRUE)||!wait_style(&apps[1],TRUE)||!wait_ready())goto done;
        puts("REENABLE_EXISTING_UWP_PASS");
        first=apps[0].surface;second=apps[1].surface;
        pump(250);
        if(!choose(TRUE,FALSE)||!wait_style(&apps[0],FALSE)||!wait_style(&apps[1],FALSE))goto done;
        enabled=FALSE;
        if(IsWindow(first)||IsWindow(second)||WaitForSingleObject(process.hProcess,0)!=WAIT_TIMEOUT||!IsWindow(tray))goto done;
        puts("SECOND_UNCHECK_RESIDENT_PASS");goto exit_tray;
    }
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
        if(enabled&&tray){if(stop_owned_controller()){
            if(!styles){wait_style(&apps[0],FALSE);if(apps[1].frame)wait_style(&apps[1],FALSE);}
            enabled=FALSE;}}
        if(tray)choose(enabled,TRUE);
        if(WaitForSingleObject(process.hProcess,5000)!=WAIT_OBJECT_0){TerminateProcess(process.hProcess,ERROR_TIMEOUT);WaitForSingleObject(process.hProcess,1000);}
    }
    if(process.hThread)CloseHandle(process.hThread);
    if(process.hProcess)CloseHandle(process.hProcess);
    if(output!=INVALID_HANDLE_VALUE)CloseHandle(output);
    if(input_handle!=INVALID_HANDLE_VALUE)CloseHandle(input_handle);
    cleanup(&apps[1]);cleanup(&apps[0]);for(unsigned i=0;i<ARRAYSIZE(frames_test);++i)frame_cleanup(&frames_test[i]);CoUninitialize();
    if(restore_pointer)SetCursorPos(pointer.x,pointer.y);
    printf("UWP_TRAY_%s mode=%s enabled_at_cleanup=%d\n",ok?"PASS":"FAIL",styles?"published-styles":menu_only?"menu-exit":toggle?"toggle-resident":"enable-shutdown",enabled);return ok?0:1;
}
