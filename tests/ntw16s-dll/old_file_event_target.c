/* Confirm actual old common-dialog FILEOKSTRING timing and veto semantics. */
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <wchar.h>
static DWORD thread_id;static volatile LONG found,calls,valid,kept;static HWND dialog;static UINT fileok;
static WCHAR expected[MAX_PATH];static const WCHAR title[]=L"Win16S old file event";
static UINT_PTR CALLBACK hook(HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    (void)w;(void)wp;
    if(message==fileok){OPENFILENAMEW *ofn=(OPENFILENAMEW *)lp;LONG count=InterlockedIncrement(&calls);
        if(ofn&&ofn->lpstrFile&&!lstrcmpiW(expected,ofn->lpstrFile))InterlockedIncrement(&valid);
        return count==1?1:0;
    }
    return 0;
}
static BOOL CALLBACK scan(HWND w,LPARAM arg)
{
    (void)arg;WCHAR name[100];GetWindowTextW(w,name,ARRAYSIZE(name));
    if(IsWindowVisible(w)&&!lstrcmpW(name,title)&&GetDlgItem(w,1121)){dialog=w;InterlockedExchange(&found,1);return FALSE;}
    return TRUE;
}
static DWORD WINAPI observe(void *unused)
{
    (void)unused;BOOL first=FALSE,second=FALSE;
    for(int i=0;i<150;++i){if(!found)EnumThreadWindows(thread_id,scan,0);
        if(found && IsWindow(dialog) && i>10 && !first){HWND ok=GetDlgItem(dialog,IDOK);PostMessageW(dialog,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)ok);first=TRUE;}
        if(first && !second && calls>=1 && IsWindow(dialog)){InterlockedExchange(&kept,1);HWND ok=GetDlgItem(dialog,IDOK);PostMessageW(dialog,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)ok);second=TRUE;}
        if(second && !IsWindow(dialog))return 0;
        Sleep(50);}
    PostThreadMessageW(thread_id,WM_QUIT,0,0);return 1;
}
int wmain(void)
{
    WCHAR temp[MAX_PATH];if(!GetTempPathW(MAX_PATH,temp)||!GetTempFileNameW(temp,L"w16",0,expected))return 2;
    fileok=RegisterWindowMessageW(FILEOKSTRINGW);if(!fileok)return 3;
    WCHAR path[MAX_PATH];lstrcpyW(path,expected);OPENFILENAMEW o={0};o.lStructSize=sizeof(o);o.lpstrFile=path;o.nMaxFile=MAX_PATH;
    o.lpstrTitle=title;o.lpstrFilter=L"All files\0*.*\0\0";o.Flags=OFN_ENABLEHOOK|OFN_FILEMUSTEXIST|OFN_NOCHANGEDIR;o.lpfnHook=hook;
    thread_id=GetCurrentThreadId();HANDLE worker=CreateThread(NULL,0,observe,NULL,0,NULL);if(!worker)return 4;
    BOOL result=GetOpenFileNameW(&o);DWORD error=CommDlgExtendedError(),wait=WaitForSingleObject(worker,10000);CloseHandle(worker);DeleteFileW(expected);
    printf("OLD_FILE_EVENT bits=%u seen=%ld calls=%ld valid=%ld kept=%ld result=%d error=%lu wait=%lu\n",(unsigned)(sizeof(void*)*8),found,calls,valid,kept,result,error,wait);
    return found&&calls==2&&valid==2&&kept&&result&&!error&&wait==WAIT_OBJECT_0?0:1;
}
