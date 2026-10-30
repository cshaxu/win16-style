#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <winspool.h>
#include <objbase.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
static BOOL use_apply,unfile;static int seen,phase;static HWND copies;
static BOOL CALLBACK find_copies(HWND w,LPARAM id)
{if(GetDlgCtrlID(w)==(int)id)copies=w;return TRUE;}
static VOID CALLBACK drive(HWND w,UINT message,UINT_PTR timer,DWORD time)
{
    (void)message;(void)time;BOOL old=GetDlgItem(w,1139)!=NULL;
    if(!phase){EnumChildWindows(w,find_copies,old?1154:1153);
        if(copies)SetWindowTextW(copies,L"3");
        if(unfile){copies=NULL;EnumChildWindows(w,find_copies,old?1040:1002);if(copies && SendMessageW(copies,BM_GETCHECK,0,0)==BST_CHECKED)SendMessageW(copies,BM_CLICK,0,0);}
        if(use_apply){HWND apply=GetDlgItem(w,old?9001:12321);if(apply)SendMessageW(apply,BM_CLICK,0,0);phase=1;return;}
        KillTimer(w,timer);PostMessageW(w,WM_COMMAND,IDOK,0);return;}
    KillTimer(w,timer);PostMessageW(w,WM_COMMAND,IDCANCEL,0);
}
static LRESULT CALLBACK observe(int code,WPARAM wparam,LPARAM lparam)
{
    if(code>=0 && !seen){CWPSTRUCT *m=(CWPSTRUCT*)lparam;
        if(m->message==WM_SHOWWINDOW && m->wParam){WCHAR cls[20];GetClassNameW(m->hwnd,cls,20);
            if(!lstrcmpW(cls,L"#32770") && GetDlgItem(m->hwnd,IDCANCEL)){seen=1;SetTimer(m->hwnd,1,200,drive);}}}
    return CallNextHookEx(NULL,code,wparam,lparam);
}
static BOOL virtual_printer(HANDLE printer)
{
    DWORD bytes=0;GetPrinterW(printer,2,NULL,0,&bytes);if(!bytes)return FALSE;
    PRINTER_INFO_2W *info=HeapAlloc(GetProcessHeap(),0,bytes);if(!info)return FALSE;
    BOOL valid=GetPrinterW(printer,2,(BYTE*)info,bytes,&bytes) && info->pPrinterName && info->pDriverName && info->pPortName &&
        !lstrcmpW(info->pPrinterName,L"Microsoft Print to PDF") && !lstrcmpW(info->pDriverName,L"Microsoft Print To PDF") && !lstrcmpW(info->pPortName,L"PORTPROMPT:");
    HeapFree(GetProcessHeap(),0,info);return valid;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc!=4)return 2;
    setvbuf(stdout,NULL,_IONBF,0);unfile=!lstrcmpW(argv[1],L"unfile");use_apply=unfile || !lstrcmpW(argv[1],L"apply");BOOL ansi=!lstrcmpW(argv[3],L"ansi");
    if((!use_apply && lstrcmpW(argv[1],L"print")) || (!ansi && lstrcmpW(argv[3],L"wide")))return 2;
    const WCHAR *printer=L"Microsoft Print to PDF",*driver=L"Microsoft Print To PDF",*port=unfile?L"FILE:":L"PORTPROMPT:";
    HANDLE h=NULL;if(!OpenPrinterW((WCHAR*)printer,&h,NULL)){printf("VIRTUAL_SKIP printer_unavailable error=%lu\n",GetLastError());return 77;}
    if(!virtual_printer(h)){ClosePrinter(h);puts("VIRTUAL_SKIP printer_configuration_mismatch");return 77;}
    LONG bytes=ansi?DocumentPropertiesA(NULL,h,"Microsoft Print to PDF",NULL,NULL,0):DocumentPropertiesW(NULL,h,(WCHAR*)printer,NULL,NULL,0);
    if(bytes<=0){ClosePrinter(h);return 4;}
    HGLOBAL mode=GlobalAlloc(GMEM_MOVEABLE,(SIZE_T)bytes);void *dm=mode?GlobalLock(mode):NULL;
    if(!dm){ClosePrinter(h);return 5;}
    LONG result=ansi?DocumentPropertiesA(NULL,h,"Microsoft Print to PDF",dm,NULL,DM_OUT_BUFFER):DocumentPropertiesW(NULL,h,(WCHAR*)printer,dm,NULL,DM_OUT_BUFFER);
    GlobalUnlock(mode);ClosePrinter(h);if(result!=IDOK){GlobalFree(mode);return 6;}
    HGLOBAL names=NULL;DEVNAMES *dn=NULL;
    if(ansi){const char *d="Microsoft Print To PDF",*p="Microsoft Print to PDF",*o=unfile?"FILE:":"PORTPROMPT:";
        size_t dl=strlen(d)+1,pl=strlen(p)+1,ol=strlen(o)+1;names=GlobalAlloc(GMEM_MOVEABLE,sizeof(DEVNAMES)+dl+pl+ol);dn=names?GlobalLock(names):NULL;
        if(dn){dn->wDriverOffset=sizeof(DEVNAMES);dn->wDeviceOffset=(WORD)(sizeof(DEVNAMES)+dl);dn->wOutputOffset=(WORD)(sizeof(DEVNAMES)+dl+pl);dn->wDefault=0;
            strcpy((char*)dn+dn->wDriverOffset,d);strcpy((char*)dn+dn->wDeviceOffset,p);strcpy((char*)dn+dn->wOutputOffset,o);}}
    else{size_t dl=wcslen(driver)+1,pl=wcslen(printer)+1,ol=wcslen(port)+1,head=(sizeof(DEVNAMES)+sizeof(WCHAR)-1)/sizeof(WCHAR);
        names=GlobalAlloc(GMEM_MOVEABLE,(head+dl+pl+ol)*sizeof(WCHAR));dn=names?GlobalLock(names):NULL;
        if(dn){dn->wDriverOffset=(WORD)head;dn->wDeviceOffset=(WORD)(head+dl);dn->wOutputOffset=(WORD)(head+dl+pl);dn->wDefault=0;
            wcscpy((WCHAR*)dn+dn->wDriverOffset,driver);wcscpy((WCHAR*)dn+dn->wDeviceOffset,printer);wcscpy((WCHAR*)dn+dn->wOutputOffset,port);}}
    if(!dn){GlobalFree(mode);if(names)GlobalFree(names);return 7;}GlobalUnlock(names);
    if(FAILED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)))return 8;
    HWND owner=CreateWindowExW(0,L"STATIC",L"Owner",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!owner)return 9;
    ShowWindow(owner,SW_SHOW);HHOOK hook=SetWindowsHookExW(WH_CALLWNDPROC,observe,NULL,GetCurrentThreadId());if(!hook)return 10;
    HRESULT hr;HDC dc=NULL;DWORD action=0,copies_count=0,flags=0;
    if(ansi){PRINTDLGEXA x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=PD_RETURNDC|PD_NOPAGENUMS|(unfile?PD_PRINTTOFILE:0);x.hDevMode=mode;x.hDevNames=names;x.nCopies=1;x.nStartPage=START_PAGE_GENERAL;
        hr=PrintDlgExA(&x);mode=x.hDevMode;names=x.hDevNames;dc=x.hDC;action=x.dwResultAction;copies_count=x.nCopies;flags=x.Flags;}
    else{PRINTDLGEXW x={0};x.lStructSize=sizeof(x);x.hwndOwner=owner;x.Flags=PD_RETURNDC|PD_NOPAGENUMS|(unfile?PD_PRINTTOFILE:0);x.hDevMode=mode;x.hDevNames=names;x.nCopies=1;x.nStartPage=START_PAGE_GENERAL;
        hr=PrintDlgExW(&x);mode=x.hDevMode;names=x.hDevNames;dc=x.hDC;action=x.dwResultAction;copies_count=x.nCopies;flags=x.Flags;}
    dn=names?GlobalLock(names):NULL;BOOL match=dn && (ansi?!lstrcmpA((char*)dn+dn->wDeviceOffset,"Microsoft Print to PDF"):!lstrcmpW((WCHAR*)dn+dn->wDeviceOffset,printer));
    BOOL port_match=!unfile || (dn && (ansi?!lstrcmpA((char*)dn+dn->wOutputOffset,"PORTPROMPT:"):!lstrcmpW((WCHAR*)dn+dn->wOutputOffset,L"PORTPROMPT:")));
    if(dn)GlobalUnlock(names);
    BOOL ok=hr==S_OK && seen && match && port_match && (!unfile || !(flags&PD_PRINTTOFILE)) && dc && copies_count==3 && action==(use_apply?PD_RESULT_APPLY:PD_RESULT_PRINT);
    if(unfile)printf("VIRTUAL_UNFILE port_match=%d flag_cleared=%d\n",port_match,(flags&PD_PRINTTOFILE)==0);
    printf("VIRTUAL_RESULT bits=%u ansi=%d hr=%08lx action=%lu copies=%lu dc=%d printer_match=%d seen=%d\n",(unsigned)(sizeof(void*)*8),ansi,(DWORD)hr,action,copies_count,dc!=NULL,match,seen);
    if(ok){DOCINFOW info={0};info.cbSize=sizeof(info);info.lpszDocName=L"Win16Style virtual regression";info.lpszOutput=argv[2];int job=StartDocW(dc,&info);
        if(job>0){int page=StartPage(dc);BOOL written=page>0 && TextOutW(dc,100,100,L"win16style virtual print probe",29);int endpage=page>0?EndPage(dc):0;int enddoc=endpage>0?EndDoc(dc):0;
            if(enddoc<=0)AbortDoc(dc);
            printf("VIRTUAL_END page=%d text=%d endpage=%d enddoc=%d\n",page,written,endpage,enddoc);ok=page>0 && written && endpage>0 && enddoc>0;}
        else{printf("VIRTUAL_START_FAIL error=%lu\n",GetLastError());ok=FALSE;}}
    if(dc)DeleteDC(dc);
    if(mode)GlobalFree(mode);
    if(names)GlobalFree(names);
    UnhookWindowsHookEx(hook);DestroyWindow(owner);CoUninitialize();return ok?0:1;
}
