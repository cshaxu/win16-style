#include "lib-win32-style/application/control/basic.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"BASIC line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct fixture {style_instance instance;style_control_state state;style_control_basic_kind kind;unsigned rendered,native;BOOL ordered,remove_native;} fixture;
static void dispose(void *context){(void)context;}
static void render(HWND w,void *context,HDC dc,BOOL buffered)
{(void)w;(void)dc;(void)buffered;fixture *f=context;++f->rendered;f->ordered=f->native>0;}
static LRESULT CALLBACK procedure(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;fixture *f=(fixture *)data;style_instance_retain(&f->instance);LRESULT result;
    if(m==WM_NCDESTROY){style_instance_detach(&f->instance);result=DefSubclassProc(w,m,a,b);}
    else {const style_control_draw draw={f,render};result=style_control_basic_message(w,&f->instance,&f->state,m,a,b,f->kind,&draw);}
    style_instance_release(&f->instance);return result;
}
static LRESULT CALLBACK owner(HWND w,UINT m,WPARAM a,LPARAM b)
{
    fixture *f=(fixture *)GetWindowLongPtrW(w,GWLP_USERDATA);
    if(f && m==WM_PRINTCLIENT){++f->native;if(f->remove_native)style_instance_detach(&f->instance);return 123;}
    return DefWindowProcW(w,m,a,b);
}
int main(void)
{
    HWND root=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,100,100,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(root);
    HWND image=CreateWindowW(L"STATIC",L"",WS_CHILD|SS_BITMAP,0,0,40,40,root,NULL,GetModuleHandleW(NULL),NULL);CHECK(image);
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);HBITMAP output=CreateBitmap(64,64,1,32,NULL),bitmap=CreateBitmap(16,16,1,1,NULL);CHECK(output && bitmap);
    HGDIOBJ old=SelectObject(dc,output);CHECK(old);SendMessageW(image,STM_SETIMAGE,IMAGE_BITMAP,(LPARAM)bitmap);
    fixture f={.kind=STYLE_CONTROL_BITMAP};style_instance_init(&f.instance,&f,dispose);CHECK(style_instance_attach(&f.instance,image,procedure,NULL));
    SetViewportOrgEx(dc,3,4,NULL);IntersectClipRect(dc,1,2,32,33);RECT before,after;CHECK(GetClipBox(dc,&before)!=ERROR);
    SendMessageW(image,WM_PRINTCLIENT,(WPARAM)dc,PRF_CLIENT);POINT origin;CHECK(GetViewportOrgEx(dc,&origin));CHECK(GetClipBox(dc,&after)!=ERROR && EqualRect(&before,&after) && origin.x==3 && origin.y==4 && !f.rendered);
    BITMAP info;CHECK(GetObjectW(bitmap,sizeof(info),&info));
    CHECK(style_instance_detach(&f.instance));CHECK(SendMessageW(image,STM_SETIMAGE,IMAGE_BITMAP,0)==(LRESULT)bitmap);
    CHECK(DestroyWindow(image) && DeleteObject(bitmap));
    WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"PublicNativeOverlay",.lpfnWndProc=owner};CHECK(RegisterClassW(&cls));
    HWND overlay=CreateWindowW(cls.lpszClassName,L"",WS_CHILD,0,0,40,40,root,NULL,cls.hInstance,NULL);CHECK(overlay);
    fixture g={.kind=STYLE_CONTROL_NATIVE_OVERLAY};style_instance_init(&g.instance,&g,dispose);SetWindowLongPtrW(overlay,GWLP_USERDATA,(LONG_PTR)&g);CHECK(style_instance_attach(&g.instance,overlay,procedure,NULL));
    CHECK(SendMessageW(overlay,WM_PRINTCLIENT,(WPARAM)dc,0)==123 && g.native==1 && g.rendered==1 && g.ordered);
    g.remove_native=TRUE;CHECK(SendMessageW(overlay,WM_PRINTCLIENT,(WPARAM)dc,0)==123 && g.native==2 && g.rendered==1 && g.instance.detached && !g.instance.calls);
    CHECK(DestroyWindow(overlay) && UnregisterClassW(cls.lpszClassName,cls.hInstance));
    CHECK(SelectObject(dc,old) && DeleteObject(output) && DeleteDC(dc) && DestroyWindow(root));
    puts("BASIC borrowed bitmap/DC ownership, native-before-overlay return and callback detach PASS");return 0;
}
