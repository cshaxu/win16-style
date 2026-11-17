/* Per-object adapter for the common file dialog.  The system COM object still
 * validates configuration; simple filesystem requests use the classic dialog. */
#define COBJMACROS
#include "lib-win32-style/application/dialog/classic_hidden_files.h"
#include "lib-win32-style/application/dialog/classic_dialog_state.h"
#include "lib-win32-style/application/dialog/classic_dialog_events.h"
#include "classic_com_controller.h"
#include "lib-win32/process-hook/unwind.h"
#include "lib-win32-style/application/dialog/classic_shell_item.h"
#include "lib-win32-style/application/dialog/classic_file_contract.h"
#include "lib-win32-style/application/dialog/classic_file_helpers.h"
#include "lib-win32-style/application/dialog/dialog_customize.h"
#include "lib-win32-style/application/dialog/file_events.h"
#include "lib-win32-style/application/dialog/dialog_ole_window.h"
#include "lib-win32-style/application/dialog/dialog_v2.h"
#include "lib-win32-style/application/dialog/dialog_primary.h"
#include "lib-win32-style/application/dialog/classic_com_show.h"
#include "lib-win32-style/application/dialog/classic_file_live.h"
#include "lib-win32-style/application/dialog/classic_com_hook.h"
#include "lib-win32-style/application/dialog/classic_dialog_proxy.h"
#include "lib-win32-style/application/dialog/classic_dialog_proxy_dispatch.h"
#include "lib-win32-style/application/dialog/modern_file_dialog.h"
#include <shlobj.h>
#include <shobjidl.h>
#include <commdlg.h>
#include <dlgs.h>
#include <wchar.h>
#include <string.h>
#include <stddef.h>

static style_classic_com_options configuration;
typedef style_classic_dialog_proxy file_proxy;
static HRESULT (WINAPI *native_create)(REFCLSID,LPUNKNOWN,DWORD,REFIID,LPVOID *);
static HRESULT (WINAPI *native_create_ex)(REFCLSID,IUnknown *,DWORD,COSERVERINFO *,DWORD,MULTI_QI *);
static HRESULT (WINAPI *native_class_object)(REFCLSID,DWORD,LPVOID,REFIID,LPVOID *);
static _Thread_local unsigned native_creation_depth;
static HRESULT v2_dispatch(void *,style_dialog_v2_operation,ULONG_PTR,ULONG_PTR);
static ULONG event_owner_add(void *owner){return style_classic_dialog_proxy_add(owner);}
static ULONG event_owner_release(void *owner){return style_classic_dialog_proxy_release(owner);}
static const style_dialog_event_services event_services={event_owner_add,event_owner_release};
static HRESULT proxy_advise(file_proxy *p,IFileDialogEvents *target,DWORD *cookie)
{return style_dialog_events_advise(p->native,&p->events,p,target,cookie,&event_services);}
static HRESULT proxy_unadvise(file_proxy *p,DWORD cookie)
{return style_dialog_events_unadvise(p->native,&p->events,cookie);}

typedef struct classic_event_call {file_proxy *proxy;style_dialog_event_snapshot snapshot;} classic_event_call;
static void classic_event_cleanup(void *data)
{
    classic_event_call *call=data;style_dialog_event_snapshot_free(&call->snapshot);style_classic_dialog_proxy_release(call->proxy);
}
static PROCESS_HOOK_UNWIND_FUNCTION HRESULT classic_event_dispatch(file_proxy *p,enum style_classic_event_kind kind,IShellItem *item)
{
    classic_event_call call={.proxy=p};
    if(!style_dialog_event_snapshot_init(p->events,&call.snapshot))return E_OUTOFMEMORY;
    style_classic_dialog_proxy_add(p);process_hook_unwind_scope unwind;HRESULT result=S_OK;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,classic_event_cleanup,&call);
    result=style_dialog_event_dispatch(&call.snapshot,p,kind,item);
    PROCESS_HOOK_UNWIND_END(unwind);return result;
}
static PROCESS_HOOK_UNWIND_FUNCTION FDE_OVERWRITE_RESPONSE classic_overwrite_dispatch(file_proxy *p,IShellItem *item)
{
    classic_event_call call={.proxy=p};FDE_OVERWRITE_RESPONSE result=FDEOR_DEFAULT;
    if(!style_dialog_event_snapshot_init(p->events,&call.snapshot))return result;
    style_classic_dialog_proxy_add(p);process_hook_unwind_scope unwind;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,classic_event_cleanup,&call);
    result=style_dialog_event_overwrite(&call.snapshot,p,item);
    PROCESS_HOOK_UNWIND_END(unwind);return result;
}
static PROCESS_HOOK_UNWIND_FUNCTION FDE_SHAREVIOLATION_RESPONSE classic_share_dispatch(file_proxy *p,IShellItem *item)
{
    classic_event_call call={.proxy=p};FDE_SHAREVIOLATION_RESPONSE result=FDESVR_DEFAULT;
    if(!style_dialog_event_snapshot_init(p->events,&call.snapshot))return result;
    style_classic_dialog_proxy_add(p);process_hook_unwind_scope unwind;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,classic_event_cleanup,&call);
    result=style_dialog_event_share(&call.snapshot,p,item);
    PROCESS_HOOK_UNWIND_END(unwind);return result;
}
typedef struct classic_owned_event {IShellItem *item;WCHAR *path;} classic_owned_event;
static void classic_owned_event_cleanup(void *data)
{
    classic_owned_event *owned=data;if(owned->item)IShellItem_Release(owned->item);
    if(owned->path)CoTaskMemFree(owned->path);
}
/* Takes the Shell-item reference. A path, if supplied, stays owned by the
 * caller on normal return and is reclaimed here during exceptional unwind. */
static PROCESS_HOOK_UNWIND_FUNCTION HRESULT classic_dispatch_owned(file_proxy *p,enum style_classic_event_kind kind,IShellItem *item,WCHAR *path)
{
    classic_owned_event owned={item,path};process_hook_unwind_scope unwind;HRESULT hr;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,classic_owned_event_cleanup,&owned);
    hr=classic_event_dispatch(p,kind,item);owned.path=NULL;
    PROCESS_HOOK_UNWIND_END(unwind);return hr;
}
static PROCESS_HOOK_UNWIND_FUNCTION FDE_OVERWRITE_RESPONSE classic_overwrite_owned(file_proxy *p,IShellItem *item)
{
    classic_owned_event owned={item,NULL};process_hook_unwind_scope unwind;FDE_OVERWRITE_RESPONSE result;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,classic_owned_event_cleanup,&owned);
    result=classic_overwrite_dispatch(p,item);
    PROCESS_HOOK_UNWIND_END(unwind);return result;
}
static PROCESS_HOOK_UNWIND_FUNCTION FDE_SHAREVIOLATION_RESPONSE classic_share_owned(file_proxy *p,IShellItem *item)
{
    classic_owned_event owned={item,NULL};process_hook_unwind_scope unwind;FDE_SHAREVIOLATION_RESPONSE result;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,classic_owned_event_cleanup,&owned);
    result=classic_share_dispatch(p,item);
    PROCESS_HOOK_UNWIND_END(unwind);return result;
}
static UINT_PTR classic_event_file_ok(file_proxy *p,OPENFILENAMEW *ofn);

#define folder_message_name configuration.folder_message
static HRESULT classic_changing_owned(file_proxy *p,IShellItem *item)
{return classic_dispatch_owned(p,STYLE_CLASSIC_FOLDER_CHANGING,item,NULL);}
static BOOL classic_list_changing(HWND w,file_proxy *p)
{return style_classic_dialog_list_changing(w,p,classic_changing_owned);}
static BOOL classic_typed_changing(HWND w,file_proxy *p)
{return style_classic_dialog_typed_changing(w,p,classic_changing_owned);}
static BOOL classic_drive_changing(HWND w,file_proxy *p)
{return style_classic_dialog_drive_changing(w,p,classic_changing_owned);}
static BOOL classic_overwrite_check(HWND w,file_proxy *p)
{return style_classic_dialog_overwrite_check(w,p,classic_overwrite_owned);}

static HRESULT proxy_set_folder(file_proxy *p,IShellItem *item)
{return style_classic_dialog_proxy_set_folder(p,item,folder_message_name);}
static PROCESS_HOOK_UNWIND_FUNCTION void classic_navigate_folder(HWND w,file_proxy *p,PCWSTR key)
{
    style_classic_dialog_navigation_scope navigation;
    if(!configuration.experimental_events){
        if(!style_classic_dialog_navigation_begin(p,FALSE,&navigation))return;
        style_classic_dialog_navigation_run(w,key,&navigation,NULL);
        style_classic_dialog_navigation_cleanup(&navigation);return;
    }
    if(!style_classic_dialog_navigation_begin(p,TRUE,&navigation))return;
    process_hook_unwind_scope unwind;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,style_classic_dialog_navigation_cleanup,&navigation);
    style_classic_dialog_navigation_run(w,key,&navigation,classic_dispatch_owned);
    PROCESS_HOOK_UNWIND_END(unwind);
}
static PROCESS_HOOK_UNWIND_FUNCTION void classic_initial_events(HWND w,file_proxy *p,PCWSTR key)
{
    style_classic_dialog_initial_scope state={p,p->live_folder,NULL,p->classic_initializing};
    process_hook_unwind_scope unwind;IShellItem_AddRef(state.item);
    PROCESS_HOOK_UNWIND_BEGIN(unwind,style_classic_dialog_initial_cleanup,&state);
    style_classic_dialog_initial_run(w,key,&state,classic_dispatch_owned,classic_navigate_folder,classic_event_dispatch);
    PROCESS_HOOK_UNWIND_END(unwind);
}
static void classic_hidden_files(HWND w,file_proxy *p)
{style_classic_hidden_files(w,&p->filter,style_classic_file_folder);}
static void classic_hidden_schedule(HWND w,file_proxy *p)
{if(p && p->classic_hidden){UINT refresh=RegisterWindowMessageW(configuration.hidden_message);
    if(refresh)PostMessageW(w,refresh,0,0);}}

static style_classic_dialog_message_services classic_messages;
static UINT_PTR CALLBACK classic_hook(HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    const style_classic_com_hook_services services={&classic_messages,folder_message_name,classic_navigate_folder};
    return style_classic_com_hook(w,message,wp,lp,&services);
}
/* The legacy multi-select contract returns space-separated 8.3 names. The
 * file offset separates the directory from its first name even for one file. */
static PROCESS_HOOK_UNWIND_FUNCTION UINT_PTR classic_event_file_ok(file_proxy *p,OPENFILENAMEW *ofn)
{
    if(!style_classic_dialog_prepare_result(p,ofn))return 1;
    IShellItem *item=p->result;HRESULT hr;
    style_classic_dialog_result_scope result={p,item,p->results,TRUE};process_hook_unwind_scope unwind;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,style_classic_dialog_result_cleanup,&result);
    hr=classic_event_dispatch(p,STYLE_CLASSIC_OK,NULL);
    result.revoke=hr==S_FALSE || FAILED(hr);
    PROCESS_HOOK_UNWIND_END(unwind);return result.revoke?1:0;
}
static HRESULT proxy_show(file_proxy *p,HWND owner)
{
    const style_classic_com_show_services services={
        .classic_hook=classic_hook,.session_active=configuration.session_active,
        .scope=configuration.scope,
        .experimental_events=configuration.experimental_events,
        .experimental_hidden=configuration.experimental_hidden,
    };
    return style_classic_com_show(p,owner,&services);
}
static HRESULT v2_dispatch(void *owner,style_dialog_v2_operation op,ULONG_PTR a,ULONG_PTR b)
{
    const style_classic_dialog_proxy_dispatch_services services={proxy_show,proxy_advise,proxy_unadvise,proxy_set_folder};
    return style_classic_dialog_proxy_dispatch(owner,op,a,b,&services,configuration.experimental_events);
}

static file_proxy *proxy_new(IUnknown *original,BOOL save)
{
    return style_classic_dialog_proxy_new(original,save,v2_dispatch,configuration.experimental_events);
}
static void proxy_replace(REFIID iid,void **out,BOOL save)
{
    IUnknown *original=(IUnknown *)*out;file_proxy *p=proxy_new(original,save);if(!p)return;
    void *replacement=NULL;HRESULT hr=style_classic_dialog_proxy_query(p,iid,&replacement);style_classic_dialog_proxy_release(p);
    if(FAILED(hr))return;
    IUnknown_Release(original);*out=replacement;
}
static void creation_end(void *unused){(void)unused;--native_creation_depth;}
static PROCESS_HOOK_UNWIND_FUNCTION HRESULT WINAPI bridge_create(REFCLSID clsid,LPUNKNOWN outer,DWORD context,REFIID iid,LPVOID *out)
{
    if(!configuration.session_active())return native_create(clsid,outer,context,iid,out);
    BOOL nested=native_creation_depth!=0;process_hook_unwind_scope unwind;HRESULT original_hr;
    ++native_creation_depth;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,creation_end,NULL); {original_hr=native_create(clsid,outer,context,iid,out);}
    PROCESS_HOOK_UNWIND_END(unwind);
    if(FAILED(original_hr)||!out||!*out||outer||nested||style_dialog_modern_active())return original_hr;
    BOOL save=IsEqualCLSID(clsid,&CLSID_FileSaveDialog),open=IsEqualCLSID(clsid,&CLSID_FileOpenDialog);
    if(!(save||open)||!style_dialog_proxy_iid(iid,save))return original_hr;
    proxy_replace(iid,out,save);return original_hr;
}
static PROCESS_HOOK_UNWIND_FUNCTION HRESULT WINAPI bridge_create_ex(REFCLSID clsid,IUnknown *outer,DWORD context,
    COSERVERINFO *server,DWORD count,MULTI_QI *results)
{
    if(!configuration.session_active())return native_create_ex(clsid,outer,context,server,count,results);
    BOOL nested=native_creation_depth!=0;process_hook_unwind_scope unwind;HRESULT original_hr;
    ++native_creation_depth;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,creation_end,NULL); {original_hr=native_create_ex(clsid,outer,context,server,count,results);}
    PROCESS_HOOK_UNWIND_END(unwind);
    if(FAILED(original_hr)||outer||server||nested||style_dialog_modern_active()||!results||!count||count>64)return original_hr;
    BOOL save=IsEqualCLSID(clsid,&CLSID_FileSaveDialog),open=IsEqualCLSID(clsid,&CLSID_FileOpenDialog);
    if(!(save||open))return original_hr;
    IUnknown *original=NULL;
    for(DWORD i=0;i<count;++i){if(SUCCEEDED(results[i].hr) && results[i].pItf){
        if(!style_dialog_proxy_iid(results[i].pIID,save))return original_hr;
        if(!original)original=results[i].pItf;
    }}
    if(!original)return original_hr;
    file_proxy *p=proxy_new(original,save);if(!p)return original_hr;
    void *replacement[64]={0};BOOL complete=TRUE;
    for(DWORD i=0;i<count;++i)if(SUCCEEDED(results[i].hr) && results[i].pItf){
        if(FAILED(style_classic_dialog_proxy_query(p,results[i].pIID,&replacement[i]))){complete=FALSE;break;}
    }
    if(complete){for(DWORD i=0;i<count;++i)if(replacement[i]){
        IUnknown_Release(results[i].pItf);results[i].pItf=(IUnknown *)replacement[i];
    }}else for(DWORD i=0;i<count;++i)if(replacement[i])IUnknown_Release((IUnknown *)replacement[i]);
    style_classic_dialog_proxy_release(p);return original_hr;
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
static PROCESS_HOOK_UNWIND_FUNCTION HRESULT STDMETHODCALLTYPE factory_create(IClassFactory *self,IUnknown *outer,REFIID iid,void **out)
{
    factory_proxy *factory=(factory_proxy *)self;process_hook_unwind_scope unwind;HRESULT original_hr;
    if(!configuration.session_active())return IClassFactory_CreateInstance(factory->native,outer,iid,out);
    ++native_creation_depth;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,creation_end,NULL); {original_hr=IClassFactory_CreateInstance(factory->native,outer,iid,out);}
    PROCESS_HOOK_UNWIND_END(unwind);
    if(FAILED(original_hr)||outer||!out||!*out||style_dialog_modern_active()||!style_dialog_proxy_iid(iid,factory->save))return original_hr;
    proxy_replace(iid,out,factory->save);return original_hr;
}
static HRESULT STDMETHODCALLTYPE factory_lock(IClassFactory *self,BOOL lock)
{return IClassFactory_LockServer(((factory_proxy *)self)->native,lock);}
static IClassFactoryVtbl factory_vtable={factory_query,factory_add,factory_release,factory_create,factory_lock};
static HRESULT WINAPI bridge_class_object(REFCLSID clsid,DWORD context,LPVOID server,REFIID iid,LPVOID *out)
{
    if(!configuration.session_active())return native_class_object(clsid,context,server,iid,out);
    HRESULT original_hr=native_class_object(clsid,context,server,iid,out);
    if(FAILED(original_hr)||!out||!*out||server||native_creation_depth||style_dialog_modern_active()||
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
DWORD style_classic_com_attach(DWORD (*attach)(void *,void *),const style_classic_com_options *options)
{
    if(!attach || !options || !options->session_active || !options->scope ||
       !options->property || !options->folder_message || !options->hidden_message)return ERROR_INVALID_PARAMETER;
    configuration=*options;
    classic_messages=(style_classic_dialog_message_services){
        .property=configuration.property,.hidden_message=configuration.hidden_message,
        .events_enabled=configuration.experimental_events,.hidden_enabled=configuration.experimental_hidden,
        .list_changing=configuration.experimental_events?classic_list_changing:NULL,
        .typed_changing=configuration.experimental_events?classic_typed_changing:NULL,
        .drive_changing=configuration.experimental_events?classic_drive_changing:NULL,
        .overwrite_check=configuration.experimental_events?classic_overwrite_check:NULL,
        .event_dispatch=configuration.experimental_events?classic_event_dispatch:NULL,
        .file_ok=configuration.experimental_events?classic_event_file_ok:NULL,
        .share=configuration.experimental_events?classic_share_owned:NULL,
        .initial=configuration.experimental_events?classic_initial_events:NULL,
        .hidden_schedule=configuration.experimental_hidden?classic_hidden_schedule:NULL,
        .hidden_files=configuration.experimental_hidden?classic_hidden_files:NULL
    };
    native_create=CoCreateInstance;native_create_ex=CoCreateInstanceEx;native_class_object=CoGetClassObject;
    DWORD error=attach(&native_create,(void *)bridge_create);
    if(!error)error=attach(&native_create_ex,(void *)bridge_create_ex);
    if(!error)error=attach(&native_class_object,(void *)bridge_class_object);
    return error;
}
