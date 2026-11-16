#include "lib-win32-style/application/window/messages.h"
#include <stdio.h>
#include <string.h>

typedef struct fixture {
    style_instance instance;
    style_native_frame native;
    style_frame_message_ops ops;
    unsigned paints,fonts,disposed;
    BOOL deny_client,remove_in_text,test_native;
    DWORD observed_error;
    LRESULT native_flags;
    char order[16];unsigned order_size;
} fixture;
static void record(fixture *s,char value)
{if(s->order_size+1<sizeof(s->order)){s->order[s->order_size++]=value;s->order[s->order_size]=0;}}
static void dispose(void *context){++((fixture *)context)->disposed;}
static BOOL client(HWND window,void *context,const RECT *outer,RECT *result)
{
    (void)window;fixture *s=context;record(s,'M');
    if(s->deny_client)return FALSE;
    *result=(RECT){4,20,outer->right-outer->left-4,outer->bottom-outer->top-4};return TRUE;
}
static void paint(HWND window,void *context,HDC dc)
{(void)window;(void)dc;++((fixture *)context)->paints;SetLastError(999);}
static BOOL font(HWND window,void *context)
{(void)window;++((fixture *)context)->fonts;SetLastError(998);return TRUE;}
static BOOL hit(HWND window,void *context,LPARAM point,LRESULT *result)
{(void)window;(void)context;(void)point;*result=HTCAPTION;return TRUE;}
static LRESULT CALLBACK native(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    fixture *s=(fixture *)GetWindowLongPtrW(window,GWLP_USERDATA);
    if(s && s->test_native){
        if(message==WM_NCCALCSIZE){record(s,'N');return s->native_flags;}
        if(message==WM_SETTEXT){
            if(s->remove_in_text)style_instance_detach(&s->instance);
            SetLastError(734);return 123;
        }
        if(message==WM_DPICHANGED){SetLastError(734);return 321;}
    }
    return DefWindowProcW(window,message,wp,lp);
}
static LRESULT CALLBACK procedure(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR ref)
{
    (void)id;fixture *s=(fixture *)ref;style_instance_retain(&s->instance);LRESULT result=0;
    if(message==WM_NCDESTROY){style_instance_detach(&s->instance);result=DefSubclassProc(window,message,wp,lp);}
    else if(!style_frame_message(&s->instance,&s->native,message,wp,lp,&s->ops,&result))
        result=DefSubclassProc(window,message,wp,lp);
    DWORD error=GetLastError();s->observed_error=error;style_instance_release(&s->instance);SetLastError(error);return result;
}
#define CHECK(test) do {if(!(test)){failure=__LINE__;goto done;}} while(0)
int main(void)
{
    int failure=0;fixture s={0};HWND window=NULL;HDC dc=NULL;
    HINSTANCE module=GetModuleHandleW(NULL);
    WNDCLASSW cls={.hInstance=module,.lpfnWndProc=native,.lpszClassName=L"SharedFrameMessages"};
    CHECK(RegisterClassW(&cls));
    window=CreateWindowW(cls.lpszClassName,L"Messages",WS_OVERLAPPEDWINDOW,50,50,300,200,NULL,NULL,module,NULL);
    CHECK(window);SetWindowLongPtrW(window,GWLP_USERDATA,(LONG_PTR)&s);
    style_instance_init(&s.instance,&s,dispose);
    s.ops=(style_frame_message_ops){.context=&s,.client=client,.paint=paint,.buffered=paint,.hit_test=hit,.refresh_font=font};
    CHECK(style_instance_attach(&s.instance,window,procedure,NULL));s.test_native=TRUE;
    NCCALCSIZE_PARAMS parameters={.rgrc={{30,50,330,250}}};
    s.native_flags=WVR_VALIDRECTS|WVR_REDRAW;
    CHECK(SendMessageW(window,WM_NCCALCSIZE,TRUE,(LPARAM)&parameters)==0);
    RECT expected={34,70,326,246};CHECK(EqualRect(&expected,&parameters.rgrc[0]) && !strcmp(s.order,"MN"));
    parameters.rgrc[0]=(RECT){30,50,330,250};s.order_size=0;s.order[0]=0;s.native_flags=WVR_REDRAW|WVR_ALIGNTOP;
    CHECK(SendMessageW(window,WM_NCCALCSIZE,TRUE,(LPARAM)&parameters)==WVR_REDRAW);
    CHECK(EqualRect(&expected,&parameters.rgrc[0]) && !strcmp(s.order,"MN"));
    s.deny_client=TRUE;parameters.rgrc[0]=(RECT){30,50,330,250};expected=parameters.rgrc[0];
    CHECK(SendMessageW(window,WM_NCCALCSIZE,TRUE,(LPARAM)&parameters)==s.native_flags && EqualRect(&expected,&parameters.rgrc[0]));
    s.deny_client=FALSE;
    CHECK(SendMessageW(window,WM_NCHITTEST,0,0)==HTCAPTION);
    unsigned before=s.paints;CHECK(SendMessageW(window,WM_NCACTIVATE,TRUE,0)==TRUE && s.native.active && s.paints==before+1);
    before=s.paints;SendMessageW(window,WM_PRINT,0,PRF_CHECKVISIBLE|PRF_NONCLIENT);CHECK(s.paints==before);
    dc=CreateCompatibleDC(NULL);CHECK(dc);
    SendMessageW(window,WM_PRINT,(WPARAM)dc,PRF_NONCLIENT);CHECK(s.paints==before+1);
    before=s.paints;CHECK(SendMessageW(window,WM_SETTEXT,0,(LPARAM)L"Changed")==123);
    CHECK(s.paints==before+1 && s.observed_error==734);
    RECT dpi={50,50,350,250};CHECK(SendMessageW(window,WM_DPICHANGED,MAKELPARAM(96,96),(LPARAM)&dpi)==321);
    CHECK(s.fonts==1 && s.observed_error==734);
    s.ops.refresh_font=NULL;before=s.paints;
    CHECK(SendMessageW(window,WM_DPICHANGED,MAKELPARAM(96,96),(LPARAM)&dpi)==321 && s.paints==before+1 && s.fonts==1 && s.observed_error==734);
    s.remove_in_text=TRUE;before=s.paints;
    CHECK(SendMessageW(window,WM_SETTEXT,0,(LPARAM)L"Detach in native callback")==123);
    CHECK(s.instance.detached && s.disposed==1 && s.paints==before && s.observed_error==734);
done:
    if(dc)DeleteDC(dc);
    if(window)DestroyWindow(window);
    UnregisterClassW(cls.lpszClassName,module);
    printf("SHARED FRAME MESSAGES bits=%u failure=%d paint=%u font=%u disposed=%u\n",(unsigned)(sizeof(void *)*8),failure,s.paints,s.fonts,s.disposed);
    return failure?1:0;
}
