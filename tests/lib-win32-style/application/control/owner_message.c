#include "lib-win32-style/application/control/basic.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"OWNER_MESSAGE FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static style_instance instance;
static unsigned disposed,forwarded,selected,rendered,bad;
static BOOL consume,detach_native,destroy_native,detach_render;
static HDC borrowed;
static UINT active_message;
static void dispose(void *context){(void)context;++disposed;}
static BOOL input(HWND w,void *context,UINT m,WPARAM a,LPARAM b)
{(void)w;(void)context;(void)m;(void)a;(void)b;forwarded=0;return consume;}
static style_control_owner_action select_action(HWND w,void *context,UINT m,LPARAM b)
{
    (void)w;(void)context;
    if(m!=WM_PAINT && m!=WM_PRINTCLIENT && m!=WM_PRINT && m!=WM_NCPAINT && m!=WM_APP+1)return STYLE_CONTROL_OWNER_NONE;
    ++selected;if(forwarded!=(consume?0U:1U))++bad;
    if(m==WM_PAINT)return STYLE_CONTROL_OWNER_PAINT;
    if(m==WM_PRINTCLIENT)return STYLE_CONTROL_OWNER_PRINT;
    if(m==WM_NCPAINT || (m==WM_PRINT && (b&PRF_NONCLIENT)))return STYLE_CONTROL_OWNER_FRAME;
    return STYLE_CONTROL_OWNER_NONE;
}
static void render(HWND w,void *context,HDC dc,style_control_owner_action action)
{
    (void)context;++rendered;
    if(action==STYLE_CONTROL_OWNER_PAINT){if(!dc || WindowFromDC(dc)!=w)++bad;}
    else if(action==STYLE_CONTROL_OWNER_PRINT || active_message==WM_PRINT){if(dc!=borrowed)++bad;}
    else if(dc)++bad;
    if(detach_render)style_instance_detach(&instance);
}
static LRESULT CALLBACK native(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_PAINT || m==WM_PRINTCLIENT || m==WM_PRINT || m==WM_NCPAINT || m==WM_APP+1){
        ++forwarded;if(detach_native)style_instance_detach(&instance);if(destroy_native)DestroyWindow(w);return 41;
    }
    return DefWindowProcW(w,m,a,b);
}
static LRESULT CALLBACK subclass(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;style_instance *s=(void *)data;style_instance_retain(s);LRESULT result;
    if(m==WM_NCDESTROY){style_instance_detach(s);result=DefSubclassProc(w,m,a,b);}
    else {const style_control_owner_ops ops={NULL,input,select_action,render};result=style_control_owner_message(w,s,m,a,b,&ops);}
    style_instance_release(s);return result;
}
static HWND create(HINSTANCE module)
{
    style_instance_init(&instance,NULL,dispose);style_instance_retain(&instance);
    HWND w=CreateWindowW(L"StyleOwnerTest",L"",WS_POPUP,0,0,120,80,NULL,NULL,module,NULL);
    return w && style_instance_attach(&instance,w,subclass,NULL)?w:NULL;
}
static LRESULT send_message(HWND w,UINT m,WPARAM a,LPARAM b){active_message=m;return SendMessageW(w,m,a,b);}
int main(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);WNDCLASSW cls={.lpfnWndProc=native,.hInstance=module,.lpszClassName=L"StyleOwnerTest"};CHECK(RegisterClassW(&cls));
    HWND w=create(module);CHECK(w);borrowed=GetDC(w);CHECK(borrowed);unsigned before=rendered;
    CHECK(send_message(w,WM_PAINT,0,0)==41 && rendered==before+1 && !bad);
    CHECK(send_message(w,WM_PRINTCLIENT,(WPARAM)borrowed,0)==41 && GetMapMode(borrowed)==MM_TEXT && !bad);
    CHECK(send_message(w,WM_NCPAINT,1,0)==41 && !bad);
    CHECK(send_message(w,WM_PRINT,(WPARAM)borrowed,PRF_NONCLIENT)==41 && !bad);
    before=rendered;CHECK(send_message(w,WM_PRINT,(WPARAM)borrowed,PRF_CLIENT)==41 && rendered==before);
    consume=TRUE;CHECK(send_message(w,WM_APP+1,0,0)==0 && !bad);consume=FALSE;
    before=selected;detach_native=TRUE;CHECK(send_message(w,WM_PAINT,0,0)==41 && selected==before && instance.detached && instance.calls==1);
    CHECK(ReleaseDC(w,borrowed) && DestroyWindow(w));style_instance_release(&instance);CHECK(disposed==1);detach_native=FALSE;
    w=create(module);CHECK(w);before=selected;destroy_native=TRUE;CHECK(send_message(w,WM_PAINT,0,0)==41 && selected==before && instance.detached && !IsWindow(w) && instance.calls==1);
    style_instance_release(&instance);CHECK(disposed==2);destroy_native=FALSE;
    w=create(module);CHECK(w);detach_render=TRUE;CHECK(send_message(w,WM_PAINT,0,0)==41 && instance.detached && !bad && instance.calls==1);
    CHECK(DestroyWindow(w));style_instance_release(&instance);CHECK(disposed==3);CHECK(UnregisterClassW(cls.lpszClassName,module));
    puts("OWNER_OVERLAY PASS native_first=1 input=1 paint_dc=1 print_borrow=1 frame=1 flags=1 retired=1 destroyed=1 reentry=1 cleanup=1");return 0;
}
