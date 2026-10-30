/* Compare the live folder reported after navigating the actual modal dialog. */
#define _UNICODE
#define COBJMACROS
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <stdio.h>
#include <wchar.h>
static IFileDialog *dialog;
static IShellItem *child;
static WCHAR root[MAX_PATH],sub[MAX_PATH],match_file[MAX_PATH],other_file[MAX_PATH];
static DWORD ui;
static HWND window;
static volatile LONG seen,done,phase,initial_ok,final_ok,stable,v2_ok,parent_ok,name_ok,filter_ok,filter_live_ok,selection_stable;
static BOOL old,programmatic,change_filter;
static int filter_step;
static HWND filter_control;
static const WCHAR title[]=L"Win16S live folder";
static BOOL CALLBACK find_filter(HWND w,LPARAM unused)
{
    (void)unused;WCHAR cls[32];
    if(GetClassNameW(w,cls,ARRAYSIZE(cls)) && !lstrcmpiW(cls,L"ComboBox") && SendMessageW(w,CB_GETCOUNT,0,0)==2){
        WCHAR first[256]=L"",second[256]=L"";
        if(SendMessageW(w,CB_GETLBTEXTLEN,0,0)<255 && SendMessageW(w,CB_GETLBTEXTLEN,1,0)<255){
            SendMessageW(w,CB_GETLBTEXT,0,(LPARAM)first);SendMessageW(w,CB_GETLBTEXT,1,(LPARAM)second);
            if(wcsstr(first,L"Text")&&wcsstr(second,L"Log")){filter_control=w;return FALSE;}}
    }
    return TRUE;
}
static BOOL folder_is(PCWSTR expected)
{
    IShellItem *item=NULL;PWSTR path=NULL;
    HRESULT hr=IFileDialog_GetFolder(dialog,&item);
    if(SUCCEEDED(hr))hr=IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&path);
    BOOL ok=SUCCEEDED(hr)&&path&&!lstrcmpiW(path,expected);
    printf("LIVE_FOLDER_GET hr=%08lx match=%d path=%ls\n",(unsigned long)hr,ok,path?path:L"(none)");
    if(path)CoTaskMemFree(path);
    if(item)IShellItem_Release(item);
    return ok;
}
static void CALLBACK tick(HWND w,UINT msg,UINT_PTR id,DWORD time)
{
    (void)msg;(void)time;
    if(!phase){initial_ok=folder_is(root);HWND list=GetDlgItem(w,1121);old=list!=NULL;
        if(programmatic){HRESULT set=IFileDialog_SetFolder(dialog,child);printf("LIVE_FOLDER_SET hr=%08lx\n",(unsigned long)set);}
        else if(old){LRESULT index=SendMessageW(list,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)L"child");
            if(index!=LB_ERR){SendMessageW(list,LB_SETCURSEL,(WPARAM)index,0);
                SendMessageW(w,WM_COMMAND,MAKEWPARAM(1121,LBN_SELCHANGE),(LPARAM)list);
                selection_stable=folder_is(root);printf("LIVE_FOLDER_SELECT before_enter=%ld\n",selection_stable);
                SendMessageW(w,WM_COMMAND,MAKEWPARAM(1121,LBN_DBLCLK),(LPARAM)list);}
            WCHAR cwd[MAX_PATH]=L"";GetCurrentDirectoryW(ARRAYSIZE(cwd),cwd);printf("LIVE_FOLDER_NAV index=%lld cwd=%ls\n",(long long)index,cwd);}
        else (void)IFileDialog_SetFolder(dialog,child);
        if(!old || programmatic)selection_stable=1;
        phase=1;return;
    }
    if(phase==1 && folder_is(sub)){final_ok=1;
        if(change_filter && filter_step<3){filter_control=NULL;EnumChildWindows(w,find_filter,0);HWND combo=filter_control;UINT selected=0;
            if(!combo){if(!filter_step){printf("FILTER_CONTROL_MISSING old=%d\n",old);filter_step=-1;}return;}
            if(!filter_step){HWND parent=GetParent(combo);WCHAR cls[80]=L"";GetClassNameW(parent,cls,ARRAYSIZE(cls));
                printf("FILTER_PARENT class=%ls id=%d root=%d count=%lld\n",cls,GetDlgCtrlID(parent),parent==w,(long long)SendMessageW(combo,CB_GETCOUNT,0,0));
                UINT id=(UINT)GetDlgCtrlID(combo);SendMessageW(combo,CB_SETCURSEL,0,0);SendMessageW(parent,WM_COMMAND,MAKEWPARAM(id,CBN_SELCHANGE),(LPARAM)combo);
                SendMessageW(parent,WM_COMMAND,MAKEWPARAM(id,CBN_SELENDOK),(LPARAM)combo);filter_step=1;return;}
            HRESULT get=IFileDialog_GetFileTypeIndex(dialog,&selected);
            if(filter_step==1){if(FAILED(get)||selected!=1)return;
                IFileDialog2 *filter_v2=NULL;HRESULT set=IFileDialog_QueryInterface(dialog,&IID_IFileDialog2,(void **)&filter_v2);
                if(SUCCEEDED(set))set=IFileDialog2_SetFileTypeIndex(filter_v2,2);
                if(filter_v2)IFileDialog2_Release(filter_v2);
                selected=0;get=IFileDialog_GetFileTypeIndex(dialog,&selected);
                filter_live_ok=SUCCEEDED(set)&&SUCCEEDED(get)&&selected==1;
                printf("LIVE_FILTER selected=1 set=%08lx remains=%u\n",(unsigned long)set,selected);
                UINT id=(UINT)GetDlgCtrlID(combo);SendMessageW(combo,CB_SETCURSEL,1,0);SendMessageW(GetParent(combo),WM_COMMAND,MAKEWPARAM(id,CBN_SELCHANGE),(LPARAM)combo);
                SendMessageW(GetParent(combo),WM_COMMAND,MAKEWPARAM(id,CBN_SELENDOK),(LPARAM)combo);filter_step=2;return;}
            if(FAILED(get)||selected!=2)return;
            filter_step=3;
        }
        PWSTR name=NULL;HRESULT name_hr=IFileDialog_GetFileName(dialog,&name);
        name_ok=SUCCEEDED(name_hr)&&name&&!lstrcmpW(name,L"preserved.txt");
        if(name)CoTaskMemFree(name);
        UINT filter=0;HRESULT filter_hr=IFileDialog_GetFileTypeIndex(dialog,&filter);
        HWND filter_combo=GetDlgItem(w,1136);LRESULT visual=filter_combo?SendMessageW(filter_combo,CB_GETCURSEL,0,0):1;
        filter_ok=SUCCEEDED(filter_hr)&&filter==2&&visual==1;
        HWND files=GetDlgItem(w,1120);
        if(old)filter_ok=filter_ok&&files&&SendMessageW(files,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)L"match.log")!=LB_ERR&&
            SendMessageW(files,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)L"other.txt")==LB_ERR;
        printf("LIVE_FOLDER_FILTER hr=%08lx index=%u visual=%lld\n",(unsigned long)filter_hr,filter,(long long)visual);
        WCHAR cwd[32768];DWORD length=GetCurrentDirectoryW(ARRAYSIZE(cwd),cwd);
        if(length&&length<ARRAYSIZE(cwd)&&SetCurrentDirectoryW(root)){stable=folder_is(sub);(void)SetCurrentDirectoryW(cwd);}
        IFileDialog2 *v2=NULL;IShellItem *item=NULL;PWSTR path=NULL;
        HRESULT hr=IFileDialog_QueryInterface(dialog,&IID_IFileDialog2,(void **)&v2);
        if(SUCCEEDED(hr))hr=IFileDialog2_GetFolder(v2,&item);
        if(SUCCEEDED(hr))hr=IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&path);
        v2_ok=SUCCEEDED(hr)&&path&&!lstrcmpiW(path,sub);
        if(path)CoTaskMemFree(path);
        if(item)IShellItem_Release(item);
        if(v2)IFileDialog2_Release(v2);
        if(programmatic){IShellItem *parent=NULL;if(SUCCEEDED(SHCreateItemFromParsingName(root,NULL,&IID_IShellItem,(void **)&parent))){
            IFileDialog2 *v2=NULL;if(SUCCEEDED(IFileDialog_QueryInterface(dialog,&IID_IFileDialog2,(void **)&v2))){(void)IFileDialog2_SetFolder(v2,parent);IFileDialog2_Release(v2);}IShellItem_Release(parent);}}
        else if(old){HWND list=GetDlgItem(w,1121);const WCHAR *leaf=wcsrchr(root,L'\\');
            LRESULT index=SendMessageW(list,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)(leaf?leaf+1:root));
            if(index!=LB_ERR){SendMessageW(list,LB_SETCURSEL,(WPARAM)index,0);
                SendMessageW(w,WM_COMMAND,MAKEWPARAM(1121,LBN_SELCHANGE),(LPARAM)list);
                selection_stable=selection_stable && folder_is(sub);printf("LIVE_FOLDER_SELECT before_parent=%ld\n",selection_stable);
                SendMessageW(w,WM_COMMAND,MAKEWPARAM(1121,LBN_DBLCLK),(LPARAM)list);}}
        else {IShellItem *parent=NULL;if(SUCCEEDED(SHCreateItemFromParsingName(root,NULL,&IID_IShellItem,(void **)&parent))){
            (void)IFileDialog_SetFolder(dialog,parent);IShellItem_Release(parent);}}
        phase=2;return;}
    if(phase==2 && folder_is(root)){parent_ok=1;KillTimer(w,id);InterlockedExchange(&done,1);PostMessageW(w,WM_CLOSE,0,0);phase=3;}
}
static BOOL CALLBACK find(HWND w,LPARAM p)
{
    (void)p;WCHAR text[80];GetWindowTextW(w,text,ARRAYSIZE(text));
    if(IsWindowVisible(w)&&!lstrcmpW(text,title)){window=w;seen=1;SetTimer(w,981,200,tick);return FALSE;}return TRUE;
}
static DWORD WINAPI observe(void *p)
{
    (void)p;for(int i=0;i<100;++i){if(!seen)EnumThreadWindows(ui,find,0);if(done)return 0;Sleep(50);}
    PostThreadMessageW(ui,WM_QUIT,0,0);return 1;
}
int wmain(int argc,WCHAR **argv)
{
    change_filter=argc>1&&!lstrcmpW(argv[1],L"filter");
    programmatic=change_filter||(argc>1&&!lstrcmpW(argv[1],L"set"));
    BOOL save=argc>2&&!lstrcmpW(argv[2],L"save");
    WCHAR temp[MAX_PATH];if(!GetTempPathW(MAX_PATH,temp)||swprintf(root,MAX_PATH,L"%lsWin16SFolder-%lu-%lu",temp,GetCurrentProcessId(),GetTickCount())<0||!CreateDirectoryW(root,NULL))return 2;
    if(swprintf(sub,MAX_PATH,L"%ls\\child",root)<0||!CreateDirectoryW(sub,NULL))return 3;
    if(swprintf(match_file,MAX_PATH,L"%ls\\match.log",sub)<0||swprintf(other_file,MAX_PATH,L"%ls\\other.txt",sub)<0)return 3;
    HANDLE file=CreateFileW(match_file,GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL);if(file==INVALID_HANDLE_VALUE)return 3;CloseHandle(file);
    file=CreateFileW(other_file,GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL);if(file==INVALID_HANDLE_VALUE)return 3;CloseHandle(file);
    HRESULT hr=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(hr))return 4;
    IShellItem *parent=NULL;if(FAILED(SHCreateItemFromParsingName(root,NULL,&IID_IShellItem,(void **)&parent))||FAILED(SHCreateItemFromParsingName(sub,NULL,&IID_IShellItem,(void **)&child)))return 5;
    hr=CoCreateInstance(save?&CLSID_FileSaveDialog:&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IFileDialog,(void **)&dialog);
    if(FAILED(hr)||FAILED(IFileDialog_SetTitle(dialog,title))||FAILED(IFileDialog_SetFolder(dialog,parent))||FAILED(IFileDialog_SetFileName(dialog,L"preserved.txt")))return 6;
    COMDLG_FILTERSPEC filters[]={{L"Text",L"*.txt"},{L"Log",L"*.log"}};
    if(FAILED(IFileDialog_SetFileTypes(dialog,2,filters))||FAILED(IFileDialog_SetFileTypeIndex(dialog,2)))return 6;
    IShellItem_Release(parent);ui=GetCurrentThreadId();HANDLE thread=CreateThread(NULL,0,observe,NULL,0,NULL);if(!thread)return 7;
    hr=IFileDialog_Show(dialog,NULL);InterlockedExchange(&done,1);DWORD wait=WaitForSingleObject(thread,10000),code=99;
    if(wait==WAIT_OBJECT_0)GetExitCodeThread(thread,&code);
    CloseHandle(thread);IFileDialog_Release(dialog);IShellItem_Release(child);CoUninitialize();DeleteFileW(match_file);DeleteFileW(other_file);RemoveDirectoryW(sub);RemoveDirectoryW(root);
    printf("LIVE_FOLDER_RESULT bits=%u old=%d initial=%ld final=%ld stable=%ld v2=%ld parent=%ld name=%ld filter=%ld phase=%ld hr=%08lx wait=%lu worker=%lu livefilter=%ld\n",(unsigned)(sizeof(void*)*8),old,initial_ok,final_ok,stable,v2_ok,parent_ok,name_ok,filter_ok,phase,(unsigned long)hr,wait,code,filter_live_ok);
    return seen&&initial_ok&&final_ok&&stable&&selection_stable&&v2_ok&&parent_ok&&(!programmatic||((name_ok||change_filter)&&filter_ok))&&(!change_filter||filter_live_ok)&&phase==3&&hr==HRESULT_FROM_WIN32(ERROR_CANCELLED)&&wait==WAIT_OBJECT_0&&code==0?0:1;
}
