#define _UNICODE
#define COBJMACROS
#include <windows.h>
#include <shobjidl.h>
#include <shlobj.h>
#include <stdio.h>
static IFileDialog *dialog;
static DWORD ui;
static volatile LONG done,seen,correct,old;
static const WCHAR title[]=L"Win16S root probe";
static WCHAR drive_root[MAX_PATH];
static void CALLBACK tick(HWND w,UINT message,UINT_PTR id,DWORD time)
{
    (void)message;(void)time;IShellItem *item=NULL;PWSTR path=NULL;
    HRESULT hr=IFileDialog_GetFolder(dialog,&item);
    if(SUCCEEDED(hr))hr=IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&path);
    correct=SUCCEEDED(hr)&&path&&!lstrcmpiW(path,drive_root);old=GetDlgItem(w,1121)!=NULL;
    printf("ROOT_FOLDER bits=%u old=%ld hr=%08lx path=%ls match=%ld\n",(unsigned)(sizeof(void*)*8),old,(unsigned long)hr,path?path:L"(null)",correct);
    if(path)CoTaskMemFree(path);
    if(item)IShellItem_Release(item);
    KillTimer(w,id);InterlockedExchange(&done,1);PostMessageW(w,WM_CLOSE,0,0);
}
static BOOL CALLBACK find(HWND w,LPARAM ignored)
{
    (void)ignored;WCHAR text[80];GetWindowTextW(w,text,ARRAYSIZE(text));
    if(IsWindowVisible(w)&&!lstrcmpW(text,title)){seen=1;SetTimer(w,981,200,tick);return FALSE;}return TRUE;
}
static DWORD WINAPI observe(void *unused)
{
    (void)unused;for(int i=0;i<100;++i){if(!seen)EnumThreadWindows(ui,find,0);if(done)return 0;Sleep(50);}
    PostThreadMessageW(ui,WM_QUIT,0,0);return 1;
}
int wmain(int argc,WCHAR **argv)
{
    HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(hr))return 2;
    WCHAR windows[MAX_PATH];DWORD n=GetWindowsDirectoryW(windows,ARRAYSIZE(windows));
    if(!n || n>=ARRAYSIZE(windows) || !GetVolumePathNameW(windows,drive_root,ARRAYSIZE(drive_root)))return 3;
    IShellItem *root=NULL;hr=SHCreateItemFromParsingName(drive_root,NULL,&IID_IShellItem,(void **)&root);
    if(FAILED(hr))return 3;
    hr=CoCreateInstance(&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IFileDialog,(void **)&dialog);
    if(FAILED(hr)||FAILED(IFileDialog_SetTitle(dialog,title))||FAILED(IFileDialog_SetFolder(dialog,root)))return 4;
    IShellItem_Release(root);ui=GetCurrentThreadId();HANDLE worker=CreateThread(NULL,0,observe,NULL,0,NULL);if(!worker)return 5;
    hr=IFileDialog_Show(dialog,NULL);InterlockedExchange(&done,1);DWORD wait=WaitForSingleObject(worker,10000),code=99;
    if(wait==WAIT_OBJECT_0)GetExitCodeThread(worker,&code);
    CloseHandle(worker);IFileDialog_Release(dialog);CoUninitialize();
    printf("ROOT_RESULT hr=%08lx wait=%lu worker=%lu\n",(unsigned long)hr,wait,code);
    BOOL expected_old=argc>1&&!lstrcmpW(argv[1],L"old");
    return seen&&correct&&old==expected_old&&hr==HRESULT_FROM_WIN32(ERROR_CANCELLED)&&wait==WAIT_OBJECT_0&&code==0?0:1;
}
