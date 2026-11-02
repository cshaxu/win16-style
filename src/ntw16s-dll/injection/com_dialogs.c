/* Per-object adapter for the common file dialog.  The system COM object still
 * validates configuration; simple filesystem requests use the classic dialog. */
#define COBJMACROS
#include "intercept.h"
#include "startup_hooks.h"
#include "unwind.h"
#include "../../lib/win16-dialog/shell_folder.h"
#include "../../lib/win16-dialog/shell_item.h"
#include "../../lib/win16-dialog/file_contract.h"
#include "../../lib/win16-dialog/file_helpers.h"
#include "../../lib/win16-dialog/customize.h"
#include "../../lib/win16-dialog/events.h"
#include "../../lib/win16-dialog/ole_window.h"
#include "../../lib/win16-dialog/v2.h"
#include "../../lib/win16-dialog/primary.h"
#include "../../lib/win16-dialog/com_show.h"
#include "../../lib/win16-dialog/file_live.h"
#include "../../lib/win16-dialog/com_hook.h"
#include "../../lib/win16-dialog/proxy.h"
#include <shlobj.h>
#include <shobjidl.h>
#include <commdlg.h>
#include <dlgs.h>
#include <wchar.h>
#include <string.h>
#include <stddef.h>

typedef win16_dialog_proxy file_proxy;
static HRESULT (WINAPI *native_create)(REFCLSID,LPUNKNOWN,DWORD,REFIID,LPVOID *);
static HRESULT (WINAPI *native_create_ex)(REFCLSID,IUnknown *,DWORD,COSERVERINFO *,DWORD,MULTI_QI *);
static HRESULT (WINAPI *native_class_object)(REFCLSID,DWORD,LPVOID,REFIID,LPVOID *);
static _Thread_local unsigned native_creation_depth;
static HRESULT v2_dispatch(void *,win16_dialog_v2_operation,ULONG_PTR,ULONG_PTR);
static ULONG proxy_add(file_proxy *p){return win16_dialog_proxy_add(p);}
static ULONG proxy_release(file_proxy *p){return win16_dialog_proxy_release(p);}
static HRESULT proxy_query(file_proxy *p,REFIID id,void **out){return win16_dialog_proxy_query(p,id,out);}
static ULONG event_owner_add(void *owner){return proxy_add(owner);}
static ULONG event_owner_release(void *owner){return proxy_release(owner);}
static const win16_dialog_event_services event_services={event_owner_add,event_owner_release};
static HRESULT proxy_advise(file_proxy *p,IFileDialogEvents *target,DWORD *cookie)
{return win16_dialog_events_advise(p->native,&p->events,p,target,cookie,&event_services);}
static HRESULT proxy_unadvise(file_proxy *p,DWORD cookie)
{return win16_dialog_events_unadvise(p->native,&p->events,cookie);}

#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
/* Research build only: copy references before calling application code because
 * a listener may unsubscribe itself or another listener in the callback. */
typedef struct classic_event_snapshot {
    event_adapter **items;size_t count;
} classic_event_snapshot;
static BOOL classic_event_snapshot_init(file_proxy *p,classic_event_snapshot *snapshot)
{
    size_t count=0;for(event_adapter *e=p->events;e;e=e->next)++count;
    snapshot->items=NULL;snapshot->count=0;if(!count)return TRUE;
    if(count>SIZE_MAX/sizeof(*snapshot->items))return FALSE;
    snapshot->items=HeapAlloc(GetProcessHeap(),0,count*sizeof(*snapshot->items));
    if(!snapshot->items)return FALSE;
    snapshot->count=count;
    for(event_adapter *e=p->events;e;e=e->next){win16_dialog_event_add(&e->iface);snapshot->items[--count]=e;}
    return TRUE;
}
static void classic_event_snapshot_free(classic_event_snapshot *snapshot)
{
    for(size_t i=0;i<snapshot->count;++i)win16_dialog_event_release(&snapshot->items[i]->iface);
    HeapFree(GetProcessHeap(),0,snapshot->items);snapshot->items=NULL;snapshot->count=0;
}
typedef struct classic_event_call {file_proxy *proxy;classic_event_snapshot snapshot;} classic_event_call;
static void classic_event_cleanup(void *data)
{
    classic_event_call *call=data;classic_event_snapshot_free(&call->snapshot);proxy_release(call->proxy);
}
enum classic_event_kind { CLASSIC_OK,CLASSIC_FOLDER_CHANGING,CLASSIC_FOLDER_CHANGED,CLASSIC_SELECTION_CHANGED,CLASSIC_TYPE_CHANGED };
static NTW16S_UNWIND_FUNCTION HRESULT classic_event_dispatch(file_proxy *p,enum classic_event_kind kind,IShellItem *item)
{
    classic_event_call call={.proxy=p};
    if(!classic_event_snapshot_init(p,&call.snapshot))return E_OUTOFMEMORY;
    proxy_add(p);ntw16s_unwind_scope unwind;HRESULT result=S_OK;
    NTW16S_UNWIND_BEGIN(unwind,classic_event_cleanup,&call);
    for(size_t i=0;i<call.snapshot.count;++i){event_adapter *e=call.snapshot.items[i];if(e->owner!=p)continue;
        HRESULT hr=kind==CLASSIC_OK?IFileDialogEvents_OnFileOk(e->target,(IFileDialog *)p):
            kind==CLASSIC_FOLDER_CHANGING?IFileDialogEvents_OnFolderChanging(e->target,(IFileDialog *)p,item):
            kind==CLASSIC_FOLDER_CHANGED?IFileDialogEvents_OnFolderChange(e->target,(IFileDialog *)p):
            kind==CLASSIC_SELECTION_CHANGED?IFileDialogEvents_OnSelectionChange(e->target,(IFileDialog *)p):
            IFileDialogEvents_OnTypeChange(e->target,(IFileDialog *)p);
        if(kind==CLASSIC_OK && hr!=S_OK && hr!=E_NOTIMPL){result=FAILED(hr)?hr:S_FALSE;break;}
        if(kind==CLASSIC_FOLDER_CHANGING && hr!=S_OK && hr!=E_NOTIMPL){result=hr;break;}
    }
    NTW16S_UNWIND_END(unwind);return result;
}
static NTW16S_UNWIND_FUNCTION FDE_OVERWRITE_RESPONSE classic_overwrite_dispatch(file_proxy *p,IShellItem *item)
{
    classic_event_call call={.proxy=p};FDE_OVERWRITE_RESPONSE result=FDEOR_DEFAULT;
    if(!classic_event_snapshot_init(p,&call.snapshot))return result;
    proxy_add(p);ntw16s_unwind_scope unwind;
    NTW16S_UNWIND_BEGIN(unwind,classic_event_cleanup,&call);
    for(size_t i=0;i<call.snapshot.count;++i){event_adapter *e=call.snapshot.items[i];
        if(e->owner!=p)continue;
        FDE_OVERWRITE_RESPONSE response=FDEOR_DEFAULT;
        HRESULT hr=IFileDialogEvents_OnOverwrite(e->target,(IFileDialog *)p,item,&response);
        if(SUCCEEDED(hr)){result=response;break;}
    }
    NTW16S_UNWIND_END(unwind);return result;
}
static NTW16S_UNWIND_FUNCTION FDE_SHAREVIOLATION_RESPONSE classic_share_dispatch(file_proxy *p,IShellItem *item)
{
    classic_event_call call={.proxy=p};FDE_SHAREVIOLATION_RESPONSE result=FDESVR_DEFAULT;
    if(!classic_event_snapshot_init(p,&call.snapshot))return result;
    proxy_add(p);ntw16s_unwind_scope unwind;
    NTW16S_UNWIND_BEGIN(unwind,classic_event_cleanup,&call);
    for(size_t i=0;i<call.snapshot.count;++i){event_adapter *e=call.snapshot.items[i];
        if(e->owner!=p)continue;
        FDE_SHAREVIOLATION_RESPONSE response=FDESVR_DEFAULT;
        HRESULT hr=IFileDialogEvents_OnShareViolation(e->target,(IFileDialog *)p,item,&response);
        if(SUCCEEDED(hr)){result=response;break;}
    }
    NTW16S_UNWIND_END(unwind);return result;
}
typedef struct classic_owned_event {IShellItem *item;WCHAR *path;} classic_owned_event;
static void classic_owned_event_cleanup(void *data)
{
    classic_owned_event *owned=data;if(owned->item)IShellItem_Release(owned->item);
    if(owned->path)CoTaskMemFree(owned->path);
}
/* Takes the Shell-item reference. A path, if supplied, stays owned by the
 * caller on normal return and is reclaimed here during exceptional unwind. */
static NTW16S_UNWIND_FUNCTION HRESULT classic_dispatch_owned(file_proxy *p,enum classic_event_kind kind,IShellItem *item,WCHAR *path)
{
    classic_owned_event owned={item,path};ntw16s_unwind_scope unwind;HRESULT hr;
    NTW16S_UNWIND_BEGIN(unwind,classic_owned_event_cleanup,&owned);
    hr=classic_event_dispatch(p,kind,item);owned.path=NULL;
    NTW16S_UNWIND_END(unwind);return hr;
}
static NTW16S_UNWIND_FUNCTION FDE_OVERWRITE_RESPONSE classic_overwrite_owned(file_proxy *p,IShellItem *item)
{
    classic_owned_event owned={item,NULL};ntw16s_unwind_scope unwind;FDE_OVERWRITE_RESPONSE result;
    NTW16S_UNWIND_BEGIN(unwind,classic_owned_event_cleanup,&owned);
    result=classic_overwrite_dispatch(p,item);
    NTW16S_UNWIND_END(unwind);return result;
}
static NTW16S_UNWIND_FUNCTION FDE_SHAREVIOLATION_RESPONSE classic_share_owned(file_proxy *p,IShellItem *item)
{
    classic_owned_event owned={item,NULL};ntw16s_unwind_scope unwind;FDE_SHAREVIOLATION_RESPONSE result;
    NTW16S_UNWIND_BEGIN(unwind,classic_owned_event_cleanup,&owned);
    result=classic_share_dispatch(p,item);
    NTW16S_UNWIND_END(unwind);return result;
}
static UINT_PTR classic_event_file_ok(file_proxy *p,OPENFILENAMEW *ofn);
#endif

typedef struct native_call {IFileDialog *dialog;HWND owner;} native_call;
static HRESULT native_show(void *data)
{native_call *call=data;return IFileDialog_Show(call->dialog,call->owner);}
static HRESULT show_native(file_proxy *p,HWND owner)
{native_call call={p->native,owner};return ntw16s_dialog_style_scope(native_show,&call);}
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
static UINT_PTR classic_folder_observe(HWND,UINT,WPARAM,LPARAM,void *);
#endif
/* Read the template's selected ancestor path rather than the mutable
 * process current directory when reporting the dialog's current folder. */
static void classic_capture_folder(file_proxy *p)
{
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    win16_dialog_proxy_capture_folder(p,TRUE);
#else
    win16_dialog_proxy_capture_folder(p,FALSE);
#endif
}
static const WCHAR folder_message_name[]=L"Win16Style.T20.ComSetFolder";
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
static BOOL classic_list_changing(HWND w,file_proxy *p)
{
    IShellItem *target=win16_dialog_classic_list_target(w,p->live_folder);
    if(!target)return TRUE;
    HRESULT hr=classic_dispatch_owned(p,CLASSIC_FOLDER_CHANGING,target,NULL);
    return hr==S_OK && !p->pending_folder;
}
static BOOL classic_typed_changing(HWND w,file_proxy *p)
{
    HWND edit=GetDlgItem(w,edt1);int length=edit?GetWindowTextLengthW(edit):-1;
    if(length<=0 || length>=32768)return TRUE;
    WCHAR *path=CoTaskMemAlloc(((size_t)length+1)*sizeof(WCHAR));if(!path)return FALSE;
    GetWindowTextW(edit,path,length+1);
    /* A single leading separator uses the dialog folder's drive, not the
     * process's potentially unrelated current drive. UNC stays untouched. */
    if((path[0]==L'\\' || path[0]==L'/') && path[1]!=L'\\' && path[1]!=L'/' && p->live_folder){
        PWSTR base=NULL;if(SUCCEEDED(IShellItem_GetDisplayName(p->live_folder,SIGDN_FILESYSPATH,&base))){
            if(base[0] && base[1]==L':'){WCHAR *rooted=CoTaskMemAlloc(((size_t)length+3)*sizeof(WCHAR));
                if(!rooted){CoTaskMemFree(base);CoTaskMemFree(path);return FALSE;}
                rooted[0]=base[0];rooted[1]=L':';memcpy(rooted+2,path,((size_t)length+1)*sizeof(WCHAR));
                CoTaskMemFree(path);path=rooted;}
            CoTaskMemFree(base);}
    }
    /* The legacy dialog resolves a drive-relative name on its displayed
     * drive against the displayed folder, even if the process current
     * directory on that drive differs. Normalize only that same-drive case;
     * another drive retains its native per-drive current-directory rules. */
    if(length>2 && path[1]==L':' && path[2]!=L'\\' && path[2]!=L'/' && p->live_folder){
        PWSTR base=NULL;
        if(SUCCEEDED(IShellItem_GetDisplayName(p->live_folder,SIGDN_FILESYSPATH,&base))){
            if(base[0] && base[1]==L':' && _wcsnicmp(path,base,1)==0){
                memmove(path,path+2,((size_t)length-1)*sizeof(WCHAR));length-=2;
            }
            CoTaskMemFree(base);
        }
    }
    /* Relative input belongs to the dialog's displayed folder, which can
     * differ from the process-wide current directory. */
    if(path[0]!=L'\\' && path[0]!=L'/' && !(length>=2 && path[1]==L':') && p->live_folder){
        PWSTR base=NULL;
        if(SUCCEEDED(IShellItem_GetDisplayName(p->live_folder,SIGDN_FILESYSPATH,&base))){
            size_t n=wcslen(base),total=n+(n && base[n-1]!=L'\\'?1:0)+(size_t)length+1;
            if(total<=32768){WCHAR *combined=CoTaskMemAlloc(total*sizeof(WCHAR));
                if(!combined){CoTaskMemFree(base);CoTaskMemFree(path);return FALSE;}
                swprintf(combined,total,L"%ls%ls%ls",base,n && base[n-1]!=L'\\'?L"\\":L"",path);
                CoTaskMemFree(path);path=combined;}
            CoTaskMemFree(base);
        }
    }
    WCHAR normalized[32768];DWORD written=GetFullPathNameW(path,ARRAYSIZE(normalized),normalized,NULL);
    if(!written || written>=ARRAYSIZE(normalized)){CoTaskMemFree(path);return TRUE;}
    CoTaskMemFree(path);DWORD attrs=GetFileAttributesW(normalized);
    if(attrs==INVALID_FILE_ATTRIBUTES || !(attrs&FILE_ATTRIBUTE_DIRECTORY))return TRUE;
    IShellItem *target=NULL;HRESULT hr=SHCreateItemFromParsingName(normalized,NULL,&IID_IShellItem,(void **)&target);
    if(FAILED(hr))return TRUE;
    hr=classic_dispatch_owned(p,CLASSIC_FOLDER_CHANGING,target,NULL);
    return hr==S_OK && !p->pending_folder;
}
static BOOL classic_drive_changing(HWND w,file_proxy *p)
{
    HWND combo=GetDlgItem(w,cmb2);LRESULT index=combo?SendMessageW(combo,CB_GETCURSEL,0,0):CB_ERR;
    LRESULT length=index==CB_ERR?CB_ERR:SendMessageW(combo,CB_GETLBTEXTLEN,(WPARAM)index,0);
    if(length<2 || length>=MAX_PATH)return TRUE;
    WCHAR label[MAX_PATH],relative[4],path[32768];
    if(SendMessageW(combo,CB_GETLBTEXT,(WPARAM)index,(LPARAM)label)==CB_ERR || label[1]!=L':' ||
       !((label[0]>=L'A'&&label[0]<=L'Z')||(label[0]>=L'a'&&label[0]<=L'z')))return TRUE;
    relative[0]=label[0];relative[1]=L':';relative[2]=L'.';relative[3]=0;
    DWORD written=GetFullPathNameW(relative,ARRAYSIZE(path),path,NULL);
    if(!written || written>=ARRAYSIZE(path))return TRUE;
    IShellItem *target=NULL;if(FAILED(SHCreateItemFromParsingName(path,NULL,&IID_IShellItem,(void **)&target)))return TRUE;
    HRESULT hr=classic_dispatch_owned(p,CLASSIC_FOLDER_CHANGING,target,NULL);
    if(hr!=S_OK || p->pending_folder){if(p->classic_drive_index!=CB_ERR)SendMessageW(combo,CB_SETCURSEL,(WPARAM)p->classic_drive_index,0);return FALSE;}
    p->classic_drive_index=index;return TRUE;
}
static BOOL classic_overwrite_check(HWND w,file_proxy *p)
{
    OPENFILENAMEW *ofn=p->classic_ofn;if(!ofn)return TRUE;
    if(p->classic_overwrite_suppressed){ofn->Flags|=OFN_OVERWRITEPROMPT;p->classic_overwrite_suppressed=FALSE;}
    if(!p->save || !(ofn->Flags&OFN_OVERWRITEPROMPT))return TRUE;
    WCHAR entered[32768],path[32768];HWND edit=GetDlgItem(w,edt1);
    int length=edit?GetWindowTextW(edit,entered,ARRAYSIZE(entered)):0;
    if(length<=0 || length>=(int)ARRAYSIZE(entered)-1)return TRUE;
    PCWSTR source=entered;WCHAR combined[32768];PWSTR base=NULL;
    if((entered[0]==L'\\' || entered[0]==L'/') && entered[1]!=L'\\' && entered[1]!=L'/' && p->live_folder &&
       SUCCEEDED(IShellItem_GetDisplayName(p->live_folder,SIGDN_FILESYSPATH,&base))){
        if(base[0] && base[1]==L':' && (size_t)length+3<=ARRAYSIZE(combined)){
            combined[0]=base[0];combined[1]=L':';memcpy(combined+2,entered,((size_t)length+1)*sizeof(WCHAR));source=combined;}
        CoTaskMemFree(base);base=NULL;
    }
    PCWSTR relative=entered;
    if(length>2 && entered[1]==L':' && entered[2]!=L'\\' && entered[2]!=L'/' && p->live_folder &&
       SUCCEEDED(IShellItem_GetDisplayName(p->live_folder,SIGDN_FILESYSPATH,&base))){
        if(base[0] && base[1]==L':' && _wcsnicmp(entered,base,1)==0)relative=entered+2;
        CoTaskMemFree(base);base=NULL;
    }
    if(relative[0]!=L'\\' && relative[0]!=L'/' && !(relative[0] && relative[1]==L':') && p->live_folder &&
       SUCCEEDED(IShellItem_GetDisplayName(p->live_folder,SIGDN_FILESYSPATH,&base))){
        size_t n=wcslen(base);int written=swprintf(combined,ARRAYSIZE(combined),L"%ls%ls%ls",base,
            n && base[n-1]!=L'\\'?L"\\":L"",relative);
        CoTaskMemFree(base);
        if(written<0 || (size_t)written>=ARRAYSIZE(combined))return TRUE;
        source=combined;
    }
    DWORD written=GetFullPathNameW(source,ARRAYSIZE(path),path,NULL);
    if(!written || written>=ARRAYSIZE(path))return TRUE;
    if(p->extension && *p->extension){
        WCHAR *name=wcsrchr(path,L'\\');name=name?name+1:path;
        if(*name && !wcschr(name,L'.')){size_t base=wcslen(path),ext=wcslen(p->extension);
            if(base+1+ext>=ARRAYSIZE(path))return TRUE;
            path[base]=L'.';memcpy(path+base+1,p->extension,(ext+1)*sizeof(WCHAR));}
    }
    DWORD attrs=GetFileAttributesW(path);
    if(attrs==INVALID_FILE_ATTRIBUTES || (attrs&FILE_ATTRIBUTE_DIRECTORY))return TRUE;
    IShellItem *item=NULL;if(FAILED(SHCreateItemFromParsingName(path,NULL,&IID_IShellItem,(void **)&item)))return TRUE;
    FDE_OVERWRITE_RESPONSE response=classic_overwrite_owned(p,item);
    if(!IsWindow(w) || p->active_window!=w || p->classic_ofn!=ofn)return FALSE;
    if(response==FDEOR_REFUSE)return FALSE;
    if(response==FDEOR_ACCEPT){ofn->Flags&=~OFN_OVERWRITEPROMPT;p->classic_overwrite_suppressed=TRUE;}
    return TRUE;
}
#endif
static HRESULT proxy_set_folder(file_proxy *p,IShellItem *item)
{return win16_dialog_proxy_set_folder(p,item,folder_message_name);}
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
typedef struct classic_navigation_scope {file_proxy *proxy;WCHAR *folder,*filename;} classic_navigation_scope;
static void classic_navigation_cleanup(void *data)
{
    classic_navigation_scope *scope=data;scope->proxy->classic_navigating=FALSE;
    if(scope->folder)CoTaskMemFree(scope->folder);
    if(scope->filename)CoTaskMemFree(scope->filename);
}
static NTW16S_UNWIND_FUNCTION void classic_navigate_folder(HWND w,file_proxy *p,PCWSTR key)
#else
static void classic_navigate_folder(HWND w,file_proxy *p,PCWSTR key)
#endif
{
    WCHAR *folder=p->pending_folder;p->pending_folder=NULL;if(!folder)return;
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    classic_navigation_scope navigation={p,folder,NULL};ntw16s_unwind_scope unwind;
    NTW16S_UNWIND_BEGIN(unwind,classic_navigation_cleanup,&navigation);
    if(p->events){IShellItem *destination=NULL;HRESULT hr=SHCreateItemFromParsingName(folder,NULL,&IID_IShellItem,(void **)&destination);
        if(SUCCEEDED(hr))hr=classic_dispatch_owned(p,CLASSIC_FOLDER_CHANGING,destination,NULL);
        if(hr!=S_OK || p->pending_folder || !IsWindow(w) || GetPropW(w,key)!=p)goto done;}
#endif
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    win16_dialog_proxy_navigate_contents(w,p,key,folder,&navigation.filename,TRUE);
#else
    WCHAR *filename=NULL;
    win16_dialog_proxy_navigate_contents(w,p,key,folder,&filename,FALSE);
    if(filename)CoTaskMemFree(filename);
#endif
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
done:
    NTW16S_UNWIND_END(unwind);
#else
    CoTaskMemFree(folder);
#endif
}
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
typedef struct classic_initial_scope {
    file_proxy *proxy;IShellItem *item;PWSTR path;
    BOOL initializing;
} classic_initial_scope;
static void classic_initial_cleanup(void *data)
{
    classic_initial_scope *scope=data;
    scope->proxy->classic_initializing=scope->initializing;
    if(scope->path)CoTaskMemFree(scope->path);
    if(scope->item)IShellItem_Release(scope->item);
}
static NTW16S_UNWIND_FUNCTION void classic_initial_events(HWND w,file_proxy *p,PCWSTR key)
{
    classic_initial_scope state={p,p->live_folder,NULL,p->classic_initializing};
    ntw16s_unwind_scope unwind;IShellItem_AddRef(state.item);
    NTW16S_UNWIND_BEGIN(unwind,classic_initial_cleanup,&state);
    (void)IShellItem_GetDisplayName(state.item,SIGDN_FILESYSPATH,&state.path);
    IShellItem_AddRef(state.item);p->classic_initializing=TRUE;
    HRESULT changing=classic_dispatch_owned(p,CLASSIC_FOLDER_CHANGING,state.item,NULL);
    p->classic_initializing=state.initializing;
    if(changing!=S_OK && !p->pending_folder && p->initial_default && IsWindow(w) && GetPropW(w,key)==p){
        PWSTR fallback=NULL;
        if(SUCCEEDED(IShellItem_GetDisplayName(p->initial_default,SIGDN_FILESYSPATH,&fallback)))
            p->pending_folder=fallback;
    }
    if(IsWindow(w) && GetPropW(w,key)==p && p->pending_folder)
        classic_navigate_folder(w,p,key);
    if(IsWindow(w) && GetPropW(w,key)==p && state.path && p->live_folder!=state.item){
        if(p->pending_folder)CoTaskMemFree(p->pending_folder);
        p->pending_folder=state.path;state.path=NULL;
        classic_navigate_folder(w,p,key);
    }else if(IsWindow(w) && GetPropW(w,key)==p && p->events)
        (void)classic_event_dispatch(p,CLASSIC_FOLDER_CHANGED,NULL);
    if(IsWindow(w) && GetPropW(w,key)==p && p->events && p->filter)
        (void)classic_event_dispatch(p,CLASSIC_TYPE_CHANGED,NULL);
    NTW16S_UNWIND_END(unwind);
}
#endif
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_HIDDEN
/* Experimental: refresh only the file list, never the process directory.
 * Folder navigation and full Shell filtering are not yet implemented. */
static void classic_hidden_files(HWND w,file_proxy *p)
{
    HWND files=GetDlgItem(w,lst1),filters=GetDlgItem(w,cmb1);WCHAR folder[MAX_PATH];
    if(!files || !filters || !ntw16s_old_dialog_folder(w,folder,ARRAYSIZE(folder)))return;
    LRESULT chosen=SendMessageW(filters,CB_GETCURSEL,0,0);if(chosen<0)return;
    PCWSTR spec=L"*.*";
    if(p->filter){PCWSTR at=p->filter;
        for(LRESULT i=0;i<chosen && *at;++i){at+=wcslen(at)+1;if(!*at)return;at+=wcslen(at)+1;}
        if(!*at)return;
        at+=wcslen(at)+1;
        if(!*at)return;
        spec=at;}
    while(*spec){PCWSTR end=wcschr(spec,L';');size_t n=end?(size_t)(end-spec):wcslen(spec);
        WCHAR pattern[32768];size_t dir=wcslen(folder);
        if(n && dir+1+n+1<=ARRAYSIZE(pattern)){memcpy(pattern,folder,dir*sizeof(WCHAR));
            if(dir && pattern[dir-1]!=L'\\')pattern[dir++]=L'\\';
            memcpy(pattern+dir,spec,n*sizeof(WCHAR));pattern[dir+n]=0;
            WIN32_FIND_DATAW data;HANDLE search=FindFirstFileW(pattern,&data);
            if(search!=INVALID_HANDLE_VALUE){do{
                if((data.dwFileAttributes&(FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM)) &&
                   !(data.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) &&
                   SendMessageW(files,LB_FINDSTRINGEXACT,(WPARAM)-1,(LPARAM)data.cFileName)==LB_ERR)
                    (void)SendMessageW(files,LB_ADDSTRING,0,(LPARAM)data.cFileName);
            }while(FindNextFileW(search,&data));FindClose(search);}}
        if(!end)break;
        spec=end+1;
    }
}
static void classic_hidden_schedule(HWND w,file_proxy *p)
{
    if(p && p->classic_hidden){UINT refresh=RegisterWindowMessageW(L"Win16Style.T20.HiddenFilesRefresh");
        if(refresh)PostMessageW(w,refresh,0,0);}
}
#endif
static void com_hook_attach(void *owner,HWND w,OPENFILENAMEW *ofn)
{
    file_proxy *p=owner;(void)ofn;p->active_window=w;classic_capture_folder(p);
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_HIDDEN
    classic_hidden_schedule(w,p);
#endif
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    p->classic_ofn=ofn;HWND drives=GetDlgItem(w,cmb2);p->classic_drive_index=drives?SendMessageW(drives,CB_GETCURSEL,0,0):CB_ERR;
    if(p->events && p->live_folder)classic_initial_events(w,p,L"Win16Style.T20.ComDialog");
#endif
}
static PCWSTR com_hook_label(void *owner,int which)
{file_proxy *p=owner;return which==0?p->ok_label:which==1?p->cancel_label:p->file_label;}
static void com_hook_detach(void *owner,HWND w)
{
    file_proxy *p=owner;if(p->active_window==w)p->active_window=NULL;
    if(p->live_folder){IShellItem_Release(p->live_folder);p->live_folder=NULL;}
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    if(p->live_selection){IShellItem_Release(p->live_selection);p->live_selection=NULL;}
    p->classic_ofn=NULL;p->classic_overwrite_suppressed=FALSE;
#endif
    if(p->pending_folder){CoTaskMemFree(p->pending_folder);p->pending_folder=NULL;}
}
static BOOL com_hook_command(void *owner,HWND w,WPARAM wp,UINT_PTR *result)
{
    file_proxy *p=owner;(void)p;(void)w;(void)wp;(void)result;
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    if(LOWORD(wp)==lst2 && HIWORD(wp)==LBN_DBLCLK){if(p && p->events && !classic_list_changing(w,p)){*result=1;return TRUE;}return FALSE;}
    if(LOWORD(wp)==IDOK){if(p && p->events && !p->classic_navigating && (!classic_typed_changing(w,p)||!classic_overwrite_check(w,p))){*result=1;return TRUE;}return FALSE;}
    if(LOWORD(wp)==cmb2 && HIWORD(wp)==CBN_SELCHANGE){if(p && p->events && !classic_drive_changing(w,p)){*result=1;return TRUE;}return FALSE;}
#endif
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_HIDDEN
    if(LOWORD(wp)==cmb1 && HIWORD(wp)==CBN_SELCHANGE)classic_hidden_schedule(w,p);
#endif
    return FALSE;
}
static BOOL com_hook_before(void *owner,HWND w,UINT message,WPARAM wp,UINT_PTR *result)
{
    file_proxy *p=owner;(void)p;(void)w;(void)message;(void)wp;(void)result;
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_HIDDEN
    UINT refresh=RegisterWindowMessageW(L"Win16Style.T20.HiddenFilesRefresh");
    if(refresh && message==refresh){if(p && p->classic_hidden)classic_hidden_files(w,p);*result=1;return TRUE;}
    UINT selection=RegisterWindowMessageW(LBSELCHSTRINGW);
    if(selection && message==selection && (wp==lst2 || wp==cmb2))classic_hidden_schedule(w,p);
#endif
    return FALSE;
}
static void com_hook_selection(void *owner,HWND w,WPARAM wp)
{
    file_proxy *p=owner;(void)w;if(!p)return;
    if(wp==lst2 || wp==cmb2){classic_capture_folder(p);
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
        if(wp==cmb2){HWND drives=GetDlgItem(w,cmb2);p->classic_drive_index=drives?SendMessageW(drives,CB_GETCURSEL,0,0):CB_ERR;}
        if(p->events)(void)classic_event_dispatch(p,CLASSIC_FOLDER_CHANGED,NULL);
#endif
    }
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    else if(wp==lst1){win16_dialog_proxy_capture_selection(w,p);if(p->events)(void)classic_event_dispatch(p,CLASSIC_SELECTION_CHANGED,NULL);}
    else if(wp==cmb1){FILEOPENDIALOGOPTIONS options=0;
        if(p->events && (p->filter || FAILED(IFileDialog_GetOptions(p->native,&options)) || !(options&FOS_PICKFOLDERS)))
            (void)classic_event_dispatch(p,CLASSIC_TYPE_CHANGED,NULL);}
#endif
}
static void com_hook_navigate(void *owner,HWND w,PCWSTR key){classic_navigate_folder(w,owner,key);}
static BOOL com_hook_after(void *owner,HWND w,UINT message,LPARAM lp,UINT_PTR *result)
{
    file_proxy *p=owner;(void)p;(void)w;(void)message;(void)lp;(void)result;
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    UINT fileok=RegisterWindowMessageW(FILEOKSTRINGW);
    if(fileok && message==fileok && p && p->events){*result=classic_event_file_ok(p,(OPENFILENAMEW *)lp);return TRUE;}
    UINT share=RegisterWindowMessageW(SHAREVISTRINGW);
    if(share && message==share){FILEOPENDIALOGOPTIONS options=0;IShellItem *item=NULL;
        if(p && p->events && lp && SUCCEEDED(IFileDialog_GetOptions(p->native,&options)) && (options&FOS_SHAREAWARE) &&
            SUCCEEDED(SHCreateItemFromParsingName((PCWSTR)lp,NULL,&IID_IShellItem,(void **)&item))){
            FDE_SHAREVIOLATION_RESPONSE response=classic_share_owned(p,item);
            if(response==FDESVR_ACCEPT){*result=OFN_SHAREFALLTHROUGH;return TRUE;}
            if(response==FDESVR_REFUSE){*result=OFN_SHARENOWARN;return TRUE;}}}
#endif
    return FALSE;
}
static UINT_PTR CALLBACK classic_hook(HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    const win16_dialog_com_hook_services services={folder_message_name,com_hook_attach,com_hook_label,com_hook_detach,
        com_hook_command,com_hook_before,com_hook_selection,com_hook_navigate,com_hook_after};
    return win16_dialog_com_hook(w,message,wp,lp,&services);
}
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
typedef struct folder_init_scope {OPENFILENAMEW *ofn;LPARAM saved;} folder_init_scope;
static void folder_init_cleanup(void *value)
{folder_init_scope *scope=value;scope->ofn->lCustData=scope->saved;}
static NTW16S_UNWIND_FUNCTION UINT_PTR classic_folder_observe(HWND w,UINT message,WPARAM wp,LPARAM lp,void *context)
{
    file_proxy *p=context;
    if(message==WM_INITDIALOG && lp){
        OPENFILENAMEW *ofn=(OPENFILENAMEW *)lp;folder_init_scope scope={ofn,ofn->lCustData};ntw16s_unwind_scope unwind;
        ofn->lCustData=(LPARAM)p;NTW16S_UNWIND_BEGIN(unwind,folder_init_cleanup,&scope);
        UINT_PTR result=classic_hook(w,message,wp,lp);
        NTW16S_UNWIND_END(unwind);return result;
    }
    if(message>=0xc000 && message<=0xffff && message==RegisterWindowMessageW(FILEOKSTRINGW) && lp){
        OPENFILENAMEW *original=(OPENFILENAMEW *)lp;WCHAR folder[MAX_PATH];
        if(!original->lpstrFile || wcslen(original->lpstrFile)>=ARRAYSIZE(folder))return 1;
        wcscpy(folder,original->lpstrFile);WCHAR *slash=wcsrchr(folder,L'\\');if(!slash)return 1;
        if(slash==folder+2)slash[1]=0;else *slash=0;
        OPENFILENAMEW adjusted=*original;adjusted.lpstrFile=folder;adjusted.nMaxFile=ARRAYSIZE(folder);
        return classic_event_file_ok(p,&adjusted);
    }
    return classic_hook(w,message,wp,lp);
}
#endif
/* The legacy multi-select contract returns space-separated 8.3 names. The
 * file offset separates the directory from its first name even for one file. */
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
static HRESULT proxy_multi_result(file_proxy *p,WCHAR *file,WORD offset,BOOL keep_link);
typedef struct classic_result_scope {file_proxy *proxy;IShellItem *item;IShellItemArray *items;BOOL revoke;} classic_result_scope;
static void classic_result_cleanup(void *data)
{
    classic_result_scope *scope=data;
    if(scope->revoke && scope->item && scope->proxy->result==scope->item){IShellItem_Release(scope->item);
        scope->proxy->result=NULL;scope->proxy->completed=FALSE;}
    if(scope->revoke && scope->items && scope->proxy->results==scope->items){IShellItemArray_Release(scope->items);
        scope->proxy->results=NULL;scope->proxy->completed=FALSE;}
}
static NTW16S_UNWIND_FUNCTION UINT_PTR classic_event_file_ok(file_proxy *p,OPENFILENAMEW *ofn)
{
    if(!ofn || !ofn->lpstrFile)return 1;
    if(p->result){IShellItem_Release(p->result);p->result=NULL;}
    if(p->results){IShellItemArray_Release(p->results);p->results=NULL;}
    p->completed=FALSE;
    IShellItem *item=NULL;HRESULT hr;
    if(ofn->Flags&OFN_ALLOWMULTISELECT){size_t length=wcsnlen(ofn->lpstrFile,ofn->nMaxFile);
        if(length==ofn->nMaxFile)return 1;
        WCHAR *copy=CoTaskMemAlloc((length+1)*sizeof(WCHAR));if(!copy)return 1;
        memcpy(copy,ofn->lpstrFile,(length+1)*sizeof(WCHAR));
        hr=proxy_multi_result(p,copy,ofn->nFileOffset,(ofn->Flags&OFN_NODEREFERENCELINKS)!=0);CoTaskMemFree(copy);
        item=p->result;
    }else hr=p->save?SHCreateItemFromParsingName(ofn->lpstrFile,NULL,&IID_IShellItem,(void **)&item):
        win16_dialog_open_item(ofn->lpstrFile,(ofn->Flags&OFN_NODEREFERENCELINKS)!=0,&item);
    if(p->save && hr==HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))hr=win16_dialog_save_item(ofn->lpstrFile,&item);
    if(FAILED(hr))return 1;
    p->result=item;p->completed=TRUE;
    classic_result_scope result={p,item,p->results,TRUE};ntw16s_unwind_scope unwind;
    NTW16S_UNWIND_BEGIN(unwind,classic_result_cleanup,&result);
    hr=classic_event_dispatch(p,CLASSIC_OK,NULL);
    result.revoke=hr==S_FALSE || FAILED(hr);
    NTW16S_UNWIND_END(unwind);return result.revoke?1:0;
}
#endif
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
static HRESULT proxy_multi_result(file_proxy *p,WCHAR *file,WORD offset,BOOL keep_link)
{return win16_dialog_multi_result(file,offset,keep_link,&p->result,&p->results,&p->completed);}
#endif
static BOOL proxy_has_events(void *owner){return ((file_proxy *)owner)->events!=NULL;}
static HRESULT proxy_show_native(void *owner,HWND window){return show_native(owner,window);}
static HRESULT proxy_show_impl(file_proxy *p,HWND owner)
{
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    p->classic_close_pending=FALSE;
#endif
    win16_dialog_com_show_state state={
        .owner=p,.native=p->native,.save=p->save,.unsupported=&p->unsupported,.completed=&p->completed,
        .result=&p->result,.results=&p->results,.title=&p->title,.filter=&p->filter,.extension=&p->extension,
        .ok_label=&p->ok_label,.cancel_label=&p->cancel_label,.file_label=&p->file_label,
        .filter_index=&p->filter_index,.classic_hook=classic_hook,.session_active=ntw16s_process_session_active,
        .has_events=proxy_has_events,.show_native=proxy_show_native,.scope=ntw16s_dialog_style_scope,
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
        .experimental_events=TRUE,.classic_close_pending=&p->classic_close_pending,.classic_close_result=&p->classic_close_result,
#endif
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_HIDDEN
        .experimental_hidden=TRUE,.classic_hidden=&p->classic_hidden,
#endif
    };
    return win16_dialog_com_show(&state,owner);
}
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
typedef struct classic_show_scope {file_proxy *proxy;BOOL completed;} classic_show_scope;
static void classic_proxy_release(void *data)
{
    classic_show_scope *scope=data;
    if(!scope->completed)scope->proxy->classic_show_aborted=TRUE;
    proxy_release(scope->proxy);
}
static NTW16S_UNWIND_FUNCTION HRESULT proxy_show(file_proxy *p,HWND owner)
{
    if(p->classic_show_aborted)return E_UNEXPECTED;
    proxy_add(p);ntw16s_unwind_scope unwind;HRESULT hr;classic_show_scope scope={p,FALSE};
    NTW16S_UNWIND_BEGIN(unwind,classic_proxy_release,&scope);
    hr=proxy_show_impl(p,owner);scope.completed=TRUE;
    NTW16S_UNWIND_END(unwind);return hr;
}
#else
static HRESULT proxy_show(file_proxy *p,HWND owner){return proxy_show_impl(p,owner);}
#endif
static HRESULT v2_dispatch(void *owner,win16_dialog_v2_operation op,ULONG_PTR a,ULONG_PTR b)
{
    file_proxy *p=owner;
    switch(op){
    case V2_SHOW:return proxy_show(p,(HWND)a);
    case V2_TYPES:return win16_dialog_proxy_types(p,(UINT)a,(const COMDLG_FILTERSPEC *)b);
    case V2_SET_INDEX:return win16_dialog_proxy_filter_index(p,(UINT)a);
    case V2_GET_INDEX:return win16_dialog_proxy_get_filter_index(p,(UINT *)a);
    case V2_ADVISE:return proxy_advise(p,(IFileDialogEvents *)a,(DWORD *)b);
    case V2_UNADVISE:return proxy_unadvise(p,(DWORD)a);
    case V2_SET_FOLDER:return proxy_set_folder(p,(IShellItem *)a);
    case V2_GET_FOLDER:return win16_dialog_proxy_get_folder(p,(IShellItem **)a);
    case V2_GET_SELECTION:
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
        return win16_dialog_proxy_get_selection(p,(IShellItem **)a);
#else
        return IFileDialog2_GetCurrentSelection(p->v2_native,(IShellItem **)a);
#endif
    case V2_SET_NAME:return win16_dialog_proxy_set_filename(p,(LPCWSTR)a);
    case V2_GET_NAME:return win16_dialog_proxy_get_filename(p,(LPWSTR *)a);
    case V2_TITLE:return win16_dialog_proxy_title(p,(LPCWSTR)a);
    case V2_LABEL:return win16_dialog_proxy_label(p,(LPCWSTR)a,(BOOL)b);
    case V2_RESULT:return win16_dialog_proxy_result(p,(IShellItem **)a);
    case V2_EXTENSION:return win16_dialog_proxy_extension(p,(LPCWSTR)a);
    case V2_PRIMARY_GET_SELECTION:
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
        return win16_dialog_proxy_get_selection(p,(IShellItem **)a);
#else
        return p->save?IFileSaveDialog_GetCurrentSelection((IFileSaveDialog *)p->variant,(IShellItem **)a):
            IFileOpenDialog_GetCurrentSelection((IFileOpenDialog *)p->variant,(IShellItem **)a);
#endif
    case V2_PRIMARY_CLOSE:
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
        return win16_dialog_proxy_close(p,(HRESULT)a);
#else
        p->unsupported=TRUE;return p->save?IFileSaveDialog_Close((IFileSaveDialog *)p->variant,(HRESULT)a):
            IFileOpenDialog_Close((IFileOpenDialog *)p->variant,(HRESULT)a);
#endif
    case V2_PRIMARY_GET_RESULTS: {
        IShellItemArray **out=(IShellItemArray **)a;
        if(p->completed && p->results){if(!out)return E_POINTER;*out=p->results;IShellItemArray_AddRef(*out);return S_OK;}
        if(p->completed && p->result)return SHCreateShellItemArrayFromShellItem(p->result,&IID_IShellItemArray,(void **)out);
        return IFileOpenDialog_GetResults((IFileOpenDialog *)p->variant,out);
    }
    case V2_CLOSE:
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
        return win16_dialog_proxy_close(p,(HRESULT)a);
#else
        p->unsupported=TRUE;return IFileDialog2_Close(p->v2_native,(HRESULT)a);
#endif
    default:return E_INVALIDARG;
    }
}

static file_proxy *proxy_new(IUnknown *original,BOOL save)
{
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    return win16_dialog_proxy_new(original,save,v2_dispatch,TRUE);
#else
    return win16_dialog_proxy_new(original,save,v2_dispatch,FALSE);
#endif
}
static void creation_end(void *unused){(void)unused;--native_creation_depth;}
static NTW16S_UNWIND_FUNCTION HRESULT WINAPI bridge_create(REFCLSID clsid,LPUNKNOWN outer,DWORD context,REFIID iid,LPVOID *out)
{
    if(!ntw16s_process_session_active())return native_create(clsid,outer,context,iid,out);
    BOOL nested=native_creation_depth!=0;ntw16s_unwind_scope unwind;HRESULT original_hr;
    ++native_creation_depth;
    NTW16S_UNWIND_BEGIN(unwind,creation_end,NULL); {original_hr=native_create(clsid,outer,context,iid,out);}
    NTW16S_UNWIND_END(unwind);
    if(FAILED(original_hr)||!out||!*out||outer||nested||ntw16s_modern_dialog_active())return original_hr;
    BOOL save=IsEqualCLSID(clsid,&CLSID_FileSaveDialog),open=IsEqualCLSID(clsid,&CLSID_FileOpenDialog);
    if(!(save||open)||!win16_dialog_proxy_iid(iid,save))return original_hr;
    IUnknown *original=(IUnknown *)*out;file_proxy *p=proxy_new(original,save);if(!p)return original_hr;
    void *replacement=NULL;HRESULT hr=proxy_query(p,iid,&replacement);proxy_release(p);
    if(FAILED(hr))return original_hr;
    IUnknown_Release(original);*out=replacement;return original_hr;
}
static NTW16S_UNWIND_FUNCTION HRESULT WINAPI bridge_create_ex(REFCLSID clsid,IUnknown *outer,DWORD context,
    COSERVERINFO *server,DWORD count,MULTI_QI *results)
{
    if(!ntw16s_process_session_active())return native_create_ex(clsid,outer,context,server,count,results);
    BOOL nested=native_creation_depth!=0;ntw16s_unwind_scope unwind;HRESULT original_hr;
    ++native_creation_depth;
    NTW16S_UNWIND_BEGIN(unwind,creation_end,NULL); {original_hr=native_create_ex(clsid,outer,context,server,count,results);}
    NTW16S_UNWIND_END(unwind);
    if(FAILED(original_hr)||outer||server||nested||ntw16s_modern_dialog_active()||!results||!count||count>64)return original_hr;
    BOOL save=IsEqualCLSID(clsid,&CLSID_FileSaveDialog),open=IsEqualCLSID(clsid,&CLSID_FileOpenDialog);
    if(!(save||open))return original_hr;
    IUnknown *original=NULL;
    for(DWORD i=0;i<count;++i){if(SUCCEEDED(results[i].hr) && results[i].pItf){
        if(!win16_dialog_proxy_iid(results[i].pIID,save))return original_hr;
        if(!original)original=results[i].pItf;
    }}
    if(!original)return original_hr;
    file_proxy *p=proxy_new(original,save);if(!p)return original_hr;
    void *replacement[64]={0};BOOL complete=TRUE;
    for(DWORD i=0;i<count;++i)if(SUCCEEDED(results[i].hr) && results[i].pItf){
        if(FAILED(proxy_query(p,results[i].pIID,&replacement[i]))){complete=FALSE;break;}
    }
    if(complete){for(DWORD i=0;i<count;++i)if(replacement[i]){
        IUnknown_Release(results[i].pItf);results[i].pItf=(IUnknown *)replacement[i];
    }}else for(DWORD i=0;i<count;++i)if(replacement[i])IUnknown_Release((IUnknown *)replacement[i]);
    proxy_release(p);return original_hr;
}
/* A caller can activate a dialog through the class factory directly. */
typedef struct factory_proxy {IClassFactory iface;LONG refs;IClassFactory *native;BOOL save;} factory_proxy;
static HRESULT STDMETHODCALLTYPE factory_query(IClassFactory *self,REFIID iid,void **out)
{
    if(!out)return E_POINTER;
    *out=NULL;if(!IsEqualIID(iid,&IID_IUnknown)&&!IsEqualIID(iid,&IID_IClassFactory))
        return IClassFactory_QueryInterface(((factory_proxy *)self)->native,iid,out);
    *out=self;InterlockedIncrement(&((factory_proxy *)self)->refs);return S_OK;
}
static ULONG STDMETHODCALLTYPE factory_add(IClassFactory *self)
{return (ULONG)InterlockedIncrement(&((factory_proxy *)self)->refs);}
static ULONG STDMETHODCALLTYPE factory_release(IClassFactory *self)
{
    factory_proxy *factory=(factory_proxy *)self;LONG count=InterlockedDecrement(&factory->refs);
    if(!count){IClassFactory_Release(factory->native);HeapFree(GetProcessHeap(),0,factory);}
    return (ULONG)count;
}
static NTW16S_UNWIND_FUNCTION HRESULT STDMETHODCALLTYPE factory_create(IClassFactory *self,IUnknown *outer,REFIID iid,void **out)
{
    factory_proxy *factory=(factory_proxy *)self;ntw16s_unwind_scope unwind;HRESULT original_hr;
    if(!ntw16s_process_session_active())return IClassFactory_CreateInstance(factory->native,outer,iid,out);
    ++native_creation_depth;
    NTW16S_UNWIND_BEGIN(unwind,creation_end,NULL); {original_hr=IClassFactory_CreateInstance(factory->native,outer,iid,out);}
    NTW16S_UNWIND_END(unwind);
    if(FAILED(original_hr)||outer||!out||!*out||ntw16s_modern_dialog_active()||!win16_dialog_proxy_iid(iid,factory->save))return original_hr;
    IUnknown *original=(IUnknown *)*out;file_proxy *p=proxy_new(original,factory->save);if(!p)return original_hr;
    void *replacement=NULL;HRESULT hr=proxy_query(p,iid,&replacement);proxy_release(p);
    if(FAILED(hr))return original_hr;
    IUnknown_Release(original);*out=replacement;return original_hr;
}
static HRESULT STDMETHODCALLTYPE factory_lock(IClassFactory *self,BOOL lock)
{return IClassFactory_LockServer(((factory_proxy *)self)->native,lock);}
static IClassFactoryVtbl factory_vtable={factory_query,factory_add,factory_release,factory_create,factory_lock};
static HRESULT WINAPI bridge_class_object(REFCLSID clsid,DWORD context,LPVOID server,REFIID iid,LPVOID *out)
{
    if(!ntw16s_process_session_active())return native_class_object(clsid,context,server,iid,out);
    HRESULT original_hr=native_class_object(clsid,context,server,iid,out);
    if(FAILED(original_hr)||!out||!*out||server||native_creation_depth||ntw16s_modern_dialog_active()||
       !(context&CLSCTX_INPROC_SERVER)||!(IsEqualIID(iid,&IID_IUnknown)||IsEqualIID(iid,&IID_IClassFactory)))return original_hr;
    BOOL save=IsEqualCLSID(clsid,&CLSID_FileSaveDialog),open=IsEqualCLSID(clsid,&CLSID_FileOpenDialog);
    if(!(save||open))return original_hr;
    IUnknown *original=(IUnknown *)*out;IClassFactory *native=NULL;
    if(FAILED(IUnknown_QueryInterface(original,&IID_IClassFactory,(void **)&native)))return original_hr;
    factory_proxy *factory=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*factory));
    if(!factory){IClassFactory_Release(native);return original_hr;}
    factory->iface.lpVtbl=&factory_vtable;factory->refs=1;factory->native=native;factory->save=save;
    IUnknown_Release(original);*out=&factory->iface;return original_hr;
}
LONG ntw16s_attach_com_dialogs_using(DWORD (*attach)(void *,void *))
{
    if(!attach)return ERROR_INVALID_PARAMETER;
    if(!(ntw16s_process_context.flags & NTW16S_POLICY_STYLE))return NO_ERROR;
    native_create=CoCreateInstance;native_create_ex=CoCreateInstanceEx;native_class_object=CoGetClassObject;
    LONG error=(LONG)attach(&native_create,(void *)bridge_create);
    if(!error)error=(LONG)attach(&native_create_ex,(void *)bridge_create_ex);
    if(!error)error=(LONG)attach(&native_class_object,(void *)bridge_class_object);
    return error;
}

LONG ntw16s_attach_com_dialogs(void){return ntw16s_attach_com_dialogs_using(ntw16s_hooks_attach);}
