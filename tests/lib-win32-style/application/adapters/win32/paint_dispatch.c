#include "lib-win32-style/application/adapters/win32/message.h"
#include <stdio.h>
#include "../../../../ntw16s-dll/exception_probe.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"RUNTIME_PAINT FAIL line=%d error=%lu kind=%d action=%u child=%d/%u bad=%u refs=%u/%u clean=%d expr=%s\n",__LINE__,GetLastError(),first.kind,first.action,second.kind,second.action,bad,first.instance.calls,second.instance.calls,clean(),#x);dump();return 1;}}while(0)
typedef struct fixture {
    style_instance instance;style_control_runtime runtime;HWND window;HDC dc;style_runtime_dispatch kind;
    unsigned action,calls,normal,disposed;BOOL witnessed;
    style_runtime_text_scope text;style_control_status_scope status,*status_prior;style_control_spin_scope spin,*spin_prior;
    style_control_native_overlay overlay;HRGN last_region;DRAWITEMSTRUCT item;
} fixture;
static fixture first,second;static style_runtime_paint_manager manager;
static style_control_status_scope initial_status;static style_control_spin_scope initial_spin;
static unsigned bad,seen[9],guards,prepares;static BOOL transactions;
static void dispose(void *context){++((fixture *)context)->disposed;}
static BOOL owned(HWND w,void *context){return ((fixture *)context)->window==w;}
static BOOL include_text(HWND w,UINT m){(void)w;(void)m;return TRUE;}
static style_runtime_text_role role(HWND w,void *context){(void)context;return (style_runtime_text_role){w,FALSE};}
static const style_runtime_text_ops text_ops={NULL,include_text,role};
static BOOL clean(void)
{GdiFlush();return !manager.paint && !manager.cool && !manager.box && !manager.bit && manager.lookup_popup==second.window && manager.status==&initial_status && manager.spin==&initial_spin && manager.spin_stack==&initial_spin && !style_runtime_text_current() && !first.spin.buffers && !second.spin.buffers && !first.overlay.clip.region && !second.overlay.clip.region && (!first.last_region || !GetObjectType(first.last_region)) && (!second.last_region || !GetObjectType(second.last_region));}
static void dump(void)
{
    HRGN baseline=CreateRectRgn(0,0,10,10);BOOL deleted=DeleteObject(baseline);DWORD type=GetObjectType(baseline);GdiFlush();
    fprintf(stderr,"PAINT_DIAG heads=%p,%p,%p,%p lookup=%d status=%d spin=%d stack=%d text=%p heap=%p,%p region=%p,%p last_type=%lu,%lu native_deleted=%d type=%lu flushed=%lu\n",(void *)manager.paint,(void *)manager.cool,(void *)manager.box,(void *)manager.bit,manager.lookup_popup==second.window,manager.status==&initial_status,manager.spin==&initial_spin,manager.spin_stack==&initial_spin,(void *)style_runtime_text_current(),(void *)first.spin.buffers,(void *)second.spin.buffers,(void *)first.overlay.clip.region,(void *)second.overlay.clip.region,GetObjectType(first.last_region),GetObjectType(second.last_region),deleted,type,GetObjectType(baseline));
}
static LRESULT CALLBACK native(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_PAINT){
        fixture *f=w==first.window?&first:&second;++f->calls;++seen[f->kind];
        if(f->kind==STYLE_RUNTIME_DISPATCH_STATIC && (style_runtime_paint_eligible(&f->instance,&f->runtime,m)?(manager.paint==NULL || manager.paint->observation.owner!=f):manager.paint!=NULL))++bad;
        if(f->kind==STYLE_RUNTIME_DISPATCH_COOL){if(!manager.cool || manager.cool->observation.owner!=f)++bad;else {manager.cool->ours=TRUE;manager.cool->observation.native=DefWindowProcW;}}
        if(f->kind==STYLE_RUNTIME_DISPATCH_BOX){if(!manager.box || manager.box->observation.owner!=f)++bad;else manager.box->witnessed=TRUE;}
        if(f->kind==STYLE_RUNTIME_DISPATCH_LOOKUP && manager.lookup_popup!=w)++bad;
        if(f->kind==STYLE_RUNTIME_DISPATCH_BIT && (!manager.bit || manager.bit->observation.owner!=f || manager.bit->observation.window!=f->item.hwndItem || manager.bit->dc!=f->dc || manager.bit->state!=ODS_SELECTED || !EqualRect(&manager.bit->item,&f->item.rcItem)))++bad;
        if(f->kind==STYLE_RUNTIME_DISPATCH_SPIN && (manager.spin!=&f->spin || manager.spin_stack!=&f->spin))++bad;
        if(f->action==3){if(!style_instance_detach(&f->instance))++bad;style_instance_release(&f->instance);RaiseException(NTW16S_TEST_EXCEPTION_CODE,0,0,NULL);}
        if(f->action==1)RaiseException(NTW16S_TEST_EXCEPTION_CODE,0,0,NULL);
        if(f->action==2){SendMessageW(second.window,WM_PAINT,0,0);if(f->kind==STYLE_RUNTIME_DISPATCH_STATIC && (!manager.paint || manager.paint->observation.owner!=f))++bad;}
        SetLastError(0x4567);return 73;
    }
    return DefWindowProcW(w,m,a,b);
}
static void transaction_guard(const style_runtime_observation *o,UINT message)
{
    (void)message;++guards;
    if(o->instance->calls!=2 || o->owner!=(o->window==first.window?(void *)&first:(void *)&second))++bad;
    SetLastError(0x2345);
}
static void transaction_prepare(const style_runtime_observation *o,UINT message,WPARAM p,style_runtime_text_scope *text)
{
    (void)p;++prepares;
    if(manager.spin || o->instance->calls!=2)++bad;
    style_runtime_text_begin(text,o->window,message,o->owner,&text_ops);
    SetLastError(0x3456);
}
static LRESULT CALLBACK observe(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;fixture *f=(void *)data;
    if(m!=WM_PAINT)return DefSubclassProc(w,m,a,b);
    if(transactions){
        const style_runtime_observation observation={.instance=&f->instance,.runtime=&f->runtime,.owner=f,.window=w,.current=owned};
        const style_runtime_message_ops ops={.guard=transaction_guard,.prepare=transaction_prepare,.status_class=L"TStatusBar"};
        return style_runtime_message_dispatch(&manager,&observation,m,a,b,&ops);
    }
    style_instance_retain(&f->instance);f->witnessed=FALSE;f->status_prior=manager.status;f->spin_prior=manager.spin;
    if(f->kind!=STYLE_RUNTIME_DISPATCH_BIT){
        style_runtime_text_begin(&f->text,w,m,f,&text_ops);f->text.native_dc=f->dc;
        f->status=(style_control_status_scope){.window=w};manager.status=&f->status;if(f->spin_prior)manager.spin=NULL;
        if(f->kind==STYLE_RUNTIME_DISPATCH_SPIN){
            f->spin=(style_control_spin_scope){.previous_stack=manager.spin_stack};f->spin.buffers=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*f->spin.buffers));if(!f->spin.buffers)++bad;
            manager.spin=&f->spin;manager.spin_stack=&f->spin;
        }
        if(f->kind==STYLE_RUNTIME_DISPATCH_COLOR){f->overlay=(style_control_native_overlay){.clip={CreateRectRgn(0,0,10,10),1}};f->last_region=f->overlay.clip.region;if(!f->overlay.clip.region)++bad;}
    }
    f->item=(DRAWITEMSTRUCT){.hwndItem=w,.hDC=f->dc,.rcItem={0,0,10,10},.itemState=ODS_SELECTED};
    const style_runtime_observation observation={.instance=&f->instance,.runtime=&f->runtime,.owner=f,.window=w,.current=owned};
    const style_runtime_paint_cleanup cleanup={&f->text,f->status_prior,f->spin_prior,&f->spin,&f->overlay,&f->witnessed,&f->item};
    LRESULT result=style_runtime_paint_dispatch(&manager,&observation,f->kind,w,m,a,b,&cleanup);DWORD error=GetLastError();
    if(f->kind!=STYLE_RUNTIME_DISPATCH_BIT){
        style_runtime_text_end(&f->text);manager.status=f->status_prior;
        if(f->kind==STYLE_RUNTIME_DISPATCH_SPIN){manager.spin_stack=f->spin.previous_stack;style_control_spin_buffers_clear(&f->spin);}
        manager.spin=f->spin_prior;style_control_native_overlay_dispose(&f->overlay);
    }
    style_instance_release(&f->instance);++f->normal;SetLastError(error);return result;
}
static BOOL create(fixture *f,HINSTANCE module)
{
    f->window=CreateWindowW(L"RuntimePaintContract",L"",WS_POPUP,0,0,100,100,NULL,NULL,module,NULL);f->dc=CreateCompatibleDC(NULL);f->runtime.roles.text.candidate=TRUE;
    style_instance_init(&f->instance,f,dispose);style_instance_retain(&f->instance);
    return f->window && f->dc && style_instance_attach(&f->instance,f->window,observe,NULL);
}
int main(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);WNDCLASSW cls={.lpfnWndProc=native,.hInstance=module,.lpszClassName=L"RuntimePaintContract"};CHECK(RegisterClassW(&cls) && create(&first,module) && create(&second,module));
    manager.lookup_popup=second.window;manager.status=&initial_status;manager.spin=manager.spin_stack=&initial_spin;
    style_control_runtime r={0};style_instance instance={0};CHECK(style_runtime_dispatch_kind(NULL,&instance,&r,WM_PAINT,0,0,0,0,0,0)==STYLE_RUNTIME_DISPATCH_NATIVE);
    r.roles.text.candidate=TRUE;CHECK(style_runtime_paint_eligible(&instance,&r,WM_PAINT));instance.detached=TRUE;CHECK(!style_runtime_paint_eligible(&instance,&r,WM_PAINT));instance.detached=FALSE;
    r.spin.modern=TRUE;CHECK(style_runtime_dispatch_kind(NULL,&instance,&r,WM_PRINTCLIENT,0,0,1,1,1,0)==STYLE_RUNTIME_DISPATCH_STATUS);CHECK(style_runtime_dispatch_kind(NULL,&instance,&r,WM_PRINTCLIENT,0,0,0,1,1,0)==STYLE_RUNTIME_DISPATCH_COLOR);
    r.cool.candidate=TRUE;CHECK(style_runtime_dispatch_kind(NULL,&instance,&r,WM_PAINT,0,0,1,1,1,0)==STYLE_RUNTIME_DISPATCH_COOL);r.cool.candidate=FALSE;CHECK(style_runtime_dispatch_kind(NULL,&instance,&r,WM_USER,0,0,0,0,0,1)==STYLE_RUNTIME_DISPATCH_STATIC);
    for(unsigned kind=1;kind<=STYLE_RUNTIME_DISPATCH_BIT;++kind){
        first.kind=(style_runtime_dispatch)kind;first.action=0;SetLastError(0x1234);CHECK(SendMessageW(first.window,WM_PAINT,0,0)==73 && GetLastError()==0x4567 && clean() && !bad && first.instance.calls==1);
        if(kind==STYLE_RUNTIME_DISPATCH_BOX)CHECK(first.witnessed);
        first.action=1;CHECK(test_unwind(first.window,WM_PAINT,0,0) && clean() && !bad && first.instance.calls==1);
        first.action=2;second.kind=(style_runtime_dispatch)kind;second.action=0;CHECK(SendMessageW(first.window,WM_PAINT,0,0)==73 && clean() && !bad && first.instance.calls==1 && second.instance.calls==1);
        second.action=1;CHECK(test_unwind(first.window,WM_PAINT,0,0) && clean() && !bad && first.instance.calls==1 && second.instance.calls==1);
    }
    first.kind=STYLE_RUNTIME_DISPATCH_STATIC;first.action=2;
    for(unsigned kind=1;kind<=STYLE_RUNTIME_DISPATCH_BIT;++kind){second.kind=(style_runtime_dispatch)kind;second.action=1;CHECK(test_unwind(first.window,WM_PAINT,0,0) && clean() && !bad && first.instance.calls==1 && second.instance.calls==1);}
    second.kind=STYLE_RUNTIME_DISPATCH_STATIC;second.action=0;second.runtime.roles.text.candidate=FALSE;CHECK(SendMessageW(first.window,WM_PAINT,0,0)==73 && clean() && !bad);second.runtime.roles.text.candidate=TRUE;
    first.kind=STYLE_RUNTIME_DISPATCH_NATIVE;first.action=0;CHECK(SendMessageW(first.window,WM_PAINT,0,0)==73 && clean() && !bad);
    for(unsigned kind=1;kind<=STYLE_RUNTIME_DISPATCH_BIT;++kind)CHECK(seen[kind]>0);
    transactions=TRUE;first.kind=STYLE_RUNTIME_DISPATCH_STATIC;first.action=0;
    first.runtime.status.present=TRUE;SetLastError(0x1234);
    CHECK(SendMessageW(first.window,WM_PAINT,0,0)==73 && GetLastError()==0x4567 && clean() && !bad && first.instance.calls==1);
    first.action=1;CHECK(test_unwind(first.window,WM_PAINT,0,0) && clean() && !bad && first.instance.calls==1);
    first.action=2;second.kind=STYLE_RUNTIME_DISPATCH_COOL;second.runtime.cool.candidate=TRUE;second.action=0;
    CHECK(SendMessageW(first.window,WM_PAINT,0,0)==73 && clean() && !bad && first.instance.calls==1 && second.instance.calls==1);
    second.action=1;CHECK(test_unwind(first.window,WM_PAINT,0,0) && clean() && !bad && first.instance.calls==1 && second.instance.calls==1);
    first.runtime.status.present=FALSE;CHECK(guards==prepares && guards>=6);
    first.kind=STYLE_RUNTIME_DISPATCH_STATIC;first.action=2;second.kind=STYLE_RUNTIME_DISPATCH_COOL;second.action=3;
    CHECK(test_unwind(first.window,WM_PAINT,0,0) && clean() && !bad && first.instance.calls==1 && !second.instance.calls && second.disposed==1);
    CHECK(style_instance_detach(&first.instance));style_instance_release(&first.instance);
    CHECK(first.disposed==1 && second.disposed==1 && DeleteDC(first.dc) && DeleteDC(second.dc) && DestroyWindow(first.window) && DestroyWindow(second.window) && UnregisterClassW(cls.lpszClassName,module));
    puts("RUNTIME_PAINT PASS variants=8 native_subclass=1 nested=1 native_unwind=1 nested_unwind=1 no_double_release=1 last_reference=1 static_barrier=1 bit_payload=1 spin_heap=1 color_region=1 native_error=1 message_transaction=1 prepare_order=1 no_style=1 cleanup=1");return 0;
}
