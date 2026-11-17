#include "lib-win32-style/application/control/native.h"
#include <commctrl.h>
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"NATIVE CONTROL FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct fixture {style_instance instance;style_control_native_state state;style_control_native_kind kind;unsigned draws,queries,disposed;BOOL allow,stop;POINT origin;} fixture;
static LRESULT CALLBACK procedure(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static void dispose(void *context){++((fixture *)context)->disposed;}
static BOOL supported(HWND w)
{fixture *f=(fixture *)style_instance_lookup(w,procedure);++f->queries;return f->allow;}
static void draw(HWND w,void *context,HDC dc,BOOL buffered)
{
    (void)w;(void)buffered;fixture *f=context;++f->draws;GetViewportOrgEx(dc,&f->origin);
    if(f->stop)(void)style_instance_detach(&f->instance);
}
static LRESULT CALLBACK underlying(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;
    if(m==WM_APP){SetLastError(0x41ab);return 73;}
    if(m==WM_PRINT){SetViewportOrgEx((HDC)a,30,40,NULL);return 42;}
    if(m==WM_LBUTTONDOWN || m==WM_LBUTTONUP)return 0;
    return DefSubclassProc(w,m,a,b);
}
static LRESULT CALLBACK procedure(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;fixture *f=(fixture *)data;style_instance_retain(&f->instance);
    style_control_native_observe(&f->state,f->kind,m);
    const style_control_draw renderer={f,draw};
    LRESULT result=style_control_native_message(w,&f->instance,&f->state,f->kind,m,a,b,supported,&renderer);
    style_instance_release(&f->instance);return result;
}
int main(void)
{
    INITCOMMONCONTROLSEX init={sizeof(init),ICC_TAB_CLASSES|ICC_LISTVIEW_CLASSES|ICC_PROGRESS_CLASS};CHECK(InitCommonControlsEx(&init));
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,300,200,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent);
    const WCHAR *classes[]={WC_TABCONTROLW,WC_HEADERW,PROGRESS_CLASSW};
    HDC dc=CreateCompatibleDC(NULL);HBITMAP bitmap=CreateBitmap(200,80,1,32,NULL);CHECK(dc && bitmap);HGDIOBJ old=SelectObject(dc,bitmap);
    for(unsigned kind=0;kind<3;++kind){
        HWND w=CreateWindowW(classes[kind],L"",WS_CHILD,0,0,200,60,parent,NULL,GetModuleHandleW(NULL),NULL);CHECK(w);
        if(kind==STYLE_CONTROL_HEADER){HDITEMW item={.mask=HDI_WIDTH|HDI_TEXT|HDI_FORMAT,.cxy=100,.pszText=L"Header",.fmt=HDF_STRING};CHECK(SendMessageW(w,HDM_INSERTITEMW,0,(LPARAM)&item)==0);}
        fixture f={.kind=(style_control_native_kind)kind,.allow=TRUE};style_instance_init(&f.instance,&f,dispose);style_control_native_init(&f.state);
        CHECK(f.state.pressed==-1 && !f.state.epoch && SetWindowSubclass(w,underlying,1,0));
        CHECK(style_instance_attach(&f.instance,w,procedure,NULL));
        CHECK(SendMessageW(w,WM_APP,0,0)==73 && GetLastError()==0x41ab && !f.state.epoch);
        CHECK(f.queries==(kind==STYLE_CONTROL_PROGRESS?1u:0u));
        if(kind==STYLE_CONTROL_HEADER){SendMessageW(w,WM_LBUTTONDOWN,0,MAKELPARAM(10,5));CHECK(f.state.pressed==0);SendMessageW(w,WM_LBUTTONUP,0,0);CHECK(f.state.pressed==-1);}
        SetViewportOrgEx(dc,3,4,NULL);SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,0);CHECK(f.draws==1 && f.origin.x==3 && f.origin.y==4);
        unsigned version=f.state.epoch;SendMessageW(w,WM_ENABLE,TRUE,0);CHECK(f.state.epoch==version+1);
        if(kind==STYLE_CONTROL_PROGRESS){CHECK(SendMessageW(w,WM_PRINT,(WPARAM)dc,PRF_CLIENT|PRF_NONCLIENT)==42);CHECK(f.draws==2 && f.origin.x==3 && f.origin.y==4);}
        unsigned before=f.draws;f.allow=FALSE;SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,0);CHECK(f.draws==before);
        f.allow=TRUE;f.stop=TRUE;SendMessageW(w,WM_PRINTCLIENT,(WPARAM)dc,0);CHECK(f.instance.detached && f.disposed==1 && !style_instance_lookup(w,procedure));
        CHECK(DestroyWindow(w));
    }
    SelectObject(dc,old);CHECK(DeleteObject(bitmap) && DeleteDC(dc) && DestroyWindow(parent));
    puts("NATIVE CONTROL PASS families=3 native_result=1 query_timing=1 header_input=1 epoch=1 print_dc=1 eligibility=1 retire=1 cleanup=1");return 0;
}
