/* Read actual modern Shell view on a private desktop; no settings are changed. */
#define UNICODE
#define _UNICODE
#define COBJMACROS
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <servprov.h>
#include <shlguid.h>
#include <stdio.h>
#include <wchar.h>
static IFileDialog *d;static volatile LONG found,passed,phase,view_ok;static DWORD ui, started;static HWND dialog;static BOOL expect_hidden;
static const WCHAR title[]=L"Win16S hidden navigation";
static void CALLBACK check(HWND w,UINT msg,UINT_PTR id,DWORD now){(void)msg;(void)now;IServiceProvider *sp=NULL;IShellBrowser *browser=NULL;IShellView *view=NULL;IFolderView2 *folder=NULL;HRESULT hr=IFileDialog_QueryInterface(d,&IID_IServiceProvider,(void **)&sp);printf("PROBE_SERVICE hr=%08lx\n",(unsigned long)hr);if(SUCCEEDED(hr))hr=IServiceProvider_QueryService(sp,&SID_STopLevelBrowser,&IID_IShellBrowser,(void **)&browser);printf("PROBE_BROWSER hr=%08lx\n",(unsigned long)hr);if(SUCCEEDED(hr))hr=IShellBrowser_QueryActiveShellView(browser,&view);if(SUCCEEDED(hr))hr=IShellView_QueryInterface(view,&IID_IFolderView2,(void **)&folder);int count=0;passed=0;view_ok=0;if(SUCCEEDED(hr))hr=IFolderView2_ItemCount(folder,SVGIO_ALLVIEW,&count);printf("PROBE_VIEW hr=%08lx count=%d\n",(unsigned long)hr,count);if(SUCCEEDED(hr))view_ok=1;for(int i=0;i<count;++i){IShellItem *item=NULL;PWSTR name=NULL;HRESULT h=IFolderView2_GetItem(folder,i,&IID_IShellItem,(void **)&item);if(SUCCEEDED(h))h=IShellItem_GetDisplayName(item,SIGDN_NORMALDISPLAY,&name);if(FAILED(h)||!name)view_ok=0;if(SUCCEEDED(h)&&name){printf("VIEW_ITEM %ls\n",name);if(!lstrcmpiW(name,L"root.txt"))passed|=1;if(!lstrcmpiW(name,L"system.txt"))passed|=2;if(!lstrcmpiW(name,L"child"))passed|=4;if(!lstrcmpiW(name,L"sysonly.txt"))passed|=8;if(!lstrcmpiW(name,L"normal.txt"))passed|=16;}if(name)CoTaskMemFree(name);if(item)IShellItem_Release(item);}if(folder)IFolderView2_Release(folder);if(view)IShellView_Release(view);if(browser)IShellBrowser_Release(browser);if(sp)IServiceProvider_Release(sp);if(view_ok && (passed&(expect_hidden?29:16))==(expect_hidden?29:16)){phase=1;KillTimer(w,id);PostMessageW(w,WM_CLOSE,0,0);}else if(GetTickCount()-started>=5000){phase=2;KillTimer(w,id);PostMessageW(w,WM_CLOSE,0,0);}}
static BOOL CALLBACK find(HWND w,LPARAM arg)
{(void)arg;WCHAR text[128];GetWindowTextW(w,text,ARRAYSIZE(text));if(!IsWindowVisible(w)||lstrcmpW(text,title))return TRUE;
    dialog=w;started=GetTickCount();InterlockedExchange(&found,1);SetTimer(w,974,200,check);return FALSE;}
static DWORD WINAPI observe(void *unused)
{(void)unused;for(int i=0;i<160;++i){if(!found)EnumThreadWindows(ui,find,0);if(found && !IsWindow(dialog))return 0;Sleep(50);}PostThreadMessageW(ui,WM_QUIT,0,0);return 1;}
static BOOL make(PCWSTR dir,PCWSTR name,DWORD attrs)
{WCHAR path[MAX_PATH];if(swprintf(path,MAX_PATH,L"%ls\\%ls",dir,name)<0)return FALSE;HANDLE h=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);if(h==INVALID_HANDLE_VALUE)return FALSE;CloseHandle(h);return SetFileAttributesW(path,attrs);}
static BOOL erase(PCWSTR dir,PCWSTR name)
{WCHAR path[MAX_PATH];if(swprintf(path,MAX_PATH,L"%ls\\%ls",dir,name)<0)return FALSE;BOOL attributes=SetFileAttributesW(path,FILE_ATTRIBUTE_NORMAL);BOOL removed=DeleteFileW(path);return attributes&&removed;}
int wmain(int argc,WCHAR **argv)
{
    WCHAR temp[MAX_PATH],root[MAX_PATH],child[MAX_PATH];if(!GetTempPathW(MAX_PATH,temp)||!GetTempFileNameW(temp,L"w16",0,root)||!DeleteFileW(root)||!CreateDirectoryW(root,NULL))return 2;
    if(swprintf(child,MAX_PATH,L"%ls\\child",root)<0||!CreateDirectoryW(child,NULL)||!SetFileAttributesW(child,FILE_ATTRIBUTE_HIDDEN))return 3;
    if(!make(root,L"normal.txt",FILE_ATTRIBUTE_NORMAL)||!make(root,L"sysonly.txt",FILE_ATTRIBUTE_SYSTEM)||!make(root,L"root.txt",FILE_ATTRIBUTE_HIDDEN)||!make(root,L"root.log",FILE_ATTRIBUTE_HIDDEN)||!make(root,L"system.txt",FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)||
       !make(child,L"child.txt",FILE_ATTRIBUTE_HIDDEN)||!make(child,L"child.log",FILE_ATTRIBUTE_HIDDEN)||!make(child,L"system.log",FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM))return 4;
    HRESULT init=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(init))return 5;IShellItem *folder=NULL;
    HRESULT hr=CoCreateInstance(&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IFileDialog,(void **)&d);FILEOPENDIALOGOPTIONS opts=0;
    COMDLG_FILTERSPEC filters[]={{L"Text",L"*.txt"},{L"Log",L"*.log"}};
    if(FAILED(hr)||FAILED(IFileDialog_SetTitle(d,title))||FAILED(IFileDialog_GetOptions(d,&opts))||FAILED(IFileDialog_SetOptions(d,(argc>1&&!lstrcmpW(argv[1],L"off"))?(opts&~FOS_FORCESHOWHIDDEN):(opts|FOS_FORCESHOWHIDDEN)))||
       FAILED(IFileDialog_SetFileTypes(d,2,filters))||FAILED(SHCreateItemFromParsingName(root,NULL,&IID_IShellItem,(void **)&folder))||FAILED(IFileDialog_SetFolder(d,folder)))return 6;
    IShellItem_Release(folder);expect_hidden=!(argc>1&&!lstrcmpW(argv[1],L"off"));ui=GetCurrentThreadId();HANDLE worker=CreateThread(NULL,0,observe,NULL,0,NULL);if(!worker)return 7;
    hr=IFileDialog_Show(d,NULL);DWORD wait=WaitForSingleObject(worker,10000);CloseHandle(worker);IFileDialog_Release(d);CoUninitialize();
    BOOL n=erase(root,L"normal.txt"),so=erase(root,L"sysonly.txt");BOOL a=erase(root,L"root.txt"),b=erase(root,L"root.log"),c=erase(root,L"system.txt");
    BOOL d1=erase(child,L"child.txt"),e=erase(child,L"child.log"),f=erase(child,L"system.log");
    BOOL attrs=SetFileAttributesW(child,FILE_ATTRIBUTE_NORMAL),child_removed=RemoveDirectoryW(child),root_removed=RemoveDirectoryW(root);
    BOOL cleanup=n&&so&&a&&b&&c&&d1&&e&&f&&attrs&&child_removed&&root_removed;
    printf("COM_HIDDEN_NAV bits=%u found=%ld passed=%ld phase=%ld hr=%08lx wait=%lu cleanup=%d\n",(unsigned)(sizeof(void*)*8),found,passed,phase,(unsigned long)hr,wait,cleanup);
    SHELLSTATE settings={0};SHGetSetSettings(&settings,SSF_SHOWALLOBJECTS|SSF_SHOWSUPERHIDDEN,FALSE);printf("OPTIONS off=%d show_hidden=%u show_superhidden=%u mask=%ld\n",argc>1&&!lstrcmpW(argv[1],L"off"),settings.fShowAllObjects,settings.fShowSuperHidden,passed);
    BOOL off=argc>1&&!lstrcmpW(argv[1],L"off");BOOL mandatory=off?((passed&16)==16):((passed&29)==29);
    return cleanup&&found&&view_ok&&mandatory&&phase==1&&hr==HRESULT_FROM_WIN32(ERROR_CANCELLED)&&wait==WAIT_OBJECT_0?0:1;
}
