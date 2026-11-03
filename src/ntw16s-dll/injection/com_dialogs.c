/* Per-object adapter for the common file dialog.  The system COM object still
 * validates configuration; simple filesystem requests use the classic dialog. */
#define COBJMACROS
#include "../../lib/win16-dialog/hidden_files.h"
#include "../../lib/win16-dialog/hook_state.h"
#include "../../lib/win16-dialog/classic_events.h"
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
#include "../../lib/win16-dialog/proxy_dispatch.h"
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
typedef struct classic_event_call {file_proxy *proxy;win16_dialog_event_snapshot snapshot;} classic_event_call;
static void classic_event_cleanup(void *data)
{
    classic_event_call *call=data;win16_dialog_event_snapshot_free(&call->snapshot);proxy_release(call->proxy);
}
static NTW16S_UNWIND_FUNCTION HRESULT classic_event_dispatch(file_proxy *p,enum classic_event_kind kind,IShellItem *item)
{
    classic_event_call call={.proxy=p};
    if(!win16_dialog_event_snapshot_init(p->events,&call.snapshot))return E_OUTOFMEMORY;
    proxy_add(p);ntw16s_unwind_scope unwind;HRESULT result=S_OK;
    NTW16S_UNWIND_BEGIN(unwind,classic_event_cleanup,&call);
    result=win16_dialog_event_dispatch(&call.snapshot,p,kind,item);
    NTW16S_UNWIND_END(unwind);return result;
}
static NTW16S_UNWIND_FUNCTION FDE_OVERWRITE_RESPONSE classic_overwrite_dispatch(file_proxy *p,IShellItem *item)
{
    classic_event_call call={.proxy=p};FDE_OVERWRITE_RESPONSE result=FDEOR_DEFAULT;
    if(!win16_dialog_event_snapshot_init(p->events,&call.snapshot))return result;
    proxy_add(p);ntw16s_unwind_scope unwind;
    NTW16S_UNWIND_BEGIN(unwind,classic_event_cleanup,&call);
    result=win16_dialog_event_overwrite(&call.snapshot,p,item);
    NTW16S_UNWIND_END(unwind);return result;
}
static NTW16S_UNWIND_FUNCTION FDE_SHAREVIOLATION_RESPONSE classic_share_dispatch(file_proxy *p,IShellItem *item)
{
    classic_event_call call={.proxy=p};FDE_SHAREVIOLATION_RESPONSE result=FDESVR_DEFAULT;
    if(!win16_dialog_event_snapshot_init(p->events,&call.snapshot))return result;
    proxy_add(p);ntw16s_unwind_scope unwind;
    NTW16S_UNWIND_BEGIN(unwind,classic_event_cleanup,&call);
    result=win16_dialog_event_share(&call.snapshot,p,item);
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
static HRESULT classic_changing_owned(file_proxy *p,IShellItem *item)
{return classic_dispatch_owned(p,CLASSIC_FOLDER_CHANGING,item,NULL);}
static BOOL classic_list_changing(HWND w,file_proxy *p)
{return win16_dialog_list_changing(w,p,classic_changing_owned);}
static BOOL classic_typed_changing(HWND w,file_proxy *p)
{return win16_dialog_typed_changing(w,p,classic_changing_owned);}
static BOOL classic_drive_changing(HWND w,file_proxy *p)
{return win16_dialog_drive_changing(w,p,classic_changing_owned);}
static BOOL classic_overwrite_check(HWND w,file_proxy *p)
{return win16_dialog_overwrite_check(w,p,classic_overwrite_owned);}

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
static void classic_hidden_files(HWND w,file_proxy *p)
{win16_dialog_hidden_files(w,&p->filter,ntw16s_old_dialog_folder);}
static void classic_hidden_schedule(HWND w,file_proxy *p)
{win16_dialog_hidden_schedule(w,p && p->classic_hidden);}

#endif
static void com_hook_attach(void *owner,HWND w,OPENFILENAMEW *ofn)
{
    file_proxy *p=owner;(void)ofn;
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    win16_dialog_hook_attach_base(p,w,TRUE);
#else
    win16_dialog_hook_attach_base(p,w,FALSE);
#endif
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_HIDDEN
    classic_hidden_schedule(w,p);
#endif
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    p->classic_ofn=ofn;HWND drives=GetDlgItem(w,cmb2);p->classic_drive_index=drives?SendMessageW(drives,CB_GETCURSEL,0,0):CB_ERR;
    if(p->events && p->live_folder)classic_initial_events(w,p,L"Win16Style.T20.ComDialog");
#endif
}
static PCWSTR com_hook_label(void *owner,int which)
{return win16_dialog_hook_label(owner,which);}
static void com_hook_detach(void *owner,HWND w)
{
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    win16_dialog_hook_detach(owner,w,TRUE);
#else
    win16_dialog_hook_detach(owner,w,FALSE);
#endif
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
        WCHAR folder[MAX_PATH];OPENFILENAMEW adjusted;
        if(!win16_dialog_classic_folder_adjust((OPENFILENAMEW *)lp,folder,ARRAYSIZE(folder),&adjusted))return 1;
        return classic_event_file_ok(p,&adjusted);
    }
    return classic_hook(w,message,wp,lp);
}
#endif
/* The legacy multi-select contract returns space-separated 8.3 names. The
 * file offset separates the directory from its first name even for one file. */
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
static NTW16S_UNWIND_FUNCTION UINT_PTR classic_event_file_ok(file_proxy *p,OPENFILENAMEW *ofn)
{
    if(!win16_dialog_classic_prepare_result(p,ofn))return 1;
    IShellItem *item=p->result;HRESULT hr;
    win16_dialog_classic_result_scope result={p,item,p->results,TRUE};ntw16s_unwind_scope unwind;
    NTW16S_UNWIND_BEGIN(unwind,win16_dialog_classic_result_cleanup,&result);
    hr=classic_event_dispatch(p,CLASSIC_OK,NULL);
    result.revoke=hr==S_FALSE || FAILED(hr);
    NTW16S_UNWIND_END(unwind);return result.revoke?1:0;
}
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
    const win16_dialog_proxy_dispatch_services services={proxy_show,proxy_advise,proxy_unadvise,proxy_set_folder};
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    return win16_dialog_proxy_dispatch(owner,op,a,b,&services,TRUE);
#else
    return win16_dialog_proxy_dispatch(owner,op,a,b,&services,FALSE);
#endif
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
