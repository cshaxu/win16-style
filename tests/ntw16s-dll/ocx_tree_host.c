/* Optional native compatibility probe for an explicit, licensed OCX. It
 * creates an in-place public OLE object, never registers or installs the
 * component, and checks the renderer's exported attachment state. It is
 * intentionally not a required CTest. */
#define COBJMACROS
#include <windows.h>
#include <commctrl.h>
#include <ole2.h>
#include <oleidl.h>
#include <olectl.h>
#include <stdio.h>
#include "ntw16s-dll/control_protocol.h"

#define CHECK(x) do{if(!(x)){fwprintf(stderr,L"OCX_TREE_HOST_FAIL line=%u error=%lu: %hs\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
typedef struct host host;
struct host {IOleClientSite client;IOleInPlaceSite place;IOleInPlaceFrame frame;HWND window;};
#define CLIENT(p) CONTAINING_RECORD(p,host,client)
#define PLACE(p) CONTAINING_RECORD(p,host,place)
#define FRAME(p) CONTAINING_RECORD(p,host,frame)
static BOOL (__cdecl *attached)(HWND);static BOOL list_mode,generic_mode,toolbar_test,report_view,allow_empty;static CLSID generic_clsid;static WCHAR expected_class[64];static UINT list_messages[32],list_message_count;
static HWND toolbar_window;static unsigned toolbar_pre,toolbar_item,toolbar_post;
static HWND list_window;static unsigned list_pre,list_item,list_post;
static HWND status_window;static unsigned status_pre,status_item,status_post;
static LRESULT CALLBACK toolbar_monitor(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;
    if(m==WM_NOTIFY && b){NMHDR *header=(NMHDR *)b;
        if(header->hwndFrom==toolbar_window && header->code==NM_CUSTOMDRAW){
            NMCUSTOMDRAW *draw=(NMCUSTOMDRAW *)b;
            if(draw->dwDrawStage==CDDS_PREPAINT)++toolbar_pre;
            else if(draw->dwDrawStage==CDDS_ITEMPREPAINT)++toolbar_item;
            else if(draw->dwDrawStage==CDDS_POSTPAINT)++toolbar_post;
        }}
    return DefSubclassProc(w,m,a,b);
}
static LRESULT CALLBACK list_monitor(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;
    if(m==WM_NOTIFY && b){NMHDR *header=(NMHDR *)b;
        if(header->hwndFrom==list_window && header->code==NM_CUSTOMDRAW){
            NMCUSTOMDRAW *draw=(NMCUSTOMDRAW *)b;
            if(draw->dwDrawStage==CDDS_PREPAINT)++list_pre;
            else if(draw->dwDrawStage==CDDS_ITEMPREPAINT)++list_item;
            else if(draw->dwDrawStage==CDDS_POSTPAINT)++list_post;
        }}
    return DefSubclassProc(w,m,a,b);
}
static LRESULT CALLBACK status_monitor(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;
    if(m==WM_NOTIFY && b){NMHDR *header=(NMHDR *)b;
        if(header->hwndFrom==status_window && header->code==NM_CUSTOMDRAW){
            NMCUSTOMDRAW *draw=(NMCUSTOMDRAW *)b;
            if(draw->dwDrawStage==CDDS_PREPAINT)++status_pre;
            else if(draw->dwDrawStage==CDDS_ITEMPREPAINT)++status_item;
            else if(draw->dwDrawStage==CDDS_POSTPAINT)++status_post;
        }}
    return DefSubclassProc(w,m,a,b);
}
static LRESULT CALLBACK list_tracker(int code,WPARAM w,LPARAM l)
{
    if(code==HC_ACTION && list_mode && list_message_count<ARRAYSIZE(list_messages)){
        const CWPSTRUCT *message=(const CWPSTRUCT *)l;WCHAR name[64];
        if(GetClassNameW(message->hwnd,name,ARRAYSIZE(name)) && !lstrcmpW(name,L"ListView20WndClass"))
            list_messages[list_message_count++]=message->message;
    }
    return CallNextHookEx(NULL,code,w,l);
}

static HRESULT query(host *h,REFIID iid,void **out)
{
    if(!out)return E_POINTER;
    *out=NULL;
    if(IsEqualIID(iid,&IID_IUnknown)||IsEqualIID(iid,&IID_IOleClientSite))*out=&h->client;
    else if(IsEqualIID(iid,&IID_IOleInPlaceSite)||IsEqualIID(iid,&IID_IOleWindow))*out=&h->place;
    else if(IsEqualIID(iid,&IID_IOleInPlaceFrame)||IsEqualIID(iid,&IID_IOleInPlaceUIWindow))*out=&h->frame;
    return *out?S_OK:E_NOINTERFACE;
}
#define CLIENT_Q(p,i,o) query(CLIENT(p),i,o)
#define PLACE_Q(p,i,o) query(PLACE(p),i,o)
#define FRAME_Q(p,i,o) query(FRAME(p),i,o)
static ULONG STDMETHODCALLTYPE addref(IUnknown *p){(void)p;return 2;}
static ULONG STDMETHODCALLTYPE release(IUnknown *p){(void)p;return 1;}
static HRESULT STDMETHODCALLTYPE client_query(IOleClientSite *p,REFIID i,void **o){return CLIENT_Q(p,i,o);}
static HRESULT STDMETHODCALLTYPE client_save(IOleClientSite *p){(void)p;return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE client_moniker(IOleClientSite *p,DWORD a,DWORD b,IMoniker **c){(void)p;(void)a;(void)b;(void)c;return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE client_container(IOleClientSite *p,IOleContainer **c){(void)p;if(c)*c=NULL;return E_NOINTERFACE;}
static HRESULT STDMETHODCALLTYPE client_show(IOleClientSite *p){(void)p;return S_OK;}
static HRESULT STDMETHODCALLTYPE client_window(IOleClientSite *p,BOOL visible){(void)p;(void)visible;return S_OK;}
static HRESULT STDMETHODCALLTYPE client_layout(IOleClientSite *p){(void)p;return E_NOTIMPL;}
static IOleClientSiteVtbl client_vtable={client_query,(void*)addref,(void*)release,client_save,client_moniker,client_container,client_show,client_window,client_layout};

static HRESULT STDMETHODCALLTYPE place_query(IOleInPlaceSite *p,REFIID i,void **o){return PLACE_Q(p,i,o);}
static HRESULT STDMETHODCALLTYPE place_window(IOleInPlaceSite *p,HWND *out){if(!out)return E_POINTER;*out=PLACE(p)->window;return S_OK;}
static HRESULT STDMETHODCALLTYPE place_help(IOleInPlaceSite *p,BOOL enter){(void)p;(void)enter;return S_OK;}
static HRESULT STDMETHODCALLTYPE place_ok(IOleInPlaceSite *p){(void)p;return S_OK;}
static HRESULT STDMETHODCALLTYPE place_context(IOleInPlaceSite *p,IOleInPlaceFrame **frame,IOleInPlaceUIWindow **doc,LPRECT pos,LPRECT clip,LPOLEINPLACEFRAMEINFO info)
{
    host *h=PLACE(p);RECT r;if(!frame||!doc||!pos||!clip||!info)return E_POINTER;
    GetClientRect(h->window,&r);*frame=&h->frame;IOleInPlaceFrame_AddRef(*frame);*doc=NULL;*pos=r;*clip=r;
    info->cb=sizeof(*info);info->hwndFrame=h->window;return S_OK;
}
static HRESULT STDMETHODCALLTYPE place_scroll(IOleInPlaceSite *p,SIZE s){(void)p;(void)s;return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE place_deactivate(IOleInPlaceSite *p,BOOL undo){(void)p;(void)undo;return S_OK;}
static HRESULT STDMETHODCALLTYPE place_noarg(IOleInPlaceSite *p){(void)p;return S_OK;}
static HRESULT STDMETHODCALLTYPE place_rect(IOleInPlaceSite *p,LPCRECT r){(void)p;(void)r;return S_OK;}
static IOleInPlaceSiteVtbl place_vtable={place_query,(void*)addref,(void*)release,place_window,place_help,place_ok,place_ok,place_ok,place_context,place_scroll,place_deactivate,place_noarg,place_noarg,place_noarg,place_rect};

static HRESULT STDMETHODCALLTYPE frame_query(IOleInPlaceFrame *p,REFIID i,void **o){return FRAME_Q(p,i,o);}
static HRESULT STDMETHODCALLTYPE frame_window(IOleInPlaceFrame *p,HWND *out){if(!out)return E_POINTER;*out=FRAME(p)->window;return S_OK;}
static HRESULT STDMETHODCALLTYPE frame_help(IOleInPlaceFrame *p,BOOL enter){(void)p;(void)enter;return S_OK;}
static HRESULT STDMETHODCALLTYPE frame_border(IOleInPlaceFrame *p,LPRECT r){(void)p;(void)r;return INPLACE_E_NOTOOLSPACE;}
static HRESULT STDMETHODCALLTYPE frame_widths(IOleInPlaceFrame *p,LPCBORDERWIDTHS w){(void)p;(void)w;return INPLACE_E_NOTOOLSPACE;}
static HRESULT STDMETHODCALLTYPE frame_set_widths(IOleInPlaceFrame *p,LPCBORDERWIDTHS w){(void)p;(void)w;return S_OK;}
static HRESULT STDMETHODCALLTYPE frame_active(IOleInPlaceFrame *p,IOleInPlaceActiveObject *o,LPCOLESTR n){(void)p;(void)o;(void)n;return S_OK;}
static HRESULT STDMETHODCALLTYPE frame_menus(IOleInPlaceFrame *p,HMENU m,LPOLEMENUGROUPWIDTHS w){(void)p;(void)m;(void)w;return E_NOTIMPL;}
static HRESULT STDMETHODCALLTYPE frame_menu(IOleInPlaceFrame *p,HMENU m,HOLEMENU h,HWND c){(void)p;(void)m;(void)h;(void)c;return S_OK;}
static HRESULT STDMETHODCALLTYPE frame_menu1(IOleInPlaceFrame *p,HMENU m){(void)p;(void)m;return S_OK;}
static HRESULT STDMETHODCALLTYPE frame_text(IOleInPlaceFrame *p,LPCOLESTR s){(void)p;(void)s;return S_OK;}
static HRESULT STDMETHODCALLTYPE frame_modeless(IOleInPlaceFrame *p,BOOL enabled){(void)p;(void)enabled;return S_OK;}
static HRESULT STDMETHODCALLTYPE frame_accel(IOleInPlaceFrame *p,LPMSG m,WORD id){(void)p;(void)m;(void)id;return S_FALSE;}
static IOleInPlaceFrameVtbl frame_vtable={frame_query,(void*)addref,(void*)release,frame_window,frame_help,frame_border,frame_widths,frame_set_widths,frame_active,frame_menus,frame_menu,frame_menu1,frame_text,frame_modeless,frame_accel};

static LRESULT CALLBACK window_proc(HWND w,UINT m,WPARAM a,LPARAM b){return DefWindowProcW(w,m,a,b);}
typedef struct outcome {BOOL seen,match,expected;} outcome;
static BOOL CALLBACK found(HWND w,LPARAM value)
{
    WCHAR name[64];outcome *state=(outcome *)value;
    if(generic_mode){
        WCHAR module[MAX_PATH],parent_name[64],real[64];HMODULE owner=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE);HWND parent=GetParent(w);LRESULT value=0;ntw16s_control_protocol *protocol;
        if(!GetClassNameW(w,name,ARRAYSIZE(name)))return TRUE;
        if(!RealGetWindowClassW(w,real,ARRAYSIZE(real)))real[0]=0;
        if(owner)GetModuleFileNameW(owner,module,ARRAYSIZE(module));else module[0]=0;
        if(parent)GetClassNameW(parent,parent_name,ARRAYSIZE(parent_name));else parent_name[0]=0;
        if(wcsstr(name,L"TreeView")){
            TVINSERTSTRUCTW item={0};item.hParent=TVI_ROOT;item.hInsertAfter=TVI_LAST;item.item.mask=TVIF_TEXT;item.item.pszText=L"probe";
            value=SendMessageW(w,TVM_INSERTITEMW,0,(LPARAM)&item)?SendMessageW(w,TVM_GETCOUNT,0,0):-1;
        }
        else if(wcsstr(name,L"ListView"))value=SendMessageW(w,LVM_GETITEMCOUNT,0,0);
        else if(wcsstr(name,L"ProgressBar"))value=SendMessageW(w,PBM_GETPOS,0,0);
        else if(wcsstr(name,L"MSMask")){WCHAR mask[32];DWORD begin=0,end=0;CHECK(SetWindowTextW(w,L"12/34"));SendMessageW(w,EM_SETSEL,1,4);SendMessageW(w,EM_GETSEL,(WPARAM)&begin,(LPARAM)&end);CHECK(begin==1 && end==4);value=GetWindowTextW(w,mask,ARRAYSIZE(mask));wprintf(L"OCX_MASK_TEXT %ls selection=%lu:%lu\n",mask,begin,end);}
        else if(!lstrcmpW(name,L"Edit")){WCHAR text[32];DWORD begin=0,end=0;CHECK(SetWindowTextW(w,L"alpha"));SendMessageW(w,EM_SETSEL,1,4);SendMessageW(w,EM_GETSEL,(WPARAM)&begin,(LPARAM)&end);CHECK(begin==1 && end==4);value=GetWindowTextW(w,text,ARRAYSIZE(text));CHECK(value==5 && !lstrcmpW(text,L"alpha"));wprintf(L"OCX_EDIT_TEXT %ls selection=%lu:%lu\n",text,begin,end);}
        else if(!lstrcmpW(name,L"ScrollBar")){CHECK(SetScrollRange(w,SB_CTL,0,20,FALSE));CHECK(SetScrollPos(w,SB_CTL,12,TRUE)==0);value=GetScrollPos(w,SB_CTL);CHECK(value==12);wprintf(L"OCX_SCROLL_POS value=%lld\n",(long long)value);}
        else if(wcsstr(name,L"Slider")){SendMessageW(w,TBM_SETRANGEMIN,FALSE,0);SendMessageW(w,TBM_SETRANGEMAX,FALSE,20);SendMessageW(w,TBM_SETPOS,FALSE,12);value=SendMessageW(w,TBM_GETPOS,0,0);}
        else if(!lstrcmpW(name,TOOLBARCLASSNAMEW)){
            if(toolbar_test){
                TBBUTTON buttons[2]={{.iBitmap=I_IMAGENONE,.idCommand=7301,.fsState=TBSTATE_ENABLED,.fsStyle=BTNS_AUTOSIZE,.iString=(INT_PTR)L"Open"},
                    {.iBitmap=I_IMAGENONE,.idCommand=7302,.fsState=TBSTATE_ENABLED,.fsStyle=BTNS_CHECK|BTNS_AUTOSIZE,.iString=(INT_PTR)L"Toggle"}};
                toolbar_window=w;CHECK(SetWindowSubclass(GetParent(w),toolbar_monitor,7301,0));
                SendMessageW(w,TB_BUTTONSTRUCTSIZE,sizeof(buttons[0]),0);CHECK(SendMessageW(w,TB_ADDBUTTONSW,ARRAYSIZE(buttons),(LPARAM)buttons));
                CHECK(SendMessageW(w,TB_CHECKBUTTON,7302,TRUE));CHECK(SendMessageW(w,TB_GETSTATE,7302,0)&TBSTATE_CHECKED);
                SendMessageW(w,TB_AUTOSIZE,0,0);RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
            }
            value=SendMessageW(w,TB_BUTTONCOUNT,0,0);
        }
        else if(wcsstr(name,L"StatusBar"))value=SendMessageW(w,SB_GETPARTS,0,0);
        else if(wcsstr(name,L"TabStrip"))value=SendMessageW(w,TCM_GETITEMCOUNT,0,0);
        protocol=ntw16s_control_protocol_bind(w);
        if(wcsstr(name,L"ListView") && report_view){
            list_window=w;CHECK(SetWindowSubclass(GetParent(w),list_monitor,7302,0));
            RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_UPDATENOW);
        }
        if(wcsstr(name,L"StatusBar")){
            status_window=w;CHECK(SetWindowSubclass(GetParent(w),status_monitor,7303,0));
            RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_UPDATENOW);
        }
        wprintf(L"OCX_GENERIC_CHILD class=%ls real=%ls parent=%ls owner=%ls attached=%u protocol_bound=%u public_value=%lld style=%08lx dpi=%u\n",name,real,parent_name,module,
            attached&&attached(w),protocol!=NULL,(long long)value,(DWORD)GetWindowLongPtrW(w,GWL_STYLE),GetDpiForWindow(w));
        if(!lstrcmpW(name,TOOLBARCLASSNAMEW) && toolbar_test)wprintf(L"OCX_TOOLBAR_DRAW pre=%u item=%u post=%u\n",toolbar_pre,toolbar_item,toolbar_post);
        if(wcsstr(name,L"ListView") && report_view)wprintf(L"OCX_LIST_DRAW pre=%u item=%u post=%u\n",list_pre,list_item,list_post);
        if(wcsstr(name,L"StatusBar"))wprintf(L"OCX_STATUS_DRAW pre=%u item=%u post=%u\n",status_pre,status_item,status_post);
        if(protocol)ntw16s_control_protocol_release(protocol);
        if(wcsstr(name,L"ProgressBar")){
            PBRANGE range={INT_MAX,INT_MIN};LRESULT low=SendMessageW(w,PBM_GETRANGE,TRUE,(LPARAM)&range);
            LRESULT high=SendMessageW(w,PBM_GETRANGE,FALSE,0),state=SendMessageW(w,PBM_GETSTATE,0,0);
            wprintf(L"OCX_PROGRESS_PROTOCOL low=%lld range=%d:%d high=%lld pos=%lld state=%lld bar=%08lx background=%08lx\n",
                (long long)low,range.iLow,range.iHigh,(long long)high,(long long)SendMessageW(w,PBM_GETPOS,0,0),
                (long long)state,(DWORD)SendMessageW(w,PBM_GETBARCOLOR,0,0),(DWORD)SendMessageW(w,PBM_GETBKCOLOR,0,0));
        }
        if(expected_class[0] && !lstrcmpW(name,expected_class)){
            BOOL is_attached=attached && attached(w);
            state->seen=TRUE;state->match=is_attached==state->expected;
            wprintf(L"OCX_EXPECTED_CLASS class=%ls attached=%u expected=%u\n",name,is_attached,state->expected);
        }else if(!expected_class[0]){state->seen=TRUE;state->match=TRUE;}
        return TRUE;
    }
    if(GetClassNameW(w,name,ARRAYSIZE(name)) && !lstrcmpW(name,list_mode?L"ListView20WndClass":L"TreeView20WndClass")){
        BOOL is_attached=attached && attached(w);
        state->seen=TRUE;state->match=is_attached==state->expected;
        if(list_mode){
            LRESULT previous=SendMessageW(w,LVM_SETVIEW,LVS_REPORT,0);HWND header=(HWND)SendMessageW(w,LVM_GETHEADER,0,0);WCHAR header_name[64],module[MAX_PATH];
            HMODULE header_owner=header?(HMODULE)GetClassLongPtrW(header,GCLP_HMODULE):NULL;
            ntw16s_control_protocol *protocol=is_attached?NULL:ntw16s_control_protocol_bind(w);BOOL public_bound=protocol!=NULL;
            if(protocol)ntw16s_control_protocol_release(protocol);
            if(header_owner)GetModuleFileNameW(header_owner,module,ARRAYSIZE(module));else module[0]=0;
            if(header)GetClassNameW(header,header_name,ARRAYSIZE(header_name));else header_name[0]=0;
            wprintf(L"OCX_LIST_HOST_RESULT class=%ls attached=%u expected=%u public_bound=%u setview_previous=%lld style=%08lx count=%lld header=%p header_class=%ls header_module=%ls dpi=%u\n",name,is_attached,state->expected,public_bound,(long long)previous,(DWORD)GetWindowLongPtrW(w,GWL_STYLE),(long long)SendMessageW(w,LVM_GETITEMCOUNT,0,0),header,header_name,module,GetDpiForWindow(w));
        }
        else wprintf(L"OCX_TREE_HOST_RESULT class=%ls attached=%u expected=%u dpi=%u\n",name,is_attached,state->expected,GetDpiForWindow(w));
    }
    return TRUE;
}
typedef HRESULT (STDAPICALLTYPE *get_class_fn)(REFCLSID,REFIID,LPVOID *);
static const CLSID tree_clsid={0xc74190b6,0x8589,0x11d1,{0xb1,0x6a,0x00,0xc0,0xf0,0x28,0x36,0x28}};
static const CLSID list_clsid={0xbdd1f04b,0x858b,0x11d1,{0xb1,0x6a,0x00,0xc0,0xf0,0x28,0x36,0x28}};

int wmain(int argc,WCHAR **argv)
{
    HRESULT hr;host h={0};WNDCLASSW wc={0};HMODULE module=NULL,style;IClassFactory *factory=NULL;IOleObject *object=NULL;IPersistStreamInit *persist=NULL;IDispatch *dispatch=NULL;IOleInPlaceObject *inplace=NULL;RECT rect={0,0,320,200};outcome state={.expected=TRUE};
    CHECK(argc>=2 && argc<=10);
    for(int i=2;i<argc;++i){
        if(!lstrcmpW(argv[i],L"--list"))list_mode=TRUE;
        else if(!lstrcmpW(argv[i],L"--report-view"))report_view=TRUE;
        else if(!lstrcmpW(argv[i],L"--expect-detached"))state.expected=FALSE;
        else if(!lstrcmpW(argv[i],L"--expect-class") && i+1<argc){
            CHECK(wcslen(argv[++i])<ARRAYSIZE(expected_class));lstrcpyW(expected_class,argv[i]);
        }
        else if(!lstrcmpW(argv[i],L"--toolbar-test"))toolbar_test=TRUE;
        else if(!lstrcmpW(argv[i],L"--allow-empty"))allow_empty=TRUE;
        else if(!lstrcmpW(argv[i],L"--clsid") && i+1<argc){generic_mode=SUCCEEDED(CLSIDFromString(argv[++i],&generic_clsid));CHECK(generic_mode);}
        else CHECK(FALSE);
    }
    CHECK(SUCCEEDED(CoInitializeEx(NULL,COINIT_APARTMENTTHREADED)));
    h.client.lpVtbl=&client_vtable;h.place.lpVtbl=&place_vtable;h.frame.lpVtbl=&frame_vtable;
    wc.hInstance=GetModuleHandleW(NULL);wc.lpfnWndProc=window_proc;wc.lpszClassName=L"Win16StyleOcxTreeHost";CHECK(RegisterClassW(&wc));
    h.window=CreateWindowW(wc.lpszClassName,L"ocx-tree-host",WS_OVERLAPPEDWINDOW|WS_VISIBLE,0,0,320,200,NULL,NULL,wc.hInstance,NULL);CHECK(h.window);
    module=LoadLibraryW(argv[1]);CHECK(module);get_class_fn get_class=(void *)GetProcAddress(module,"DllGetClassObject");CHECK(get_class);
    hr=get_class(generic_mode?&generic_clsid:list_mode?&list_clsid:&tree_clsid,&IID_IClassFactory,(void **)&factory);CHECK(SUCCEEDED(hr));
    hr=IClassFactory_CreateInstance(factory,NULL,&IID_IOleObject,(void **)&object);CHECK(SUCCEEDED(hr));
    CHECK(SUCCEEDED(IOleObject_SetClientSite(object,&h.client)) && SUCCEEDED(IOleObject_QueryInterface(object,&IID_IPersistStreamInit,(void **)&persist)));
    CHECK(SUCCEEDED(IPersistStreamInit_InitNew(persist)));OleSetContainedObject((IUnknown *)object,TRUE);
    if(list_mode || report_view){
        OLECHAR *property=L"View";DISPID id;VARIANT value;DISPID named=DISPID_PROPERTYPUT;DISPPARAMS args;
        CHECK(SUCCEEDED(IOleObject_QueryInterface(object,&IID_IDispatch,(void **)&dispatch)) &&
            SUCCEEDED(IDispatch_GetIDsOfNames(dispatch,&IID_NULL,&property,1,LOCALE_USER_DEFAULT,&id)));
        VariantInit(&value);value.vt=VT_I4;value.lVal=3;
        args.rgvarg=&value;args.rgdispidNamedArgs=&named;args.cArgs=1;args.cNamedArgs=1;
        CHECK(SUCCEEDED(IDispatch_Invoke(dispatch,id,&IID_NULL,LOCALE_USER_DEFAULT,DISPATCH_PROPERTYPUT,&args,NULL,NULL,NULL)));
    }
    HHOOK tracker=list_mode?SetWindowsHookExW(WH_CALLWNDPROC,list_tracker,NULL,GetCurrentThreadId()):NULL;CHECK(!list_mode || tracker);
    CHECK(SUCCEEDED(IOleObject_DoVerb(object,OLEIVERB_INPLACEACTIVATE,NULL,&h.client,0,h.window,&rect)));
    if(tracker){CHECK(UnhookWindowsHookEx(tracker));wprintf(L"OCX_LIST_MESSAGES");for(UINT i=0;i<list_message_count;++i)wprintf(L" %04x",list_messages[i]);wprintf(L"\n");}
    CHECK(SUCCEEDED(IOleObject_QueryInterface(object,&IID_IOleInPlaceObject,(void **)&inplace)) && SUCCEEDED(IOleInPlaceObject_SetObjectRects(inplace,&rect,&rect)));
    style=GetModuleHandleW(L"ntw16s32.dll");attached=style?(void *)GetProcAddress(style,"Ntw16sWindowAttached"):NULL;CHECK(attached);
    RedrawWindow(h.window,NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_UPDATENOW);
    CHECK(EnumChildWindows(h.window,found,(LPARAM)&state));
    if(!state.seen && allow_empty){wprintf(L"OCX_NO_CHILD class_factory=%08lx\n",(DWORD)generic_clsid.Data1);state.seen=TRUE;state.match=TRUE;}
    CHECK(state.seen && state.match);
    IOleInPlaceObject_Release(inplace);if(dispatch)IDispatch_Release(dispatch);IPersistStreamInit_Release(persist);IOleObject_Close(object,OLECLOSE_NOSAVE);IOleObject_Release(object);IClassFactory_Release(factory);FreeLibrary(module);DestroyWindow(h.window);CoUninitialize();return 0;
}
