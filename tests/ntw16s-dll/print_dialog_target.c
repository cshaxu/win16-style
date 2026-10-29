#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <cderr.h>
#include <stdio.h>
static BOOL (__cdecl *attached)(HWND);static int initialized;static BOOL frame;
static DWORD main_thread;
static void CALLBACK close_dialog(HWND window,UINT message,UINT_PTR timer,DWORD time)
{(void)message;(void)time;KillTimer(window,timer);frame=attached&&attached(window);PostMessageW(window,WM_COMMAND,IDCANCEL,0);}
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
    if(argc!=3)return 2;
    BOOL page=argv[1][0]==L'p' && argv[1][1]==L'a',extended=!lstrcmpW(argv[1],L"printex"),ansi=argv[2][0]==L'a';
    HMODULE m=GetModuleHandleW(sizeof(void*)==4?L"ntw16s32.dll":L"ntw16s64.dll");
    attached=m?(BOOL (__cdecl *)(HWND))(void *)GetProcAddress(m,"Ntw16sWindowAttached"):NULL;
    BOOL result;HGLOBAL devmode=NULL,devnames=NULL;
    if(!lstrcmpW(argv[1],L"invalid")){
        PRINTDLGEXW x={0};x.lStructSize=sizeof(x);HRESULT hr=PrintDlgExW(&x);
        printf("PRINT_EX_INVALID bits=%u hr=%08lx\n",(unsigned)(sizeof(void*)*8),(DWORD)hr);
        return hr==HRESULT_FROM_WIN32(ERROR_INVALID_HANDLE)?0:1;
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
