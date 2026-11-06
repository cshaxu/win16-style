#include "run16s-exe/uwp.h"
#include "ntw16s-dll/uwp/hook.h"
#include "lib-win32/win16-window-uwp/frame_layout.h"
#include <stdio.h>
#include <stdint.h>
#include <tlhelp32.h>
#include <wchar.h>
static DWORD pid;
static HWND frame,core,surface;
static PCWSTR target_image=L"CalculatorApp.exe";
static BOOL resize_pointer(void)
{
    RECT r;POINT previous;
    if(!surface||!GetWindowRect(surface,&r)||!GetCursorPos(&previous))return FALSE;
    BOOL topmost=!!(GetWindowLongPtrW(frame,GWL_EXSTYLE)&WS_EX_TOPMOST);
    if(!topmost&&!SetWindowPos(frame,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE))return FALSE;
    INPUT movement={.type=INPUT_MOUSE};
    movement.mi.dwFlags=MOUSEEVENTF_MOVE|MOUSEEVENTF_ABSOLUTE|MOUSEEVENTF_VIRTUALDESK;
    movement.mi.dx=MulDiv(r.left+2-GetSystemMetrics(SM_XVIRTUALSCREEN),65535,GetSystemMetrics(SM_CXVIRTUALSCREEN)-1);
    movement.mi.dy=MulDiv((r.top+r.bottom)/2-GetSystemMetrics(SM_YVIRTUALSCREEN),65535,GetSystemMetrics(SM_CYVIRTUALSCREEN)-1);
    BOOL sent=SendInput(1,&movement,sizeof(movement))==1,ok=FALSE;
    DWORD last_error=0,flags=0;HCURSOR last=NULL;
    ULONGLONG deadline=GetTickCount64()+500;
    while(sent&&GetTickCount64()<deadline){
        /* cbSize is an input on every call; do not reuse an output structure. */
        CURSORINFO cursor={.cbSize=sizeof(cursor)};
        if(GetCursorInfo(&cursor)){
            flags=cursor.flags;last=cursor.hCursor;
            POINT p=cursor.ptScreenPos;
            BOOL edge=p.x>=r.left&&p.x<r.left+8&&p.y>r.top+40&&p.y<r.bottom-40;
            if(edge&&WindowFromPoint(p)==surface&&(flags&CURSOR_SHOWING)&&last==LoadCursorW(NULL,IDC_SIZEWE)){ok=TRUE;break;}
        }else last_error=GetLastError();
        MsgWaitForMultipleObjects(0,NULL,FALSE,20,0);
    }
    BOOL restored=SetCursorPos(previous.x,previous.y);
    if(!topmost)restored=SetWindowPos(frame,HWND_NOTOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE)&&restored;
    printf("UWP_RESIZE_POINTER sent=%d match=%d flags=%lu cursor=%p error=%lu restored=%d\n",sent,ok,flags,(void*)last,last_error,restored);
    return ok&&restored;
}
static BOOL copied_entry(PCWSTR system_image)
{
    WCHAR root[MAX_PATH],directory[MAX_PATH],copy[MAX_PATH];
    if(!GetTempPathW(ARRAYSIZE(root),root)||!GetTempFileNameW(root,L"wca",0,directory))return FALSE;
    DeleteFileW(directory);
    if(!CreateDirectoryW(directory,NULL))return FALSE;
    BOOL ok=swprintf(copy,ARRAYSIZE(copy),L"%ls\\calc.exe",directory)>0&&CopyFileW(system_image,copy,FALSE);
    if(ok)ok=run16s_uwp_calculator_image(copy);
    HANDLE file=CreateFileW(copy,GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);DWORD written=0;BYTE byte=0;
    if(file!=INVALID_HANDLE_VALUE){ok=WriteFile(file,&byte,1,&written,NULL)&&written==1&&ok;CloseHandle(file);}
    else ok=FALSE;
    ok=!run16s_uwp_calculator_image(copy)&&ok;
    DeleteFileW(copy);RemoveDirectoryW(directory);return ok;
}
static BOOL CALLBACK child(HWND w,LPARAM unused)
{
    (void)unused;DWORD id=0;WCHAR name[80];GetWindowThreadProcessId(w,&id);GetClassNameW(w,name,80);
    if(id==pid&&!wcscmp(name,L"Windows.UI.Core.CoreWindow")){core=w;frame=GetAncestor(w,GA_ROOT);return FALSE;}
    return TRUE;
}
static BOOL CALLBACK top(HWND w,LPARAM unused)
{child(w,unused);if(!frame)EnumChildWindows(w,child,0);return !frame;}
static BOOL CALLBACK owned(HWND w,LPARAM unused)
{
    (void)unused;
    if(GetWindow(w,GW_OWNER)==frame&&(GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_LAYERED))surface=w;
    return TRUE;
}
static DWORD target_pid(void)
{
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);if(snapshot==INVALID_HANDLE_VALUE)return MAXDWORD;
    PROCESSENTRY32W entry={.dwSize=sizeof(entry)};DWORD found=0;
    for(BOOL next=Process32FirstW(snapshot,&entry);next;next=Process32NextW(snapshot,&entry))
        if(!_wcsicmp(entry.szExeFile,target_image)){found=entry.th32ProcessID;break;}
    CloseHandle(snapshot);return found;
}
static BOOL button(int hit)
{
    RECT outer,client,band;HWND title=NULL;
    for(HWND w=GetWindow(frame,GW_CHILD);w;w=GetWindow(w,GW_HWNDNEXT)){
        WCHAR name[80];GetClassNameW(w,name,80);
        if(IsWindowVisible(w)&&!wcscmp(name,L"ApplicationFrameTitleBarWindow")){title=w;break;}
    }
    if(!title||!IsWindow(surface)||!IsWindowVisible(surface))return FALSE;
    GetWindowRect(surface,&outer);GetWindowRect(title,&band);GetClientRect(frame,&client);
    POINT origin={0,0};ClientToScreen(frame,&origin);OffsetRect(&client,origin.x-outer.left,origin.y-outer.top);client.top=band.bottom-outer.top;
    SIZE size={outer.right-outer.left,outer.bottom-outer.top};win16_style_layout layout;
    if(!win16_uwp_frame_layout(size,GetDpiForWindow(frame),(DWORD)GetWindowLongPtrW(frame,GWL_STYLE),IsZoomed(frame),client,&layout))return FALSE;
    RECT r=hit==HTMAXBUTTON?layout.maximize:layout.minimize;
    LPARAM point=MAKELPARAM((r.left+r.right)/2,(r.top+r.bottom)/2);DWORD_PTR ignored;
    return SendMessageTimeoutW(surface,WM_LBUTTONDOWN,MK_LBUTTON,point,SMTO_ABORTIFHUNG,1000,&ignored)&&
        SendMessageTimeoutW(surface,WM_LBUTTONUP,0,point,SMTO_ABORTIFHUNG,1000,&ignored);
}
static BOOL await_state(BOOL minimized,BOOL maximized)
{
    ULONGLONG deadline=GetTickCount64()+1500;
    do{
        RECT expected,actual;BOOL geometry=FALSE;
        if(GetWindowRect(frame,&expected)&&GetWindowRect(surface,&actual)){
            if(maximized){MONITORINFO monitor={.cbSize=sizeof(monitor)};RECT clipped;
                if(GetMonitorInfoW(MonitorFromWindow(frame,MONITOR_DEFAULTTONEAREST),&monitor)&&
                   IntersectRect(&clipped,&expected,&monitor.rcWork))expected=clipped;
            }
            geometry=EqualRect(&expected,&actual);
        }
        if(IsIconic(frame)==minimized&&IsZoomed(frame)==maximized&&
           (minimized||(geometry&&GetAncestor(core,GA_ROOT)==frame&&IsWindowVisible(surface))))return TRUE;
        MsgWaitForMultipleObjects(0,NULL,FALSE,20,QS_ALLINPUT);
        MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
    }while(GetTickCount64()<deadline);
    return FALSE;
}
static void titles(PCSTR phase)
{
    printf("FRAME phase=%s hwnd=%p surface_exists=%d\n",phase,(void*)frame,IsWindow(surface));
    for(HWND w=GetWindow(frame,GW_CHILD);w;w=GetWindow(w,GW_HWNDNEXT)){
        WCHAR name[80];GetClassNameW(w,name,80);
        if(!wcscmp(name,L"ApplicationFrameTitleBarWindow"))printf("TITLE hwnd=%p visible=%d\n",(void*)w,IsWindowVisible(w));
    }
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==2&&!wcscmp(argv[1],L"--resolve")){
        WCHAR path[MAX_PATH];UINT n=GetSystemDirectoryW(path,ARRAYSIZE(path));
        if(!n||n+10>=ARRAYSIZE(path))return 1;
        wcscpy(path+n,L"\\calc.exe");
        BOOL ok=run16s_uwp_calculator_image(path)&&copied_entry(path);CharUpperBuffW(path,(DWORD)wcslen(path));
        ok=ok&&run16s_uwp_calculator_image(path);
        ok=ok&&!run16s_uwp_calculator_image(NULL)&&!run16s_uwp_calculator_image(L"calc.exe")&&
            !run16s_uwp_calculator_image(L"C:\\other\\calc.exe")&&!run16s_uwp_calculator_image(L"C:\\other\\notepad.exe");
        BOOL handled=TRUE;
        ok=ok&&!run16s_uwp_command(L"C:\\other\\calc.exe",L"",FALSE,&handled)&&!handled;
        printf("UWP_CALCULATOR_RESOLVE_%s\n",ok?"PASS":"FAIL");return ok?0:1;
    }
    BOOL command_mode=argc==3&&!wcscmp(argv[1],L"--command-api");
    BOOL pointer_mode=argc==4&&!wcscmp(argv[1],L"--activate-pointer");
    BOOL api_only=command_mode||(argc>=2&&!wcscmp(argv[1],L"--activate-api"));
    PCWSTR app_id=L"Microsoft.WindowsCalculator_8wekyb3d8bbwe!App";
    if(argc==4&&(!wcscmp(argv[1],L"--activate-api")||pointer_mode)){
        app_id=argv[2];target_image=argv[3];
        if(!wcschr(app_id,L'!')||!*target_image||wcschr(target_image,L'\\')||wcschr(target_image,L'/'))return 2;
    }else if((argc!=2&&!command_mode)||(!api_only&&wcscmp(argv[1],L"--activate"))){puts("SKIP use --activate, --activate-api [AUMID image.exe], or --command-api launcher");return 77;}
    HDESK desktop=OpenInputDesktop(0,FALSE,DESKTOP_READOBJECTS);WCHAR name[80]={0};DWORD needed;
    if(desktop){GetUserObjectInformationW(desktop,UOI_NAME,name,sizeof(name),&needed);CloseDesktop(desktop);}
    if((!api_only&&wcscmp(name,L"Default"))||target_pid()){puts("SKIP desktop or existing target");return 77;}
    if(api_only)puts("DISPLAY_UNVERIFIED API-only activation; no physical input or screenshot assertions");
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    DWORD error=0;
    if(command_mode){
        WCHAR command_line[2048];STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child_process={0};
        int n=swprintf(command_line,ARRAYSIZE(command_line),L"\"%ls\" calc.exe",argv[2]);
        FILETIME started;GetSystemTimeAsFileTime(&started);
        if(n<=0||(size_t)n>=ARRAYSIZE(command_line)||!CreateProcessW(argv[2],command_line,NULL,NULL,FALSE,0,NULL,NULL,&startup,&child_process))return 1;
        CloseHandle(child_process.hThread);DWORD code=~0u;
        if(WaitForSingleObject(child_process.hProcess,15000)==WAIT_OBJECT_0)GetExitCodeProcess(child_process.hProcess,&code);
        else {TerminateProcess(child_process.hProcess,1);WaitForSingleObject(child_process.hProcess,1000);}
        CloseHandle(child_process.hProcess);printf("CALC_COMMAND exit=%lu\n",code);
        pid=target_pid();
        HANDLE candidate=pid&&pid!=MAXDWORD?OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid):NULL;
        FILETIME created,exited,kernel,user;
        BOOL fresh=candidate&&GetProcessTimes(candidate,&created,&exited,&kernel,&user)&&CompareFileTime(&created,&started)>=0;
        if(candidate)CloseHandle(candidate);
        if(!fresh){puts("FAIL no new owned calculator candidate");return 1;}
        error=code;
    }else error=run16s_uwp_activate(app_id,&pid);
    printf("ACTIVATE error=%lu own_pid=%lu\n",error,pid);fflush(stdout);
    HANDLE process=pid?OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE|PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid):NULL;
    EnumWindows(top,0);EnumWindows(owned,0);BOOL ok=!error&&core&&frame&&surface;
    printf("TARGET core=%p frame=%p surface=%p dpi=%u\n",(void*)core,(void*)frame,(void*)surface,frame?GetDpiForWindow(frame):0);
    if(ok){
        BOOL pointer_ok=!pointer_mode||resize_pointer();
        titles("before");
        BOOL zoomed=IsZoomed(frame);
        BOOL motion=await_state(FALSE,zoomed)&&button(HTMAXBUTTON)&&await_state(FALSE,!zoomed)&&button(HTMAXBUTTON)&&await_state(FALSE,zoomed);
        BOOL minimize=motion&&button(HTMINBUTTON);
        if(minimize){ULONGLONG deadline=GetTickCount64()+1500;while(!IsIconic(frame)&&GetTickCount64()<deadline)MsgWaitForMultipleObjects(0,NULL,FALSE,20,0);minimize=IsIconic(frame);}
        ShowWindowAsync(frame,SW_RESTORE);BOOL restored=await_state(FALSE,zoomed);
        printf("BUTTONS maximize_restore=%d minimize=%d restored=%d surface_visible=%d\n",motion,minimize,restored,IsWindowVisible(surface));
        titles("restored");
        BOOL actions=motion&&minimize&&restored&&IsWindowVisible(surface);
        RECT before,after;GetWindowRect(frame,&before);HMODULE module=LoadLibraryW(L"ntw16s64.dll");
        HOOKPROC callback=module?(HOOKPROC)(uintptr_t)GetProcAddress(module,"Ntw16sUwpFrameHook"):NULL;
        HHOOK hook=callback?SetWindowsHookExW(WH_CALLWNDPROC,callback,module,GetWindowThreadProcessId(frame,NULL)):NULL;
        DWORD_PTR ignored=0;UINT message=RegisterWindowMessageW(NTW16S_UWP_MESSAGE);
        ok=hook&&SendMessageTimeoutW(frame,message,NTW16S_UWP_REMOVE,(LPARAM)core,SMTO_ABORTIFHUNG|SMTO_ERRORONEXIT,3000,&ignored)&&
            (uintptr_t)GetPropW(frame,NTW16S_UWP_RESULT)==1;
        printf("REMOVE status=%llu\n",(unsigned long long)(uintptr_t)GetPropW(frame,NTW16S_UWP_RESULT));
        RemovePropW(frame,NTW16S_UWP_RESULT);GetWindowRect(frame,&after);
        ok=ok&&!IsWindow(surface)&&EqualRect(&before,&after);
        if(hook)ok=UnhookWindowsHookEx(hook)&&ok;
        if(module)FreeLibrary(module);
        printf("REMOVE_AND_GEOMETRY=%d\n",ok);
        ok=ok&&actions&&pointer_ok;
    }
    if(frame)PostMessageW(frame,WM_CLOSE,0,0);
    if(process){if(WaitForSingleObject(process,3000)!=WAIT_OBJECT_0){TerminateProcess(process,0);WaitForSingleObject(process,1000);}CloseHandle(process);}
    printf("UWP_LAUNCH_REAL_%s\n",ok?"PASS":"FAIL");return ok?0:1;
}
