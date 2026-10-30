/* Verify hidden files remain filtered after type and folder changes. */
#define UNICODE
#define _UNICODE
#define COBJMACROS
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <stdio.h>
#include <wchar.h>
static volatile LONG found,passed,phase;static DWORD ui;static HWND dialog;
static const WCHAR title[]=L"Win16S hidden navigation";
static BOOL in_list(HWND list,PCWSTR name){return SendMessageW(list,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)name)!=LB_ERR;}
static void CALLBACK check(HWND w,UINT msg,UINT_PTR id,DWORD now)
{
    (void)msg;(void)now;HWND list=GetDlgItem(w,1120),dirs=GetDlgItem(w,1121),filters=GetDlgItem(w,1136);
    if(phase==0){BOOL ok=in_list(list,L"root.txt")&&!in_list(list,L"root.log")&&in_list(list,L"system.txt");
        printf("HIDDEN_ROOT_FILTER ok=%d count=%lld\n",ok,(long long)SendMessageW(list,LB_GETCOUNT,0,0));if(!ok){KillTimer(w,id);PostMessageW(w,WM_CLOSE,0,0);return;}++passed;
        SendMessageW(filters,CB_SETCURSEL,1,0);SendMessageW(w,WM_COMMAND,MAKEWPARAM(1136,CBN_SELCHANGE),(LPARAM)filters);phase=1;
    }else if(phase==1){BOOL ok=in_list(list,L"root.log")&&!in_list(list,L"root.txt");
        printf("HIDDEN_FILTER_CHANGE ok=%d count=%lld\n",ok,(long long)SendMessageW(list,LB_GETCOUNT,0,0));if(!ok){KillTimer(w,id);PostMessageW(w,WM_CLOSE,0,0);return;}++passed;
        LRESULT index=SendMessageW(dirs,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)L"child");if(index==LB_ERR){KillTimer(w,id);PostMessageW(w,WM_CLOSE,0,0);return;}
        SendMessageW(dirs,LB_SETCURSEL,index,0);SendMessageW(w,WM_COMMAND,MAKEWPARAM(1121,LBN_DBLCLK),(LPARAM)dirs);phase=2;
    }else{BOOL ok=in_list(list,L"child.log")&&!in_list(list,L"root.log")&&!in_list(list,L"child.txt")&&in_list(list,L"system.log");
        printf("HIDDEN_FOLDER_CHANGE ok=%d count=%lld\n",ok,(long long)SendMessageW(list,LB_GETCOUNT,0,0));if(ok)++passed;KillTimer(w,id);PostMessageW(w,WM_CLOSE,0,0);phase=3;}
}
static BOOL CALLBACK find(HWND w,LPARAM arg)
{(void)arg;WCHAR text[128];GetWindowTextW(w,text,ARRAYSIZE(text));if(!IsWindowVisible(w)||lstrcmpW(text,title)||!GetDlgItem(w,1121))return TRUE;
    dialog=w;InterlockedExchange(&found,1);SetTimer(w,974,200,check);return FALSE;}
static DWORD WINAPI observe(void *unused)
{(void)unused;for(int i=0;i<160;++i){if(!found)EnumThreadWindows(ui,find,0);if(found && !IsWindow(dialog))return 0;Sleep(50);}PostThreadMessageW(ui,WM_QUIT,0,0);return 1;}
static BOOL make(PCWSTR dir,PCWSTR name,DWORD attrs)
{WCHAR path[MAX_PATH];if(swprintf(path,MAX_PATH,L"%ls\\%ls",dir,name)<0)return FALSE;HANDLE h=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);if(h==INVALID_HANDLE_VALUE)return FALSE;CloseHandle(h);return SetFileAttributesW(path,attrs);}
static BOOL erase(PCWSTR dir,PCWSTR name)
{WCHAR path[MAX_PATH];if(swprintf(path,MAX_PATH,L"%ls\\%ls",dir,name)<0)return FALSE;BOOL attributes=SetFileAttributesW(path,FILE_ATTRIBUTE_NORMAL);BOOL removed=DeleteFileW(path);return attributes&&removed;}
int wmain(void)
{
    WCHAR temp[MAX_PATH],root[MAX_PATH],child[MAX_PATH];if(!GetTempPathW(MAX_PATH,temp)||!GetTempFileNameW(temp,L"w16",0,root)||!DeleteFileW(root)||!CreateDirectoryW(root,NULL))return 2;
    if(swprintf(child,MAX_PATH,L"%ls\\child",root)<0||!CreateDirectoryW(child,NULL)||!SetFileAttributesW(child,FILE_ATTRIBUTE_HIDDEN))return 3;
    if(!make(root,L"root.txt",FILE_ATTRIBUTE_HIDDEN)||!make(root,L"root.log",FILE_ATTRIBUTE_HIDDEN)||!make(root,L"system.txt",FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)||
       !make(child,L"child.txt",FILE_ATTRIBUTE_HIDDEN)||!make(child,L"child.log",FILE_ATTRIBUTE_HIDDEN)||!make(child,L"system.log",FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM))return 4;
    HRESULT init=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(init))return 5;IFileDialog *d=NULL;IShellItem *folder=NULL;
    HRESULT hr=CoCreateInstance(&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IFileDialog,(void **)&d);FILEOPENDIALOGOPTIONS opts=0;
    COMDLG_FILTERSPEC filters[]={{L"Text",L"*.txt"},{L"Log",L"*.log"}};
    if(FAILED(hr)||FAILED(IFileDialog_SetTitle(d,title))||FAILED(IFileDialog_GetOptions(d,&opts))||FAILED(IFileDialog_SetOptions(d,opts|FOS_FORCESHOWHIDDEN))||
       FAILED(IFileDialog_SetFileTypes(d,2,filters))||FAILED(SHCreateItemFromParsingName(root,NULL,&IID_IShellItem,(void **)&folder))||FAILED(IFileDialog_SetFolder(d,folder)))return 6;
    IShellItem_Release(folder);ui=GetCurrentThreadId();HANDLE worker=CreateThread(NULL,0,observe,NULL,0,NULL);if(!worker)return 7;
    hr=IFileDialog_Show(d,NULL);DWORD wait=WaitForSingleObject(worker,10000);CloseHandle(worker);IFileDialog_Release(d);CoUninitialize();
    BOOL a=erase(root,L"root.txt"),b=erase(root,L"root.log"),c=erase(root,L"system.txt");
    BOOL d1=erase(child,L"child.txt"),e=erase(child,L"child.log"),f=erase(child,L"system.log");
    BOOL attrs=SetFileAttributesW(child,FILE_ATTRIBUTE_NORMAL),child_removed=RemoveDirectoryW(child),root_removed=RemoveDirectoryW(root);
    BOOL cleanup=a&&b&&c&&d1&&e&&f&&attrs&&child_removed&&root_removed;
    printf("COM_HIDDEN_NAV bits=%u found=%ld passed=%ld phase=%ld hr=%08lx wait=%lu cleanup=%d\n",(unsigned)(sizeof(void*)*8),found,passed,phase,(unsigned long)hr,wait,cleanup);
    return cleanup&&found&&passed==3&&phase==3&&hr==HRESULT_FROM_WIN32(ERROR_CANCELLED)&&wait==WAIT_OBJECT_0?0:1;
}
