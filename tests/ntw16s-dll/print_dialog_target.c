#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <cderr.h>
#include <stdio.h>
static BOOL (__cdecl *attached)(HWND);static int initialized;static BOOL frame;
static DWORD main_thread;
static BOOL classic_test,classic_found,classic_accept;
static int extended_case,extended_step;
static BOOL extended_old;
static HWND range_radio,range_edit;
static BOOL CALLBACK locate_range(HWND w,LPARAM data)
{(void)data;if(GetDlgCtrlID(w)==1059 || GetDlgCtrlID(w)==1058)range_radio=w;if(GetDlgCtrlID(w)==1152 && !range_edit)range_edit=w;if(GetDlgCtrlID(w)==9002)range_edit=w;return TRUE;}
static void CALLBACK close_dialog(HWND window,UINT message,UINT_PTR timer,DWORD time)
{(void)message;(void)time;
 if(extended_case>=4){extended_old=GetDlgItem(window,1139)!=NULL;
    if(extended_case==6){KillTimer(window,timer);frame=attached&&attached(window);PostMessageW(window,WM_COMMAND,IDCANCEL,0);return;}
    if(!extended_step){SetDlgItemTextW(window,1154,L"3");if(extended_case==5){HWND apply=GetDlgItem(window,9001);if(apply)SendMessageW(apply,BM_CLICK,0,0);SetDlgItemTextW(window,1154,L"5");extended_step=1;return;}}
    KillTimer(window,timer);frame=attached&&attached(window);PostMessageW(window,WM_COMMAND,extended_case==4?IDOK:IDCANCEL,0);return;}
 if(extended_case==3){extended_old=GetDlgItem(window,1139)!=NULL;KillTimer(window,timer);frame=attached&&attached(window);PostMessageW(window,WM_COMMAND,IDCANCEL,0);return;}
 if(extended_case && extended_step==0){extended_old=GetDlgItem(window,1139)!=NULL;EnumChildWindows(window,locate_range,0);if(range_radio && range_edit){SendMessageW(range_radio,BM_CLICK,0,0);SetWindowTextW(range_edit,L"2,4-5");SetDlgItemTextW(window,1154,L"3");}extended_step=1;return;}
 if(extended_case==2 && extended_step==1){HWND apply=GetDlgItem(window,9001);if(!apply)apply=GetDlgItem(window,12321);if(apply && IsWindowEnabled(apply))SendMessageW(apply,BM_CLICK,0,0);if(extended_old){SetDlgItemTextW(window,1154,L"5");SetWindowTextW(range_edit,L"8");}extended_step=2;return;}
 KillTimer(window,timer);frame=attached&&attached(window);if(classic_test)classic_found=GetDlgItem(window,1139)!=NULL;
 PostMessageW(window,WM_COMMAND,(classic_accept || extended_case==1)?IDOK:IDCANCEL,0);}
static LRESULT CALLBACK observe(int code,WPARAM wparam,LPARAM parameter)
{
    if(code>=0 && !initialized){const CWPSTRUCT *m=(const CWPSTRUCT *)parameter;
        if(m->message==WM_SHOWWINDOW && m->wParam){WCHAR cls[32],title[80];GetClassNameW(m->hwnd,cls,32);GetWindowTextW(m->hwnd,title,80);(void)title;
            if(!lstrcmpW(cls,L"#32770") && GetDlgItem(m->hwnd,IDCANCEL)){++initialized;SetTimer(m->hwnd,917,100,close_dialog);}}}
    return CallNextHookEx(NULL,code,wparam,parameter);
}
static BOOL CALLBACK enumerate_print(HWND window,LPARAM ignored)
{
    (void)ignored;DWORD pid=0,owner_pid=0;GetWindowThreadProcessId(window,&pid);
    HWND owner=GetWindow(window,GW_OWNER);if(owner)GetWindowThreadProcessId(owner,&owner_pid);
    if((pid!=GetCurrentProcessId() && owner_pid!=GetCurrentProcessId()) || !IsWindowVisible(window))return TRUE;
    WCHAR cls[64],title[128];GetClassNameW(window,cls,64);GetWindowTextW(window,title,128);
    if(!lstrcmpW(cls,L"#32770")){
        fprintf(stderr,"PRINT_WINDOW class=%ls title=%ls pid=%lu owner_pid=%lu\n",cls,title,pid,owner_pid);
        if(!initialized){++initialized;if(pid==GetCurrentProcessId())SetTimer(window,917,100,close_dialog);
            else PostMessageW(window,WM_COMMAND,IDCANCEL,0);
            return FALSE;}
    }
    return TRUE;
}
static DWORD WINAPI print_observer(void *ignored)
{
    (void)ignored;for(int i=0;i<100 && !initialized;++i){EnumWindows(enumerate_print,0);Sleep(50);}
    if(!initialized)PostThreadMessageW(main_thread,WM_QUIT,0,0);
    return 0;
}
static UINT_PTR CALLBACK hook(HWND window,UINT message,WPARAM wparam,LPARAM lparam)
{(void)lparam;if(message==WM_INITDIALOG){++initialized;SetTimer(window,916,100,NULL);}if(message==WM_TIMER && wparam==916){KillTimer(window,916);frame=attached&&attached(window);PostMessageW(window,WM_COMMAND,IDCANCEL,0);}return 0;}
int wmain(int argc,WCHAR **argv)
{
    if(argc!=3 && argc!=4)return 2;
    BOOL page=argv[1][0]==L'p' && argv[1][1]==L'a',extended=!lstrcmpW(argv[1],L"printex"),ansi=argv[2][0]==L'a';
    HMODULE m=GetModuleHandleW(sizeof(void*)==4?L"ntw16s32.dll":L"ntw16s64.dll");
    attached=m?(BOOL (__cdecl *)(HWND))(void *)GetProcAddress(m,"Ntw16sWindowAttached"):NULL;
    BOOL result;HGLOBAL devmode=NULL,devnames=NULL;
    if(!lstrcmpW(argv[1],L"invalid")){
        PRINTDLGEXW x={0};x.lStructSize=sizeof(x);HRESULT hr=PrintDlgExW(&x);
        printf("PRINT_EX_INVALID bits=%u hr=%08lx\n",(unsigned)(sizeof(void*)*8),(DWORD)hr);
        return hr==HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE)?0:1;
    }
    if(!lstrcmpW(argv[1],L"ex-cancel") || !lstrcmpW(argv[1],L"ex-fallback")){
        BOOL fallback=!lstrcmpW(argv[1],L"ex-fallback");extended_case=3;
        HRESULT com=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(com))return 9;
        HWND owner=CreateWindowExW(0,L"STATIC",L"Owner",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);if(!owner)return 7;
        ShowWindow(owner,SW_SHOW);HHOOK observer=SetWindowsHookExW(WH_CALLWNDPROC,observe,NULL,GetCurrentThreadId());if(!observer)return 6;
        PRINTPAGERANGE ranges[10]={{77,88}};PRINTDLGEXW x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=PD_RETURNDC|(fallback?PD_CURRENTPAGE:0);x.nStartPage=START_PAGE_GENERAL;x.nMaxPageRanges=10;x.lpPageRanges=ranges;x.nMinPage=1;x.nMaxPage=1000;x.nCopies=1;
        HRESULT hr=PrintDlgExW(&x);BOOL valid=hr==S_OK && x.dwResultAction==PD_RESULT_CANCEL && x.Flags==(PD_RETURNDC|(fallback?PD_CURRENTPAGE:0)) && !x.hDC && !x.hDevMode && !x.hDevNames && ranges[0].nFromPage==77 && ranges[0].nToPage==88 && extended_old!=fallback;
        printf("EX_CANCEL bits=%u fallback=%d old=%d hr=%08lx action=%lu untouched=%d frame=%d\n",(unsigned)(sizeof(void*)*8),fallback,extended_old,(DWORD)hr,x.dwResultAction,valid,frame);
        if(x.hDC)DeleteDC(x.hDC);
        if(x.hDevMode)GlobalFree(x.hDevMode);
        if(x.hDevNames)GlobalFree(x.hDevNames);
        UnhookWindowsHookEx(observer);DestroyWindow(owner);CoUninitialize();return initialized==1 && frame && valid?0:1;
    }
    if(!lstrcmpW(argv[1],L"ex-nopages-print") || !lstrcmpW(argv[1],L"ex-nopages-apply") || !lstrcmpW(argv[1],L"ex-nopages-cancel")){
        extended_case=!lstrcmpW(argv[1],L"ex-nopages-print")?4:!lstrcmpW(argv[1],L"ex-nopages-apply")?5:6;
        HRESULT com=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(com))return 9;
        HWND owner=CreateWindowExW(0,L"STATIC",L"Owner",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);if(!owner)return 7;
        ShowWindow(owner,SW_SHOW);HHOOK observer=SetWindowsHookExW(WH_CALLWNDPROC,observe,NULL,GetCurrentThreadId());if(!observer)return 6;
        PRINTDLGEXW x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=PD_RETURNDC|PD_NOPAGENUMS;x.nStartPage=START_PAGE_GENERAL;x.nCopies=1;
        HRESULT hr=PrintDlgExW(&x);DWORD expected=extended_case==4?PD_RESULT_PRINT:extended_case==5?PD_RESULT_APPLY:PD_RESULT_CANCEL;
        BOOL valid=hr==S_OK && x.dwResultAction==expected && (x.Flags & PD_NOPAGENUMS) && x.nPageRanges==0 && !x.lpPageRanges && extended_old && frame &&
            (extended_case==6?(!x.hDC && !x.hDevMode && !x.hDevNames):(x.hDC && x.hDevMode && x.hDevNames && x.nCopies==3));
        printf("EX_NOPAGES bits=%u mode=%d hr=%08lx action=%lu old=%d frame=%d valid=%d\n",(unsigned)(sizeof(void*)*8),extended_case,(DWORD)hr,x.dwResultAction,extended_old,frame,valid);
        if(x.hDC)DeleteDC(x.hDC);
        if(x.hDevMode)GlobalFree(x.hDevMode);
        if(x.hDevNames)GlobalFree(x.hDevNames);
        UnhookWindowsHookEx(observer);DestroyWindow(owner);CoUninitialize();return initialized==1 && valid?0:1;
    }
    if(!lstrcmpW(argv[1],L"ex-ranges") || !lstrcmpW(argv[1],L"ex-apply")){
        if(ansi)return 2;
        extended_case=!lstrcmpW(argv[1],L"ex-apply")?2:1;
        HRESULT com=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(com))return 9;
        HWND owner=CreateWindowExW(0,L"STATIC",L"Print test owner",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
        if(!owner)return 7;
        ShowWindow(owner,SW_SHOW);HHOOK observer=SetWindowsHookExW(WH_CALLWNDPROC,observe,NULL,GetCurrentThreadId());if(!observer)return 6;
        PRINTPAGERANGE ranges[10]={0};PRINTDLGEXW x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=PD_RETURNDC;
        x.nStartPage=START_PAGE_GENERAL;x.nMaxPageRanges=10;x.lpPageRanges=ranges;x.nMinPage=1;x.nMaxPage=1000;x.nCopies=1;
        HRESULT hr=PrintDlgExW(&x);BOOL valid=hr==S_OK && x.dwResultAction==(extended_case==2?PD_RESULT_APPLY:PD_RESULT_PRINT) &&
            x.nPageRanges==2 && ranges[0].nFromPage==2 && ranges[0].nToPage==2 && ranges[1].nFromPage==4 && ranges[1].nToPage==5 && x.hDC && (!extended_old || x.nCopies==3);
        printf("PRINT_EX_RANGE bits=%u mode=%d hr=%08lx action=%lu ranges=%lu first=%lu-%lu second=%lu-%lu frame=%d valid=%d old=%d\n",(unsigned)(sizeof(void*)*8),extended_case,(DWORD)hr,x.dwResultAction,x.nPageRanges,ranges[0].nFromPage,ranges[0].nToPage,ranges[1].nFromPage,ranges[1].nToPage,frame,valid,extended_old);
        if(x.hDC)DeleteDC(x.hDC);
        if(x.hDevMode)GlobalFree(x.hDevMode);
        if(x.hDevNames)GlobalFree(x.hDevNames);
        UnhookWindowsHookEx(observer);DestroyWindow(owner);CoUninitialize();return initialized==1 && frame && valid?0:1;
    }
    if(!lstrcmpW(argv[1],L"classic")){
        if(argc!=4)return 2;
        classic_test=TRUE;classic_accept=!lstrcmpW(argv[3],L"accept");
        HWND owner=CreateWindowExW(0,L"STATIC",L"Print test owner",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
        if(!owner)return 7;
        ShowWindow(owner,SW_SHOW);
        HHOOK observer=SetWindowsHookExW(WH_CALLWNDPROC,observe,NULL,GetCurrentThreadId());if(!observer)return 6;
        BOOL clean=FALSE,returned=FALSE;DWORD error;
        if(ansi){PRINTDLGA x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=PD_RETURNDC;x.nCopies=1;x.nMinPage=1;x.nMaxPage=1000;
            result=PrintDlgA(&x);error=CommDlgExtendedError();clean=!(x.Flags & PD_ENABLEPRINTHOOK) && !x.lpfnPrintHook;
            returned=x.hDC && x.hDevMode && x.hDevNames && x.nCopies==1;if(x.hDC)DeleteDC(x.hDC);devmode=x.hDevMode;devnames=x.hDevNames;}
        else{PRINTDLGW x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=PD_RETURNDC;x.nCopies=1;x.nMinPage=1;x.nMaxPage=1000;
            result=PrintDlgW(&x);error=CommDlgExtendedError();clean=!(x.Flags & PD_ENABLEPRINTHOOK) && !x.lpfnPrintHook;
            returned=x.hDC && x.hDevMode && x.hDevNames && x.nCopies==1;if(x.hDC)DeleteDC(x.hDC);devmode=x.hDevMode;devnames=x.hDevNames;}
        if(devmode)GlobalFree(devmode);
        if(devnames)GlobalFree(devnames);
        UnhookWindowsHookEx(observer);DestroyWindow(owner);
        printf("PRINT_CLASSIC bits=%u ansi=%d accept=%d old=%d frame=%d clean=%d result=%d returned=%d error=%lu\n",(unsigned)(sizeof(void*)*8),ansi,classic_accept,classic_found,frame,clean,result,returned,error);
        return initialized==1 && classic_found && frame && clean && !error && (classic_accept?(result&&returned):!result)?0:1;
    }
    if(extended){
        HRESULT com=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
        if(FAILED(com))return 9;
        main_thread=GetCurrentThreadId();
        HANDLE watcher=CreateThread(NULL,0,print_observer,NULL,0,NULL);
        if(!watcher)return 8;
        HHOOK observer=SetWindowsHookExW(WH_CALLWNDPROC,observe,NULL,GetCurrentThreadId());
        if(!observer)return 6;
        HWND owner=CreateWindowExW(0,L"STATIC",L"Print test owner",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
        if(!owner){UnhookWindowsHookEx(observer);return 7;}
        ShowWindow(owner,SW_SHOW);
        HRESULT hr;DWORD action;
        if(ansi){PRINTDLGEXA x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=PD_NOPAGENUMS;x.nStartPage=START_PAGE_GENERAL;hr=PrintDlgExA(&x);action=x.dwResultAction;devmode=x.hDevMode;devnames=x.hDevNames;}
        else{PRINTDLGEXW x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=PD_NOPAGENUMS;x.nStartPage=START_PAGE_GENERAL;hr=PrintDlgExW(&x);action=x.dwResultAction;devmode=x.hDevMode;devnames=x.hDevNames;}
        WaitForSingleObject(watcher,6000);CloseHandle(watcher);DestroyWindow(owner);CoUninitialize();UnhookWindowsHookEx(observer);if(devmode)GlobalFree(devmode);if(devnames)GlobalFree(devnames);
        printf("PRINT_EX bits=%u ansi=%d hr=%08lx action=%lu init=%d frame=%d\n",(unsigned)(sizeof(void*)*8),ansi,(DWORD)hr,action,initialized,frame);
        return hr==S_OK && action==PD_RESULT_CANCEL && initialized==1 && frame?0:1;
    }
    if(page && ansi){PAGESETUPDLGA x={0};x.lStructSize=sizeof(x);x.Flags=PSD_ENABLEPAGESETUPHOOK;x.lpfnPageSetupHook=hook;result=PageSetupDlgA(&x);devmode=x.hDevMode;devnames=x.hDevNames;}
    else if(page){PAGESETUPDLGW x={0};x.lStructSize=sizeof(x);x.Flags=PSD_ENABLEPAGESETUPHOOK;x.lpfnPageSetupHook=hook;result=PageSetupDlgW(&x);devmode=x.hDevMode;devnames=x.hDevNames;}
    else if(ansi){PRINTDLGA x={0};x.lStructSize=sizeof(x);x.Flags=PD_ENABLEPRINTHOOK;x.lpfnPrintHook=hook;result=PrintDlgA(&x);devmode=x.hDevMode;devnames=x.hDevNames;if(x.hDC)DeleteDC(x.hDC);}
    else{PRINTDLGW x={0};x.lStructSize=sizeof(x);x.Flags=PD_ENABLEPRINTHOOK;x.lpfnPrintHook=hook;result=PrintDlgW(&x);devmode=x.hDevMode;devnames=x.hDevNames;if(x.hDC)DeleteDC(x.hDC);}
    DWORD error=CommDlgExtendedError();if(devmode)GlobalFree(devmode);if(devnames)GlobalFree(devnames);
    printf("PRINT_DIALOG bits=%u page=%d ansi=%d result=%d error=%lu init=%d frame=%d\n",(unsigned)(sizeof(void*)*8),page,ansi,result,error,initialized,frame);
    if(!initialized && error==PDERR_NODEFAULTPRN){puts("PRINT_SKIP no_default_printer");return 77;}
    return initialized==1 && frame && !result && !error?0:1;
}
