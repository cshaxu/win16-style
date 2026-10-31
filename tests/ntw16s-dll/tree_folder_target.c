#define COBJMACROS
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <stdio.h>
#include <wchar.h>
static DWORD owner_thread;
static WCHAR wanted[MAX_PATH];
static volatile LONG seen,trees,windows,direct_tree,new_button_visible;
static HWND dialog;
static BOOL cancel_mode,validate_mode,recover_mode;
static BOOL temp_mode;
static volatile LONG validated,callback_initialized,callback_unknown;
static HWND edit_window;
static BOOL CALLBACK child(HWND w,LPARAM data)
{
    WCHAR cls[64]=L"";(void)data;
    if(GetClassNameW(w,cls,64) && !lstrcmpiW(cls,L"SysTreeView32"))InterlockedIncrement(&trees);
    if(!lstrcmpiW(cls,L"Edit"))edit_window=w;
    return TRUE;
}
static BOOL CALLBACK scan(HWND w,LPARAM data)
{
    WCHAR cls[64];(void)data;
    if(!IsWindowVisible(w) || !GetClassNameW(w,cls,64) || lstrcmpW(cls,L"#32770"))return TRUE;
    InterlockedIncrement(&windows);dialog=w;EnumChildWindows(w,child,0);
    HWND new_button=GetDlgItem(w,14150);if(new_button && IsWindowVisible(new_button))InterlockedExchange(&new_button_visible,1);
    HWND tree=GetDlgItem(w,14145);WCHAR tree_class[64]=L"";
    if(tree && GetClassNameW(tree,tree_class,64) && !lstrcmpiW(tree_class,L"SysTreeView32"))InterlockedExchange(&direct_tree,1);
    return TRUE;
}
static DWORD WINAPI observe(void *unused)
{
    (void)unused;BOOL posted=FALSE,recovered=FALSE;
    for(unsigned i=0;i<140;++i){
        if(!seen){windows=trees=0;EnumThreadWindows(owner_thread,scan,0);if(dialog && trees)InterlockedExchange(&seen,1);}
        if(seen && IsWindow(dialog)){
            if(validate_mode){if(!posted && edit_window){SendMessageW(edit_window,WM_SETTEXT,0,(LPARAM)L"Z:\\nonexistent-win16s-probe");PostMessageW(dialog,WM_COMMAND,IDOK,0);posted=TRUE;}
                if(validated && recover_mode && !recovered){SendMessageW(edit_window,WM_SETTEXT,0,(LPARAM)wanted);PostMessageW(dialog,WM_COMMAND,IDOK,0);recovered=TRUE;}
                else if(validated && !recover_mode)PostMessageW(dialog,WM_COMMAND,IDCANCEL,0);}
            else if(cancel_mode){if(i>10)PostMessageW(dialog,WM_COMMAND,IDCANCEL,0);}
            else {if(!posted){PostMessageW(dialog,BFFM_SETSELECTIONW,TRUE,(LPARAM)wanted);posted=TRUE;}
                HWND ok=GetDlgItem(dialog,IDOK);if(i>20 && ok && IsWindowEnabled(ok))PostMessageW(dialog,WM_COMMAND,IDOK,0);}
        }
        if(seen && !IsWindow(dialog))return 0;
        Sleep(50);
    }
    if(dialog && IsWindow(dialog))PostMessageW(dialog,WM_COMMAND,IDCANCEL,0);
    return 1;
}
static int CALLBACK callback(HWND w,UINT message,LPARAM parameter,LPARAM data)
{if(data==0x12345678 && message==BFFM_INITIALIZED){InterlockedIncrement(&callback_initialized);if(temp_mode)SendMessageW(w,BFFM_SETSELECTIONW,TRUE,(LPARAM)wanted);}if(data==0x12345678 && message==BFFM_IUNKNOWN && parameter)InterlockedIncrement(&callback_unknown);if(data==0x12345678 && message==BFFM_VALIDATEFAILEDW){InterlockedIncrement(&validated);return 1;}return 0;}
int wmain(int argc,WCHAR **argv)
{
    if(argc!=3)return 2;
    BOOL nonew=!wcscmp(argv[1],L"shell-new-nonew") || !wcscmp(argv[1],L"shell-new-ansi-nonew"); BOOL com=!wcscmp(argv[1],L"com"),new_callback=!wcscmp(argv[1],L"shell-new-callback"),root_mode=!wcscmp(argv[1],L"shell-new-root") || !wcscmp(argv[1],L"shell-new-ansi-root"),new_ansi=!wcscmp(argv[1],L"shell-new-ansi-nonew") || !wcscmp(argv[1],L"shell-new-ansi") || !wcscmp(argv[1],L"shell-new-ansi-root"),new_shell=nonew || new_callback || root_mode || new_ansi || !wcscmp(argv[1],L"shell-new"),simple=!wcscmp(argv[1],L"shell-simple") || !wcscmp(argv[1],L"shell-simple-callback"),simple_callback=!wcscmp(argv[1],L"shell-simple-callback");cancel_mode=!wcscmp(argv[2],L"cancel");recover_mode=!wcscmp(argv[2],L"recover");validate_mode=recover_mode || !wcscmp(argv[2],L"validate");
    temp_mode=!wcscmp(argv[1],L"shell-temp");
    if((!com && !simple && !new_shell && !temp_mode && wcscmp(argv[1],L"shell")) || (!cancel_mode && !validate_mode && wcscmp(argv[2],L"accept")) || ((com || new_shell) && validate_mode))return 2;
    UINT length=GetWindowsDirectoryW(wanted,ARRAYSIZE(wanted));if(!length || length>=ARRAYSIZE(wanted))return 3;
    if(temp_mode){WCHAR temp[MAX_PATH];if(!GetTempPathW(ARRAYSIZE(temp),temp)||!GetTempFileNameW(temp,L"w16",0,wanted)||!DeleteFileW(wanted)||!CreateDirectoryW(wanted,NULL))return 3;printf("TREE_TEMP_PATH %ls\n",wanted);fflush(stdout);}
    HRESULT init=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(init))return 4;
    owner_thread=GetCurrentThreadId();HANDLE thread=CreateThread(NULL,0,observe,NULL,0,NULL);if(!thread)return 5;
    BOOL correct=FALSE;HRESULT hr=E_FAIL;
    if(com){IFileDialog *d=NULL;FILEOPENDIALOGOPTIONS flags=0;
        hr=CoCreateInstance(&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IFileDialog,(void **)&d);
        if(SUCCEEDED(hr))hr=IFileDialog_GetOptions(d,&flags);
        if(SUCCEEDED(hr))hr=IFileDialog_SetOptions(d,flags|FOS_PICKFOLDERS|FOS_FORCEFILESYSTEM);
        if(SUCCEEDED(hr))hr=IFileDialog_Show(d,NULL);
        if(cancel_mode)correct=hr==HRESULT_FROM_WIN32(ERROR_CANCELLED);
        else if(SUCCEEDED(hr)){IShellItem *item=NULL;PWSTR path=NULL;hr=IFileDialog_GetResult(d,&item);
            if(SUCCEEDED(hr))hr=IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&path);
            correct=SUCCEEDED(hr) && path && !lstrcmpiW(path,wanted);
            if(path)CoTaskMemFree(path);
            if(item)IShellItem_Release(item);}
        if(d)IFileDialog_Release(d);
    }else{PIDLIST_ABSOLUTE item,root=NULL;
        if(root_mode && FAILED(SHParseDisplayName(wanted,NULL,&root,0,NULL)))return 6;
        if(new_ansi){BROWSEINFOA b={0};b.ulFlags=BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE|(nonew?BIF_NONEWFOLDERBUTTON:0);b.pidlRoot=root;item=SHBrowseForFolderA(&b);}
        else{BROWSEINFOW b={0};b.ulFlags=temp_mode?BIF_RETURNONLYFSDIRS:BIF_RETURNONLYFSDIRS|(nonew?BIF_NONEWFOLDERBUTTON:0)|(new_shell?BIF_NEWDIALOGSTYLE:BIF_USENEWUI|(simple?0:BIF_VALIDATE));b.pidlRoot=root;
            if(validate_mode || simple_callback || new_callback || temp_mode){b.lpfn=callback;b.lParam=0x12345678;}item=SHBrowseForFolderW(&b);}
        if(root)CoTaskMemFree(root);
        WCHAR path[MAX_PATH]=L"";
        correct=(cancel_mode || (validate_mode && !recover_mode))?!item:(item && SHGetPathFromIDListW(item,path) && !lstrcmpiW(path,wanted));
        if(item)CoTaskMemFree(item);}
    DWORD wait=WaitForSingleObject(thread,8000),code=1;if(wait==WAIT_OBJECT_0)GetExitCodeThread(thread,&code);CloseHandle(thread);CoUninitialize();if(temp_mode)printf("TREE_TEMP_CLEANUP %d\n",RemoveDirectoryW(wanted));
    printf("TREE_FOLDER bits=%u com=%d cancel=%d tree=%ld windows=%ld result=%d hr=%08lx wait=%lu worker=%lu validated=%ld direct=%ld callback=%ld unknown=%ld\n",
        (unsigned)(sizeof(void*)*8),com,cancel_mode,trees,windows,correct,(DWORD)hr,wait,code,validated,direct_tree,callback_initialized,callback_unknown);
    printf("TREE_NONEW requested=%d visible=%ld\n",nonew,new_button_visible);
    return (!nonew || !new_button_visible) && seen && trees && windows==1 && correct && wait==WAIT_OBJECT_0 && !code && (!validate_mode || (validated==1 && !direct_tree && callback_unknown==1)) && (!((simple && !simple_callback) || (new_shell && !new_callback)) || direct_tree) && (!(new_callback || simple_callback) || (!direct_tree && callback_unknown==1)) && (!(simple_callback || new_callback) || callback_initialized==1)?0:1;
}
