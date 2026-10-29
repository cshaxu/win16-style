/* Verify IFileDialog file-name methods while its real modal window is open. */
#define _UNICODE
#define COBJMACROS
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <stdio.h>
#include <wchar.h>
static IFileDialog *dialog;
static DWORD ui_thread;
static HWND window;
static volatile LONG seen,done,passed;
static BOOL accept_mode;
static WCHAR root[MAX_PATH],target[MAX_PATH];
static const WCHAR title[]=L"Win16S live filename";
static void CALLBACK tick(HWND w,UINT message,UINT_PTR id,DWORD time)
{
    (void)message;(void)time;KillTimer(w,id);
    PWSTR before=NULL;
    HRESULT first=IFileDialog_GetFileName(dialog,&before);
    BOOL initial=SUCCEEDED(first)&&before&&!lstrcmpW(before,L"first.txt");
    if(before)CoTaskMemFree(before);
    HRESULT set=IFileDialog_SetFileName(dialog,L"second.txt");
    PWSTR after=NULL;HRESULT second=IFileDialog_GetFileName(dialog,&after);
    BOOL updated=SUCCEEDED(second)&&after&&!lstrcmpW(after,L"second.txt");
    HWND edit=GetDlgItem(w,1152);WCHAR text[128]=L"";
    if(edit)GetWindowTextW(edit,text,ARRAYSIZE(text));
    BOOL visible=!edit||!lstrcmpW(text,L"second.txt");
    BOOL user_change=TRUE;
    if(edit){SetWindowTextW(edit,L"manual.txt");PWSTR current=NULL;HRESULT get=IFileDialog_GetFileName(dialog,&current);
        user_change=SUCCEEDED(get)&&current&&!lstrcmpW(current,L"manual.txt");if(current)CoTaskMemFree(current);}
    IFileDialog2 *v2=NULL;BOOL v2_ok=SUCCEEDED(IFileDialog_QueryInterface(dialog,&IID_IFileDialog2,(void **)&v2));
    if(v2_ok){PWSTR current=NULL;v2_ok=SUCCEEDED(IFileDialog2_SetFileName(v2,L"via-v2.txt"))&&
        SUCCEEDED(IFileDialog2_GetFileName(v2,&current))&&current&&!lstrcmpW(current,L"via-v2.txt");
        if(current)CoTaskMemFree(current);
        IFileDialog2_Release(v2);}
    printf("LIVE_NAME initial=%d updated=%d visible=%d user=%d v2=%d old=%d first=%08lx set=%08lx second=%08lx\n",initial,updated,visible,user_change,v2_ok,edit!=NULL,(unsigned long)first,(unsigned long)set,(unsigned long)second);
    if(after)CoTaskMemFree(after);
    passed=initial&&SUCCEEDED(set)&&updated&&visible&&user_change&&v2_ok;
    InterlockedExchange(&done,1);
    if(accept_mode){HWND button=GetDlgItem(w,IDOK);PostMessageW(w,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)button);}
    else PostMessageW(w,WM_CLOSE,0,0);
}
static BOOL CALLBACK find(HWND w,LPARAM value)
{
    (void)value;WCHAR text[80];GetWindowTextW(w,text,ARRAYSIZE(text));
    if(IsWindowVisible(w)&&!lstrcmpW(text,title)){window=w;seen=1;SetTimer(w,980,150,tick);return FALSE;}return TRUE;
}
static DWORD WINAPI observe(void *unused)
{
    (void)unused;for(int i=0;i<160;++i){if(!seen)EnumThreadWindows(ui_thread,find,0);if(done)return 0;Sleep(50);}
    PostThreadMessageW(ui_thread,WM_QUIT,0,0);return 1;
}
int wmain(int argc,WCHAR **argv)
{
    BOOL open=argc>1&&!lstrcmpW(argv[1],L"open");accept_mode=argc>2&&!lstrcmpW(argv[2],L"accept");
    WCHAR temp[MAX_PATH];if(!GetTempPathW(MAX_PATH,temp)||swprintf(root,MAX_PATH,L"%lsWin16SLive-%lu-%lu",temp,GetCurrentProcessId(),GetTickCount())<0||!CreateDirectoryW(root,NULL))return 5;
    if(swprintf(target,MAX_PATH,L"%ls\\via-v2.txt",root)<0)return 5;
    if(open){HANDLE file=CreateFileW(target,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);if(file==INVALID_HANDLE_VALUE)return 5;CloseHandle(file);}
    HRESULT init=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(init))return 2;
    HRESULT hr=CoCreateInstance(open?&CLSID_FileOpenDialog:&CLSID_FileSaveDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IFileDialog,(void **)&dialog);
    if(FAILED(hr)||FAILED(IFileDialog_SetTitle(dialog,title))||FAILED(IFileDialog_SetFileName(dialog,L"first.txt")))return 3;
    IShellItem *folder=NULL;if(FAILED(SHCreateItemFromParsingName(root,NULL,&IID_IShellItem,(void **)&folder)))return 3;
    hr=IFileDialog_SetFolder(dialog,folder);IShellItem_Release(folder);if(FAILED(hr))return 3;
    ui_thread=GetCurrentThreadId();HANDLE thread=CreateThread(NULL,0,observe,NULL,0,NULL);if(!thread)return 4;
    hr=IFileDialog_Show(dialog,NULL);InterlockedExchange(&done,1);DWORD wait=WaitForSingleObject(thread,10000),code=99;
    if(wait==WAIT_OBJECT_0)GetExitCodeThread(thread,&code);
    CloseHandle(thread);
    BOOL result_ok=!accept_mode&&hr==HRESULT_FROM_WIN32(ERROR_CANCELLED);
    if(accept_mode&&SUCCEEDED(hr)){IShellItem *item=NULL;PWSTR path=NULL;HRESULT result=IFileDialog_GetResult(dialog,&item);
        if(SUCCEEDED(result))result=IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&path);
        result_ok=SUCCEEDED(result)&&path&&!lstrcmpiW(path,target);if(path)CoTaskMemFree(path);if(item)IShellItem_Release(item);}
    IFileDialog_Release(dialog);CoUninitialize();if(open)DeleteFileW(target);RemoveDirectoryW(root);
    printf("LIVE_RESULT bits=%u seen=%ld passed=%ld result=%d hr=%08lx wait=%lu worker=%lu\n",(unsigned)(sizeof(void*)*8),seen,passed,result_ok,(unsigned long)hr,wait,code);
    return seen&&passed&&result_ok&&wait==WAIT_OBJECT_0&&code==0?0:1;
}
