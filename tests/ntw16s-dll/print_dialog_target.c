#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <cderr.h>
#include <stdio.h>
#include <stdint.h>
static BOOL (__cdecl *attached)(HWND);static int initialized;static BOOL frame;
static DWORD main_thread;
static BOOL classic_test,classic_found,classic_accept;
static int extended_case,extended_step;
static BOOL extended_old,extended_option_state,initial_range_state,initial_device_state;
static HGLOBAL device_to_lock;static void *device_lock;static BOOL apply_initial,apply_changed,apply_done,apply_again,apply_done_again;
static uint64_t global_digest(HGLOBAL h){SIZE_T n=GlobalSize(h);BYTE *p=GlobalLock(h);uint64_t hash=14695981039346656037ull;if(!p)return 0;for(SIZE_T i=0;i<n;++i)hash=(hash^p[i])*1099511628211ull;GlobalUnlock(h);return hash;}
static HWND range_radio,range_edit;
static BOOL CALLBACK locate_range(HWND w,LPARAM data)
{(void)data;if(GetDlgCtrlID(w)==1059 || GetDlgCtrlID(w)==1058)range_radio=w;if(GetDlgCtrlID(w)==1152 && !range_edit)range_edit=w;if(GetDlgCtrlID(w)==9002)range_edit=w;return TRUE;}
static void CALLBACK close_dialog(HWND window,UINT message,UINT_PTR timer,DWORD time)
{(void)message;(void)time;
 if(extended_case>=28){extended_old=GetDlgItem(window,1139)!=NULL;int option=(extended_case-28)%3;HWND check=GetDlgItem(window,option==0?1041:1040);
    if(!extended_step){initial_range_state=check && (option==2?!IsWindowEnabled(check):IsWindowEnabled(check) && SendMessageW(check,BM_GETCHECK,0,0)==BST_CHECKED);
        if(extended_case>=31){SetDlgItemTextW(window,1154,option==0?L"3":L"1");HWND apply=GetDlgItem(window,9001);if(apply)SendMessageW(apply,BM_CLICK,0,0);extended_step=1;return;}}
    KillTimer(window,timer);frame=attached&&attached(window);PostMessageW(window,WM_COMMAND,IDCANCEL,0);return;}
 if(extended_case>=25){extended_old=GetDlgItem(window,1139)!=NULL;HWND select=GetDlgItem(window,1057);
    if(!extended_step){initial_range_state=select && IsWindowEnabled(select) && SendMessageW(select,BM_GETCHECK,0,0)==BST_CHECKED;
        if(extended_case==27){SetDlgItemTextW(window,1154,L"1");HWND apply=GetDlgItem(window,9001);if(apply)SendMessageW(apply,BM_CLICK,0,0);extended_step=1;return;}}
    KillTimer(window,timer);frame=attached&&attached(window);PostMessageW(window,WM_COMMAND,extended_case==26?IDOK:IDCANCEL,0);return;}
 if(extended_case>=22){extended_old=GetDlgItem(window,1139)!=NULL;
    if(!extended_step && extended_case==23){SetDlgItemTextW(window,1154,L"1");HWND apply=GetDlgItem(window,9001);if(apply)SendMessageW(apply,BM_CLICK,0,0);extended_step=1;return;}
    KillTimer(window,timer);frame=attached&&attached(window);PostMessageW(window,WM_COMMAND,extended_case==22?IDOK:IDCANCEL,0);return;}
 if(extended_case>=19){extended_old=GetDlgItem(window,1139)!=NULL;
    if(!extended_step && extended_case!=21){SetDlgItemTextW(window,1154,L"3");if(extended_case==20){HWND apply=GetDlgItem(window,9001);if(apply)SendMessageW(apply,BM_CLICK,0,0);SetDlgItemTextW(window,1154,L"5");extended_step=1;return;}}
    KillTimer(window,timer);frame=attached&&attached(window);PostMessageW(window,WM_COMMAND,extended_case==19?IDOK:IDCANCEL,0);return;}
 if(extended_case==18){HWND button=GetDlgItem(window,9001);extended_old=GetDlgItem(window,1139)!=NULL;
    if(!extended_step){apply_initial=button && !IsWindowEnabled(button);SetDlgItemTextW(window,1154,L"3");apply_changed=button && IsWindowEnabled(button);if(button)SendMessageW(button,BM_CLICK,0,0);apply_done=button && !IsWindowEnabled(button);SetDlgItemTextW(window,1154,L"4");apply_again=button && IsWindowEnabled(button);if(button)SendMessageW(button,BM_CLICK,0,0);apply_done_again=button && !IsWindowEnabled(button);SetDlgItemTextW(window,1154,L"5");extended_step=1;return;}
    KillTimer(window,timer);frame=attached&&attached(window);PostMessageW(window,WM_COMMAND,IDCANCEL,0);return;}
 if(extended_case>=14){extended_old=GetDlgItem(window,1139)!=NULL;
    if(!extended_step){WCHAR text[16]=L"";GetDlgItemTextW(window,1154,text,16);initial_device_state=!lstrcmpW(text,L"3");
        if(extended_case==17)device_lock=GlobalLock(device_to_lock);
        if(extended_case==15){SetDlgItemTextW(window,1154,L"4");HWND apply=GetDlgItem(window,9001);if(apply)SendMessageW(apply,BM_CLICK,0,0);SetDlgItemTextW(window,1154,L"5");extended_step=1;return;}}
    KillTimer(window,timer);frame=attached&&attached(window);PostMessageW(window,WM_COMMAND,(extended_case==14 || extended_case==17)?IDOK:IDCANCEL,0);return;}
 if(extended_case>=10){extended_old=GetDlgItem(window,1139)!=NULL;
    if(!extended_step){WCHAR text[80]=L"";HWND edit=GetDlgItem(window,9002);if(edit)GetWindowTextW(edit,text,80);initial_range_state=edit && !lstrcmpW(text,L"4-5,2") && SendDlgItemMessageW(window,1058,BM_GETCHECK,0,0)==BST_CHECKED;
      if(extended_case!=12)SetDlgItemTextW(window,1154,L"3");
      if(extended_case==13)SendDlgItemMessageW(window,1056,BM_CLICK,0,0);
      if(extended_case==11 || extended_case==13){HWND apply=GetDlgItem(window,9001);if(apply)SendMessageW(apply,BM_CLICK,0,0);SetDlgItemTextW(window,1154,L"5");SetWindowTextW(edit,L"8");extended_step=1;return;}}
    KillTimer(window,timer);frame=attached&&attached(window);PostMessageW(window,WM_COMMAND,extended_case==10?IDOK:IDCANCEL,0);return;}
 if(extended_case>=4){extended_old=GetDlgItem(window,1139)!=NULL;
    if(extended_case>=7){HWND select=GetDlgItem(window,1057),file=GetDlgItem(window,1040);extended_option_state=select && !IsWindowEnabled(select) && file && !IsWindowVisible(file);}
    if(extended_case==6 || extended_case==9){KillTimer(window,timer);frame=attached&&attached(window);PostMessageW(window,WM_COMMAND,IDCANCEL,0);return;}
    if(!extended_step){SetDlgItemTextW(window,1154,L"3");if(extended_case==5 || extended_case==8){HWND apply=GetDlgItem(window,9001);if(apply)SendMessageW(apply,BM_CLICK,0,0);SetDlgItemTextW(window,1154,L"5");extended_step=1;return;}}
    KillTimer(window,timer);frame=attached&&attached(window);PostMessageW(window,WM_COMMAND,(extended_case==4 || extended_case==7)?IDOK:IDCANCEL,0);return;}
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
    if(!lstrcmpW(argv[1],L"ex-nopages-print") || !lstrcmpW(argv[1],L"ex-nopages-apply") || !lstrcmpW(argv[1],L"ex-nopages-cancel") || !lstrcmpW(argv[1],L"ex-options-print") || !lstrcmpW(argv[1],L"ex-options-apply") || !lstrcmpW(argv[1],L"ex-options-cancel") || !lstrcmpW(argv[1],L"ex-nowarning-print") || !lstrcmpW(argv[1],L"ex-nowarning-apply") || !lstrcmpW(argv[1],L"ex-nowarning-cancel") || !lstrcmpW(argv[1],L"ex-nocurrent-print") || !lstrcmpW(argv[1],L"ex-nocurrent-apply") || !lstrcmpW(argv[1],L"ex-nocurrent-cancel")){
        BOOL options=!wcsncmp(argv[1],L"ex-options-",11),nowarning=!wcsncmp(argv[1],L"ex-nowarning-",13),nocurrent=!wcsncmp(argv[1],L"ex-nocurrent-",13);const WCHAR *action=argv[1]+((nowarning || nocurrent)?13:11);
        extended_case=!wcscmp(action,L"print")?(options?7:4):!wcscmp(action,L"apply")?(options?8:5):(options?9:6);
        HRESULT com=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(com))return 9;
        HWND owner=CreateWindowExW(0,L"STATIC",L"Owner",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);if(!owner)return 7;
        ShowWindow(owner,SW_SHOW);HHOOK observer=SetWindowsHookExW(WH_CALLWNDPROC,observe,NULL,GetCurrentThreadId());if(!observer)return 6;
        PRINTDLGEXW x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=PD_RETURNDC|PD_NOPAGENUMS|(options?(PD_NOSELECTION|PD_HIDEPRINTTOFILE):0)|(nowarning?PD_NOWARNING:0)|(nocurrent?PD_NOCURRENTPAGE:0);x.nStartPage=START_PAGE_GENERAL;x.nCopies=1;
        HRESULT hr=PrintDlgExW(&x);DWORD expected=(extended_case==4 || extended_case==7)?PD_RESULT_PRINT:(extended_case==5 || extended_case==8)?PD_RESULT_APPLY:PD_RESULT_CANCEL;
        BOOL valid=hr==S_OK && x.dwResultAction==expected && (x.Flags & PD_NOPAGENUMS) && (!nowarning || (x.Flags & PD_NOWARNING)) && (!nocurrent || (x.Flags & PD_NOCURRENTPAGE)) && (!options || ((x.Flags & (PD_NOSELECTION|PD_HIDEPRINTTOFILE))==(PD_NOSELECTION|PD_HIDEPRINTTOFILE) && extended_option_state)) && x.nPageRanges==0 && !x.lpPageRanges && extended_old && frame &&
            ((extended_case==6 || extended_case==9)?(!x.hDC && !x.hDevMode && !x.hDevNames):(x.hDC && x.hDevMode && x.hDevNames && x.nCopies==3));
        printf("EX_NOPAGES bits=%u mode=%d hr=%08lx action=%lu old=%d frame=%d valid=%d\n",(unsigned)(sizeof(void*)*8),extended_case,(DWORD)hr,x.dwResultAction,extended_old,frame,valid);
        if(x.hDC)DeleteDC(x.hDC);
        if(x.hDevMode)GlobalFree(x.hDevMode);
        if(x.hDevNames)GlobalFree(x.hDevNames);
        UnhookWindowsHookEx(observer);DestroyWindow(owner);CoUninitialize();return initialized==1 && valid?0:1;
    }
    if(!lstrcmpW(argv[1],L"ex-initial-option") || !lstrcmpW(argv[1],L"ex-option-apply")){
        if(argc!=4)return 2;
        DWORD opt=!lstrcmpW(argv[3],L"collate")?PD_COLLATE:!lstrcmpW(argv[3],L"file")?PD_PRINTTOFILE:!lstrcmpW(argv[3],L"disable")?PD_DISABLEPRINTTOFILE:0;if(!opt)return 2;
        extended_case=(opt==PD_COLLATE?28:opt==PD_PRINTTOFILE?29:30)+(!lstrcmpW(argv[1],L"ex-option-apply")?3:0);if(FAILED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)))return 9;
        HWND owner=CreateWindowExW(0,L"STATIC",L"Owner",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);if(!owner)return 7;
        ShowWindow(owner,SW_SHOW);HHOOK observer=SetWindowsHookExW(WH_CALLWNDPROC,observe,NULL,GetCurrentThreadId());if(!observer)return 6;
        DWORD expected=PD_RETURNDC|PD_NOPAGENUMS|opt,flags,action;HRESULT hr;BOOL port_ok=TRUE;
        if(ansi){PRINTDLGEXA x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=expected;x.nCopies=opt==PD_COLLATE?3:1;x.nStartPage=START_PAGE_GENERAL;hr=PrintDlgExA(&x);flags=x.Flags;action=x.dwResultAction;if(opt==PD_PRINTTOFILE && extended_case>=31){DEVNAMES *n=x.hDevNames?GlobalLock(x.hDevNames):NULL;port_ok=n && !lstrcmpA((char *)n+n->wOutputOffset,"FILE:");if(n)GlobalUnlock(x.hDevNames);}if(x.hDC)DeleteDC(x.hDC);if(x.hDevMode)GlobalFree(x.hDevMode);if(x.hDevNames)GlobalFree(x.hDevNames);}
        else{PRINTDLGEXW x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=expected;x.nCopies=opt==PD_COLLATE?3:1;x.nStartPage=START_PAGE_GENERAL;hr=PrintDlgExW(&x);flags=x.Flags;action=x.dwResultAction;if(opt==PD_PRINTTOFILE && extended_case>=31){DEVNAMES *n=x.hDevNames?GlobalLock(x.hDevNames):NULL;port_ok=n && !lstrcmpW((WCHAR *)n+n->wOutputOffset,L"FILE:");if(n)GlobalUnlock(x.hDevNames);}if(x.hDC)DeleteDC(x.hDC);if(x.hDevMode)GlobalFree(x.hDevMode);if(x.hDevNames)GlobalFree(x.hDevNames);}
        BOOL valid=hr==S_OK && action==(extended_case>=31?PD_RESULT_APPLY:PD_RESULT_CANCEL) && (flags&opt) && port_ok && initial_range_state && extended_old && frame;
        printf("EX_INITIAL_OPTION bits=%u ansi=%d opt=%lu action=%lu initial=%d port=%d old=%d valid=%d\n",(unsigned)(sizeof(void*)*8),ansi,opt,action,initial_range_state,port_ok,extended_old,valid);
        UnhookWindowsHookEx(observer);DestroyWindow(owner);CoUninitialize();return initialized==1 && valid?0:1;
    }
    if(!lstrcmpW(argv[1],L"ex-initial-selection") || !lstrcmpW(argv[1],L"ex-selection-print") || !lstrcmpW(argv[1],L"ex-selection-apply")){
        extended_case=!lstrcmpW(argv[1],L"ex-selection-print")?26:!lstrcmpW(argv[1],L"ex-selection-apply")?27:25;if(FAILED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)))return 9;
        HWND owner=CreateWindowExW(0,L"STATIC",L"Owner",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);if(!owner)return 7;
        ShowWindow(owner,SW_SHOW);HHOOK observer=SetWindowsHookExW(WH_CALLWNDPROC,observe,NULL,GetCurrentThreadId());if(!observer)return 6;
        DWORD expected=PD_RETURNDC|PD_NOPAGENUMS|PD_SELECTION,flags,action;HRESULT hr;
        if(ansi){PRINTDLGEXA x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=expected;x.nCopies=1;x.nStartPage=START_PAGE_GENERAL;hr=PrintDlgExA(&x);flags=x.Flags;action=x.dwResultAction;if(x.hDC)DeleteDC(x.hDC);if(x.hDevMode)GlobalFree(x.hDevMode);if(x.hDevNames)GlobalFree(x.hDevNames);}
        else{PRINTDLGEXW x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=expected;x.nCopies=1;x.nStartPage=START_PAGE_GENERAL;hr=PrintDlgExW(&x);flags=x.Flags;action=x.dwResultAction;if(x.hDC)DeleteDC(x.hDC);if(x.hDevMode)GlobalFree(x.hDevMode);if(x.hDevNames)GlobalFree(x.hDevNames);}
        DWORD wanted=extended_case==26?PD_RESULT_PRINT:extended_case==27?PD_RESULT_APPLY:PD_RESULT_CANCEL;
        BOOL valid=hr==S_OK && action==wanted && (flags&PD_SELECTION) && initial_range_state && extended_old && frame;
        printf("EX_INITIAL_SELECTION bits=%u ansi=%d action=%lu selected=%d old=%d valid=%d\n",(unsigned)(sizeof(void*)*8),ansi,action,initial_range_state,extended_old,valid);
        UnhookWindowsHookEx(observer);DestroyWindow(owner);CoUninitialize();return initialized==1 && valid?0:1;
    }
    if(!lstrcmpW(argv[1],L"ex-driver-print") || !lstrcmpW(argv[1],L"ex-driver-apply") || !lstrcmpW(argv[1],L"ex-driver-cancel")){
        extended_case=!lstrcmpW(argv[1],L"ex-driver-print")?22:!lstrcmpW(argv[1],L"ex-driver-apply")?23:24;
        if(FAILED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)))return 9;
        HWND owner=CreateWindowExW(0,L"STATIC",L"Owner",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);if(!owner)return 7;
        ShowWindow(owner,SW_SHOW);HHOOK observer=SetWindowsHookExW(WH_CALLWNDPROC,observe,NULL,GetCurrentThreadId());if(!observer)return 6;
        HGLOBAL mode=NULL,names=NULL;HDC dc=NULL;HRESULT hr;DWORD action,copies,flags;
        if(ansi){PRINTDLGEXA x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=PD_RETURNDC|PD_NOPAGENUMS|PD_USEDEVMODECOPIESANDCOLLATE;x.nCopies=1;x.nStartPage=START_PAGE_GENERAL;hr=PrintDlgExA(&x);mode=x.hDevMode;names=x.hDevNames;dc=x.hDC;action=x.dwResultAction;copies=x.nCopies;flags=x.Flags;}
        else{PRINTDLGEXW x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=PD_RETURNDC|PD_NOPAGENUMS|PD_USEDEVMODECOPIESANDCOLLATE;x.nCopies=1;x.nStartPage=START_PAGE_GENERAL;hr=PrintDlgExW(&x);mode=x.hDevMode;names=x.hDevNames;dc=x.hDC;action=x.dwResultAction;copies=x.nCopies;flags=x.Flags;}
        DWORD expected=extended_case==22?PD_RESULT_PRINT:extended_case==23?PD_RESULT_APPLY:PD_RESULT_CANCEL;
        BOOL valid=hr==S_OK && action==expected && copies==1 && (flags&PD_USEDEVMODECOPIESANDCOLLATE) && !(flags&PD_COLLATE) && extended_old && frame &&
            (extended_case==24?(!mode && !names && !dc):(mode && names && dc));
        printf("EX_DRIVER bits=%u ansi=%d action=%lu copies=%lu old=%d valid=%d\n",(unsigned)(sizeof(void*)*8),ansi,action,copies,extended_old,valid);
        if(dc)DeleteDC(dc);
        if(mode)GlobalFree(mode);
        if(names)GlobalFree(names);
        UnhookWindowsHookEx(observer);DestroyWindow(owner);CoUninitialize();return initialized==1 && valid?0:1;
    }
    if(!lstrcmpW(argv[1],L"ex-context-print") || !lstrcmpW(argv[1],L"ex-context-apply") || !lstrcmpW(argv[1],L"ex-context-cancel")){
        if(argc!=4)return 2;
        extended_case=!lstrcmpW(argv[1],L"ex-context-print")?19:!lstrcmpW(argv[1],L"ex-context-apply")?20:21;
        DWORD requested=!lstrcmpW(argv[3],L"ic")?PD_RETURNIC:!lstrcmpW(argv[3],L"both")?(PD_RETURNIC|PD_RETURNDC):0;
        HRESULT com=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(com))return 9;
        HWND owner=CreateWindowExW(0,L"STATIC",L"Owner",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);if(!owner)return 7;
        ShowWindow(owner,SW_SHOW);HHOOK observer=SetWindowsHookExW(WH_CALLWNDPROC,observe,NULL,GetCurrentThreadId());if(!observer)return 6;
        HGLOBAL mode=NULL,names=NULL;HDC dc=NULL;HRESULT hr;DWORD action,copies,flags;
        if(ansi){PRINTDLGEXA x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=requested|PD_NOPAGENUMS;x.nCopies=1;x.nStartPage=START_PAGE_GENERAL;hr=PrintDlgExA(&x);mode=x.hDevMode;names=x.hDevNames;dc=x.hDC;action=x.dwResultAction;copies=x.nCopies;flags=x.Flags;}
        else{PRINTDLGEXW x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=requested|PD_NOPAGENUMS;x.nCopies=1;x.nStartPage=START_PAGE_GENERAL;hr=PrintDlgExW(&x);mode=x.hDevMode;names=x.hDevNames;dc=x.hDC;action=x.dwResultAction;copies=x.nCopies;flags=x.Flags;}
        DWORD expected=extended_case==19?PD_RESULT_PRINT:extended_case==20?PD_RESULT_APPLY:PD_RESULT_CANCEL;
        BOOL valid=hr==S_OK && action==expected && (flags & (PD_RETURNDC|PD_RETURNIC))==requested && extended_old && frame &&
            (extended_case==21?(!mode && !names && !dc):(mode && names && copies==3 && (!requested || dc)));
        printf("EX_CONTEXT bits=%u ansi=%d mode=%d requested=%lu hr=%08lx action=%lu flags=%lu copies=%lu dc=%d mode_handle=%d names=%d old=%d frame=%d valid=%d\n",(unsigned)(sizeof(void*)*8),ansi,extended_case,requested,(DWORD)hr,action,flags,copies,dc!=NULL,mode!=NULL,names!=NULL,extended_old,frame,valid);
        if(dc)DeleteDC(dc);
        if(mode)GlobalFree(mode);
        if(names)GlobalFree(names);
        UnhookWindowsHookEx(observer);DestroyWindow(owner);CoUninitialize();return initialized==1 && valid?0:1;
    }
    if(!lstrcmpW(argv[1],L"ex-apply-state")){
        extended_case=18;HRESULT com=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(com))return 9;
        HWND owner=CreateWindowExW(0,L"STATIC",L"Owner",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);if(!owner)return 7;
        ShowWindow(owner,SW_SHOW);HHOOK observer=SetWindowsHookExW(WH_CALLWNDPROC,observe,NULL,GetCurrentThreadId());if(!observer)return 6;
        HGLOBAL mode=NULL,names=NULL;HDC dc=NULL;HRESULT hr;DWORD action,copies;
        if(ansi){PRINTDLGEXA x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=PD_RETURNDC|PD_NOPAGENUMS;x.nCopies=1;x.nStartPage=START_PAGE_GENERAL;hr=PrintDlgExA(&x);mode=x.hDevMode;names=x.hDevNames;dc=x.hDC;action=x.dwResultAction;copies=x.nCopies;}
        else{PRINTDLGEXW x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=PD_RETURNDC|PD_NOPAGENUMS;x.nCopies=1;x.nStartPage=START_PAGE_GENERAL;hr=PrintDlgExW(&x);mode=x.hDevMode;names=x.hDevNames;dc=x.hDC;action=x.dwResultAction;copies=x.nCopies;}
        BOOL valid=hr==S_OK && action==PD_RESULT_APPLY && copies==4 && mode && names && dc && extended_old && frame && apply_initial && apply_changed && apply_done && apply_again && apply_done_again;
        printf("EX_APPLY_STATE bits=%u ansi=%d initial=%d changed=%d applied=%d again=%d applied_again=%d copies=%lu old=%d valid=%d\n",(unsigned)(sizeof(void*)*8),ansi,apply_initial,apply_changed,apply_done,apply_again,apply_done_again,copies,extended_old,valid);
        if(dc)DeleteDC(dc);
        if(mode)GlobalFree(mode);
        if(names)GlobalFree(names);
        UnhookWindowsHookEx(observer);DestroyWindow(owner);CoUninitialize();return initialized==1 && valid?0:1;
    }
    if(!lstrcmpW(argv[1],L"ex-device-print") || !lstrcmpW(argv[1],L"ex-device-apply") || !lstrcmpW(argv[1],L"ex-device-cancel") || !lstrcmpW(argv[1],L"ex-device-locked")){
        extended_case=!lstrcmpW(argv[1],L"ex-device-print")?14:!lstrcmpW(argv[1],L"ex-device-apply")?15:!lstrcmpW(argv[1],L"ex-device-locked")?17:16;
        HRESULT com=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(com))return 9;
        HWND owner=CreateWindowExW(0,L"STATIC",L"Owner",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);if(!owner)return 7;
        ShowWindow(owner,SW_SHOW);HHOOK observer=SetWindowsHookExW(WH_CALLWNDPROC,observe,NULL,GetCurrentThreadId());if(!observer)return 6;
        HGLOBAL mode=NULL,names=NULL;BOOL default_ok;
        if(ansi){PRINTDLGA x={0};x.lStructSize=sizeof(x);x.Flags=PD_RETURNDEFAULT;default_ok=PrintDlgA(&x);mode=x.hDevMode;names=x.hDevNames;}
        else{PRINTDLGW x={0};x.lStructSize=sizeof(x);x.Flags=PD_RETURNDEFAULT;default_ok=PrintDlgW(&x);mode=x.hDevMode;names=x.hDevNames;}
        if(!default_ok || !mode || !names)return 8;
        if(ansi){DEVMODEA *d=GlobalLock(mode);if(!d)return 8;d->dmFields|=DM_COPIES;d->dmCopies=3;GlobalUnlock(mode);}
        else{DEVMODEW *d=GlobalLock(mode);if(!d)return 8;d->dmFields|=DM_COPIES;d->dmCopies=3;GlobalUnlock(mode);}
        HGLOBAL oldmode=mode,oldnames=names;device_to_lock=mode;uint64_t mode_before=global_digest(mode),names_before=global_digest(names);HDC dc=NULL;HRESULT hr;DWORD action=0,copies=0;
        if(ansi){PRINTDLGEXA x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=PD_RETURNDC|PD_NOPAGENUMS;x.hDevMode=mode;x.hDevNames=names;x.nCopies=1;x.nStartPage=START_PAGE_GENERAL;hr=PrintDlgExA(&x);mode=x.hDevMode;names=x.hDevNames;dc=x.hDC;action=x.dwResultAction;copies=x.nCopies;}
        else{PRINTDLGEXW x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=PD_RETURNDC|PD_NOPAGENUMS;x.hDevMode=mode;x.hDevNames=names;x.nCopies=1;x.nStartPage=START_PAGE_GENERAL;hr=PrintDlgExW(&x);mode=x.hDevMode;names=x.hDevNames;dc=x.hDC;action=x.dwResultAction;copies=x.nCopies;}
        if(device_lock)GlobalUnlock(oldmode);
        BOOL oldmode_live=GlobalFlags(oldmode)!=GMEM_INVALID_HANDLE,oldnames_live=GlobalFlags(oldnames)!=GMEM_INVALID_HANDLE;
        DWORD expected=extended_case==14?PD_RESULT_PRINT:extended_case==15?PD_RESULT_APPLY:PD_RESULT_CANCEL;
        BOOL preserved=mode==oldmode && names==oldnames && oldmode_live && oldnames_live && global_digest(mode)==mode_before && global_digest(names)==names_before;
        BOOL replaced=mode && names && dc && mode!=oldmode && names!=oldnames && !oldmode_live && !oldnames_live;
        BOOL valid=extended_old && frame && initial_device_state && (extended_case==17?(hr==E_FAIL && device_lock && preserved && !dc):(hr==S_OK && action==expected && (extended_case==16?preserved:(replaced && copies==(extended_case==15?4:3)))));
        printf("EX_DEVICE bits=%u ansi=%d mode=%d hr=%08lx action=%lu initial=%d preserved=%d replaced=%d copies=%lu old=%d valid=%d\n",(unsigned)(sizeof(void*)*8),ansi,extended_case,(DWORD)hr,action,initial_device_state,preserved,replaced,copies,extended_old,valid);
        if(dc)DeleteDC(dc);
        if(mode)GlobalFree(mode);
        if(names)GlobalFree(names);
        if(oldmode!=mode && oldmode_live)GlobalFree(oldmode);
        if(oldnames!=names && oldnames_live)GlobalFree(oldnames);
        UnhookWindowsHookEx(observer);DestroyWindow(owner);CoUninitialize();return initialized==1 && valid?0:1;
    }
    if(!lstrcmpW(argv[1],L"ex-initial-print") || !lstrcmpW(argv[1],L"ex-initial-apply") || !lstrcmpW(argv[1],L"ex-initial-cancel") || !lstrcmpW(argv[1],L"ex-initial-all")){
        extended_case=!lstrcmpW(argv[1],L"ex-initial-print")?10:!lstrcmpW(argv[1],L"ex-initial-apply")?11:!lstrcmpW(argv[1],L"ex-initial-all")?13:12;
        HRESULT com=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(com))return 9;
        HWND owner=CreateWindowExW(0,L"STATIC",L"Owner",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);if(!owner)return 7;
        ShowWindow(owner,SW_SHOW);HHOOK observer=SetWindowsHookExW(WH_CALLWNDPROC,observe,NULL,GetCurrentThreadId());if(!observer)return 6;
        PRINTPAGERANGE ranges[10]={{4,5},{2,2}};HGLOBAL mode=NULL,names=NULL;HDC dc=NULL;HRESULT hr;DWORD action=0,flags=0,count=0,copies=0;
        if(ansi){PRINTDLGEXA x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=PD_RETURNDC|PD_PAGENUMS;x.nPageRanges=2;x.nMaxPageRanges=10;x.lpPageRanges=ranges;x.nMinPage=1;x.nMaxPage=1000;x.nCopies=1;x.nStartPage=START_PAGE_GENERAL;
            hr=PrintDlgExA(&x);mode=x.hDevMode;names=x.hDevNames;dc=x.hDC;action=x.dwResultAction;flags=x.Flags;count=x.nPageRanges;copies=x.nCopies;}
        else{PRINTDLGEXW x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=PD_RETURNDC|PD_PAGENUMS;x.nPageRanges=2;x.nMaxPageRanges=10;x.lpPageRanges=ranges;x.nMinPage=1;x.nMaxPage=1000;x.nCopies=1;x.nStartPage=START_PAGE_GENERAL;
            hr=PrintDlgExW(&x);mode=x.hDevMode;names=x.hDevNames;dc=x.hDC;action=x.dwResultAction;flags=x.Flags;count=x.nPageRanges;copies=x.nCopies;}
        DWORD expected=extended_case==10?PD_RESULT_PRINT:(extended_case==11 || extended_case==13)?PD_RESULT_APPLY:PD_RESULT_CANCEL;
        BOOL valid=hr==S_OK && action==expected && (extended_case==13?(!(flags & PD_PAGENUMS) && count==0):((flags & PD_PAGENUMS) && count==2)) && ranges[0].nFromPage==4 && ranges[0].nToPage==5 && ranges[1].nFromPage==2 && ranges[1].nToPage==2 && extended_old && frame && initial_range_state &&
            (extended_case==12?(!dc && !mode && !names && copies==1):(dc && mode && names && copies==3));
        printf("EX_INITIAL bits=%u ansi=%d mode=%d hr=%08lx action=%lu initial=%d ranges=%lu old=%d valid=%d\n",(unsigned)(sizeof(void*)*8),ansi,extended_case,(DWORD)hr,action,initial_range_state,count,extended_old,valid);
        if(dc)DeleteDC(dc);
        if(mode)GlobalFree(mode);
        if(names)GlobalFree(names);
        UnhookWindowsHookEx(observer);DestroyWindow(owner);CoUninitialize();return initialized==1 && valid?0:1;
    }
    if(!lstrcmpW(argv[1],L"ex-ansi-cancel") || !lstrcmpW(argv[1],L"ex-ansi-fallback")){
        BOOL fallback=!lstrcmpW(argv[1],L"ex-ansi-fallback");extended_case=3;
        HRESULT com=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(com))return 9;
        HWND owner=CreateWindowExW(0,L"STATIC",L"Owner",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);if(!owner)return 7;
        ShowWindow(owner,SW_SHOW);HHOOK observer=SetWindowsHookExW(WH_CALLWNDPROC,observe,NULL,GetCurrentThreadId());if(!observer)return 6;
        PRINTPAGERANGE ranges[10]={{77,88}};PRINTDLGEXA x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=PD_RETURNDC|(fallback?PD_CURRENTPAGE:0);x.nStartPage=START_PAGE_GENERAL;x.nMaxPageRanges=10;x.lpPageRanges=ranges;x.nMinPage=1;x.nMaxPage=1000;x.nCopies=1;
        HRESULT hr=PrintDlgExA(&x);BOOL valid=hr==S_OK && x.dwResultAction==PD_RESULT_CANCEL && x.Flags==(PD_RETURNDC|(fallback?PD_CURRENTPAGE:0)) && !x.hDC && !x.hDevMode && !x.hDevNames && ranges[0].nFromPage==77 && ranges[0].nToPage==88 && extended_old!=fallback;
        printf("EX_ANSI_CANCEL bits=%u fallback=%d old=%d hr=%08lx action=%lu untouched=%d frame=%d\n",(unsigned)(sizeof(void*)*8),fallback,extended_old,(DWORD)hr,x.dwResultAction,valid,frame);
        if(x.hDC)DeleteDC(x.hDC);
        if(x.hDevMode)GlobalFree(x.hDevMode);
        if(x.hDevNames)GlobalFree(x.hDevNames);
        UnhookWindowsHookEx(observer);DestroyWindow(owner);CoUninitialize();return initialized==1 && frame && valid?0:1;
    }
    if(!lstrcmpW(argv[1],L"ex-ansi-ranges") || !lstrcmpW(argv[1],L"ex-ansi-apply")){
        extended_case=!lstrcmpW(argv[1],L"ex-ansi-apply")?2:1;
        HRESULT com=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(com))return 9;
        HWND owner=CreateWindowExW(0,L"STATIC",L"Owner",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);if(!owner)return 7;
        ShowWindow(owner,SW_SHOW);HHOOK observer=SetWindowsHookExW(WH_CALLWNDPROC,observe,NULL,GetCurrentThreadId());if(!observer)return 6;
        PRINTPAGERANGE ranges[10]={0};PRINTDLGEXA x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=PD_RETURNDC;
        x.nStartPage=START_PAGE_GENERAL;x.nMaxPageRanges=10;x.lpPageRanges=ranges;x.nMinPage=1;x.nMaxPage=1000;x.nCopies=1;
        HRESULT hr=PrintDlgExA(&x);DEVNAMES *names=x.hDevNames?GlobalLock(x.hDevNames):NULL;DEVMODEA *mode=x.hDevMode?GlobalLock(x.hDevMode):NULL;
        BOOL layout=names && mode && GlobalSize(x.hDevMode)>=sizeof(DEVMODEA) && mode->dmSize==sizeof(DEVMODEA) && names->wDeviceOffset>=sizeof(DEVNAMES) &&
            names->wDeviceOffset<GlobalSize(x.hDevNames) && !lstrcmpA((char*)names+names->wDeviceOffset,(char*)mode->dmDeviceName);
        BOOL valid=hr==S_OK && x.dwResultAction==(extended_case==2?PD_RESULT_APPLY:PD_RESULT_PRINT) && x.nPageRanges==2 && ranges[0].nFromPage==2 && ranges[0].nToPage==2 && ranges[1].nFromPage==4 && ranges[1].nToPage==5 && x.hDC && x.nCopies==3 && layout && extended_old && frame;
        printf("EX_ANSI bits=%u mode=%d hr=%08lx action=%lu ranges=%lu copies=%lu ansi_layout=%d old=%d frame=%d valid=%d\n",(unsigned)(sizeof(void*)*8),extended_case,(DWORD)hr,x.dwResultAction,x.nPageRanges,x.nCopies,layout,extended_old,frame,valid);
        if(names)GlobalUnlock(x.hDevNames);
        if(mode)GlobalUnlock(x.hDevMode);
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
