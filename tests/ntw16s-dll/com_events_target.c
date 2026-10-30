/* Real COM event observer. The driver owns the target and its dialog. */
#define UNICODE
#define _UNICODE
#define COBJMACROS
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <stdio.h>
#include <wchar.h>
typedef struct listener {IFileDialogEvents iface;LONG refs;int tag;} listener;
static volatile LONG called,correct,identity,seen,old_layout,kept,ole_window,ole_identity;static DWORD owner;static HWND window;
static volatile LONG selected_correct,filter_changed;static BOOL select_mode,filter_mode;
static BOOL save_mode,folder_mode,folder_new,folder_invalid,folder_cwd,folder_noevents,folder_unicode,folder_veto_new,folder_exists,folder_select;static volatile LONG folder_prompt,folder_rejected,folder_vetoes,folder_child_changed,folder_veto_root;
static PCWSTR overwrite_other_drive;static BOOL overwrite_drive_relative,overwrite_rooted,overwrite_relative,overwrite_mode,overwrite_extension,overwrite_default,overwrite_multi,overwrite_multi_default,overwrite_notimpl,overwrite_denied;static LONG overwrite_called,overwrite_correct,overwrite_other,prompt_seen,prompt_order;
static BOOL share_mode,share_refuse;static LONG share_called,share_correct;
static WCHAR test_nested[MAX_PATH];static WCHAR expected[MAX_PATH];static IUnknown *expected_identity;static BOOL self_unadvise;static DWORD event_cookie;static HRESULT self_result=E_FAIL;
static BOOL mutation_mode,multi_veto,multi_error,multi_notimpl,close_mode,close_veto,fileok_error,fileok_notimpl,fileok_success2;static DWORD other_cookie;static HRESULT other_result=E_FAIL,close_result=E_FAIL;static LONG other_called,done;
static HRESULT STDMETHODCALLTYPE qi(IFileDialogEvents *p,REFIID id,void **out)
{if(!out)return E_POINTER;*out=NULL;if(!IsEqualIID(id,&IID_IUnknown)&&!IsEqualIID(id,&IID_IFileDialogEvents))return E_NOINTERFACE;*out=p;InterlockedIncrement(&((listener *)p)->refs);return S_OK;}
static ULONG STDMETHODCALLTYPE add(IFileDialogEvents *p){return (ULONG)InterlockedIncrement(&((listener *)p)->refs);}
static ULONG STDMETHODCALLTYPE release(IFileDialogEvents *p){return (ULONG)InterlockedDecrement(&((listener *)p)->refs);}
static HRESULT STDMETHODCALLTYPE ok(IFileDialogEvents *p,IFileDialog *dialog)
{
    if(multi_veto){listener *e=(listener *)p;if(e->tag==2){++other_called;return S_OK;}return ++called==1?(multi_notimpl?E_NOTIMPL:multi_error?E_ACCESSDENIED:S_FALSE):S_OK;}
    if(mutation_mode){listener *e=(listener *)p;
        if(e->tag==2){++other_called;return S_OK;}
        ++called;other_result=IFileDialog_Unadvise(dialog,other_cookie);return S_OK;}
    if(close_mode){++called;close_result=IFileDialog_Close(dialog,E_ABORT);return close_veto?S_FALSE:S_OK;}
    (void)p;IUnknown *id=NULL;IShellItem *item=NULL;PWSTR path=NULL;
    if(SUCCEEDED(IFileDialog_QueryInterface(dialog,&IID_IUnknown,(void **)&id)) && id==expected_identity)InterlockedIncrement(&identity);
    if(SUCCEEDED(IFileDialog_GetResult(dialog,&item)) && SUCCEEDED(IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&path)) && !lstrcmpiW(path,expected))InterlockedIncrement(&correct);
    IOleWindow *ole=NULL;IUnknown *ole_id=NULL;HWND native_window=NULL;
    if(SUCCEEDED(IFileDialog_QueryInterface(dialog,&IID_IOleWindow,(void **)&ole))){
        if(SUCCEEDED(IOleWindow_GetWindow(ole,&native_window)) && native_window && IsWindow(native_window))InterlockedIncrement(&ole_window);
        if(SUCCEEDED(IOleWindow_QueryInterface(ole,&IID_IUnknown,(void **)&ole_id)) && ole_id==expected_identity)InterlockedIncrement(&ole_identity);
        if(ole_id)IUnknown_Release(ole_id);
        IOleWindow_Release(ole);
    }

    if(path)CoTaskMemFree(path);
    if(item)IShellItem_Release(item);
    if(id)IUnknown_Release(id);
    LONG count=InterlockedIncrement(&called);
    if(self_unadvise){self_result=IFileDialog_Unadvise(dialog,event_cookie);return S_OK;}
    if(count==1 && fileok_success2)return (HRESULT)2;
    if(count==1 && fileok_notimpl)return E_NOTIMPL;
    if(count==1 && fileok_error)return E_ACCESSDENIED;
    return !overwrite_mode && !share_mode && count==1?S_FALSE:S_OK;
}
static void CALLBACK folder_veto_check(HWND w,UINT message,UINT_PTR id,DWORD time)
{
    (void)message;(void)time;KillTimer(w,id);IShellItem *item=NULL;PWSTR path=NULL;
    IFileDialog *dialog=(IFileDialog *)RemovePropW(w,L"Win16S.TestDialog");
    HRESULT hr=dialog?IFileDialog_GetFolder(dialog,&item):E_FAIL;
    if(SUCCEEDED(hr))hr=IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&path);
    WCHAR root[MAX_PATH];lstrcpynW(root,expected,MAX_PATH);WCHAR *slash=wcsrchr(root,L'\\');if(slash)*slash=0;slash=wcsrchr(root,L'\\');
    HWND list=GetDlgItem(w,1121);LRESULT index=list?SendMessageW(list,LB_GETCURSEL,0,0):LB_ERR;
    LRESULT len=index>=0?SendMessageW(list,LB_GETTEXTLEN,(WPARAM)index,0):LB_ERR;WCHAR name[MAX_PATH]=L"";
    BOOL displayed=len>0 && len<MAX_PATH && SendMessageW(list,LB_GETTEXT,(WPARAM)index,(LPARAM)name)==len &&
        slash && !lstrcmpiW(name,slash+1);
    if(SUCCEEDED(hr) && path && !lstrcmpiW(path,root) && displayed)InterlockedExchange(&folder_veto_root,1);
    if(path)CoTaskMemFree(path);
    if(item)IShellItem_Release(item);
}
static HRESULT STDMETHODCALLTYPE folder_changing(IFileDialogEvents *p,IFileDialog *d,IShellItem *i)
{
    (void)p;PWSTR path=NULL;HRESULT hr=i?IShellItem_GetDisplayName(i,SIGDN_FILESYSPATH,&path):E_INVALIDARG;
    printf("EVENT_FOLDER_CHANGING item=%08lx path=%ls\n",(unsigned long)hr,path?path:L"(none)");
    BOOL veto=folder_veto_new && folder_prompt && path && !lstrcmpiW(path,expected);
    if(veto){InterlockedIncrement(&folder_vetoes);SetPropW(window,L"Win16S.TestDialog",d);SetTimer(window,982,1,folder_veto_check);}
    if(path)CoTaskMemFree(path);
    return veto?S_FALSE:S_OK;
}
static HRESULT STDMETHODCALLTYPE folder_changed(IFileDialogEvents *p,IFileDialog *d)
{
    (void)p;IShellItem *item=NULL;PWSTR path=NULL;HRESULT hr=IFileDialog_GetFolder(d,&item);
    if(SUCCEEDED(hr))hr=IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&path);
    printf("EVENT_FOLDER_CHANGED item=%08lx path=%ls\n",(unsigned long)hr,path?path:L"(none)");
    if(folder_veto_new && path && !lstrcmpiW(path,expected) && folder_prompt)InterlockedIncrement(&folder_child_changed);
    if(path)CoTaskMemFree(path);
    if(item)IShellItem_Release(item);
    return S_OK;
}
static HRESULT STDMETHODCALLTYPE selection_changed(IFileDialogEvents *p,IFileDialog *d){(void)p;IShellItem *item=NULL;PWSTR path=NULL;HRESULT hr=IFileDialog_GetCurrentSelection(d,&item);if(SUCCEEDED(hr))hr=IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&path);if(SUCCEEDED(hr)&&path&&!lstrcmpiW(path,expected))InterlockedIncrement(&selected_correct);printf("EVENT_SELECTION item=%08lx path=%ls\n",(unsigned long)hr,path?path:L"(none)");if(path)CoTaskMemFree(path);
    if(item)IShellItem_Release(item);
    return S_OK;}
static HRESULT STDMETHODCALLTYPE type_changed(IFileDialogEvents *p,IFileDialog *d){(void)p;UINT type=0;HRESULT hr=IFileDialog_GetFileTypeIndex(d,&type);if(SUCCEEDED(hr)&&type==2)InterlockedIncrement(&filter_changed);printf("EVENT_TYPE hr=%08lx index=%u\n",(unsigned long)hr,type);return S_OK;}
static HRESULT STDMETHODCALLTYPE share(IFileDialogEvents *p,IFileDialog *d,IShellItem *i,FDE_SHAREVIOLATION_RESPONSE *r){
    (void)p;(void)d;if(!share_mode){*r=FDESVR_DEFAULT;return S_OK;}
    PWSTR path=NULL;HRESULT hr=i?IShellItem_GetDisplayName(i,SIGDN_FILESYSPATH,&path):E_INVALIDARG;
    if(SUCCEEDED(hr)&&path&&!lstrcmpiW(path,expected))++share_correct;
    if(path)CoTaskMemFree(path);
    ++share_called;*r=share_refuse&&share_called==1?FDESVR_REFUSE:FDESVR_ACCEPT;return S_OK;}
static HRESULT STDMETHODCALLTYPE overwrite(IFileDialogEvents *p,IFileDialog *d,IShellItem *i,FDE_OVERWRITE_RESPONSE *r){
    (void)p;(void)d;if(!overwrite_mode){*r=FDEOR_DEFAULT;return S_OK;}
    if(((listener *)p)->tag==2)++overwrite_other;
    PWSTR path=NULL;HRESULT hr=i?IShellItem_GetDisplayName(i,SIGDN_FILESYSPATH,&path):E_INVALIDARG;
    if(SUCCEEDED(hr)&&path&&!lstrcmpiW(path,expected))++overwrite_correct;
    if(path)CoTaskMemFree(path);
    ++overwrite_called;*r=overwrite_default?FDEOR_DEFAULT:overwrite_multi_default?(((listener *)p)->tag==1?FDEOR_DEFAULT:FDEOR_ACCEPT):(overwrite_called==1?FDEOR_REFUSE:FDEOR_ACCEPT);
    return overwrite_notimpl&&((listener *)p)->tag==1?(overwrite_denied?E_ACCESSDENIED:E_NOTIMPL):S_OK;}
static IFileDialogEventsVtbl table={qi,add,release,ok,folder_changing,folder_changed,selection_changed,share,type_changed,overwrite};
static BOOL CALLBACK scan(HWND w,LPARAM arg)
{
    (void)arg;WCHAR title[100];GetWindowTextW(w,title,ARRAYSIZE(title));
    if(IsWindowVisible(w)&&!lstrcmpW(title,L"Win16S COM event")){window=w;old_layout=GetDlgItem(w,1121)!=NULL;InterlockedExchange(&seen,1);return FALSE;}
    return TRUE;
}
static BOOL CALLBACK confirm_button(HWND w,LPARAM arg)
{
    (void)arg;WCHAR cls[32],text[80];GetClassNameW(w,cls,ARRAYSIZE(cls));GetWindowTextW(w,text,ARRAYSIZE(text));
    if(!lstrcmpiW(cls,L"Button") && (!lstrcmpW(text,L"&Yes") || !lstrcmpW(text,L"Yes"))){
        if(overwrite_called==1 && called==0)InterlockedExchange(&prompt_order,1);
        InterlockedExchange(&prompt_seen,1);PostMessageW(w,BM_CLICK,0,0);return FALSE;}
    return TRUE;
}
static BOOL CALLBACK confirm_scan(HWND w,LPARAM arg)
{
    (void)arg;WCHAR name[100];GetWindowTextW(w,name,ARRAYSIZE(name));
    if(w!=window && IsWindowVisible(w) && !lstrcmpW(name,L"Confirm Save As"))EnumChildWindows(w,confirm_button,0);
    return TRUE;
}
static BOOL CALLBACK folder_error_text(HWND child,LPARAM data)
{
    WCHAR text[256];GetWindowTextW(child,text,ARRAYSIZE(text));
    WCHAR expected_error[512]=L"";
    if(folder_exists)FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,NULL,ERROR_ALREADY_EXISTS,0,expected_error,ARRAYSIZE(expected_error),NULL);
    else lstrcpynW(expected_error,L"Enter a valid folder name.",ARRAYSIZE(expected_error));
    if(expected_error[0] && !lstrcmpW(text,expected_error)){*(BOOL *)data=TRUE;return FALSE;}
    return TRUE;
}
static BOOL CALLBACK folder_prompt_scan(HWND w,LPARAM ignored)
{
    (void)ignored;WCHAR title[128];GetWindowTextW(w,title,ARRAYSIZE(title));
    if(w==window || !IsWindowVisible(w) || lstrcmpW(title,L"New Folder"))return TRUE;
    HWND edit=GetDlgItem(w,102);
    if(edit && !folder_prompt){
        if(!folder_invalid){if(wcslen(expected)+9>=ARRAYSIZE(expected))return FALSE;wcscat(expected,L"\\created");}
        SetWindowTextW(edit,folder_invalid?L"nested\\escape":L"created");InterlockedExchange(&folder_prompt,1);
        PostMessageW(w,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)GetDlgItem(w,IDOK));return FALSE;
    }
    if((folder_invalid||folder_exists) && !edit && folder_prompt && !folder_rejected){
        BOOL matched=FALSE;EnumChildWindows(w,folder_error_text,(LPARAM)&matched);if(!matched)return TRUE;
        HWND button=GetDlgItem(w,IDOK);if(!button)button=GetDlgItem(w,IDCANCEL);
        if(button){InterlockedExchange(&folder_rejected,1);PostMessageW(button,BM_CLICK,0,0);}return FALSE;
    }
    if((folder_invalid||folder_exists) && edit && folder_rejected && IsWindowEnabled(w)){
        PostMessageW(w,WM_COMMAND,MAKEWPARAM(IDCANCEL,BN_CLICKED),(LPARAM)GetDlgItem(w,IDCANCEL));return FALSE;
    }
    return TRUE;
}
static DWORD WINAPI observe(void *unused)
{
    (void)unused;BOOL first=FALSE,second=FALSE,selected=FALSE,filtered=FALSE,submit_new=FALSE;
    for(int i=0;i<160;++i){if(!seen)EnumThreadWindows(owner,scan,0);
        if(folder_new && first && (!folder_prompt || ((folder_invalid||folder_exists) && !IsWindowEnabled(window))))EnumThreadWindows(owner,folder_prompt_scan,0);
        if(folder_new && folder_prompt && !submit_new && (!folder_veto_new || folder_veto_root) && IsWindow(window) && IsWindowEnabled(window)){
            int id=(folder_invalid||folder_exists||folder_veto_new)?IDCANCEL:IDOK;PostMessageW(window,WM_COMMAND,MAKEWPARAM(id,BN_CLICKED),(LPARAM)GetDlgItem(window,id));submit_new=TRUE;}
        if((overwrite_default||overwrite_multi_default) && seen && !prompt_seen)EnumThreadWindows(owner,confirm_scan,0);
        if(filter_mode && seen && old_layout && !filtered){HWND combo=GetDlgItem(window,1136);if(combo && SendMessageW(combo,CB_SETCURSEL,1,0)!=CB_ERR){
            SendMessageW(window,WM_COMMAND,MAKEWPARAM(1136,CBN_SELCHANGE),(LPARAM)combo);filtered=TRUE;}}
        if(select_mode && seen && old_layout && !selected && (!filter_mode || filter_changed)){HWND list=GetDlgItem(window,1120);LRESULT index=SendMessageW(list,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)L"selected.txt");
            if(index!=LB_ERR){SendMessageW(list,LB_SETCURSEL,(WPARAM)index,0);SendMessageW(window,WM_COMMAND,MAKEWPARAM(1120,LBN_SELCHANGE),(LPARAM)list);selected=TRUE;}}
        if(seen && IsWindow(window) && i>10 && !first && (!select_mode||selected_correct)){if(overwrite_relative){WCHAR tmp[MAX_PATH];WCHAR drive_name[]={expected[0],L':',L's',L'e',L'l',L'e',L'c',L't',L'e',L'd',L'.',L't',L'x',L't',0};if(!GetTempPathW(MAX_PATH,tmp)||!SetCurrentDirectoryW(overwrite_rooted?overwrite_other_drive:tmp)||!SetDlgItemTextW(window,1152,overwrite_drive_relative?drive_name:overwrite_rooted?expected+2:L"selected.txt"))return 2;}if(folder_cwd && !SetCurrentDirectoryW(test_nested))return 2;
            if(folder_select){HWND list=GetDlgItem(window,1121);LRESULT previous=SendMessageW(list,LB_GETCURSEL,0,0);
                LRESULT child=SendMessageW(list,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)L"nested");
                if(child==LB_ERR || child==previous)return 2;
                SendMessageW(list,LB_SETCURSEL,(WPARAM)child,0);
                SendMessageW(window,WM_COMMAND,MAKEWPARAM(1121,LBN_SELCHANGE),(LPARAM)list);
                if(SendMessageW(list,LB_GETCURSEL,0,0)!=child)return 2;}
            int id=folder_new?0x5717:IDOK;HWND b=GetDlgItem(window,id);PostMessageW(window,WM_COMMAND,MAKEWPARAM(id,BN_CLICKED),(LPARAM)b);first=TRUE;}
        if(first && !second && !overwrite_default && !overwrite_multi_default && (share_refuse?share_called>=1:overwrite_mode?overwrite_called>=1:called>=1) && IsWindow(window) && !close_mode){InterlockedExchange(&kept,1);HWND b=GetDlgItem(window,IDOK);PostMessageW(window,WM_COMMAND,MAKEWPARAM(IDOK,BN_CLICKED),(LPARAM)b);second=TRUE;}
        if(done || (first&&!IsWindow(window)&&(!folder_new||folder_prompt)))return 0;
        Sleep(50);}
    PostThreadMessageW(owner,WM_QUIT,0,0);return 1;
}
int wmain(int argc,WCHAR **argv)
{
    folder_select=argc>1&&!lstrcmpW(argv[1],L"folder-select-noevents");folder_exists=argc>1&&!lstrcmpW(argv[1],L"folder-existing-noevents");folder_veto_new=argc>1&&!lstrcmpW(argv[1],L"folder-veto-new");folder_unicode=argc>1&&!lstrcmpW(argv[1],L"folder-unicode");folder_noevents=folder_select||folder_exists||(argc>1&&(!lstrcmpW(argv[1],L"folder-noevents")||!lstrcmpW(argv[1],L"folder-invalid-noevents")||!lstrcmpW(argv[1],L"folder-cwd-noevents")));folder_invalid=argc>1&&(!lstrcmpW(argv[1],L"folder-invalid")||!lstrcmpW(argv[1],L"folder-invalid-noevents"));folder_cwd=folder_select||(argc>1&&(!lstrcmpW(argv[1],L"folder-cwd")||!lstrcmpW(argv[1],L"folder-cwd-noevents")));folder_new=folder_veto_new||folder_unicode||folder_noevents||folder_invalid||folder_cwd||(argc>1&&!lstrcmpW(argv[1],L"folder-new"));folder_mode=folder_new||(argc>1&&(!lstrcmpW(argv[1],L"folder")||!lstrcmpW(argv[1],L"folder-old")));BOOL removed=argc>1&&!lstrcmpW(argv[1],L"remove");mutation_mode=argc>1&&!lstrcmpW(argv[1],L"mutation");multi_notimpl=argc>1&&(!lstrcmpW(argv[1],L"multi-notimpl")||!lstrcmpW(argv[1],L"multi-notimpl-old"));multi_error=argc>1&&(!lstrcmpW(argv[1],L"multi-error")||!lstrcmpW(argv[1],L"multi-error-old"));multi_veto=multi_notimpl||multi_error||(argc>1&&!lstrcmpW(argv[1],L"multi-veto"));close_veto=argc>1&&!lstrcmpW(argv[1],L"close-veto");close_mode=close_veto||(argc>1&&!lstrcmpW(argv[1],L"close"));filter_mode=argc>1&&!lstrcmpW(argv[1],L"filter");select_mode=filter_mode||(argc>1&&!lstrcmpW(argv[1],L"select"));
    fileok_success2=argc>1&&(!lstrcmpW(argv[1],L"fileok-success2")||!lstrcmpW(argv[1],L"fileok-success2-old"));
    fileok_error=argc>1&&(!lstrcmpW(argv[1],L"fileok-error")||!lstrcmpW(argv[1],L"fileok-error-old"));
    fileok_notimpl=argc>1&&(!lstrcmpW(argv[1],L"fileok-notimpl")||!lstrcmpW(argv[1],L"fileok-notimpl-old"));
    self_unadvise=argc>1&&!lstrcmpW(argv[1],L"self");
    overwrite_default=argc>1&&(!lstrcmpW(argv[1],L"overwrite-default")||!lstrcmpW(argv[1],L"overwrite-default-old"));
    overwrite_denied=argc>1&&(!lstrcmpW(argv[1],L"overwrite-multi-denied")||!lstrcmpW(argv[1],L"overwrite-multi-denied-old"));
    overwrite_notimpl=overwrite_denied||(argc>1&&(!lstrcmpW(argv[1],L"overwrite-multi-notimpl")||!lstrcmpW(argv[1],L"overwrite-multi-notimpl-old")));
    overwrite_multi_default=overwrite_notimpl||(argc>1&&(!lstrcmpW(argv[1],L"overwrite-multi-default")||!lstrcmpW(argv[1],L"overwrite-multi-default-old")));
    overwrite_multi=overwrite_multi_default||(argc>1&&(!lstrcmpW(argv[1],L"overwrite-multi")||!lstrcmpW(argv[1],L"overwrite-multi-old")));
    share_refuse=argc>1&&(!lstrcmpW(argv[1],L"share-refuse")||!lstrcmpW(argv[1],L"share-refuse-old"));
    share_mode=share_refuse||(argc>1&&(!lstrcmpW(argv[1],L"share")||!lstrcmpW(argv[1],L"share-old")));
    overwrite_extension=argc>1&&(!lstrcmpW(argv[1],L"overwrite-ext")||!lstrcmpW(argv[1],L"overwrite-ext-old"));
    overwrite_rooted=argc>1&&!lstrcmpW(argv[1],L"overwrite-rooted-old");
    overwrite_other_drive=overwrite_rooted&&argc>2?argv[2]:NULL;if(overwrite_rooted&&!overwrite_other_drive)return 2;
    overwrite_drive_relative=argc>1&&!lstrcmpW(argv[1],L"overwrite-drive-relative-old");
    overwrite_relative=overwrite_drive_relative||overwrite_rooted||(argc>1&&!lstrcmpW(argv[1],L"overwrite-relative-old"));
    overwrite_mode=overwrite_relative||overwrite_default||overwrite_multi||overwrite_extension||(argc>1&&(!lstrcmpW(argv[1],L"overwrite")||!lstrcmpW(argv[1],L"overwrite-old")));
    save_mode=overwrite_mode||(argc>1&&(!lstrcmpW(argv[1],L"save")||!lstrcmpW(argv[1],L"save-native")));
    BOOL require_old=removed || select_mode || (argc>1&&(!lstrcmpW(argv[1],L"fileok-error-old")||!lstrcmpW(argv[1],L"fileok-notimpl-old")||!lstrcmpW(argv[1],L"multi-error-old")||!lstrcmpW(argv[1],L"multi-notimpl-old")||!lstrcmpW(argv[1],L"fileok-success2-old"))) || (argc>1&&!lstrcmpW(argv[1],L"folder-old")) || (argc>1&&(!lstrcmpW(argv[1],L"old")||!lstrcmpW(argv[1],L"save")||!lstrcmpW(argv[1],L"overwrite-old")||!lstrcmpW(argv[1],L"overwrite-relative-old")||!lstrcmpW(argv[1],L"overwrite-drive-relative-old")||!lstrcmpW(argv[1],L"overwrite-rooted-old")||!lstrcmpW(argv[1],L"overwrite-ext-old")||!lstrcmpW(argv[1],L"overwrite-default-old")||!lstrcmpW(argv[1],L"overwrite-multi-old")||!lstrcmpW(argv[1],L"overwrite-multi-default-old")||!lstrcmpW(argv[1],L"overwrite-multi-notimpl-old")||!lstrcmpW(argv[1],L"overwrite-multi-denied-old")||!lstrcmpW(argv[1],L"share-old")||!lstrcmpW(argv[1],L"share-refuse-old"))),release_only=argc>1&&!lstrcmpW(argv[1],L"release");WCHAR temp[MAX_PATH],root[MAX_PATH];
    if(!GetTempPathW(MAX_PATH,temp))return 2;
    if(overwrite_rooted && (wcslen(overwrite_other_drive)<3 || overwrite_other_drive[1]!=L':' || overwrite_other_drive[2]!=L'\\' || temp[1]!=L':' || !_wcsnicmp(temp,overwrite_other_drive,1)))return 2;
    if(swprintf(root,MAX_PATH,folder_unicode?L"%lsWin16S Folder \u6d4b\u8bd5-%lu-%lu":L"%lsWin16SEvent-%lu-%lu",temp,GetCurrentProcessId(),GetTickCount())<0||!CreateDirectoryW(root,NULL)||
       swprintf(expected,MAX_PATH,L"%ls\\selected.txt",root)<0)return 2;
    if(!save_mode||overwrite_mode){HANDLE file=CreateFileW(expected,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);if(file==INVALID_HANDLE_VALUE)return 2;CloseHandle(file);}
    if(folder_mode){if(!DeleteFileW(expected))return 2;lstrcpynW(expected,root,MAX_PATH);}
    WCHAR marker[MAX_PATH]=L"";
    if(folder_exists){WCHAR existing[MAX_PATH];if(swprintf(existing,MAX_PATH,L"%ls\\created",root)<0 || !CreateDirectoryW(existing,NULL) ||
        swprintf(marker,MAX_PATH,L"%ls\\marker.txt",existing)<0)return 2;
        HANDLE keep=CreateFileW(marker,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
        if(keep==INVALID_HANDLE_VALUE)return 2;
        CloseHandle(keep);}
    WCHAR nested[MAX_PATH]=L"",escaped[MAX_PATH]=L"";
    if((folder_invalid||folder_cwd) && (swprintf(nested,MAX_PATH,L"%ls\\nested",root)<0 ||
        swprintf(escaped,MAX_PATH,L"%ls\\nested\\escape",root)<0 || !CreateDirectoryW(nested,NULL)))return 2;
    HANDLE locked=share_mode?CreateFileW(expected,GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL):NULL;
    if(share_mode && locked==INVALID_HANDLE_VALUE)return 2;
    HRESULT init=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);if(FAILED(init))return 3;
    IFileDialog *dialog=NULL;HRESULT hr=CoCreateInstance(save_mode?&CLSID_FileSaveDialog:&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IFileDialog,(void **)&dialog);
    if(SUCCEEDED(hr) && folder_mode){FILEOPENDIALOGOPTIONS options=0;
        if(FAILED(IFileDialog_GetOptions(dialog,&options))||FAILED(IFileDialog_SetOptions(dialog,options|FOS_PICKFOLDERS)))return 4;}
    if(SUCCEEDED(hr) && share_mode){FILEOPENDIALOGOPTIONS options=0;
        if(FAILED(IFileDialog_GetOptions(dialog,&options)) || FAILED(IFileDialog_SetOptions(dialog,options|FOS_SHAREAWARE)))return 4;}
    if(SUCCEEDED(hr) && save_mode && !overwrite_mode){FILEOPENDIALOGOPTIONS options=0;
        if(FAILED(IFileDialog_GetOptions(dialog,&options)) || FAILED(IFileDialog_SetOptions(dialog,options&~FOS_OVERWRITEPROMPT)))return 4;}
    WCHAR input[MAX_PATH];lstrcpynW(input,expected,MAX_PATH);
    if(overwrite_extension){WCHAR *extension=wcsrchr(input,L'.');if(!extension)return 4;*extension=0;
        if(FAILED(IFileDialog_SetDefaultExtension(dialog,L"txt")))return 4;}
    if(folder_cwd)lstrcpynW(test_nested,nested,MAX_PATH);
    if(FAILED(hr)||FAILED(IFileDialog_SetTitle(dialog,L"Win16S COM event"))||(!folder_mode&&FAILED(IFileDialog_SetFileName(dialog,input)))||
       FAILED(IFileDialog_QueryInterface(dialog,&IID_IUnknown,(void **)&expected_identity)))return 4;
    IShellItem *folder=NULL;if(FAILED(SHCreateItemFromParsingName(root,NULL,&IID_IShellItem,(void **)&folder)))return 4;
    HRESULT folder_hr=IFileDialog_SetFolder(dialog,folder);IShellItem_Release(folder);if(FAILED(folder_hr))return 4;
    if(filter_mode){COMDLG_FILTERSPEC filters[]={{L"Text",L"*.txt"},{L"All",L"*.*"}};
        if(FAILED(IFileDialog_SetFileTypes(dialog,2,filters))||FAILED(IFileDialog_SetFileTypeIndex(dialog,1)))return 4;}
    listener events={0};events.iface.lpVtbl=&table;events.refs=1;events.tag=1;DWORD cookie=0;
    if(!folder_noevents && FAILED(IFileDialog_Advise(dialog,&events.iface,&cookie)))return 5;
    listener other={0};other.iface.lpVtbl=&table;other.refs=1;other.tag=2;
    if((mutation_mode||multi_veto||overwrite_multi)&&FAILED(IFileDialog_Advise(dialog,&other.iface,&other_cookie)))return 5;
    event_cookie=cookie;
    if(removed && FAILED(IFileDialog_Unadvise(dialog,cookie)))return 5;
    owner=GetCurrentThreadId();HANDLE worker=CreateThread(NULL,0,observe,NULL,0,NULL);if(!worker)return 6;
    hr=IFileDialog_Show(dialog,NULL);InterlockedExchange(&done,1);DWORD wait=WaitForSingleObject(worker,10000),worker_code=99;
    if(wait==WAIT_OBJECT_0)GetExitCodeThread(worker,&worker_code);
    CloseHandle(worker);
    HRESULT close_item_hr=E_UNEXPECTED;IShellItem *close_item=NULL;
    if(close_mode){close_item_hr=IFileDialog_GetResult(dialog,&close_item);if(close_item)IShellItem_Release(close_item);}
    HRESULT unadvise=(folder_noevents||release_only||removed)?S_OK:self_unadvise?self_result:IFileDialog_Unadvise(dialog,cookie);
    if(multi_veto||overwrite_multi)(void)IFileDialog_Unadvise(dialog,other_cookie);
    BOOL folder_result_ok=TRUE;if(folder_mode){IShellItem *final_item=NULL;PWSTR final_path=NULL;HRESULT final_hr=IFileDialog_GetResult(dialog,&final_item);
        if(SUCCEEDED(final_hr))final_hr=IShellItem_GetDisplayName(final_item,SIGDN_FILESYSPATH,&final_path);
        folder_result_ok=SUCCEEDED(final_hr)&&final_path&&!lstrcmpiW(final_path,expected);
        printf("FOLDER_EVENT_FINAL correct=%d hr=%08lx\n",folder_result_ok,(unsigned long)final_hr);
        if(final_path)CoTaskMemFree(final_path);
        if(final_item)IShellItem_Release(final_item);}
    BOOL made=folder_new && (folder_invalid || folder_prompt) && GetFileAttributesW(folder_invalid?escaped:expected)!=INVALID_FILE_ATTRIBUTES;
    IUnknown_Release(expected_identity);IFileDialog_Release(dialog);CoUninitialize();if(locked)CloseHandle(locked);
    BOOL file_removed=folder_mode || DeleteFileW(expected);
    WCHAR unexpected[MAX_PATH]=L"";BOOL wrong=FALSE;
    if(folder_cwd){swprintf(unexpected,MAX_PATH,L"%ls\\created",nested);wrong=GetFileAttributesW(unexpected)!=INVALID_FILE_ATTRIBUTES;
        if(wrong)RemoveDirectoryW(unexpected);
        SetCurrentDirectoryW(temp);}
    if(folder_mode || overwrite_relative)SetCurrentDirectoryW(temp);
    BOOL marker_kept=!folder_exists || GetFileAttributesW(marker)!=INVALID_FILE_ATTRIBUTES;
    BOOL marker_removed=!folder_exists || DeleteFileW(marker);
    BOOL child_removed=marker_removed && (!folder_new || folder_invalid || !made || RemoveDirectoryW(expected));
    BOOL nested_removed=!(folder_invalid||folder_cwd) || RemoveDirectoryW(nested);BOOL root_removed=RemoveDirectoryW(root);
    printf("COM_EVENTS bits=%u seen=%ld old=%ld calls=%ld correct=%ld identity=%ld kept=%ld hr=%08lx unadvise=%08lx refs=%ld wait=%lu ole=%ld ole_identity=%ld\n",
        (unsigned)(sizeof(void*)*8),seen,old_layout,called,correct,identity,kept,(unsigned long)hr,(unsigned long)unadvise,events.refs,wait,ole_window,ole_identity);
    printf("COM_EVENT_SELECTION selected_correct=%ld filter_changed=%ld\n",selected_correct,filter_changed);
    if(folder_new){printf("COM_FOLDER_SELECT enabled=%d\n",folder_select);printf("COM_FOLDER_EXISTING exists=%d rejected=%ld marker=%d\n",folder_exists,folder_rejected,marker_kept);printf("COM_FOLDER_CREATE_VETO veto=%ld child_changed=%ld root=%ld\n",folder_vetoes,folder_child_changed,folder_veto_root);printf("COM_FOLDER_CREATE invalid=%d prompt=%ld rejected=%ld made=%d cleaned=%d wrong_directory=%d calls=%ld hr=%08lx refs=%ld worker=%lu\n",
        folder_invalid,folder_prompt,folder_rejected,made,child_removed&&nested_removed&&root_removed,wrong,called,(unsigned long)hr,events.refs,worker_code);
        return seen&&old_layout&&folder_prompt&&!wrong&&child_removed&&nested_removed&&root_removed&&events.refs==1&&wait==WAIT_OBJECT_0&&worker_code==0&&
            (folder_veto_new?(folder_vetoes==1 && !folder_child_changed && folder_veto_root && made && called==0 && hr==HRESULT_FROM_WIN32(ERROR_CANCELLED)):
             folder_exists?(folder_rejected&&made&&marker_kept&&called==0&&hr==HRESULT_FROM_WIN32(ERROR_CANCELLED)):
             folder_invalid?(folder_rejected&&!made&&called==0&&hr==HRESULT_FROM_WIN32(ERROR_CANCELLED)):
             (made&&folder_result_ok&&SUCCEEDED(hr)&&(folder_noevents?called==0:(called==2&&correct==2&&identity==2&&kept))))?0:1;}

    if(fileok_error || fileok_notimpl || fileok_success2){LONG expected_calls=fileok_notimpl?1:2;
        printf("COM_FILEOK_HRESULT notimpl=%d calls=%ld expected=%ld kept=%ld refs=%ld worker=%lu\n",fileok_notimpl,called,expected_calls,kept,events.refs,worker_code);
        return seen&&(!require_old||old_layout)&&called==expected_calls&&correct==expected_calls&&identity==expected_calls&&
            (!(fileok_error||fileok_success2)||kept)&&SUCCEEDED(hr)&&SUCCEEDED(unadvise)&&events.refs==1&&wait==WAIT_OBJECT_0&&worker_code==0?0:1;}
    if(share_mode){printf("COM_EVENT_SHARE count=%ld correct=%ld ok=%ld refs=%ld worker=%lu\n",share_called,share_correct,called,events.refs,worker_code);
        return seen&&(!require_old||old_layout)&&share_called==(share_refuse?2:1)&&share_correct==share_called&&(!share_refuse||kept)&&called==1&&correct==1&&identity==1&&events.refs==1&&SUCCEEDED(hr)&&SUCCEEDED(unadvise)&&wait==WAIT_OBJECT_0&&worker_code==0?0:1;}
    if(overwrite_mode){printf("COM_OVERWRITE_CLEANUP file=%d root=%d\n",file_removed,root_removed);printf("COM_EVENT_OVERWRITE count=%ld correct=%ld ok=%ld kept=%ld refs=%ld worker=%lu\n",overwrite_called,overwrite_correct,called,kept,events.refs,worker_code);
        if(!file_removed || !root_removed)return 1;
        if(overwrite_multi){printf("COM_EVENT_OVERWRITE_MULTI other=%ld ok=%ld refs=%ld,%ld\n",overwrite_other,called,events.refs,other.refs);
            return seen&&(!require_old||old_layout)&&overwrite_called==(overwrite_notimpl?2:overwrite_multi_default?1:2)&&overwrite_correct==overwrite_called&&overwrite_other==(overwrite_notimpl?1:0)&&called==2&&correct==2&&identity==2&&(overwrite_notimpl||(overwrite_multi_default?(prompt_seen&&prompt_order):kept))&&events.refs==1&&other.refs==1&&SUCCEEDED(hr)&&SUCCEEDED(unadvise)&&wait==WAIT_OBJECT_0&&worker_code==0?0:1;}
        if(overwrite_default){printf("COM_EVENT_OVERWRITE_DEFAULT prompt=%ld order=%ld count=%ld ok=%ld\n",prompt_seen,prompt_order,overwrite_called,called);
            return seen&&(!require_old||old_layout)&&prompt_seen&&prompt_order&&overwrite_called==1&&overwrite_correct==1&&called==1&&correct==1&&identity==1&&events.refs==1&&SUCCEEDED(hr)&&SUCCEEDED(unadvise)&&wait==WAIT_OBJECT_0&&worker_code==0?0:1;}
        return file_removed&&root_removed&&seen&&(!require_old||old_layout)&&overwrite_called==2&&overwrite_correct==2&&called==1&&correct==1&&identity==1&&kept&&events.refs==1&&SUCCEEDED(hr)&&SUCCEEDED(unadvise)&&wait==WAIT_OBJECT_0&&worker_code==0?0:1;}
    if(close_mode){printf("COM_EVENT_CLOSE calls=%ld close=%08lx show=%08lx result=%08lx refs=%ld worker=%lu\n",called,(unsigned long)close_result,(unsigned long)hr,(unsigned long)close_item_hr,events.refs,worker_code);
        return seen&&called==1&&SUCCEEDED(close_result)&&events.refs==1&&wait==WAIT_OBJECT_0&&worker_code==0&&
            (close_veto?(hr==E_ABORT&&FAILED(close_item_hr)):(hr==S_OK&&SUCCEEDED(close_item_hr)))?0:1;}
    if(mutation_mode){printf("COM_EVENT_MUTATION first=%ld other=%ld unadvise=%08lx refs=%ld,%ld\n",called,other_called,(unsigned long)other_result,events.refs,other.refs);
        return seen&&called==1&&other_called==0&&SUCCEEDED(other_result)&&events.refs==1&&other.refs==1&&SUCCEEDED(hr)&&wait==WAIT_OBJECT_0&&worker_code==0?0:1;}
    if(multi_veto){printf("COM_EVENT_MULTI_ERROR enabled=%d notimpl=%d\n",multi_error,multi_notimpl);printf("COM_EVENT_MULTI_VETO first=%ld other=%ld kept=%ld refs=%ld,%ld\n",called,other_called,kept,events.refs,other.refs);
        return seen&&(!require_old||old_layout)&&called==(multi_notimpl?1:2)&&other_called==1&&(multi_notimpl||kept)&&events.refs==1&&other.refs==1&&SUCCEEDED(hr)&&wait==WAIT_OBJECT_0&&worker_code==0?0:1;}
    return folder_result_ok&&seen&&(!require_old||old_layout)&&(!select_mode||selected_correct)&&(!filter_mode||filter_changed)&&(removed?(called==0&&correct==0&&identity==0):
        (self_unadvise?(called==1&&correct==1&&identity==1&&ole_window==1&&ole_identity==1):(called==2&&correct==2&&identity==2&&ole_window==2&&ole_identity==2&&kept)))&&SUCCEEDED(hr)&&SUCCEEDED(unadvise)&&events.refs==1&&wait==WAIT_OBJECT_0&&worker_code==0?0:1;
}
