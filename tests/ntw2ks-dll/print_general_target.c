#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <objbase.h>
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
static DWORD output_flags;
static BOOL test_preferences,preferences_pending,preferences_seen,no_context_device;
static BOOL fits_work_area;
static WCHAR choice[300];
static BOOL full,device_details,shell_list,force_fallback,request_large,large_page,expected_layout;

static LRESULT device_message(HWND list,UINT combo,UINT box,WPARAM wp,LPARAM lp)
{
    if(!shell_list)return SendMessageW(list,full?box:combo,wp,lp);
    switch(box){
    case LB_GETCURSEL:return SendMessageW(list,LVM_GETNEXTITEM,(WPARAM)-1,LVNI_SELECTED);
    case LB_GETCOUNT:return SendMessageW(list,LVM_GETITEMCOUNT,0,0);
    case LB_SETCURSEL:{
        LVITEMW item={.stateMask=LVIS_SELECTED|LVIS_FOCUSED};
        SendMessageW(list,LVM_SETITEMSTATE,(WPARAM)-1,(LPARAM)&item);
        item.state=LVIS_SELECTED|LVIS_FOCUSED;
        SendMessageW(list,LVM_SETSELECTIONMARK,0,(LPARAM)wp);
        return SendMessageW(list,LVM_SETITEMSTATE,wp,(LPARAM)&item);
    }
    default:{
        WCHAR text[ARRAYSIZE(choice)];
        LVITEMW item={.pszText=lp?(PWSTR)lp:text,.cchTextMax=ARRAYSIZE(choice)};
        return SendMessageW(list,LVM_GETITEMTEXTW,wp,(LPARAM)&item);
    }
    }
}

static BOOL CALLBACK find_printer_list(HWND child,LPARAM result)
{
    WCHAR cls[32];
    if(!IsWindowVisible(child) || !GetClassNameW(child,cls,ARRAYSIZE(cls)) || lstrcmpiW(cls,WC_LISTVIEWW))return TRUE;
    *(HWND *)result=child;
    return FALSE;
}

static LRESULT choose_device(HWND list,LRESULT current,LRESULT count)
{
    if(!context_flag)return count>1?(current==0?1:0):CB_ERR;
    for(LRESULT index=0;index<count;++index){
        LRESULT length=device_message(list,CB_GETLBTEXTLEN,LB_GETTEXTLEN,(WPARAM)index,0);
        if(length<0 || length>=(LRESULT)ARRAYSIZE(choice))continue;
        device_message(list,CB_GETLBTEXT,LB_GETTEXT,(WPARAM)index,(LPARAM)choice);
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
    WCHAR class_name[16];
    full=GetClassNameW(list,class_name,ARRAYSIZE(class_name)) && !lstrcmpiW(class_name,L"ListBox");
    if(full && !force_fallback && IsWindowVisible(list) && ticks<8)return FALSE;
    shell_list=full && !IsWindowVisible(list);
    if(shell_list){
        HWND visible=NULL;
        EnumChildWindows(page,find_printer_list,(LPARAM)&visible);
        if(visible)list=visible;
    }
    device_details=!full || (GetWindowTextLengthW(GetDlgItem(page,WIN2K_PRINT_DEVICE_STATUS))>0 &&
        GetDlgItem(page,WIN2K_PRINT_DEVICE_LOCATION) && GetDlgItem(page,WIN2K_PRINT_DEVICE_COMMENT));
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
        RECT client,reference={0,0,292,198};
        if(MapDialogRect(page,&reference) && GetClientRect(page,&client)){
            LONG width=bounds.right-bounds.left+reference.right-client.right;
            LONG height=bounds.bottom-bounds.top+reference.bottom-client.bottom;
            printf("GENERAL REFERENCE client=%ldx%ld original=%ldx%ld estimated_sheet=%ldx%ld work=%ldx%ld\n",
                client.right,client.bottom,reference.right,reference.bottom,width,height,
                work->right-work->left,work->bottom-work->top);
            RECT large={0,0,292,218};
            if(MapDialogRect(page,&large)){
                UINT dpi=GetDpiForWindow(owner);
                BOOL use_large=request_large && full && work->bottom-work->top>=MulDiv(490,(int)dpi,96);
                const RECT *layout=use_large?&large:&reference;
                large_page=full && client.right==large.right && client.bottom==large.bottom;
                expected_layout=!full || (client.right==layout->right && client.bottom==layout->bottom);
            }
        }
    }
    if(test_preferences){
        preferences_pending=TRUE;
        HWND preferences=GetDlgItem(page,WIN2K_PRINT_PREFERENCES);
        SendMessageW(page,WM_COMMAND,MAKEWPARAM(WIN2K_PRINT_PREFERENCES,BN_CLICKED),(LPARAM)preferences);
        preferences_pending=FALSE;
    }
    if(action!=PD_RESULT_CANCEL){
        LRESULT current=device_message(list,CB_GETCURSEL,LB_GETCURSEL,0,0);
        LRESULT count=device_message(list,CB_GETCOUNT,LB_GETCOUNT,0,0);
        LRESULT next=choose_device(list,current,count);
        if(no_context_device){
            PostMessageW(window,PSM_PRESSBUTTON,PSBTN_CANCEL,0);
            return FALSE;
        }
        LRESULT length=next>=0?device_message(list,CB_GETLBTEXTLEN,LB_GETTEXTLEN,(WPARAM)next,0):CB_ERR;
        two=length>=0 && length<(LRESULT)ARRAYSIZE(choice);
        if(two){
            device_message(list,CB_SETCURSEL,LB_SETCURSEL,(WPARAM)next,0);
            device_message(list,CB_GETLBTEXT,LB_GETTEXT,(WPARAM)next,(LPARAM)choice);
            if(!shell_list)SendMessageW(page,WM_COMMAND,MAKEWPARAM(WIN2K_PRINT_DEVICE,full?LBN_SELCHANGE:CBN_SELCHANGE),(LPARAM)list);
        }
        SetDlgItemInt(page,WIN2K_PRINT_COPIES,3,FALSE);
        if(output_flags){
            UINT option=output_flags&PD_SELECTION?WIN2K_PRINT_SELECTION:WIN2K_PRINT_CURRENT;
            HWND radio=GetDlgItem(page,option);
            SendMessageW(radio,BM_CLICK,0,0);
            UINT output=output_flags&PD_PRINTTOFILE?WIN2K_PRINT_FILE:WIN2K_PRINT_COLLATE;
            SendMessageW(GetDlgItem(page,output),BM_CLICK,0,0);
        }else SetDlgItemTextW(page,WIN2K_PRINT_RANGE,L"2,4-5");
        if(action==PD_RESULT_APPLY){
            applied=PropSheet_Apply(window);
            if(two){
                device_message(list,CB_SETCURSEL,LB_SETCURSEL,(WPARAM)current,0);
                if(!shell_list)SendMessageW(page,WM_COMMAND,MAKEWPARAM(WIN2K_PRINT_DEVICE,full?LBN_SELCHANGE:CBN_SELCHANGE),(LPARAM)list);
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
    request_large=argc>=4 && !wcscmp(argv[argc-1],L"large");
    if(request_large)--argc;
    if(argc!=3 && argc!=4)return 2;
    BOOL initial=argc==4 && !wcscmp(argv[3],L"initial");
    test_preferences=argc==4 && !wcscmp(argv[3],L"preferences");
    if(argc==4 && !wcscmp(argv[3],L"aware") &&
       !SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))return 77;
    if(argc==4 && !wcscmp(argv[3],L"dc"))context_flag=PD_RETURNDC;
    if(argc==4 && !wcscmp(argv[3],L"ic"))context_flag=PD_RETURNIC;
    if(argc==4 && !wcscmp(argv[3],L"selection-file"))output_flags=PD_SELECTION|PD_PRINTTOFILE;
    if(argc==4 && !wcscmp(argv[3],L"current-driver"))output_flags=PD_CURRENTPAGE|PD_USEDEVMODECOPIESANDCOLLATE;
    force_fallback=argc==4 && !wcscmp(argv[3],L"fallback");
    if(force_fallback && FAILED(CoInitializeEx(NULL,COINIT_MULTITHREADED)))return 2;
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
    DWORD layout_flag=request_large?PD_USELARGETEMPLATE:0;
    a.Flags=w.Flags=context_flag|layout_flag|(output_flags&PD_USEDEVMODECOPIESANDCOLLATE);
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
    BOOL file=FALSE;
    if(data){
        if(ansi){
            MultiByteToWideChar(CP_ACP,0,(char *)data+data->wDeviceOffset,-1,actual,ARRAYSIZE(actual));
            file=!strcmp((char *)data+data->wOutputOffset,"FILE:");
        }else{
            lstrcpynW(actual,(WCHAR *)data+data->wDeviceOffset,ARRAYSIZE(actual));
            file=!wcscmp((WCHAR *)data+data->wOutputOffset,L"FILE:");
        }
        GlobalUnlock(names);
    }
    BOOL match=two && !wcscmp(actual,choice);
    DWORD result_flags=ansi?a.Flags:w.Flags;
    BOOL pass=hr==S_OK && ready && styled && initial_ok && fits_work_area && expected_layout && device_details &&
        (result_flags&PD_USELARGETEMPLATE)==layout_flag &&
        (!force_fallback || !shell_list) && code==(no_context_device?PD_RESULT_CANCEL:action) &&
        IsWindowEnabled(owner) && (!test_preferences || preferences_seen);
    if(context_flag && action!=PD_RESULT_CANCEL)pass=pass && (no_context_device?!dc:dc && GetDeviceCaps(dc,TECHNOLOGY)==DT_RASPRINTER);
    if(no_context_device)action=PD_RESULT_CANCEL;
    if(action==PD_RESULT_CANCEL)pass=pass && mode==initial_mode && names==initial_names && !range_count;
    else{
        pass=pass && mode && mode!=initial_mode && names!=initial_names && match &&
            (action!=PD_RESULT_APPLY || applied);
        if(output_flags){
            DWORD flags=ansi?a.Flags:w.Flags;
            BOOL driver=(output_flags&PD_USEDEVMODECOPIESANDCOLLATE)!=0;
            void *settings=mode?GlobalLock(mode):NULL;
            short device_copies=0,collate=0;
            if(settings){
                device_copies=ansi?((DEVMODEA *)settings)->dmCopies:((DEVMODEW *)settings)->dmCopies;
                collate=ansi?((DEVMODEA *)settings)->dmCollate:((DEVMODEW *)settings)->dmCollate;
                GlobalUnlock(mode);
            }
            pass=pass && flags==(output_flags|layout_flag) && !range_count && copies==(driver?1:3) &&
                file==((output_flags&PD_PRINTTOFILE)!=0) && device_copies==(driver?3:1) &&
                collate==(driver?DMCOLLATE_TRUE:DMCOLLATE_FALSE);
            printf("GENERAL OPTIONS requested=%08lx returned=%08lx file=%d device_copies=%d collate=%d\n",
                output_flags,flags,file,device_copies,collate);
        }else pass=pass && copies==3 && range_count==2 && ranges[0].nFromPage==2 &&
            ranges[0].nToPage==2 && ranges[1].nFromPage==4 && ranges[1].nToPage==5;
    }
    printf("GENERAL ENTRY bits=%u ansi=%d initial=%d requested=%u result=%lu hr=%08lx frame=%d copies=%lu pages=%lu selected=%d dc=%d preferences=%d full=%d details=%d shell=%d large=%d pass=%d\n",
       (unsigned)(sizeof(void *)*8),ansi,initial,action,code,(unsigned long)hr,styled,copies,range_count,match,dc!=NULL,preferences_seen,full,device_details,shell_list,large_page,pass);
    if(dc)DeleteDC(dc);
    if(mode)GlobalFree(mode);
    if(names)GlobalFree(names);
    DestroyWindow(owner);
    if(force_fallback)CoUninitialize();
    if(!pass)return 3;
    if(no_context_device){puts("GENERAL SKIP native printer context unavailable");return 77;}
    return 0;
}
