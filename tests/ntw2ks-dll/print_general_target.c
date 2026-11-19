#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "lib-win32-style/appearance/win2000/dialog/print_general.h"

static HWND owner,sheet_window;
static UINT ticks;
static BOOL styled,ready, applied, two, initial_ok;
static unsigned expected_copies=1;
static unsigned action;
static DWORD context_flag;
static BOOL test_preferences,preferences_pending,preferences_seen,no_context_device;
static BOOL fits_work_area;
static WCHAR choice[300];

static LRESULT choose_device(HWND list,LRESULT current,LRESULT count)
{
    if(!context_flag)return count>1?(current==0?1:0):CB_ERR;
    for(LRESULT index=0;index<count;++index){
        LRESULT length=SendMessageW(list,CB_GETLBTEXTLEN,(WPARAM)index,0);
        if(length<0 || length>=(LRESULT)ARRAYSIZE(choice))continue;
        SendMessageW(list,CB_GETLBTEXT,(WPARAM)index,(LPARAM)choice);
        HDC context=context_flag==PD_RETURNIC?
            CreateICW(L"WINSPOOL",choice,NULL,NULL):CreateDCW(L"WINSPOOL",choice,NULL,NULL);
        if(context){DeleteDC(context);return index;}
    }
    no_context_device=TRUE;
    return CB_ERR;
}

static BOOL CALLBACK cancel_preferences(HWND window,LPARAM unused)
{
    (void)unused;
    if(window==sheet_window || GetAncestor(window,GA_ROOTOWNER)!=owner || !IsWindowVisible(window) ||
       !GetDlgItem(window,IDCANCEL))return TRUE;
    preferences_seen=TRUE;
    if(PropSheet_GetCurrentPageHwnd(window))PostMessageW(window,PSM_PRESSBUTTON,PSBTN_CANCEL,0);
    else PostMessageW(window,WM_COMMAND,MAKEWPARAM(IDCANCEL,BN_CLICKED),(LPARAM)GetDlgItem(window,IDCANCEL));
    return FALSE;
}
static BOOL CALLBACK sheet(HWND window,LPARAM unused)
{
    (void)unused;
    if(GetWindow(window,GW_OWNER)!=owner || !IsWindowVisible(window))return TRUE;
    HWND page=PropSheet_GetCurrentPageHwnd(window);
    HWND list=page?GetDlgItem(page,WIN2K_PRINT_DEVICE):NULL;
    if(!list)return TRUE;
    ready=TRUE;
    sheet_window=window;
    initial_ok=GetDlgItemInt(page,WIN2K_PRINT_COPIES,NULL,FALSE)==expected_copies;
    styled=GetPropW(window,L"Win2kStyle.Frame")!=NULL;
    MONITORINFO monitor={.cbSize=sizeof(monitor)};
    RECT bounds={0};
    if(GetWindowRect(window,&bounds) && GetMonitorInfoW(MonitorFromWindow(window,MONITOR_DEFAULTTONEAREST),&monitor)){
        const RECT *work=&monitor.rcWork;
        fits_work_area=bounds.left>=work->left && bounds.top>=work->top &&
            bounds.right<=work->right && bounds.bottom<=work->bottom;
        unsigned buttons=0;
        for(HWND button=GetWindow(window,GW_CHILD);button;button=GetWindow(button,GW_HWNDNEXT)){
            WCHAR cls[16];
            if(!IsWindowVisible(button) || !GetClassNameW(button,cls,ARRAYSIZE(cls)) || lstrcmpiW(cls,L"Button"))continue;
            RECT area={0};
            ++buttons;
            if(!GetWindowRect(button,&area) || area.left<work->left ||
               area.top<work->top || area.right>work->right || area.bottom>work->bottom)fits_work_area=FALSE;
        }
        fits_work_area=fits_work_area && buttons>=3;
        printf("GENERAL GEOMETRY fits=%d sheet=%ld,%ld,%ld,%ld work=%ld,%ld,%ld,%ld\n",
            fits_work_area,bounds.left,bounds.top,bounds.right,bounds.bottom,work->left,work->top,work->right,work->bottom);
    }
    if(test_preferences){
        preferences_pending=TRUE;
        HWND preferences=GetDlgItem(page,WIN2K_PRINT_PREFERENCES);
        SendMessageW(page,WM_COMMAND,MAKEWPARAM(WIN2K_PRINT_PREFERENCES,BN_CLICKED),(LPARAM)preferences);
        preferences_pending=FALSE;
    }
    if(action!=PD_RESULT_CANCEL){
        LRESULT current=SendMessageW(list,CB_GETCURSEL,0,0);
        LRESULT count=SendMessageW(list,CB_GETCOUNT,0,0);
        LRESULT next=choose_device(list,current,count);
        if(no_context_device){
            PostMessageW(window,PSM_PRESSBUTTON,PSBTN_CANCEL,0);
            return FALSE;
        }
        LRESULT length=next>=0?SendMessageW(list,CB_GETLBTEXTLEN,(WPARAM)next,0):CB_ERR;
        two=length>=0 && length<(LRESULT)ARRAYSIZE(choice);
        if(two){
            SendMessageW(list,CB_SETCURSEL,(WPARAM)next,0);
            SendMessageW(list,CB_GETLBTEXT,(WPARAM)next,(LPARAM)choice);
            SendMessageW(page,WM_COMMAND,MAKEWPARAM(WIN2K_PRINT_DEVICE,CBN_SELCHANGE),(LPARAM)list);
        }
        SetDlgItemInt(page,WIN2K_PRINT_COPIES,3,FALSE);
        SetDlgItemTextW(page,WIN2K_PRINT_RANGE,L"2,4-5");
        if(action==PD_RESULT_APPLY){
            applied=PropSheet_Apply(window);
            if(two){
                SendMessageW(list,CB_SETCURSEL,(WPARAM)current,0);
                SendMessageW(page,WM_COMMAND,MAKEWPARAM(WIN2K_PRINT_DEVICE,CBN_SELCHANGE),(LPARAM)list);
            }
            SetDlgItemInt(page,WIN2K_PRINT_COPIES,5,FALSE);
        }
    }
    PostMessageW(window,PSM_PRESSBUTTON,action==PD_RESULT_PRINT && !no_context_device?PSBTN_OK:PSBTN_CANCEL,0);
    return FALSE;
}
static void CALLBACK observe(HWND window,UINT message,UINT_PTR timer,DWORD time)
{
    (void)message;(void)time;
    if(!ready)EnumThreadWindows(GetCurrentThreadId(),sheet,0);
    else if(preferences_pending && !preferences_seen)EnumThreadWindows(GetCurrentThreadId(),cancel_preferences,0);
    if(++ticks>=40){KillTimer(window,timer);ExitProcess(4);}
}
int wmain(int argc,WCHAR **argv)
{
    if(argc!=3 && argc!=4)return 2;
    BOOL initial=argc==4 && !wcscmp(argv[3],L"initial");
    test_preferences=argc==4 && !wcscmp(argv[3],L"preferences");
    if(argc==4 && !wcscmp(argv[3],L"aware") &&
       !SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))return 77;
    if(argc==4 && !wcscmp(argv[3],L"dc"))context_flag=PD_RETURNDC;
    if(argc==4 && !wcscmp(argv[3],L"ic"))context_flag=PD_RETURNIC;
    BOOL ansi=!wcscmp(argv[1],L"a");
    action=!wcscmp(argv[2],L"apply")?PD_RESULT_APPLY:!wcscmp(argv[2],L"confirm")?PD_RESULT_PRINT:PD_RESULT_CANCEL;
    INITCOMMONCONTROLSEX controls={sizeof(controls),ICC_WIN95_CLASSES};
    if(!InitCommonControlsEx(&controls))return 2;
    owner=CreateWindowExW(0,L"STATIC",L"Owned PrintDlgEx test",WS_OVERLAPPEDWINDOW,0,0,240,120,
        NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!owner || !SetTimer(owner,42,150,observe))return 2;
    ShowWindow(owner,SW_SHOWNOACTIVATE);
    PRINTPAGERANGE ranges[4]={{0}};
    PRINTDLGEXA a={.lStructSize=sizeof(a),.hwndOwner=owner,.nMinPage=1,.nMaxPage=50,.nMaxPageRanges=4,
        .lpPageRanges=ranges,.nCopies=1,.nStartPage=START_PAGE_GENERAL};
    PRINTDLGEXW w={.lStructSize=sizeof(w),.hwndOwner=owner,.nMinPage=1,.nMaxPage=50,.nMaxPageRanges=4,
        .lpPageRanges=ranges,.nCopies=1,.nStartPage=START_PAGE_GENERAL};
    a.Flags=w.Flags=context_flag;
    HGLOBAL initial_mode=NULL,initial_names=NULL;
    if(initial){
        PRINTDLGA da={.lStructSize=sizeof(da),.Flags=PD_RETURNDEFAULT};
        PRINTDLGW dw={.lStructSize=sizeof(dw),.Flags=PD_RETURNDEFAULT};
        if(!(ansi?PrintDlgA(&da):PrintDlgW(&dw)))return 77;
        initial_mode=ansi?da.hDevMode:dw.hDevMode;
        initial_names=ansi?da.hDevNames:dw.hDevNames;
        a.hDevMode=w.hDevMode=initial_mode;
        a.hDevNames=w.hDevNames=initial_names;
        void *mode=GlobalLock(initial_mode);
        if(!mode)return 2;
        if(ansi){DEVMODEA *value=mode;value->dmFields|=DM_COPIES;value->dmCopies=7;}
        else{DEVMODEW *value=mode;value->dmFields|=DM_COPIES;value->dmCopies=7;}
        GlobalUnlock(initial_mode);
        expected_copies=7;
    }
    HRESULT hr=ansi?PrintDlgExA(&a):PrintDlgExW(&w);
    KillTimer(owner,42);
    DWORD code=ansi?a.dwResultAction:w.dwResultAction, copies=ansi?a.nCopies:w.nCopies;
    DWORD range_count=ansi?a.nPageRanges:w.nPageRanges;
    HGLOBAL mode=ansi?a.hDevMode:w.hDevMode, names=ansi?a.hDevNames:w.hDevNames;
    HDC dc=ansi?a.hDC:w.hDC;
    DEVNAMES *data=names?GlobalLock(names):NULL;
    WCHAR actual[300]=L"";
    if(data){
        if(ansi)MultiByteToWideChar(CP_ACP,0,(char *)data+data->wDeviceOffset,-1,actual,ARRAYSIZE(actual));
        else lstrcpynW(actual,(WCHAR *)data+data->wDeviceOffset,ARRAYSIZE(actual));
        GlobalUnlock(names);
    }
    BOOL match=two && !wcscmp(actual,choice);
    BOOL pass=hr==S_OK && ready && styled && initial_ok && fits_work_area && code==(no_context_device?PD_RESULT_CANCEL:action) &&
        IsWindowEnabled(owner) && (!test_preferences || preferences_seen);
    if(context_flag && action!=PD_RESULT_CANCEL)pass=pass && (no_context_device?!dc:dc && GetDeviceCaps(dc,TECHNOLOGY)==DT_RASPRINTER);
    if(no_context_device)action=PD_RESULT_CANCEL;
    if(action==PD_RESULT_CANCEL)pass=pass && mode==initial_mode && names==initial_names && !range_count;
    else pass=pass && mode && mode!=initial_mode && names!=initial_names && match && copies==3 && range_count==2 &&
        ranges[0].nFromPage==2 && ranges[0].nToPage==2 && ranges[1].nFromPage==4 && ranges[1].nToPage==5 &&
        (action!=PD_RESULT_APPLY || applied);
    printf("GENERAL ENTRY bits=%u ansi=%d initial=%d requested=%u result=%lu hr=%08lx frame=%d copies=%lu pages=%lu selected=%d dc=%d preferences=%d pass=%d\n",
       (unsigned)(sizeof(void *)*8),ansi,initial,action,code,(unsigned long)hr,styled,copies,range_count,match,dc!=NULL,preferences_seen,pass);
    if(dc)DeleteDC(dc);
    if(mode)GlobalFree(mode);
    if(names)GlobalFree(names);
    DestroyWindow(owner);
    if(!pass)return 3;
    if(no_context_device){puts("GENERAL SKIP native printer context unavailable");return 77;}
    return 0;
}
