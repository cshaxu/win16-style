#include "lib-win32-style/application/control/edit.h"
#include <commctrl.h>
#include <stdio.h>
#include <wchar.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"EDIT line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct fixture {
    style_instance instance;
    style_control_state state;
    style_scroll_resources scroll;
    unsigned borders,clients,bars;
    BOOL stop;
} fixture;
static style_control_edit_ops operations;
static void dispose(void *context){(void)context;}
static BOOL input(HWND window,void *context,UINT message,WPARAM a,LPARAM b)
{(void)window;(void)context;(void)message;(void)a;(void)b;return FALSE;}
static void border(HWND window,void *context,HDC dc)
{(void)window;(void)dc;++((fixture *)context)->borders;}
static void client(HWND window,void *context,HDC dc)
{(void)window;(void)dc;++((fixture *)context)->clients;}
static void bars(HWND window,void *context,HDC dc)
{(void)window;(void)dc;++((fixture *)context)->bars;}
static LRESULT CALLBACK witness(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;fixture *f=(fixture *)data;
    if(m==WM_PRINTCLIENT){SetViewportOrgEx((HDC)a,80,81,NULL);return 17;}
    if(m==WM_SETTEXT && f->stop)(void)style_instance_detach(&f->instance);
    return DefSubclassProc(w,m,a,b);
}
static LRESULT CALLBACK procedure(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;fixture *f=(fixture *)data;style_instance_retain(&f->instance);
    LRESULT result=style_control_edit_message(w,&f->instance,&f->state,&f->scroll,m,a,b,&operations);
    style_instance_release(&f->instance);return result;
}
int main(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);
    HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,240,130,NULL,NULL,module,NULL);
    HWND edit=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"alpha",WS_CHILD|ES_MULTILINE|WS_VSCROLL,0,0,200,90,root,NULL,module,NULL);
    CHECK(root && edit);fixture f={.scroll.edit=TRUE};style_instance_init(&f.instance,&f,dispose);
    operations=(style_control_edit_ops){&f,TRUE,input,bars,border,client};
    CHECK(SetWindowSubclass(edit,witness,1,(DWORD_PTR)&f) && style_instance_attach(&f.instance,edit,procedure,NULL));
    SendMessageW(edit,EM_SETSEL,0,5);SendMessageW(edit,EM_REPLACESEL,TRUE,(LPARAM)L"beta");
    WCHAR text[32];CHECK(GetWindowTextW(edit,text,ARRAYSIZE(text))==4 && !wcscmp(text,L"beta"));
    CHECK(SendMessageW(edit,EM_CANUNDO,0,0) && SendMessageW(edit,EM_UNDO,0,0));
    CHECK(GetWindowTextW(edit,text,ARRAYSIZE(text))==5 && !wcscmp(text,L"alpha"));
    DWORD start=0,end=0;SendMessageW(edit,EM_SETSEL,1,3);SendMessageW(edit,EM_GETSEL,(WPARAM)&start,(LPARAM)&end);
    CHECK(start==1 && end==3 && !f.state.suppression);
    SendMessageW(edit,WM_SETREDRAW,FALSE,0);CHECK(f.state.redraw_off);
    SendMessageW(edit,WM_SETREDRAW,TRUE,0);CHECK(!f.state.redraw_off);
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);SetViewportOrgEx(dc,3,4,NULL);
    unsigned before=f.clients;CHECK(SendMessageW(edit,WM_PRINTCLIENT,(WPARAM)dc,0)==17);
    POINT origin;CHECK(GetViewportOrgEx(dc,&origin) && origin.x==3 && origin.y==4 && f.clients==before+1);
    before=f.bars;SendMessageW(edit,WM_NCPAINT,1,0);CHECK(f.bars==before+1 && f.borders);
    before=f.bars;unsigned clients=f.clients;f.stop=TRUE;
    CHECK(SendMessageW(edit,WM_SETTEXT,0,(LPARAM)L"retired"));
    CHECK(f.instance.detached && !f.state.suppression && f.bars==before && f.clients==clients);
    CHECK(GetWindowTextW(edit,text,ARRAYSIZE(text))==7 && !wcscmp(text,L"retired"));
    CHECK(RemoveWindowSubclass(edit,witness,1) && DeleteDC(dc) && DestroyWindow(root));
    puts("EDIT native text/undo/selection, redraw state, borrowed print DC/return and callback retirement PASS");return 0;
}
