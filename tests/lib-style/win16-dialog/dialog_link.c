#define COBJMACROS
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <dlgs.h>
#include "lib-style/common/print_ranges.h"
#include "lib-style/win16-dialog/print_classic.h"
#include "lib-style/common/classic_file_contract.h"
#include "lib-style/win16-dialog/file_helpers.h"
#include "lib-style/common/modern_file_dialog.h"
#include "lib-style/common/classic_shell_item.h"
#include "lib-style/common/dialog_v2.h"
#include "lib-style/common/dialog_primary.h"
#include "lib-style/common/classic_com_show.h"
#include "lib-style/win16-dialog/proxy.h"
#include "lib-style/common/classic_browse.h"
#include "lib-style/win16-dialog/hidden_files.h"
#include "lib-style/win16-dialog/hook_state.h"
#include "lib-style/win16-dialog/classic_events.h"
#include "lib-style/common/file_events.h"

static void *browse_argument;static UINT browse_flags;static BOOL browse_validate;
static PIDLIST_ABSOLUTE WINAPI fake_browse_a(LPBROWSEINFOA info)
{browse_argument=info;browse_flags=info?info->ulFlags:0;return (PIDLIST_ABSOLUTE)(UINT_PTR)1;}
static PIDLIST_ABSOLUTE WINAPI fake_browse_w(LPBROWSEINFOW info)
{
    browse_argument=info;browse_flags=info?info->ulFlags:0;
    if(browse_validate && info && info->lpfn){
        PIDLIST_ABSOLUTE id=ILCreateFromPathW(L"C:\\");if(!id)return NULL;
        info->lpfn(NULL,BFFM_SELCHANGED,(LPARAM)id,info->lParam);
        info->lpfn(NULL,BFFM_VALIDATEFAILEDW,0,info->lParam);CoTaskMemFree(id);return NULL;
    }
    return (PIDLIST_ABSOLUTE)(UINT_PTR)1;
}
static WCHAR hidden_test_dir[MAX_PATH];
static BOOL hidden_folder(HWND w,PWSTR out,size_t size)
{(void)w;if(wcslen(hidden_test_dir)+1>size)return FALSE;lstrcpyW(out,hidden_test_dir);return TRUE;}
static BOOL hidden_test_file(PCWSTR name,DWORD attributes)
{
    WCHAR path[MAX_PATH];if(swprintf(path,MAX_PATH,L"%ls\\%ls",hidden_test_dir,name)<0)return FALSE;
    HANDLE file=CreateFileW(path,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
    if(file==INVALID_HANDLE_VALUE)return FALSE;
    CloseHandle(file);return SetFileAttributesW(path,attributes);
}
static BOOL test_hidden_dialog_files(void)
{
    WCHAR base[MAX_PATH],temp[MAX_PATH],path[MAX_PATH];HWND parent=NULL,list=NULL,combo=NULL;BOOL ok=FALSE;
    if(!GetEnvironmentVariableW(L"WIN16STYLE_CAPTURE_DIR",base,MAX_PATH) || !GetTempFileNameW(base,L"hdf",0,temp))return FALSE;
    if(!DeleteFileW(temp) || !CreateDirectoryW(temp,NULL))return FALSE;
    lstrcpyW(hidden_test_dir,temp);
    if(!hidden_test_file(L"hidden.txt",FILE_ATTRIBUTE_HIDDEN) || !hidden_test_file(L"visible.txt",FILE_ATTRIBUTE_NORMAL) ||
       !hidden_test_file(L"hidden.bin",FILE_ATTRIBUTE_HIDDEN))goto done;
    parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,100,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!parent)goto done;
    list=CreateWindowW(L"LISTBOX",L"",WS_CHILD,0,0,40,40,parent,(HMENU)(INT_PTR)lst1,GetModuleHandleW(NULL),NULL);
    combo=CreateWindowW(L"COMBOBOX",L"",WS_CHILD|CBS_DROPDOWNLIST,0,0,40,40,parent,(HMENU)(INT_PTR)cmb1,GetModuleHandleW(NULL),NULL);
    if(!list || !combo || SendMessageW(combo,CB_ADDSTRING,0,(LPARAM)L"Text")!=0 || SendMessageW(combo,CB_SETCURSEL,0,0)==CB_ERR)goto done;
    WCHAR filters[]=L"Text\0*.txt\0\0";WCHAR *filter=filters;
    win16_dialog_hidden_files(parent,&filter,hidden_folder);win16_dialog_hidden_files(parent,&filter,hidden_folder);
    ok=SendMessageW(list,LB_GETCOUNT,0,0)==1 && SendMessageW(list,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)L"hidden.txt")!=LB_ERR && filter==filters;
done:
    if(parent)DestroyWindow(parent);
    const WCHAR *names[]={L"hidden.txt",L"visible.txt",L"hidden.bin"};
    for(unsigned i=0;i<ARRAYSIZE(names);++i){if(swprintf(path,MAX_PATH,L"%ls\\%ls",hidden_test_dir,names[i])>=0){SetFileAttributesW(path,FILE_ATTRIBUTE_NORMAL);DeleteFileW(path);}}
    if(!RemoveDirectoryW(hidden_test_dir))ok=FALSE;
    hidden_test_dir[0]=0;return ok;
}
static UINT_PTR CALLBACK fake_print_hook(HWND w,UINT message,WPARAM wp,LPARAM lp)
{(void)w;(void)message;(void)wp;(void)lp;return 0;}
static void *observed_owner;static style_dialog_v2_operation observed_op;static ULONG_PTR observed_a,observed_b;
static HRESULT fake_v2_call(void *owner,style_dialog_v2_operation op,ULONG_PTR a,ULONG_PTR b)
{observed_owner=owner;observed_op=op;observed_a=a;observed_b=b;return S_FALSE;}
static HRESULT fake_v2_query(void *owner,REFIID iid,void **out)
{(void)iid;*out=owner;return S_OK;}
static ULONG fake_v2_ref(void *owner){return owner?7:0;}
static HRESULT fake_cancel_result;
static HRESULT STDMETHODCALLTYPE fake_cancel_label(IFileDialog2 *dialog,LPCWSTR label)
{(void)dialog;(void)label;return fake_cancel_result;}
static BOOL no_session(void){return FALSE;}
static HRESULT fake_event_result;static unsigned fake_event_calls;static IFileDialog *fake_event_owner;
static HRESULT STDMETHODCALLTYPE fake_event_ok(IFileDialogEvents *self,IFileDialog *dialog)
{(void)self;if(dialog!=fake_event_owner)return E_FAIL;++fake_event_calls;return fake_event_result;}
static HRESULT STDMETHODCALLTYPE fake_event_changing(IFileDialogEvents *self,IFileDialog *dialog,IShellItem *item)
{(void)item;return fake_event_ok(self,dialog);}
static HRESULT STDMETHODCALLTYPE fake_event_overwrite(IFileDialogEvents *self,IFileDialog *dialog,IShellItem *item,FDE_OVERWRITE_RESPONSE *response)
{(void)item;*response=FDEOR_ACCEPT;return fake_event_ok(self,dialog);}
static HRESULT STDMETHODCALLTYPE fake_event_share(IFileDialogEvents *self,IFileDialog *dialog,IShellItem *item,FDE_SHAREVIOLATION_RESPONSE *response)
{(void)item;*response=FDESVR_REFUSE;return fake_event_ok(self,dialog);}
static BOOL test_event_dispatch(void)
{
    IFileDialogEventsVtbl vtable={0};vtable.OnFileOk=fake_event_ok;vtable.OnFolderChanging=fake_event_changing;
    vtable.OnOverwrite=fake_event_overwrite;vtable.OnShareViolation=fake_event_share;
    IFileDialogEvents target={.lpVtbl=&vtable};style_dialog_event_adapter first={.owner=(void *)1,.target=&target},other={.owner=(void *)2,.target=&target};
    style_dialog_event_adapter *entries[]={&other,&first};style_dialog_event_snapshot snapshot={entries,2};
    fake_event_owner=(IFileDialog *)(UINT_PTR)1;fake_event_calls=0;fake_event_result=S_FALSE;
    if(style_dialog_event_dispatch(&snapshot,(void *)1,STYLE_CLASSIC_OK,NULL)!=S_FALSE || fake_event_calls!=1)return FALSE;
    fake_event_result=E_NOTIMPL;
    if(style_dialog_event_dispatch(&snapshot,(void *)1,STYLE_CLASSIC_FOLDER_CHANGING,NULL)!=S_OK || fake_event_calls!=2)return FALSE;
    fake_event_result=E_FAIL;
    if(style_dialog_event_dispatch(&snapshot,(void *)1,STYLE_CLASSIC_FOLDER_CHANGING,NULL)!=E_FAIL || fake_event_calls!=3)return FALSE;
    fake_event_result=S_OK;
    if(style_dialog_event_overwrite(&snapshot,(void *)1,NULL)!=FDEOR_ACCEPT ||
       style_dialog_event_share(&snapshot,(void *)1,NULL)!=FDESVR_REFUSE || fake_event_calls!=5)return FALSE;
    fake_event_result=E_FAIL;
    return style_dialog_event_overwrite(&snapshot,(void *)1,NULL)==FDEOR_DEFAULT &&
        style_dialog_event_share(&snapshot,(void *)1,NULL)==FDESVR_DEFAULT && fake_event_calls==7;
}
static BOOL test_event_snapshot(void)
{
    style_dialog_event_adapter first={0},second={0};style_dialog_event_snapshot snapshot={0};
    first.refs=second.refs=1;first.next=&second;
    if(!style_dialog_event_snapshot_init(&first,&snapshot))return FALSE;
    BOOL ok=snapshot.count==2 && snapshot.items[0]==&second && snapshot.items[1]==&first && first.refs==2 && second.refs==2;
    first.next=NULL;second.owner=NULL;
    style_dialog_event_snapshot_free(&snapshot);
    return ok && first.refs==1 && second.refs==1 && !snapshot.items && !snapshot.count;
}
static BOOL test_classic_list_target(void)
{
    WCHAR dir[MAX_PATH],root[4],wrong[]=L"Z:\\";IShellItem *folder=NULL,*selected=NULL;PWSTR path=NULL;
    HWND parent=NULL,list=NULL;BOOL ok=FALSE;
    if(!GetTempPathW(MAX_PATH,dir) || dir[1]!=L':' || dir[2]!=L'\\')return FALSE;
    root[0]=dir[0];root[1]=L':';root[2]=L'\\';root[3]=0;
    parent=CreateWindowExW(0,L"STATIC",L"",WS_OVERLAPPED,0,0,80,80,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(parent)list=CreateWindowExW(0,L"LISTBOX",L"",WS_CHILD|LBS_HASSTRINGS,0,0,40,40,parent,(HMENU)(INT_PTR)lst2,GetModuleHandleW(NULL),NULL);
    if(!list || FAILED(SHCreateItemFromParsingName(dir,NULL,&IID_IShellItem,(void **)&folder)))goto done;
    selected=win16_dialog_classic_list_target(parent,folder);if(selected)goto done;
    if(SendMessageW(list,LB_ADDSTRING,0,(LPARAM)root)==LB_ERR)goto done;
    SendMessageW(list,LB_SETCURSEL,0,0);selected=win16_dialog_classic_list_target(parent,folder);
    if(!selected || FAILED(IShellItem_GetDisplayName(selected,SIGDN_FILESYSPATH,&path)) || lstrcmpiW(path,root))goto done;
    WCHAR resolved[MAX_PATH];
    if(!ntw16s_old_dialog_folder(parent,resolved,ARRAYSIZE(resolved)) || lstrcmpiW(resolved,root) ||
       ntw16s_old_dialog_folder(parent,resolved,4))goto done;
    if(SendMessageW(list,LB_ADDSTRING,0,(LPARAM)L"..")<0)goto done;
    SendMessageW(list,LB_SETCURSEL,1,0);
    if(ntw16s_old_dialog_folder(parent,resolved,ARRAYSIZE(resolved)))goto done;
    IShellItem_Release(selected);selected=NULL;
    if(root[0]==L'Z' || root[0]==L'z')wrong[0]=L'Y';
    SendMessageW(list,LB_RESETCONTENT,0,0);SendMessageW(list,LB_ADDSTRING,0,(LPARAM)wrong);SendMessageW(list,LB_SETCURSEL,0,0);
    selected=win16_dialog_classic_list_target(parent,folder);ok=selected==NULL;
done:
    if(path)CoTaskMemFree(path);
    if(selected)IShellItem_Release(selected);
    if(folder)IShellItem_Release(folder);
    if(parent)DestroyWindow(parent);
    return ok;
}
static BOOL test_classic_entry_targets(void)
{
    WCHAR dir[MAX_PATH],path[MAX_PATH],drive[3],basename[MAX_PATH];IShellItem *folder=NULL,*item=NULL;PWSTR actual=NULL;
    HWND parent=NULL,edit=NULL,combo=NULL,selected_combo=NULL;LRESULT index=-1;BOOL ok=FALSE;
    if(!GetTempPathW(MAX_PATH,dir) || !GetTempFileNameW(dir,L"w16",0,path))return FALSE;
    parent=CreateWindowExW(0,L"STATIC",L"",WS_OVERLAPPED,0,0,80,80,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(parent)edit=CreateWindowExW(0,L"EDIT",L".",WS_CHILD,0,0,40,20,parent,(HMENU)(INT_PTR)edt1,GetModuleHandleW(NULL),NULL);
    if(parent)combo=CreateWindowExW(0,L"COMBOBOX",L"",WS_CHILD|CBS_DROPDOWNLIST,0,20,40,40,parent,(HMENU)(INT_PTR)cmb2,GetModuleHandleW(NULL),NULL);
    if(!edit || !combo || FAILED(SHCreateItemFromParsingName(dir,NULL,&IID_IShellItem,(void **)&folder)))goto done;
    if(!win16_dialog_classic_typed_target(parent,folder,&item) || !item)goto done;
    int order=1;if(FAILED(IShellItem_Compare(item,folder,SICHINT_CANONICAL,&order)) || order)goto done;
    IShellItem_Release(item);item=NULL;
    WCHAR drive_relative[4]={dir[0],L':',L'.',0};
    PCWSTR folder_inputs[]={dir+2,drive_relative};
    for(unsigned i=0;i<ARRAYSIZE(folder_inputs);++i){
        SetWindowTextW(edit,folder_inputs[i]);
        if(!win16_dialog_classic_typed_target(parent,folder,&item) || !item ||
           FAILED(IShellItem_Compare(item,folder,SICHINT_CANONICAL,&order)) || order)goto done;
        IShellItem_Release(item);item=NULL;
    }
    SetWindowTextW(edit,L"this-folder-does-not-exist-4ff47fc7");
    if(!win16_dialog_classic_typed_target(parent,folder,&item) || item)goto done;
    if(win16_dialog_classic_drive_target(parent,&index,&selected_combo))goto done;
    drive[0]=dir[0];drive[1]=L':';drive[2]=0;
    if(SendMessageW(combo,CB_ADDSTRING,0,(LPARAM)drive)==CB_ERR)goto done;
    SendMessageW(combo,CB_SETCURSEL,0,0);item=win16_dialog_classic_drive_target(parent,&index,&selected_combo);
    if(!item || index!=0 || selected_combo!=combo)goto done;
    IShellItem_Release(item);item=NULL;
    SetWindowTextW(edit,path);item=win16_dialog_classic_overwrite_target(parent,folder,NULL);
    if(!item || FAILED(IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&actual)) || lstrcmpiW(actual,path))goto done;
    CoTaskMemFree(actual);actual=NULL;IShellItem_Release(item);item=NULL;
    PCWSTR name=wcsrchr(path,L'\\');name=name?name+1:path;
    if(wcslen(name)>=ARRAYSIZE(basename))goto done;
    WCHAR relative[MAX_PATH];
    if(swprintf(relative,ARRAYSIZE(relative),L"%lc:%ls",dir[0],name)<0)goto done;
    PCWSTR file_inputs[]={path+2,relative};
    for(unsigned i=0;i<ARRAYSIZE(file_inputs);++i){
        SetWindowTextW(edit,file_inputs[i]);item=win16_dialog_classic_overwrite_target(parent,folder,NULL);
        if(!item || FAILED(IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&actual)) || lstrcmpiW(actual,path))goto done;
        CoTaskMemFree(actual);actual=NULL;IShellItem_Release(item);item=NULL;
    }
    wcscpy(basename,name);WCHAR *dot=wcsrchr(basename,L'.');if(!dot || _wcsicmp(dot,L".tmp"))goto done;
    *dot=0;SetWindowTextW(edit,basename);item=win16_dialog_classic_overwrite_target(parent,folder,L"tmp");
    if(!item || FAILED(IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&actual)) || lstrcmpiW(actual,path))goto done;
    ok=TRUE;
done:
    if(actual)CoTaskMemFree(actual);
    if(item)IShellItem_Release(item);
    if(folder)IShellItem_Release(folder);
    if(parent)DestroyWindow(parent);
    if(!DeleteFileW(path))ok=FALSE;
    return ok;
}
static HRESULT fallback_show(void *owner,HWND window){observed_owner=owner;observed_a=(ULONG_PTR)window;return E_ABORT;}
static BOOL test_capture_selection(void)
{
    WCHAR dir[MAX_PATH],path[MAX_PATH];IShellItem *folder=NULL;PWSTR actual=NULL;BOOL ok=FALSE;
    HWND parent=NULL,list=NULL;win16_dialog_proxy proxy={0};
    if(!GetTempPathW(MAX_PATH,dir) || !GetTempFileNameW(dir,L"w16",0,path))return FALSE;
    parent=CreateWindowExW(0,L"STATIC",L"",WS_OVERLAPPED,0,0,80,80,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(parent)list=CreateWindowExW(0,L"LISTBOX",L"",WS_CHILD,0,0,40,40,parent,(HMENU)(INT_PTR)lst1,GetModuleHandleW(NULL),NULL);
    if(!list || FAILED(SHCreateItemFromParsingName(dir,NULL,&IID_IShellItem,(void **)&folder)))goto done;
    proxy.live_folder=folder;
    const WCHAR *name=wcsrchr(path,L'\\');name=name?name+1:path;
    if(SendMessageW(list,LB_ADDSTRING,0,(LPARAM)name)==LB_ERR)goto done;
    SendMessageW(list,LB_SETCURSEL,0,0);win16_dialog_proxy_capture_selection(parent,&proxy);
    if(!proxy.live_selection || FAILED(IShellItem_GetDisplayName(proxy.live_selection,SIGDN_FILESYSPATH,&actual)) || lstrcmpiW(actual,path))goto done;
    SendMessageW(list,LB_SETCURSEL,(WPARAM)-1,0);win16_dialog_proxy_capture_selection(parent,&proxy);
    ok=proxy.live_selection==NULL;
done:
    if(actual)CoTaskMemFree(actual);
    if(proxy.live_selection)IShellItem_Release(proxy.live_selection);
    if(folder)IShellItem_Release(folder);
    if(parent)DestroyWindow(parent);
    if(!DeleteFileW(path))ok=FALSE;
    return ok;
}
static HRESULT changing_result;static unsigned changing_calls;static BOOL changing_pending;
static HRESULT test_changing(win16_dialog_proxy *p,IShellItem *item)
{++changing_calls;if(changing_pending)p->pending_folder=(WCHAR *)(UINT_PTR)1;IShellItem_Release(item);return changing_result;}
static FDE_OVERWRITE_RESPONSE test_overwrite(win16_dialog_proxy *p,IShellItem *item)
{(void)p;IShellItem_Release(item);return FDEOR_DEFAULT;}
static BOOL test_classic_event_policy(void)
{
    win16_dialog_proxy p={0};OPENFILENAMEW ofn={0};IShellItem *folder=NULL;HWND parent=NULL,list=NULL;BOOL ok=FALSE;
    if(FAILED(SHCreateItemFromParsingName(L"C:\\",NULL,&IID_IShellItem,(void **)&folder)))return FALSE;
    parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,100,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!parent)goto done;
    list=CreateWindowW(L"LISTBOX",L"",WS_CHILD|LBS_HASSTRINGS,0,0,50,50,parent,(HMENU)(INT_PTR)lst2,GetModuleHandleW(NULL),NULL);
    if(!list || SendMessageW(list,LB_ADDSTRING,0,(LPARAM)L"C:\\")!=0 || SendMessageW(list,LB_SETCURSEL,0,0)==LB_ERR)goto done;
    p.live_folder=folder;changing_calls=0;changing_result=S_OK;changing_pending=FALSE;
    if(!win16_dialog_list_changing(parent,&p,test_changing) || changing_calls!=1)goto done;
    changing_result=S_FALSE;if(win16_dialog_list_changing(parent,&p,test_changing) || changing_calls!=2)goto done;
    changing_result=S_OK;changing_pending=TRUE;if(win16_dialog_list_changing(parent,&p,test_changing) || changing_calls!=3)goto done;
    p.pending_folder=NULL;changing_pending=FALSE;
    if(!win16_dialog_typed_changing(parent,&p,test_changing) || !win16_dialog_drive_changing(parent,&p,test_changing))goto done;
    p.classic_ofn=&ofn;p.classic_overwrite_suppressed=TRUE;p.save=FALSE;
    if(!win16_dialog_overwrite_check(parent,&p,test_overwrite) || !(ofn.Flags&OFN_OVERWRITEPROMPT) || p.classic_overwrite_suppressed)goto done;
    ok=TRUE;
done:
    if(parent)DestroyWindow(parent);
    IShellItem_Release(folder);return ok;
}
static BOOL test_hook_state(void)
{
    IShellItem *item=NULL;win16_dialog_proxy p={0};OPENFILENAMEW ofn={0};BOOL ok=FALSE;PWSTR path=NULL;
    if(FAILED(SHCreateItemFromParsingName(L"C:\\",NULL,&IID_IShellItem,(void **)&item)))return FALSE;
    p.active_window=(HWND)(UINT_PTR)1;p.live_folder=item;IShellItem_AddRef(item);
    p.live_selection=item;IShellItem_AddRef(item);p.classic_ofn=&ofn;p.classic_overwrite_suppressed=TRUE;
    p.pending_folder=CoTaskMemAlloc(4*sizeof(WCHAR));if(!p.pending_folder)goto done;
    p.ok_label=L"ok";p.cancel_label=L"cancel";p.file_label=L"file";
    if(lstrcmpW(win16_dialog_hook_label(&p,0),L"ok") || lstrcmpW(win16_dialog_hook_label(&p,1),L"cancel") ||
       lstrcmpW(win16_dialog_hook_label(&p,2),L"file"))goto done;
    win16_dialog_hook_detach(&p,(HWND)(UINT_PTR)2,FALSE);
    if(p.active_window!=(HWND)(UINT_PTR)1 || p.live_folder || p.live_selection!=item || p.classic_ofn!=&ofn ||
       !p.classic_overwrite_suppressed || p.pending_folder)goto done;
    win16_dialog_hook_detach(&p,(HWND)(UINT_PTR)1,TRUE);
    if(p.active_window || p.live_selection || p.classic_ofn || p.classic_overwrite_suppressed ||
       FAILED(IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&path)))goto done;
    ok=TRUE;
done:
    if(path)CoTaskMemFree(path);
    if(p.live_folder)IShellItem_Release(p.live_folder);
    if(p.live_selection)IShellItem_Release(p.live_selection);
    if(p.pending_folder)CoTaskMemFree(p.pending_folder);
    IShellItem_Release(item);return ok;
}
static BOOL test_proxy_lifetime(void)
{
    IUnknown *native=NULL,*identity=NULL;IFileDialog2 *v2=NULL;win16_dialog_proxy *proxy=NULL;BOOL ok=FALSE;
    HRESULT hr=CoCreateInstance(&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IUnknown,(void **)&native);
    if(FAILED(hr))goto done;
    proxy=win16_dialog_proxy_new(native,FALSE,fake_v2_call,FALSE);if(!proxy)goto done;
    hr=win16_dialog_proxy_query(proxy,&IID_IFileDialog2,(void **)&v2);if(FAILED(hr))goto done;
    hr=IFileDialog2_QueryInterface(v2,&IID_IUnknown,(void **)&identity);
    ok=SUCCEEDED(hr) && identity==(IUnknown *)proxy;
done:
    if(identity)IUnknown_Release(identity);
    if(v2)IFileDialog2_Release(v2);
    if(proxy)win16_dialog_proxy_release(proxy);
    if(native)IUnknown_Release(native);
    return ok;
}
static BOOL test_classic_results(void)
{
    WCHAR folder[MAX_PATH],path[MAX_PATH];OPENFILENAMEW input={0},adjusted={0};win16_dialog_proxy proxy={0};
    input.lpstrFile=L"C:\\alpha\\file.txt";
    if(!win16_dialog_classic_folder_adjust(&input,folder,MAX_PATH,&adjusted) || lstrcmpW(folder,L"C:\\alpha") || adjusted.lpstrFile!=folder)return FALSE;
    input.lpstrFile=L"C:\\file.txt";
    if(!win16_dialog_classic_folder_adjust(&input,folder,MAX_PATH,&adjusted) || lstrcmpW(folder,L"C:\\"))return FALSE;
    input.lpstrFile=L"relative";if(win16_dialog_classic_folder_adjust(&input,folder,MAX_PATH,&adjusted))return FALSE;
    if(!GetModuleFileNameW(NULL,path,MAX_PATH))return FALSE;
    input.lpstrFile=path;input.nMaxFile=MAX_PATH;input.Flags=0;
    if(!win16_dialog_classic_prepare_result(&proxy,&input) || !proxy.result || !proxy.completed)return FALSE;
    win16_dialog_classic_result_scope scope={&proxy,proxy.result,proxy.results,TRUE};
    win16_dialog_classic_result_cleanup(&scope);if(proxy.result || proxy.results || proxy.completed)return FALSE;
    if(!win16_dialog_classic_prepare_result(&proxy,&input) || !proxy.result || !proxy.completed)return FALSE;
    scope=(win16_dialog_classic_result_scope){&proxy,proxy.result,proxy.results,FALSE};
    win16_dialog_classic_result_cleanup(&scope);if(!proxy.result || !proxy.completed)return FALSE;
    IShellItem_Release(proxy.result);proxy.result=NULL;proxy.completed=FALSE;
    input.Flags=OFN_ALLOWMULTISELECT;input.nMaxFile=1;
    if(win16_dialog_classic_prepare_result(&proxy,&input) || proxy.result || proxy.results || proxy.completed)return FALSE;
    puts("CLASSIC_RESULT_OK folder=1 commit=1 revoke=1 invalid_multi=1");return TRUE;
}
static int classic_reject_calls,classic_hidden_calls,classic_fileok_calls;
static BOOL classic_reject(HWND w,win16_dialog_proxy *p)
{(void)w;(void)p;++classic_reject_calls;return FALSE;}
static void classic_hidden_test(HWND w,win16_dialog_proxy *p)
{(void)w;(void)p;++classic_hidden_calls;}
static UINT_PTR classic_fileok_test(win16_dialog_proxy *p,OPENFILENAMEW *ofn)
{(void)p;(void)ofn;++classic_fileok_calls;return 7;}
static BOOL test_classic_messages(void)
{
    win16_dialog_proxy p={0};UINT_PTR result=0;
    win16_dialog_classic_message_services v={0};
    if(win16_dialog_classic_command(&p,NULL,MAKEWPARAM(lst2,LBN_DBLCLK),&result,&v))return FALSE;
    v.events_enabled=TRUE;v.list_changing=classic_reject;v.typed_changing=classic_reject;v.drive_changing=classic_reject;
    v.overwrite_check=classic_reject;v.file_ok=classic_fileok_test;p.events=(style_dialog_event_adapter *)(UINT_PTR)1;
    if(!win16_dialog_classic_command(&p,NULL,MAKEWPARAM(lst2,LBN_DBLCLK),&result,&v) || result!=1 || classic_reject_calls!=1)return FALSE;
    if(!win16_dialog_classic_command(&p,NULL,IDOK,&result,&v) || classic_reject_calls!=2)return FALSE;
    p.classic_navigating=TRUE;if(win16_dialog_classic_command(&p,NULL,IDOK,&result,&v))return FALSE;p.classic_navigating=FALSE;
    if(!win16_dialog_classic_command(&p,NULL,MAKEWPARAM(cmb2,CBN_SELCHANGE),&result,&v) || classic_reject_calls!=3)return FALSE;
    UINT fileok=RegisterWindowMessageW(FILEOKSTRINGW);
    if(!fileok || !win16_dialog_classic_after(&p,NULL,fileok,0,&result,&v) || result!=7 || classic_fileok_calls!=1)return FALSE;
    v.hidden_enabled=TRUE;v.hidden_schedule=classic_hidden_test;v.hidden_files=classic_hidden_test;
    if(win16_dialog_classic_command(&p,NULL,MAKEWPARAM(cmb1,CBN_SELCHANGE),&result,&v) || classic_hidden_calls!=1)return FALSE;
    p.classic_hidden=TRUE;UINT refresh=RegisterWindowMessageW(L"Win16Style.T20.HiddenFilesRefresh");
    if(!refresh || !win16_dialog_classic_before(&p,NULL,refresh,0,&result,&v) || result!=1 || classic_hidden_calls!=2)return FALSE;
    puts("CLASSIC_MESSAGE_OK reject=1 navigating=1 fileok=1 hidden=1");return TRUE;
}
static int classic_owned_calls,classic_changed_calls,classic_type_calls,classic_navigate_calls;
static HRESULT classic_owned_test(win16_dialog_proxy *p,enum style_classic_event_kind kind,IShellItem *item,WCHAR *path)
{(void)p;(void)path;if(kind!=STYLE_CLASSIC_FOLDER_CHANGING || !item)return E_FAIL;++classic_owned_calls;IShellItem_Release(item);return S_OK;}
static HRESULT classic_dispatch_test(win16_dialog_proxy *p,enum style_classic_event_kind kind,IShellItem *item)
{(void)p;(void)item;if(kind==STYLE_CLASSIC_FOLDER_CHANGED)++classic_changed_calls;else if(kind==STYLE_CLASSIC_TYPE_CHANGED)++classic_type_calls;return S_OK;}
static void classic_navigate_test(HWND w,win16_dialog_proxy *p,PCWSTR key)
{(void)w;(void)p;(void)key;++classic_navigate_calls;}
static BOOL test_classic_navigation(void)
{
    static const WCHAR key[]=L"w16.test.classic";
    win16_dialog_proxy p={0};win16_dialog_navigation_scope navigation;
    if(win16_dialog_navigation_begin(&p,FALSE,&navigation))return FALSE;
    p.pending_folder=CoTaskMemAlloc(4*sizeof(WCHAR));if(!p.pending_folder)return FALSE;
    lstrcpyW(p.pending_folder,L"C:\\");p.classic_navigating=TRUE;
    if(!win16_dialog_navigation_begin(&p,FALSE,&navigation) || p.pending_folder)return FALSE;
    win16_dialog_navigation_run(NULL,key,&navigation,NULL);win16_dialog_navigation_cleanup(&navigation);
    if(!p.classic_navigating)return FALSE;
    p.classic_navigating=FALSE;
    IShellItem *item=NULL;if(FAILED(SHCreateItemFromParsingName(L"C:\\",NULL,&IID_IShellItem,(void **)&item)))return FALSE;
    HWND window=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,10,10,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!window){IShellItem_Release(item);return FALSE;}
    if(!SetPropW(window,key,&p)){DestroyWindow(window);IShellItem_Release(item);return FALSE;}
    p.live_folder=item;p.events=(style_dialog_event_adapter *)(UINT_PTR)1;p.filter=L"filter";
    win16_dialog_initial_scope initial={&p,item,NULL,FALSE};IShellItem_AddRef(item);
    win16_dialog_initial_run(window,key,&initial,classic_owned_test,classic_navigate_test,classic_dispatch_test);
    BOOL ok=classic_owned_calls==1 && classic_changed_calls==1 && classic_type_calls==1 && !classic_navigate_calls && !p.classic_initializing;
    win16_dialog_initial_cleanup(&initial);RemovePropW(window,key);DestroyWindow(window);IShellItem_Release(item);
    if(!ok)return FALSE;
    puts("CLASSIC_NAVIGATION_OK scope=1 events=1 cleanup=1");return TRUE;
}
int main(void)
{
    if(!test_hidden_dialog_files())return 41;
    PRINTPAGERANGE ranges[2];DWORD count=0;WCHAR formatted[32];
    if(!style_print_ranges_parse(L"1-3,5",1,9,ranges,2,&count) || count!=2)return 1;
    if(!style_print_ranges_format(ranges,count,1,9,formatted,32) || lstrcmpW(formatted,L"1-3,5"))return 2;
    PRINTDLGA print_a={0},copy_a={0};PRINTDLGW print_w={0},copy_w={0};
    if(win16_dialog_print_prepare_a(NULL,&copy_a))return 37;
    print_a.lStructSize=sizeof(print_a);print_a.Flags=PD_PAGENUMS;print_a.lpfnPrintHook=fake_print_hook;
    if(!win16_dialog_print_prepare_a(&print_a,&copy_a) || !(copy_a.Flags&PD_ENABLEPRINTHOOK) || copy_a.lpfnPrintHook==fake_print_hook)return 38;
    copy_a.nFromPage=3;copy_a.Flags|=PD_COLLATE;win16_dialog_print_result_a(&print_a,&copy_a);
    if((print_a.Flags&PD_ENABLEPRINTHOOK) || !(print_a.Flags&PD_COLLATE) || print_a.nFromPage!=3 || print_a.lpfnPrintHook!=fake_print_hook)return 39;
    print_a.Flags=PD_ENABLEPRINTHOOK;if(win16_dialog_print_prepare_a(&print_a,&copy_a))return 40;
    print_w.lStructSize=sizeof(print_w);print_w.Flags=PD_RETURNDC;
    if(!win16_dialog_print_prepare_w(&print_w,&copy_w))return 41;
    win16_dialog_print_result_w(&print_w,&copy_w);if(print_w.Flags&PD_ENABLEPRINTHOOK)return 42;
    if(!style_classic_file_resolve_links(TRUE,OFN_NODEREFERENCELINKS,TRUE,0) ||
       style_classic_file_resolve_links(FALSE,OFN_NODEREFERENCELINKS,TRUE,0) ||
       style_classic_file_resolve_links(TRUE,0,TRUE,0) ||
       style_classic_file_resolve_links(TRUE,OFN_NODEREFERENCELINKS,FALSE,0) ||
       style_classic_file_resolve_links(TRUE,OFN_NODEREFERENCELINKS,TRUE,OFN_NOVALIDATE) ||
       style_classic_file_resolve_links(TRUE,OFN_NODEREFERENCELINKS,TRUE,OFN_NODEREFERENCELINKS))return 105;
    OPENFILENAMEA a={0},al={0};OPENFILENAMEW w={0},wl={0};
    if(style_classic_file_prepare_a(NULL,&al,0,NULL))return 3;
    a.lStructSize=sizeof(a);a.Flags=OFN_EXPLORER|OFN_READONLY;SetLastError(0x1234);
    if(!style_classic_file_prepare_a(&a,&al,0,NULL) || (al.Flags&OFN_EXPLORER) ||
        !(al.Flags&OFN_ENABLEHOOK) || !(al.Flags&OFN_LONGNAMES) || GetLastError()!=0x1234)return 4;
    al.Flags=(al.Flags&~OFN_READONLY)|OFN_EXTENSIONDIFFERENT;al.nFilterIndex=2;al.nFileOffset=5;al.nFileExtension=8;
    style_classic_file_result_a(&a,&al);
    if((a.Flags&OFN_READONLY) || !(a.Flags&OFN_EXTENSIONDIFFERENT) || !(a.Flags&OFN_EXPLORER) ||
        a.nFilterIndex!=2 || a.nFileOffset!=5 || a.nFileExtension!=8)return 5;
    a.Flags=OFN_ALLOWMULTISELECT;if(style_classic_file_prepare_a(&a,&al,0,NULL))return 6;
    a.Flags=0;a.FlagsEx=1;if(style_classic_file_prepare_a(&a,&al,0,NULL))return 7;
    w.lStructSize=OPENFILENAME_SIZE_VERSION_400W;w.Flags=OFN_NODEREFERENCELINKS;
    if(style_classic_file_prepare_w(&w,&wl,0,NULL) ||
       !style_classic_file_prepare_w(&w,&wl,OFN_NODEREFERENCELINKS,NULL) ||
       wl.lStructSize!=sizeof(wl))return 8;
    wl.nFilterIndex=3;wl.nFileOffset=4;wl.nFileExtension=6;wl.Flags|=OFN_READONLY;
    style_classic_file_result_w(&w,&wl);
    if(!(w.Flags&OFN_READONLY)||w.nFilterIndex!=3||w.nFileOffset!=4||w.nFileExtension!=6)return 9;
    w.lStructSize=1;if(style_classic_file_prepare_w(&w,&wl,0,NULL))return 10;
    struct {DWORD fos,ofn;} map[]={
        {FOS_ALLOWMULTISELECT,OFN_ALLOWMULTISELECT},{FOS_OVERWRITEPROMPT,OFN_OVERWRITEPROMPT},
        {FOS_NOCHANGEDIR,OFN_NOCHANGEDIR},{FOS_PATHMUSTEXIST,OFN_PATHMUSTEXIST},
        {FOS_FILEMUSTEXIST,OFN_FILEMUSTEXIST},{FOS_CREATEPROMPT,OFN_CREATEPROMPT},
        {FOS_NOREADONLYRETURN,OFN_NOREADONLYRETURN},{FOS_NOTESTFILECREATE,OFN_NOTESTFILECREATE},
        {FOS_NODEREFERENCELINKS,OFN_NODEREFERENCELINKS}};
    DWORD base=OFN_HIDEREADONLY|OFN_LONGNAMES|OFN_ENABLEHOOK;
    if(style_classic_com_file_flags(0)!=base)return 11;
    for(unsigned i=0;i<sizeof(map)/sizeof(map[0]);++i)
        if(style_classic_com_file_flags(map[i].fos)!=(base|map[i].ofn))return 12;
    if(!style_classic_com_can_bridge(S_OK,FOS_FORCEFILESYSTEM,FALSE,FALSE,FALSE,FALSE,FALSE) ||
       style_classic_com_can_bridge(E_FAIL,0,FALSE,FALSE,FALSE,FALSE,FALSE) ||
       style_classic_com_can_bridge(S_OK,FOS_ALLOWMULTISELECT,FALSE,FALSE,FALSE,FALSE,FALSE) ||
       style_classic_com_can_bridge(S_OK,0,TRUE,FALSE,FALSE,FALSE,FALSE) ||
       style_classic_com_can_bridge(S_OK,0,FALSE,TRUE,FALSE,FALSE,FALSE) ||
       style_classic_com_can_bridge(S_OK,FOS_NODEREFERENCELINKS,FALSE,FALSE,TRUE,FALSE,FALSE) ||
       style_classic_com_can_bridge(S_OK,FOS_FORCESHOWHIDDEN,FALSE,FALSE,FALSE,FALSE,FALSE) ||
       !style_classic_com_can_bridge(S_OK,FOS_FORCESHOWHIDDEN,FALSE,FALSE,FALSE,FALSE,TRUE) ||
       style_classic_com_can_bridge(S_OK,FOS_FORCESHOWHIDDEN|FOS_PICKFOLDERS,FALSE,FALSE,FALSE,FALSE,TRUE) ||
       style_classic_com_can_bridge(S_OK,FOS_SHAREAWARE,FALSE,FALSE,FALSE,TRUE,FALSE) ||
       !style_classic_com_can_bridge(S_OK,FOS_SHAREAWARE,FALSE,TRUE,FALSE,TRUE,FALSE))return 13;
    const COMDLG_FILTERSPEC specs[]={{L"Text",L"*.txt"},{L"All",L"*.*"}};
    const WCHAR expected[]={L'T',L'e',L'x',L't',0,L'*',L'.',L't',L'x',L't',0,L'A',L'l',L'l',0,L'*',L'.',L'*',0,0};
    WCHAR *serialized=NULL;
    if(!style_dialog_format_filters(2,specs,&serialized) || !serialized ||
       memcmp(serialized,expected,sizeof(expected)))return 14;
    CoTaskMemFree(serialized);serialized=NULL;
    const COMDLG_FILTERSPEC bad[]={{NULL,L"*.txt"}};
    if(style_dialog_format_filters(1,bad,&serialized) || serialized)return 15;
    if(!style_dialog_format_filters(0,NULL,&serialized) || !serialized || serialized[0])return 16;
    CoTaskMemFree(serialized);
    WCHAR *copy=(WCHAR *)(UINT_PTR)1;
    if(!style_dialog_copy_text(NULL,&copy) || copy)return 17;
    if(!style_dialog_copy_text(L"A\x4e2d",&copy) || !copy || lstrcmpW(copy,L"A\x4e2d"))return 18;
    CoTaskMemFree(copy);
    WCHAR temp[MAX_PATH],unused[MAX_PATH];IShellItem *item=NULL;PWSTR selected=NULL;
    if(FAILED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)))return 19;
    if(!test_classic_results()){CoUninitialize();return 44;}
    if(!test_classic_messages()){CoUninitialize();return 45;}
    if(!test_classic_navigation()){CoUninitialize();return 46;}
    if(!test_hook_state()){CoUninitialize();return 42;}
    if(!test_classic_event_policy()){CoUninitialize();return 43;}
    if(!test_event_snapshot()){CoUninitialize();return 40;}
    if(!test_event_dispatch()){CoUninitialize();return 41;}
    if(!test_classic_list_target()){CoUninitialize();return 38;}
    if(!test_classic_entry_targets()){CoUninitialize();return 39;}
    if(!test_capture_selection()){CoUninitialize();return 37;}
    if(!GetTempPathW(MAX_PATH,temp) || !GetTempFileNameW(temp,L"w16",0,unused) || !DeleteFileW(unused))return 20;
    HRESULT item_hr=style_classic_save_item(unused,&item);
    if(SUCCEEDED(item_hr))item_hr=IShellItem_GetDisplayName(item,SIGDN_FILESYSPATH,&selected);
    BOOL same=SUCCEEDED(item_hr) && selected && !lstrcmpiW(selected,unused) && GetFileAttributesW(unused)==INVALID_FILE_ATTRIBUTES;
    if(selected)CoTaskMemFree(selected);
    if(item)IShellItem_Release(item);
    if(!same){CoUninitialize();return 21;}
    WCHAR root[MAX_PATH]=L"",first[MAX_PATH]=L"",second[MAX_PATH]=L"",input[MAX_PATH*2];
    BOOL root_file=FALSE,root_dir=FALSE,have_first=FALSE,have_second=FALSE,multi_ok=FALSE,cleaned=TRUE;
    IShellItem *single=NULL;IShellItemArray *multiple=NULL;
    do {
        if(!GetTempFileNameW(temp,L"w16",0,root))break;
        root_file=TRUE;
        if(!DeleteFileW(root))break;
        root_file=FALSE;
        if(!CreateDirectoryW(root,NULL))break;
        root_dir=TRUE;
        int n1=swprintf(first,MAX_PATH,L"%ls\\one.txt",root),n2=swprintf(second,MAX_PATH,L"%ls\\two.txt",root);
        int n3=swprintf(input,MAX_PATH*2,L"%ls one.txt two.txt",root);
        if(n1<0||n1>=MAX_PATH||n2<0||n2>=MAX_PATH||n3<0||n3>=MAX_PATH*2)break;
        HANDLE f=CreateFileW(first,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
        if(f==INVALID_HANDLE_VALUE)break;
        have_first=TRUE;CloseHandle(f);
        f=CreateFileW(second,GENERIC_WRITE,0,NULL,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,NULL);
        if(f==INVALID_HANDLE_VALUE)break;
        have_second=TRUE;CloseHandle(f);
        BOOL completed=FALSE;DWORD count_items=0;
        HRESULT multi=style_classic_multi_result(input,(WORD)(wcslen(root)+1),FALSE,&single,&multiple,&completed);
        if(SUCCEEDED(multi) && multiple)multi=IShellItemArray_GetCount(multiple,&count_items);
        multi_ok=SUCCEEDED(multi) && completed && count_items==2;
    } while(0);
    if(multiple)IShellItemArray_Release(multiple);
    if(single)IShellItem_Release(single);
    if(have_first && !DeleteFileW(first))cleaned=FALSE;
    if(have_second && !DeleteFileW(second))cleaned=FALSE;
    if(root_dir && !RemoveDirectoryW(root))cleaned=FALSE;
    if(root_file && !DeleteFileW(root))cleaned=FALSE;
    BOOL proxy_ok=test_proxy_lifetime();
    CoUninitialize();
    if(!multi_ok || !cleaned || !proxy_ok)return 32;
    int owner=0;BOOL unsupported=FALSE;WCHAR *cancel_label=NULL;style_dialog_v2 adapter;
    const style_dialog_identity_services identity={fake_v2_query,fake_v2_ref,fake_v2_ref};
    style_dialog_v2_init(&adapter,&owner,NULL,&unsupported,&cancel_label,&identity,fake_v2_call);
    if(IFileDialog2_SetFileTypes(&adapter.iface,2,specs)!=S_FALSE || observed_owner!=&owner ||
       observed_op!=STYLE_DIALOG_V2_TYPES || observed_a!=2 || observed_b!=(ULONG_PTR)specs)return 22;
    if(IFileDialog2_SetOkButtonLabel(&adapter.iface,L"OK")!=S_FALSE || observed_op!=STYLE_DIALOG_V2_LABEL || observed_b!=TRUE)return 23;
    if(IFileDialog2_Close(&adapter.iface,E_FAIL)!=S_FALSE || observed_op!=STYLE_DIALOG_V2_CLOSE || (HRESULT)observed_a!=E_FAIL)return 24;
    if(IFileDialog2_AddRef(&adapter.iface)!=7 || IFileDialog2_Release(&adapter.iface)!=7)return 25;
    IFileDialog2Vtbl native_vtable={.SetCancelButtonLabel=fake_cancel_label};
    IFileDialog2 native_dialog={.lpVtbl=&native_vtable};adapter.native=&native_dialog;
    fake_cancel_result=S_OK;
    if(IFileDialog2_SetCancelButtonLabel(&adapter.iface,L"First")!=S_OK || !cancel_label || lstrcmpW(cancel_label,L"First"))return 45;
    fake_cancel_result=E_FAIL;
    if(IFileDialog2_SetCancelButtonLabel(&adapter.iface,L"Ignored")!=E_FAIL || lstrcmpW(cancel_label,L"First"))return 46;
    fake_cancel_result=S_OK;
    if(IFileDialog2_SetCancelButtonLabel(&adapter.iface,L"Second")!=S_OK || !cancel_label || lstrcmpW(cancel_label,L"Second"))return 47;
    if(IFileDialog2_SetCancelButtonLabel(&adapter.iface,NULL)!=S_OK || cancel_label || unsupported)return 48;
    if(!style_dialog_proxy_iid(&IID_IFileOpenDialog,FALSE) ||
       style_dialog_proxy_iid(&IID_IFileOpenDialog,TRUE) ||
       !style_dialog_proxy_iid(&IID_IFileSaveDialog,TRUE) ||
       style_dialog_proxy_iid(&IID_IFileSaveDialog,FALSE) ||
       style_dialog_proxy_iid(&IID_IStream,FALSE) || style_dialog_proxy_iid(NULL,FALSE))return 36;
    style_dialog_primary primary;IShellItemArray *items=NULL;
    style_dialog_primary_init(&primary,&owner,NULL,&unsupported,FALSE,&identity,fake_v2_call);
    IFileOpenDialog *open=(IFileOpenDialog *)&primary;
    if(IFileOpenDialog_AddRef(open)!=7 || IFileOpenDialog_GetResults(open,&items)!=S_FALSE ||
       observed_owner!=&owner || observed_op!=STYLE_DIALOG_V2_PRIMARY_GET_RESULTS || observed_a!=(ULONG_PTR)&items)return 26;
    style_dialog_primary_init(&primary,&owner,NULL,&unsupported,TRUE,&identity,fake_v2_call);
    IFileSaveDialog *save=(IFileSaveDialog *)&primary;
    if(IFileSaveDialog_Close(save,E_FAIL)!=S_FALSE || observed_op!=STYLE_DIALOG_V2_PRIMARY_CLOSE || (HRESULT)observed_a!=E_FAIL)return 27;
    IShellItem *show_result=NULL;IShellItemArray *show_results=NULL;BOOL show_completed=TRUE;
    style_classic_com_show_state show_state={.owner=&owner,.result=&show_result,.results=&show_results,
        .completed=&show_completed,.session_active=no_session,.show_native=fallback_show};
    if(style_classic_com_show(&show_state,(HWND)(UINT_PTR)123)!=E_ABORT || show_completed ||
       observed_owner!=&owner || observed_a!=123)return 28;
    UINT browse_base=BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE|BIF_EDITBOX|BIF_VALIDATE;
    struct {UINT input;BOOL callback;style_classic_browse_mode mode;UINT output;} plans[]={
        {browse_base,TRUE,STYLE_CLASSIC_BROWSE_NATIVE,browse_base},
        {browse_base,FALSE,STYLE_CLASSIC_BROWSE_REWRITE,BIF_RETURNONLYFSDIRS},
        {browse_base|BIF_NONEWFOLDERBUTTON,FALSE,STYLE_CLASSIC_BROWSE_VALIDATE,BIF_RETURNONLYFSDIRS|BIF_EDITBOX|BIF_VALIDATE},
        {BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE,FALSE,STYLE_CLASSIC_BROWSE_REWRITE,BIF_RETURNONLYFSDIRS},
        {BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE|BIF_NONEWFOLDERBUTTON,FALSE,STYLE_CLASSIC_BROWSE_REWRITE,BIF_RETURNONLYFSDIRS},
        {browse_base|BIF_BROWSEFORCOMPUTER,FALSE,STYLE_CLASSIC_BROWSE_NATIVE,browse_base|BIF_BROWSEFORCOMPUTER},
        {browse_base&~BIF_VALIDATE,FALSE,STYLE_CLASSIC_BROWSE_REWRITE,BIF_RETURNONLYFSDIRS},
        {browse_base&~BIF_VALIDATE,TRUE,STYLE_CLASSIC_BROWSE_NATIVE,browse_base&~BIF_VALIDATE}};
    for(unsigned i=0;i<sizeof(plans)/sizeof(plans[0]);++i){
        style_classic_browse_plan plan=style_classic_plan_browse(plans[i].input,plans[i].callback);
        if(plan.mode!=plans[i].mode || plan.flags!=plans[i].output)return 33;
    }
    style_classic_folder_selection selection={0};
    style_classic_folder_selection_callback(NULL,BFFM_VALIDATEFAILEDW,0,(LPARAM)&selection);
    if(!selection.invalid || selection.selected)return 34;
    BROWSEINFOA browse_a={0};browse_a.ulFlags=browse_base;
    if(style_classic_browse_a(&browse_a,fake_browse_a,FALSE)!=(PIDLIST_ABSOLUTE)(UINT_PTR)1 || browse_argument!=&browse_a || browse_flags!=browse_base)return 36;
    if(style_classic_browse_a(&browse_a,fake_browse_a,TRUE)!=(PIDLIST_ABSOLUTE)(UINT_PTR)1 || browse_argument==&browse_a || browse_flags!=BIF_RETURNONLYFSDIRS || browse_a.ulFlags!=browse_base)return 37;
    BROWSEINFOW browse_w={0};browse_w.ulFlags=browse_base|BIF_NONEWFOLDERBUTTON;browse_validate=TRUE;
    PIDLIST_ABSOLUTE retained=style_classic_browse_w(&browse_w,fake_browse_w,TRUE);browse_validate=FALSE;
    if(!retained || browse_argument==&browse_w || browse_flags!=(BIF_RETURNONLYFSDIRS|BIF_EDITBOX|BIF_VALIDATE) || browse_w.ulFlags!=(browse_base|BIF_NONEWFOLDERBUTTON))return 38;
    CoTaskMemFree(retained);
    browse_w.ulFlags=browse_base|BIF_BROWSEFORCOMPUTER;
    if(style_classic_browse_w(&browse_w,fake_browse_w,TRUE)!=(PIDLIST_ABSOLUTE)(UINT_PTR)1 || browse_argument!=&browse_w)return 39;
    if(style_classic_browse_w(NULL,fake_browse_w,TRUE)!=(PIDLIST_ABSOLUTE)(UINT_PTR)1 || browse_argument)return 40;
    if(style_dialog_show_modern(NULL,1,NULL,0,NULL)!=E_INVALIDARG)return 35;
    puts("WIN16_DIALOG_STANDALONE_LINK_OK ranges=1 file_contract=1 modern_invalid=1");return 0;
}
