#define COBJMACROS
#include "lib-win32-style/application/dialog/classic_com_controller.h"
#include "lib-win32-style/application/dialog/classic_dialog_proxy.h"
#include <stdio.h>

typedef HRESULT (WINAPI *create_fn)(REFCLSID,IUnknown *,DWORD,REFIID,void **);
typedef HRESULT (WINAPI *create_ex_fn)(REFCLSID,IUnknown *,DWORD,COSERVERINFO *,DWORD,MULTI_QI *);
typedef HRESULT (WINAPI *class_fn)(REFCLSID,DWORD,void *,REFIID,void **);
static create_fn create;
static create_ex_fn create_ex;
static class_fn class_object;
static unsigned attaches,fail_at,scopes;
static BOOL active=TRUE;
static BOOL explorer;
static BOOL session(void){return active;}
static BOOL explorer_session(void){return explorer;}
/* Abort before invoking a native or classic UI: this exercises the actual
 * public proxy/controller and its scope while never displaying a dialog. */
static HRESULT scope(HRESULT (*callback)(void *),void *context)
{(void)callback;(void)context;++scopes;return E_ABORT;}
static HRESULT explorer_scope(HRESULT (*callback)(void *),void *context)
{(void)callback;(void)context;return E_ACCESSDENIED;}
static DWORD capture(void *target,void *replacement)
{
    (void)target;++attaches;
    if(attaches==fail_at)return ERROR_ACCESS_DENIED;
    if(attaches==1)create=(create_fn)replacement;
    else if(attaches==2)create_ex=(create_ex_fn)replacement;
    else if(attaches==3)class_object=(class_fn)replacement;
    else return ERROR_INVALID_PARAMETER;
    return ERROR_SUCCESS;
}
static HRESULT unused_dispatch(void *owner,style_dialog_v2_operation op,ULONG_PTR a,ULONG_PTR b)
{(void)owner;(void)op;(void)a;(void)b;return E_NOTIMPL;}
static BOOL proxy_identity(IUnknown *object,BOOL save,BOOL expected)
{
    style_classic_dialog_proxy *probe=style_classic_dialog_proxy_new(object,save,unused_dispatch,FALSE);
    if(!probe)return FALSE;
    IFileDialog *dialog=NULL;
    HRESULT hr=IUnknown_QueryInterface(object,&IID_IFileDialog,(void **)&dialog);
    BOOL is_proxy=SUCCEEDED(hr) && (const void *)dialog->lpVtbl==(const void *)((IUnknown *)probe)->lpVtbl;
    if(dialog)IFileDialog_Release(dialog);
    style_classic_dialog_proxy_release(probe);return is_proxy==expected;
}
static LONG event_refs=1;
static HRESULT STDMETHODCALLTYPE event_query(IFileDialogEvents *self,REFIID iid,void **out)
{
    if(!out)return E_POINTER;
    *out=NULL;
    if(!IsEqualIID(iid,&IID_IUnknown) && !IsEqualIID(iid,&IID_IFileDialogEvents))return E_NOINTERFACE;
    *out=self;InterlockedIncrement(&event_refs);return S_OK;
}
static ULONG STDMETHODCALLTYPE event_add(IFileDialogEvents *self)
{(void)self;return (ULONG)InterlockedIncrement(&event_refs);}
static ULONG STDMETHODCALLTYPE event_release(IFileDialogEvents *self)
{(void)self;return (ULONG)InterlockedDecrement(&event_refs);}
static HRESULT STDMETHODCALLTYPE event_simple(IFileDialogEvents *self,IFileDialog *dialog)
{(void)self;(void)dialog;return S_OK;}
static HRESULT STDMETHODCALLTYPE event_folder(IFileDialogEvents *self,IFileDialog *dialog,IShellItem *item)
{(void)item;return event_simple(self,dialog);}
static HRESULT STDMETHODCALLTYPE event_share(IFileDialogEvents *self,IFileDialog *dialog,IShellItem *item,FDE_SHAREVIOLATION_RESPONSE *response)
{(void)item;*response=FDESVR_DEFAULT;return event_simple(self,dialog);}
static HRESULT STDMETHODCALLTYPE event_overwrite(IFileDialogEvents *self,IFileDialog *dialog,IShellItem *item,FDE_OVERWRITE_RESPONSE *response)
{(void)item;*response=FDEOR_DEFAULT;return event_simple(self,dialog);}
static IFileDialogEventsVtbl event_table={event_query,event_add,event_release,event_simple,event_folder,event_simple,event_simple,event_share,event_simple,event_overwrite};
#define CHECK(x) do{if(!(x)){fprintf(stderr,"COM_CONTROLLER_FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static int exercise(BOOL save)
{
    const CLSID *cls=save?&CLSID_FileSaveDialog:&CLSID_FileOpenDialog;
    IFileDialog *dialog=NULL;
    CHECK(SUCCEEDED(create(cls,NULL,CLSCTX_INPROC_SERVER,&IID_IFileDialog,(void **)&dialog)) && dialog);
    CHECK(proxy_identity((IUnknown *)dialog,save,TRUE));
    unsigned before=scopes;CHECK(IFileDialog_Show(dialog,NULL)==E_ABORT && scopes==before+1);
    active=FALSE;CHECK(IFileDialog_Show(dialog,NULL)==E_ABORT && scopes==before+2);active=TRUE;
    IFileDialogEvents listener={.lpVtbl=&event_table};DWORD cookie=0;
    CHECK(SUCCEEDED(IFileDialog_Advise(dialog,&listener,&cookie)) && event_refs>1);
    CHECK(SUCCEEDED(IFileDialog_Unadvise(dialog,cookie)) && event_refs==1);
    CHECK(IFileDialog_Release(dialog)==0);
    static const IID missing={0x2468ace0,0x4321,0x8765,{0xa1,0xb2,0xc3,0xd4,0xe5,0xf6,0x17,0x28}};
    MULTI_QI requests[]={{&IID_IUnknown,NULL,E_FAIL},{&IID_IFileDialog,NULL,E_FAIL},{&IID_IFileDialog2,NULL,E_FAIL},{&missing,NULL,E_FAIL}};
    CHECK(SUCCEEDED(create_ex(cls,NULL,CLSCTX_INPROC_SERVER,NULL,4,requests)));
    CHECK(SUCCEEDED(requests[0].hr) && SUCCEEDED(requests[1].hr) && SUCCEEDED(requests[2].hr) && requests[3].hr==E_NOINTERFACE && !requests[3].pItf);
    IUnknown *identity[3]={0};
    for(unsigned i=0;i<3;++i)CHECK(SUCCEEDED(IUnknown_QueryInterface(requests[i].pItf,&IID_IUnknown,(void **)&identity[i])));
    CHECK(identity[0]==identity[1] && identity[0]==identity[2] && proxy_identity(identity[0],save,TRUE));
    for(unsigned i=0;i<3;++i)IUnknown_Release(identity[i]);
    IUnknown_Release(requests[2].pItf);IUnknown_Release(requests[1].pItf);CHECK(IUnknown_Release(requests[0].pItf)==0);
    IClassFactory *factory=NULL;IUnknown *factory_id=NULL;
    CHECK(SUCCEEDED(class_object(cls,CLSCTX_INPROC_SERVER,NULL,&IID_IClassFactory,(void **)&factory)) && factory);
    CHECK(SUCCEEDED(IClassFactory_QueryInterface(factory,&IID_IUnknown,(void **)&factory_id)) && (void *)factory_id==(void *)factory);
    IUnknown_Release(factory_id);
    dialog=NULL;CHECK(SUCCEEDED(IClassFactory_CreateInstance(factory,NULL,&IID_IFileDialog,(void **)&dialog)) && proxy_identity((IUnknown *)dialog,save,TRUE));
    CHECK(IFileDialog_Release(dialog)==0 && IClassFactory_Release(factory)==0);
    active=FALSE;dialog=NULL;
    CHECK(SUCCEEDED(create(cls,NULL,CLSCTX_INPROC_SERVER,&IID_IFileDialog,(void **)&dialog)) && proxy_identity((IUnknown *)dialog,save,FALSE));
    IFileDialog_Release(dialog);active=TRUE;return 0;
}
static style_classic_com_route theme_route(BOOL folder)
{return folder?STYLE_COM_NATIVE:explorer?STYLE_COM_EXPLORER:STYLE_COM_CLASSIC;}
int main(void)
{
    style_classic_com_options options={.session_active=session,.scope=scope,
        .property=L"Style.Test.Com",.folder_message=L"Style.Test.Folder",.hidden_message=L"Style.Test.Hidden"};
    CHECK(style_classic_com_attach(NULL,&options)==ERROR_INVALID_PARAMETER && !attaches);
    CHECK(style_classic_com_attach(capture,NULL)==ERROR_INVALID_PARAMETER && !attaches);
    fail_at=2;CHECK(style_classic_com_attach(capture,&options)==ERROR_ACCESS_DENIED && attaches==2);
    fail_at=attaches=0;CHECK(!style_classic_com_attach(capture,&options) && attaches==3 && create && create_ex && class_object);
    CHECK(SUCCEEDED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)));
    CHECK(!exercise(FALSE) && !exercise(TRUE) && event_refs==1);
    attaches=0;options.experimental_events=TRUE;options.experimental_hidden=TRUE;
    CHECK(!style_classic_com_attach(capture,&options) && attaches==3);
    CHECK(!exercise(FALSE) && !exercise(TRUE) && event_refs==1);
    attaches=0;options.experimental_events=FALSE;options.experimental_hidden=FALSE;
    options.explorer_active=explorer_session;options.explorer_scope=explorer_scope;
    CHECK(!style_classic_com_attach(capture,&options) && attaches==3);
    active=FALSE;explorer=TRUE;
    IFileDialog *retained=NULL;
    CHECK(SUCCEEDED(create(&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IFileDialog,(void **)&retained)));
    CHECK(proxy_identity((IUnknown *)retained,FALSE,TRUE) && IFileDialog_Show(retained,NULL)==E_ACCESSDENIED);
    explorer=FALSE;active=TRUE;
    CHECK(IFileDialog_Show(retained,NULL)==E_ABORT);
    active=FALSE;
    CHECK(IFileDialog_Show(retained,NULL)==E_ABORT);
    explorer=TRUE;
    CHECK(IFileDialog_Show(retained,NULL)==E_ACCESSDENIED && IFileDialog_Release(retained)==0);
    attaches=0;options.session_active=NULL;options.explorer_active=NULL;options.route=theme_route;
    CHECK(!style_classic_com_attach(capture,&options) && attaches==3);
    retained=NULL;
    CHECK(SUCCEEDED(create(&CLSID_FileOpenDialog,NULL,CLSCTX_INPROC_SERVER,&IID_IFileDialog,(void **)&retained)));
    CHECK(proxy_identity((IUnknown *)retained,FALSE,TRUE) && IFileDialog_Show(retained,NULL)==E_ACCESSDENIED);
    explorer=FALSE;
    CHECK(IFileDialog_Show(retained,NULL)==E_ABORT && IFileDialog_Release(retained)==0);
    CoUninitialize();
    printf("COM_CONTROLLER_OK bits=%u direct=1 multi_qi=1 factory=1 scope=1 events_refs=1 inactive=1 experimental=1 attach_failure=1 retained=1\n",(unsigned)(sizeof(void *)*8));return 0;
}
