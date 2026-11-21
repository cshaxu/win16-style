#include "lib-win32-style/application/control/message.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CONTROL_MESSAGE FAIL line=%d error=%lu bad=%u refs=%u,%u leases=%u,%u\n",__LINE__,GetLastError(),bad,fixtures[0].instance.calls,fixtures[1].instance.calls,fixtures[0].leases,fixtures[1].leases);return 1;}}while(0)
typedef struct fixture {
    style_instance instance;style_control_state state;style_control_bindings bindings;
    HWND window;HFONT font;unsigned leases,phase,prepares,dispatches,native_calls,disposed,mode;BOOL held,valid;
} fixture;
static fixture fixtures[2];static unsigned bad;
static fixture *from_window(HWND w){for(unsigned i=0;i<2;++i)if(fixtures[i].window==w)return &fixtures[i];return NULL;}
static fixture *from_font(HFONT font){for(unsigned i=0;i<2;++i)if(fixtures[i].font==font)return &fixtures[i];return NULL;}
static void retain(HFONT font){if(font){fixture *f=from_font(font);if(f)++f->leases;else ++bad;}}
static void release(HFONT font){if(font){fixture *f=from_font(font);if(!f || !f->leases)++bad;else --f->leases;}}
static HFONT source(HFONT font){return font;}
static HFONT acquire(HWND w,HFONT font,BOOL native,unsigned recipe){(void)recipe;(void)w;(void)native;retain(font);return font;}
static const style_control_fonts fonts={source,acquire,retain,release};
static void dispose(void *owner)
{
    fixture *f=owner;++f->disposed;style_control_bindings_dispose(&f->bindings,&f->state);style_control_state_dispose(&f->state);
    if(f->leases || !DeleteObject(f->font))++bad;
}
static BOOL valid(HWND w){fixture *f=from_window(w);if(!f || f->instance.calls!=2)++bad;return f && f->valid;}
static BOOL no(HWND w){(void)w;return FALSE;}
static BOOL prepare(HWND w,void *owner,style_control_message *call)
{
    fixture *f=owner;++f->prepares;
    if(f!=from_window(w) || f->phase || f->instance.calls!=2 || f->leases!=2 || call->tab || call->header || call->progress || call->native_font)++bad;
    f->phase=2;call->previous=call->outbound=f->font;retain(f->font);retain(f->font);return f->mode!=1;
}
static LRESULT dispatch(HWND w,void *owner,style_control_message *call)
{
    fixture *f=owner;++f->dispatches;if(f!=from_window(w) || f->phase!=2 || f->leases!=4)++bad;f->phase=3;
    if(f->mode==3){if(SendMessageW(fixtures[1].window,WM_APP+1,0,0)!=73 || f->instance.calls!=2 || f->leases!=4)++bad;}
    if(f->mode==4){call->message=WM_APP+2;call->parameter=777;call->value=888;}
    LRESULT result=DefSubclassProc(w,call->message,call->parameter,call->value);
    if(f->mode==2){if(!style_instance_detach(&f->instance))++bad;style_instance_release(&f->instance);f->held=FALSE;}
    return result;
}
static style_control_state *lookup(HWND w){fixture *f=from_window(w);return f?&f->state:NULL;}
static LRESULT CALLBACK observer(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;fixture *f=(fixture *)data;if(m!=WM_APP+1 && m!=WM_APP+2)return DefSubclassProc(w,m,a,b);
    f->phase=0;const style_control_message_ops ops={f,{valid,no,no,no,no,no},prepare,dispatch,lookup};
    return style_control_message_dispatch(w,&f->instance,&f->state,&f->bindings,m,a,b,&ops);
}
static LRESULT CALLBACK native(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_APP+1 || m==WM_APP+2){fixture *f=from_window(w);++f->native_calls;
        if(f->instance.calls!=2 || (f->valid?f->phase!=3:f->phase!=0))++bad;
        if(m==WM_APP+2 && (a!=777 || b!=888))++bad;
        SetLastError(0x4567);return m==WM_APP+1?73:88;}
    return DefWindowProcW(w,m,a,b);
}
static BOOL create(fixture *f,HINSTANCE module)
{
    f->window=CreateWindowW(L"ControlMessageContract",L"",WS_POPUP,0,0,100,100,NULL,NULL,module,NULL);f->font=CreateFontW(-12,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Arial");
    if(!f->window || !f->font)return FALSE;
    f->held=f->valid=TRUE;retain(f->font);style_instance_init(&f->instance,f,dispose);style_instance_retain(&f->instance);style_control_state_init(&f->state,&fonts);f->state.font=f->font;style_control_bindings_init(&f->bindings);
    return style_instance_attach(&f->instance,f->window,observer,NULL);
}
int main(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);WNDCLASSW cls={.hInstance=module,.lpfnWndProc=native,.lpszClassName=L"ControlMessageContract"};
    CHECK(RegisterClassW(&cls) && create(&fixtures[0],module) && create(&fixtures[1],module) && fixtures[0].font!=fixtures[1].font);
    fixture *f=&fixtures[0];SetLastError(0x1234);CHECK(SendMessageW(f->window,WM_APP+1,0,0)==73 && GetLastError()==0x4567 && f->instance.calls==1 && f->leases==1 && !bad);
    f->mode=1;unsigned dispatched=f->dispatches,natives=f->native_calls;CHECK(!SendMessageW(f->window,WM_APP+1,0,0) && f->dispatches==dispatched && f->native_calls==natives && f->instance.calls==1 && f->leases==1 && !bad);
    f->mode=0;f->valid=FALSE;unsigned prepared=f->prepares;CHECK(SendMessageW(f->window,WM_APP+1,0,0)==73 && f->prepares==prepared && f->dispatches==dispatched && f->leases==1 && f->instance.calls==1 && !bad);f->valid=TRUE;
    f->mode=3;CHECK(SendMessageW(f->window,WM_APP+1,0,0)==73 && f->leases==1 && fixtures[1].leases==1 && f->instance.calls==1 && fixtures[1].instance.calls==1 && !bad);
    f->mode=4;CHECK(SendMessageW(f->window,WM_APP+1,0,0)==88 && f->leases==1 && f->instance.calls==1 && !bad);
    f->mode=2;CHECK(SendMessageW(f->window,WM_APP+1,0,0)==73 && f->disposed==1 && !f->leases && !f->instance.calls && !bad);
    f=&fixtures[1];CHECK(style_instance_detach(&f->instance));style_instance_release(&f->instance);f->held=FALSE;
    CHECK(f->disposed==1 && !f->leases && !f->instance.calls && DestroyWindow(fixtures[0].window) && DestroyWindow(fixtures[1].window) && UnregisterClassW(cls.lpszClassName,module) && !bad);
    puts("CONTROL_MESSAGE PASS native=1 error=1 guard=1 prepare_failure=1 leases=1 nested=1 rewritten_args=1 reentrant_detach=1 last_release=1 cleanup=1 no_style=1");return 0;
}
