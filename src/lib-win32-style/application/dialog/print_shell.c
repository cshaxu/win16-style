#define _WIN32_WINNT 0x0a00
#define COBJMACROS
#include "print_shell.h"
#include <shlobj.h>
#include <shldisp.h>
#include <shdispid.h>
#include <stdlib.h>

struct style_print_shell {
    IServiceProvider services;
    ICommDlgBrowser commands;
    IExplorerBrowserEvents events;
    IDispatch view_events;
    ULONG refs;
    HWND page,host;
    UINT change,ready;
    DWORD advice;
    DWORD view_advice;
    IConnectionPoint *view_source;
    IExplorerBrowser *browser;
    PIDLIST_ABSOLUTE folder;
    BOOL initialized;
    BOOL advised;
};

static HRESULT provide(style_print_shell *shell,REFIID id,void **object)
{
    *object=NULL;
    if(IsEqualIID(id,&IID_IUnknown) || IsEqualIID(id,&IID_IServiceProvider))*object=&shell->services;
    else if(IsEqualIID(id,&IID_ICommDlgBrowser))*object=&shell->commands;
    else if(IsEqualIID(id,&IID_IExplorerBrowserEvents))*object=&shell->events;
    else if(IsEqualIID(id,&IID_IDispatch) || IsEqualIID(id,&DIID_DShellFolderViewEvents))*object=&shell->view_events;
    if(!*object)return E_NOINTERFACE;
    ++shell->refs;
    return S_OK;
}
static ULONG release(style_print_shell *shell)
{
    ULONG refs=--shell->refs;
    if(!refs)free(shell);
    return refs;
}
static HRESULT STDMETHODCALLTYPE services_query(IServiceProvider *self,REFIID id,void **object)
{return provide(CONTAINING_RECORD(self,style_print_shell,services),id,object);}
static ULONG STDMETHODCALLTYPE services_add(IServiceProvider *self)
{return ++CONTAINING_RECORD(self,style_print_shell,services)->refs;}
static ULONG STDMETHODCALLTYPE services_release(IServiceProvider *self)
{return release(CONTAINING_RECORD(self,style_print_shell,services));}
static HRESULT STDMETHODCALLTYPE services_get(IServiceProvider *self,REFGUID service,REFIID id,void **object)
{
    *object=NULL;
    if(!IsEqualGUID(service,&SID_SExplorerBrowserFrame) && !IsEqualGUID(service,&SID_SCommDlgBrowser))return E_NOINTERFACE;
    return services_query(self,id,object);
}
static HRESULT STDMETHODCALLTYPE commands_query(ICommDlgBrowser *self,REFIID id,void **object)
{return provide(CONTAINING_RECORD(self,style_print_shell,commands),id,object);}
static ULONG STDMETHODCALLTYPE commands_add(ICommDlgBrowser *self)
{return ++CONTAINING_RECORD(self,style_print_shell,commands)->refs;}
static ULONG STDMETHODCALLTYPE commands_release(ICommDlgBrowser *self)
{return release(CONTAINING_RECORD(self,style_print_shell,commands));}
static HRESULT STDMETHODCALLTYPE commands_default(ICommDlgBrowser *self,IShellView *view)
{(void)self;(void)view;return S_OK;}
static HRESULT STDMETHODCALLTYPE commands_changed(ICommDlgBrowser *self,IShellView *view,ULONG state)
{
    (void)view;
    style_print_shell *shell=CONTAINING_RECORD(self,style_print_shell,commands);
    if(state==CDBOSC_SELCHANGE && shell->page)PostMessageW(shell->page,shell->change,0,0);
    return S_OK;
}
static HRESULT STDMETHODCALLTYPE commands_include(ICommDlgBrowser *self,IShellView *view,PCUITEMID_CHILD item)
{(void)self;(void)view;(void)item;return S_OK;}
static IServiceProviderVtbl services={services_query,services_add,services_release,services_get};
static ICommDlgBrowserVtbl commands={commands_query,commands_add,commands_release,commands_default,commands_changed,commands_include};

static HRESULT STDMETHODCALLTYPE view_query(IDispatch *self,REFIID id,void **object)
{return provide(CONTAINING_RECORD(self,style_print_shell,view_events),id,object);}
static ULONG STDMETHODCALLTYPE view_add(IDispatch *self)
{return ++CONTAINING_RECORD(self,style_print_shell,view_events)->refs;}
static ULONG STDMETHODCALLTYPE view_release(IDispatch *self)
{return release(CONTAINING_RECORD(self,style_print_shell,view_events));}
static HRESULT STDMETHODCALLTYPE view_type_count(IDispatch *self,UINT *count)
{(void)self;*count=0;return S_OK;}
static HRESULT STDMETHODCALLTYPE view_type(IDispatch *self,UINT index,LCID locale,ITypeInfo **info)
{(void)self;(void)index;(void)locale;*info=NULL;return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE view_ids(IDispatch *self,REFIID id,LPOLESTR *names,UINT count,LCID locale,DISPID *ids)
{(void)self;(void)id;(void)names;(void)count;(void)locale;(void)ids;return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE view_invoke(IDispatch *self,DISPID member,REFIID id,LCID locale,WORD flags,
    DISPPARAMS *args,VARIANT *result,EXCEPINFO *exception,UINT *invalid)
{
    (void)id;(void)locale;(void)flags;(void)args;(void)result;(void)exception;(void)invalid;
    style_print_shell *shell=CONTAINING_RECORD(self,style_print_shell,view_events);
    if(member==DISPID_FILELISTENUMDONE && shell->page)PostMessageW(shell->page,shell->ready,TRUE,0);
    return S_OK;
}
static IDispatchVtbl view_events={view_query,view_add,view_release,view_type_count,view_type,view_ids,view_invoke};

static HRESULT STDMETHODCALLTYPE events_query(IExplorerBrowserEvents *self,REFIID id,void **object)
{return provide(CONTAINING_RECORD(self,style_print_shell,events),id,object);}
static ULONG STDMETHODCALLTYPE events_add(IExplorerBrowserEvents *self)
{return ++CONTAINING_RECORD(self,style_print_shell,events)->refs;}
static ULONG STDMETHODCALLTYPE events_release(IExplorerBrowserEvents *self)
{return release(CONTAINING_RECORD(self,style_print_shell,events));}
static HRESULT STDMETHODCALLTYPE navigation_pending(IExplorerBrowserEvents *self,PCIDLIST_ABSOLUTE location)
{(void)self;(void)location;return S_OK;}
static HRESULT STDMETHODCALLTYPE view_created(IExplorerBrowserEvents *self,IShellView *view)
{
    style_print_shell *shell=CONTAINING_RECORD(self,style_print_shell,events);
    if(shell->view_source)return S_OK;
    IDispatch *automation=NULL;
    IConnectionPointContainer *connections=NULL;
    IConnectionPoint *point=NULL;
    HRESULT result=IShellView_GetItemObject(view,SVGIO_BACKGROUND,&IID_IDispatch,(void **)&automation);
    if(SUCCEEDED(result))result=IDispatch_QueryInterface(automation,&IID_IConnectionPointContainer,(void **)&connections);
    if(SUCCEEDED(result))result=IConnectionPointContainer_FindConnectionPoint(connections,&DIID_DShellFolderViewEvents,&point);
    if(SUCCEEDED(result))result=IConnectionPoint_Advise(point,(IUnknown *)&shell->view_events,&shell->view_advice);
    if(connections)IConnectionPointContainer_Release(connections);
    if(automation)IDispatch_Release(automation);
    if(SUCCEEDED(result))shell->view_source=point;
    else if(point)IConnectionPoint_Release(point);
    return S_OK;
}
static HRESULT navigation_done(IExplorerBrowserEvents *self,BOOL success)
{
    style_print_shell *shell=CONTAINING_RECORD(self,style_print_shell,events);
    if(shell->page)PostMessageW(shell->page,shell->ready,success,0);
    return S_OK;
}
static HRESULT STDMETHODCALLTYPE navigation_complete(IExplorerBrowserEvents *self,PCIDLIST_ABSOLUTE location)
{
    (void)location;
    style_print_shell *shell=CONTAINING_RECORD(self,style_print_shell,events);
    return shell->view_source?S_OK:navigation_done(self,TRUE);
}
static HRESULT STDMETHODCALLTYPE navigation_failed(IExplorerBrowserEvents *self,PCIDLIST_ABSOLUTE location)
{(void)location;return navigation_done(self,FALSE);}
static IExplorerBrowserEventsVtbl events={events_query,events_add,events_release,navigation_pending,
    view_created,navigation_complete,navigation_failed};

static PWSTR item_name(style_print_shell *shell,PCUITEMID_CHILD child)
{
    PIDLIST_ABSOLUTE item=ILCombine(shell->folder,child);
    PWSTR name=NULL;
    if(item)SHGetNameFromIDList(item,SIGDN_PARENTRELATIVEPARSING,&name);
    CoTaskMemFree(item);
    return name;
}

PWSTR style_print_shell_selected(style_print_shell *shell)
{
    IFolderView *view=NULL;
    if(FAILED(IExplorerBrowser_GetCurrentView(shell->browser,&IID_IFolderView,(void **)&view)))return NULL;
    IEnumIDList *selection=NULL;
    PITEMID_CHILD child=NULL;
    PWSTR name=NULL;
    if(SUCCEEDED(IFolderView_Items(view,SVGIO_SELECTION,&IID_IEnumIDList,(void **)&selection)) && selection){
        if(IEnumIDList_Next(selection,1,&child,NULL)==S_OK)name=item_name(shell,child);
        IEnumIDList_Release(selection);
    }
    CoTaskMemFree(child);
    IFolderView_Release(view);
    return name;
}

BOOL style_print_shell_select(style_print_shell *shell,PCWSTR name)
{
    IFolderView *view=NULL;
    if(FAILED(IExplorerBrowser_GetCurrentView(shell->browser,&IID_IFolderView,(void **)&view)))return FALSE;
    int count=0;
    BOOL selected=FALSE;
    if(SUCCEEDED(IFolderView_ItemCount(view,SVGIO_ALLVIEW,&count))){
        for(int index=0;index<count && !selected;++index){
            PITEMID_CHILD child=NULL;
            PWSTR candidate=SUCCEEDED(IFolderView_Item(view,index,&child))?item_name(shell,child):NULL;
            if(candidate && !lstrcmpiW(candidate,name))selected=SUCCEEDED(IFolderView_SelectItem(view,index,
                SVSI_SELECT|SVSI_DESELECTOTHERS|SVSI_ENSUREVISIBLE|SVSI_SELECTIONMARK));
            CoTaskMemFree(candidate);
            CoTaskMemFree(child);
        }
    }
    IFolderView_Release(view);
    return selected;
}

static HRESULT set_site(IExplorerBrowser *browser,IUnknown *client)
{
    IObjectWithSite *site=NULL;
    HRESULT result=IExplorerBrowser_QueryInterface(browser,&IID_IObjectWithSite,(void **)&site);
    if(SUCCEEDED(result)){
        result=IObjectWithSite_SetSite(site,client);
        IObjectWithSite_Release(site);
    }
    return result;
}

void style_print_shell_close(style_print_shell *shell)
{
    if(!shell)return;
    shell->page=NULL;
    if(shell->view_source){
        IConnectionPoint_Unadvise(shell->view_source,shell->view_advice);
        IConnectionPoint_Release(shell->view_source);
    }
    if(shell->browser){
        if(shell->advised)IExplorerBrowser_Unadvise(shell->browser,shell->advice);
        if(shell->initialized)IExplorerBrowser_Destroy(shell->browser);
        set_site(shell->browser,NULL);
        IExplorerBrowser_Release(shell->browser);
    }
    if(shell->host)DestroyWindow(shell->host);
    CoTaskMemFree(shell->folder);
    release(shell);
    CoUninitialize();
}

BOOL style_print_shell_show(style_print_shell *shell,HWND placeholder,PCWSTR initial)
{
    if(!style_print_shell_select(shell,initial))return FALSE;
    ShowWindow(placeholder,SW_HIDE);
    ShowWindow(shell->host,SW_SHOWNOACTIVATE);
    return TRUE;
}

style_print_shell *style_print_shell_open(HWND page,HWND placeholder,UINT change,UINT ready)
{
    RECT bounds;
    if(!GetWindowRect(placeholder,&bounds))return NULL;
    MapWindowPoints(NULL,page,(POINT *)&bounds,2);
    if(FAILED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)))return NULL;
    style_print_shell *shell=calloc(1,sizeof(*shell));
    if(!shell){CoUninitialize();return NULL;}
    shell->services.lpVtbl=&services;
    shell->commands.lpVtbl=&commands;
    shell->events.lpVtbl=&events;
    shell->view_events.lpVtbl=&view_events;
    shell->refs=1;
    shell->page=page;
    shell->change=change;
    shell->ready=ready;
    shell->host=CreateWindowExW(WS_EX_CONTROLPARENT,L"STATIC",NULL,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
        bounds.left,bounds.top,bounds.right-bounds.left,bounds.bottom-bounds.top,page,NULL,NULL,NULL);
    if(!shell->host){style_print_shell_close(shell);return NULL;}
    SetWindowPos(shell->host,placeholder,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE);
    OffsetRect(&bounds,-bounds.left,-bounds.top);
    FOLDERSETTINGS settings={FVM_LIST,FWF_SINGLESEL|FWF_ALIGNLEFT|FWF_SHOWSELALWAYS|FWF_NOCLIENTEDGE};
    HRESULT result=SHGetFolderLocation(page,CSIDL_PRINTERS,NULL,0,&shell->folder);
    if(SUCCEEDED(result))result=CoCreateInstance(&CLSID_ExplorerBrowser,NULL,CLSCTX_INPROC_SERVER,
        &IID_IExplorerBrowser,(void **)&shell->browser);
    if(SUCCEEDED(result))result=set_site(shell->browser,(IUnknown *)&shell->services);
    if(SUCCEEDED(result))result=IExplorerBrowser_Initialize(shell->browser,shell->host,&bounds,&settings);
    shell->initialized=SUCCEEDED(result);
    if(SUCCEEDED(result))result=IExplorerBrowser_SetOptions(shell->browser,
        EBO_NAVIGATEONCE|EBO_NOTRAVELLOG|EBO_NOPERSISTVIEWSTATE);
    if(SUCCEEDED(result)){
        result=IExplorerBrowser_Advise(shell->browser,&shell->events,&shell->advice);
        shell->advised=SUCCEEDED(result);
    }
    if(SUCCEEDED(result))result=IExplorerBrowser_BrowseToIDList(shell->browser,shell->folder,SBSP_ABSOLUTE);
    if(FAILED(result)){style_print_shell_close(shell);return NULL;}
    return shell;
}
