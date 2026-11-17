#include "lib-win32-style/application/control/list.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"LIST line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct fixture {
    style_instance instance;
    style_control_state state;
    unsigned draws,borders,scrolls;
    BOOL buffered,stop;
    POINT origin;
} fixture;
static style_control_list_ops operations;
static void dispose(void *context){(void)context;}
static BOOL input(HWND w,void *context,UINT m,WPARAM a,LPARAM b)
{(void)w;(void)context;(void)m;(void)a;(void)b;return FALSE;}
static void draw(HWND w,void *context,HDC dc,BOOL buffered)
{
    (void)w;fixture *f=context;++f->draws;f->buffered=buffered;GetViewportOrgEx(dc,&f->origin);
    SetViewportOrgEx(dc,90,91,NULL);
    if(f->stop)(void)style_instance_detach(&f->instance);
}
static void border(HWND w,void *context,HDC dc)
{(void)w;(void)dc;++((fixture *)context)->borders;}
static void scrollbars(HWND w,void *context,HDC dc)
{(void)w;(void)dc;++((fixture *)context)->scrolls;}
static LRESULT CALLBACK procedure(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;fixture *f=(fixture *)data;style_instance_retain(&f->instance);
    LRESULT result=style_control_list_message(w,&f->instance,&f->state,m,a,b,&operations);
    style_instance_release(&f->instance);return result;
}
int main(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);
    HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,200,100,NULL,NULL,module,NULL);
    HWND list=CreateWindowExW(WS_EX_CLIENTEDGE,L"LISTBOX",L"",WS_CHILD|LBS_USETABSTOPS,0,0,150,70,root,NULL,module,NULL);
    CHECK(root && list);fixture f={0};style_instance_init(&f.instance,&f,dispose);
    operations=(style_control_list_ops){&f,TRUE,input,draw,border,scrollbars,FALSE};
    CHECK(style_instance_attach(&f.instance,list,procedure,NULL));
    CHECK(SendMessageW(list,LB_ADDSTRING,0,(LPARAM)L"first")==0);
    CHECK(SendMessageW(list,LB_GETCOUNT,0,0)==1);
    CHECK(SendMessageW(list,WM_SETREDRAW,FALSE,0)==0 && f.state.redraw_off);
    CHECK(SendMessageW(list,WM_SETREDRAW,TRUE,0)==0 && !f.state.redraw_off);
    int stops[]={32,64};CHECK(SendMessageW(list,LB_SETTABSTOPS,2,(LPARAM)stops) && f.state.tab_count==2);
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);SetViewportOrgEx(dc,3,4,NULL);
    unsigned before=f.scrolls;
    SendMessageW(list,WM_PRINT,(WPARAM)dc,PRF_CLIENT|PRF_NONCLIENT);
    POINT origin;CHECK(GetViewportOrgEx(dc,&origin) && origin.x==3 && origin.y==4);
    CHECK(f.draws==1 && !f.buffered && f.borders==1 && f.scrolls==before+1);
    RECT window;POINT client={0,0};CHECK(GetWindowRect(list,&window) && ClientToScreen(list,&client));
    CHECK(f.origin.x==3+client.x-window.left && f.origin.y==4+client.y-window.top);
    f.stop=TRUE;before=f.scrolls;
    SendMessageW(list,WM_PRINT,(WPARAM)dc,PRF_CLIENT|PRF_NONCLIENT);
    CHECK(f.instance.detached && f.scrolls==before && f.borders==1);
    CHECK(GetViewportOrgEx(dc,&origin) && origin.x==3 && origin.y==4);
    style_control_state_dispose(&f.state);CHECK(DeleteDC(dc) && DestroyWindow(root));
    puts("LIST native forwarding/redraw/tabs, client print translation/DC restore and callback retirement PASS");return 0;
}
