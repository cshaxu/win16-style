#include "run16s-exe/uwp.h"
#include "ntw16s-dll/uwp/hook.h"
#include "lib-win32/win16-window-uwp/frame_layout.h"
#include <stdio.h>
#include <stdint.h>
#include <tlhelp32.h>
#include <wchar.h>
static DWORD pid;
static HWND frame,core,surface;
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
static BOOL existing_calculator(void)
{
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);if(snapshot==INVALID_HANDLE_VALUE)return TRUE;
    PROCESSENTRY32W entry={.dwSize=sizeof(entry)};BOOL found=FALSE;
    for(BOOL next=Process32FirstW(snapshot,&entry);next;next=Process32NextW(snapshot,&entry))
        if(!_wcsicmp(entry.szExeFile,L"CalculatorApp.exe")){found=TRUE;break;}
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
    if(argc!=2||wcscmp(argv[1],L"--activate")){puts("SKIP real UWP activation requires --activate");return 77;}
    HDESK desktop=OpenInputDesktop(0,FALSE,DESKTOP_READOBJECTS);WCHAR name[80]={0};DWORD needed;
    if(desktop){GetUserObjectInformationW(desktop,UOI_NAME,name,sizeof(name),&needed);CloseDesktop(desktop);}
    if(wcscmp(name,L"Default")||existing_calculator()){puts("SKIP desktop or existing Calculator");return 77;}
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    DWORD error=run16s_uwp_activate(L"Microsoft.WindowsCalculator_8wekyb3d8bbwe!App",&pid);
    printf("ACTIVATE error=%lu own_pid=%lu\n",error,pid);fflush(stdout);
    HANDLE process=pid?OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE|PROCESS_QUERY_LIMITED_INFORMATION,FALSE,pid):NULL;
    EnumWindows(top,0);EnumWindows(owned,0);BOOL ok=!error&&core&&frame&&surface;
    printf("TARGET core=%p frame=%p surface=%p dpi=%u\n",(void*)core,(void*)frame,(void*)surface,frame?GetDpiForWindow(frame):0);
    if(ok){
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
        ok=ok&&actions;
    }
    if(frame)PostMessageW(frame,WM_CLOSE,0,0);
    if(process){if(WaitForSingleObject(process,3000)!=WAIT_OBJECT_0){TerminateProcess(process,0);WaitForSingleObject(process,1000);}CloseHandle(process);}
    printf("UWP_LAUNCH_REAL_%s\n",ok?"PASS":"FAIL");return ok?0:1;
}
