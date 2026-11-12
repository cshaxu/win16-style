#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "lib-style/win16-dialog/folder_create.h"
static DWORD dialog_thread;
static HANDLE submitted;
static BOOL CALLBACK submit(HWND window,LPARAM unused)
{
    (void)unused;WCHAR title[64];
    if(GetWindowTextW(window,title,64) && !lstrcmpW(title,L"New Folder") && GetDlgItem(window,102)){
        SetDlgItemTextW(window,102,L"owned-child");
        PostMessageW(window,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)GetDlgItem(window,IDOK));
        SetEvent(submitted);return FALSE;
    }
    return TRUE;
}
static DWORD WINAPI drive(void *unused)
{
    (void)unused;
    for(int i=0;i<100 && WaitForSingleObject(submitted,0)==WAIT_TIMEOUT;++i){EnumThreadWindows(dialog_thread,submit,0);Sleep(20);}
    return 0;
}
int main(void)
{
    WCHAR tmp[MAX_PATH],parent[MAX_PATH],result[MAX_PATH],expected[MAX_PATH];
    if(!GetTempPathW(MAX_PATH,tmp) || !GetTempFileNameW(tmp,L"w16",0,parent))return 1;
    if(!DeleteFileW(parent) || !CreateDirectoryW(parent,NULL))return 2;
    if(swprintf(expected,MAX_PATH,L"%ls\\owned-child",parent)<0)return 3;
    dialog_thread=GetCurrentThreadId();submitted=CreateEventW(NULL,TRUE,FALSE,NULL);
    HANDLE worker=submitted?CreateThread(NULL,0,drive,NULL,0,NULL):NULL;
    if(!worker){if(submitted)CloseHandle(submitted);return 4;}
    BOOL ok=win16_dialog_create_folder(NULL,parent,result,MAX_PATH);
    DWORD waited=WaitForSingleObject(worker,3000);CloseHandle(worker);CloseHandle(submitted);submitted=NULL;
    DWORD attrs=GetFileAttributesW(expected);
    BOOL correct=ok && !lstrcmpiW(result,expected) && attrs!=INVALID_FILE_ATTRIBUTES && (attrs&FILE_ATTRIBUTE_DIRECTORY);
    BOOL cleanup=RemoveDirectoryW(expected) && RemoveDirectoryW(parent);
    printf("FOLDER_CREATE_LIB ok=%d correct=%d cleanup=%d wait=%lu\n",ok,correct,cleanup,(unsigned long)waited);
    return correct && cleanup && waited==WAIT_OBJECT_0?0:5;
}
