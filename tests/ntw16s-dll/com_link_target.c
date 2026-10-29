/* Compare real Shell link results of the COM and bridged file dialogs. */
#define _UNICODE
#define COBJMACROS
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <stdio.h>
#include <wchar.h>
static DWORD ui;static HWND window;static volatile LONG seen,done;static BOOL old;
static const WCHAR title[]=L"Win16S link option";
static BOOL CALLBACK find(HWND w,LPARAM p)
{(void)p;WCHAR text[80];GetWindowTextW(w,text,80);if(!IsWindowVisible(w)||lstrcmpW(text,title))return TRUE;window=w;old=GetDlgItem(w,1121)!=NULL;seen=1;return FALSE;}
static DWORD WINAPI observe(void *p)
{(void)p;for(int i=0;i<160;++i){if(!seen)EnumThreadWindows(ui,find,0);if(seen&&i>10&&i%10==0){HWND b=GetDlgItem(window,IDOK);PostMessageW(window,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)b);}if(done)return 0;Sleep(50);}PostThreadMessageW(ui,WM_QUIT,0,0);return 1;}
int wmain(int argc,WCHAR **argv)
{
    BOOL keep=argc>1&&!lstrcmpW(argv[1],L"keep");WCHAR temp[MAX_PATH],root[MAX_PATH],target[MAX_PATH],link[MAX_PATH];
    if(!GetTempPathW(MAX_PATH,temp)||swprintf(root,MAX_PATH,L"%lsWin16SLink-%lu-%lu",temp,GetCurrentProcessId(),GetTickCount())<0||!CreateDirectoryW(root,NULL))return 2;
    if(swprintf(target,MAX_PATH,L"%ls\\target.txt",root)<0||swprintf(link,MAX_PATH,L"%ls\\shortcut.lnk",root)<0)return 2;
    HANDLE file=CreateFileW(target,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);if(file==INVALID_HANDLE_VALUE)return 3;CloseHandle(file);
    HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(hr))return 4;
    IShellLinkW *shortcut=NULL;IPersistFile *persist=NULL;
    hr=CoCreateInstance(&CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,&IID_IShellLinkW,(void **)&shortcut);
    if(FAILED(hr)||FAILED(IShellLinkW_SetPath(shortcut,target))||FAILED(IShellLinkW_QueryInterface(shortcut,&IID_IPersistFile,(void **)&persist))||FAILED(IPersistFile_Save(persist,link,TRUE)))return 5;
    IPersistFile_Release(persist);IShellLinkW_Release(shortcut);
    IFileDialog *dialog=NULL;IShellItem *folder=NULL;
    hr=CoCreateInstance(&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IFileDialog,(void **)&dialog);
    if(FAILED(hr)||FAILED(IFileDialog_SetTitle(dialog,title))||FAILED(IFileDialog_SetFileName(dialog,link)))return 6;
    FILEOPENDIALOGOPTIONS options=0;if(FAILED(IFileDialog_GetOptions(dialog,&options))||FAILED(IFileDialog_SetOptions(dialog,keep?options|FOS_NODEREFERENCELINKS:options&~FOS_NODEREFERENCELINKS)))return 7;
    if(FAILED(SHCreateItemFromParsingName(root,NULL,&IID_IShellItem,(void **)&folder)))return 8;
    hr=IFileDialog_SetFolder(dialog,folder);IShellItem_Release(folder);if(FAILED(hr))return 8;
    ui=GetCurrentThreadId();HANDLE thread=CreateThread(NULL,0,observe,NULL,0,NULL);if(!thread)return 9;
    hr=IFileDialog_Show(dialog,NULL);InterlockedExchange(&done,1);DWORD wait=WaitForSingleObject(thread,10000),code=99;if(wait==WAIT_OBJECT_0)GetExitCodeThread(thread,&code);CloseHandle(thread);
    IShellItem *item=NULL;PWSTR path=NULL;HRESULT result=FAILED(hr)?hr:IFileDialog_GetResult(dialog,&item);if(SUCCEEDED(result))result=IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&path);
    BOOL link_result=path&&!lstrcmpiW(path,link),target_result=path&&!lstrcmpiW(path,target);
    printf("LINK_RESULT bits=%u keep=%d old=%d seen=%ld hr=%08lx result=%08lx link=%d target=%d wait=%lu worker=%lu\n",(unsigned)(sizeof(void*)*8),keep,old,seen,(unsigned long)hr,(unsigned long)result,link_result,target_result,wait,code);
    if(path)CoTaskMemFree(path);
    if(item)IShellItem_Release(item);
    IFileDialog_Release(dialog);CoUninitialize();DeleteFileW(link);DeleteFileW(target);RemoveDirectoryW(root);
    return seen&&SUCCEEDED(hr)&&SUCCEEDED(result)&&(keep?link_result:target_result)&&wait==WAIT_OBJECT_0&&code==0?0:1;
}
