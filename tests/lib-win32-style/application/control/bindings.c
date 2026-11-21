#include "lib-win32-style/application/control/bindings.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"BINDINGS FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct fixture {style_instance instance;style_control_state state,child;style_control_bindings bindings;HWND window,list;unsigned disposed;} fixture;
static fixture f;
static BOOL cancelled,detach_set,child_success;static unsigned query_mode,queries,normalized,sets,bad;
static char events[16];static unsigned count;
static void event(char code){if(count<sizeof(events))events[count++]=code;else ++bad;}
static void dispose(void *context){++((fixture *)context)->disposed;}
static HFONT source(HFONT font){++normalized;return font;}
static HFONT acquire(HWND w,HFONT font,BOOL native,unsigned recipe){(void)recipe;(void)w;if(!native)++bad;return font;}
static void font_noop(HFONT font){(void)font;}
static const style_control_fonts fonts={source,acquire,font_noop,font_noop};
static LRESULT CALLBACK passive(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{(void)id;(void)data;return DefSubclassProc(w,m,a,b);}
static LRESULT CALLBACK toolbar(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{(void)id;(void)data;return DefSubclassProc(w,m,a,b);}
static LRESULT CALLBACK observe(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;
    if(m==WM_GETFONT){++queries;if(query_mode==1)cancelled=TRUE;if(query_mode==2 && !SetWindowSubclass(w,passive,(UINT_PTR)passive,0))++bad;return (LRESULT)GetStockObject(DEFAULT_GUI_FONT);}
    if(m==WM_SETFONT){++sets;event('F');if(detach_set && !style_instance_detach(&f.instance))++bad;}
    if(m==WM_SETREDRAW)event('R');
    return DefSubclassProc(w,m,a,b);
}
static DWORD WINAPI foreign(void *context)
{HWND w=context;return !style_instance_preflight(w,passive) && GetLastError()==ERROR_INVALID_PARAMETER?0:1;}
static style_control_state *lookup(HWND w){return w==f.list?&f.child:NULL;}
static BOOL remove_list(HWND w){if(w!=f.list)++bad;event('L');return FALSE;}
static BOOL terminal_list(HWND w){if(w!=f.list)++bad;event('l');return child_success;}
static void theme_remove(HWND w,const void *context)
{
    const fixture *s=context;DWORD_PTR value;
    if(w!=s->window || s->bindings.list_theme_owned || s->bindings.combo.list || s->bindings.updown.window ||
       GetWindowSubclass(GetParent(w),passive,(UINT_PTR)w,&value) || GetWindowSubclass(GetParent(w),toolbar,(UINT_PTR)w,&value))++bad;
    event('T');
}
static BOOL theme_terminal(HWND w,const void *context)
{
    const fixture *s=context;DWORD_PTR value;
    if(w!=s->window || s->bindings.list_theme_owned || s->bindings.combo.list!=s->list || s->bindings.updown.window ||
       GetWindowSubclass(GetParent(w),passive,(UINT_PTR)w,&value) || GetWindowSubclass(GetParent(w),toolbar,(UINT_PTR)w,&value))++bad;
    event('t');return FALSE;
}
static BOOL pin(void *context){(void)context;return TRUE;}
static void unpin(void *context){(void)context;}
static BOOL supported(DWORD style,DWORD ex){(void)style;(void)ex;return TRUE;}
static BOOL paint(HWND w,void *context,HDC dc,const RECT *r,const RECT *clip,unsigned direction,BOOL pressed,BOOL disabled)
{(void)w;(void)context;(void)dc;(void)r;(void)clip;(void)direction;(void)pressed;(void)disabled;return TRUE;}
static BOOL resolve(HDC dc,HWND *w){(void)dc;*w=NULL;return FALSE;}
static BOOL prepare(HWND w)
{
    f.window=w;style_instance_init(&f.instance,&f,dispose);style_instance_retain(&f.instance);style_control_state_init(&f.state,&fonts);style_control_bindings_init(&f.bindings);
    return style_instance_attach(&f.instance,w,passive,NULL);
}
typedef struct theme_case {
    style_instance instance;style_control_state state;style_control_bindings bindings;
    HWND window;BOOL retained,cancelled,capture_ok,enable_ok;unsigned retire_stage,disposed;
} theme_case;
static theme_case *theming;
static void theme_dispose(void *context){++((theme_case *)context)->disposed;}
static BOOL theme_retained(HWND w)
{if(w!=theming->window || theming->bindings.list_theme_owned)++bad;event('R');SetLastError(0x51);return theming->retained;}
static BOOL theme_capture(HWND w)
{
    if(w!=theming->window || theming->bindings.list_theme_owned || theming->instance.calls!=1)++bad;
    event('C');if(theming->retire_stage==1 && !style_instance_detach(&theming->instance))++bad;
    SetLastError(0x52);return theming->capture_ok;
}
static BOOL theme_enable(HWND w,const void *owner)
{
    if(w!=theming->window || owner!=theming || !theming->bindings.list_theme_owned || theming->instance.calls!=1)++bad;
    event('E');if(theming->retire_stage==2 && !style_instance_detach(&theming->instance))++bad;
    SetLastError(0x53);return theming->enable_ok;
}
static void theme_discard(HWND w,const void *owner)
{if(w!=theming->window || owner!=theming || theming->bindings.list_theme_owned)++bad;event('D');}
static int theme_cases(HWND parent)
{
    static const struct {
        BOOL list,retained,cancelled,capture_ok,enable_ok;unsigned retire_stage;
        BOOL owned,result;DWORD error;const char *order;
    } cases[]={
        {TRUE,FALSE,FALSE,TRUE,TRUE,0,TRUE,TRUE,0x53,"RCE"},
        {TRUE,TRUE,FALSE,TRUE,TRUE,0,FALSE,TRUE,0x51,"R"},
        {TRUE,FALSE,FALSE,FALSE,TRUE,0,FALSE,FALSE,0x52,"RC"},
        {TRUE,FALSE,FALSE,TRUE,FALSE,0,TRUE,FALSE,0x53,"RCE"},
        {TRUE,FALSE,FALSE,TRUE,TRUE,1,TRUE,FALSE,ERROR_CANCELLED,"RCE"},
        {TRUE,FALSE,FALSE,TRUE,TRUE,2,TRUE,FALSE,ERROR_CANCELLED,"RCE"},
        {TRUE,FALSE,TRUE,TRUE,TRUE,0,TRUE,FALSE,ERROR_CANCELLED,"RCE"},
        {TRUE,TRUE,TRUE,TRUE,TRUE,0,FALSE,FALSE,ERROR_CANCELLED,"R"},
        {FALSE,FALSE,FALSE,TRUE,TRUE,0,FALSE,TRUE,0,""},
        {FALSE,FALSE,TRUE,TRUE,TRUE,0,FALSE,FALSE,ERROR_CANCELLED,""},
        {TRUE,FALSE,TRUE,FALSE,TRUE,0,FALSE,FALSE,ERROR_CANCELLED,"RC"},
        {TRUE,FALSE,TRUE,TRUE,FALSE,0,TRUE,FALSE,ERROR_CANCELLED,"RCE"}
    };
    for(unsigned i=0;i<ARRAYSIZE(cases);++i){
        theme_case c={.retained=cases[i].retained,.cancelled=cases[i].cancelled,
            .capture_ok=cases[i].capture_ok,.enable_ok=cases[i].enable_ok,.retire_stage=cases[i].retire_stage};
        c.window=CreateWindowW(cases[i].list?L"LISTBOX":L"STATIC",L"",WS_CHILD,0,0,80,30,parent,NULL,GetModuleHandleW(NULL),NULL);CHECK(c.window);
        style_instance_init(&c.instance,&c,theme_dispose);style_instance_retain(&c.instance);
        style_control_state_init(&c.state,&fonts);style_control_bindings_init(&c.bindings);
        CHECK(style_instance_attach(&c.instance,c.window,passive,NULL));theming=&c;count=0;SetLastError(0x50);
        const style_control_list_theme_ops theme={&c,&c.cancelled,theme_retained,theme_capture,theme_enable};
        const style_control_setup_options options={.frame_bars=TRUE,.list_theme=&theme};
        BOOL result=style_control_bindings_setup(c.window,&c.instance,&c.state,&c.bindings,&options);DWORD error=GetLastError();
        CHECK(result==cases[i].result && c.bindings.list_theme_owned==cases[i].owned && c.instance.calls==1 && !bad);
        CHECK(count==strlen(cases[i].order) && !memcmp(events,cases[i].order,count) && (!cases[i].error || error==cases[i].error));
        CHECK(style_instance_detach(&c.instance));
        const style_control_binding_ops ops={.context=&c,.theme_remove=theme_discard};unsigned before=count;
        CHECK(style_control_bindings_release(c.window,&c.instance,&c.state,&c.bindings,FALSE,FALSE,&ops).result);
        CHECK(count==before+(unsigned)cases[i].owned && !c.bindings.list_theme_owned && !bad);
        style_control_bindings_dispose(&c.bindings,&c.state);style_control_state_dispose(&c.state);
        style_instance_release(&c.instance);CHECK(c.disposed==1 && DestroyWindow(c.window));
    }
    theming=NULL;return 0;
}
int main(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,240,120,NULL,NULL,module,NULL);CHECK(parent);
    HWND w=CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,100,40,parent,NULL,module,NULL);CHECK(w && SetWindowSubclass(w,observe,9,0));
    CHECK(style_instance_preflight(w,passive));HANDLE thread=CreateThread(NULL,0,foreign,w,0,NULL);CHECK(thread && WaitForSingleObject(thread,2000)==WAIT_OBJECT_0);DWORD code=1;CHECK(GetExitCodeThread(thread,&code) && !code && CloseHandle(thread));
    style_control_state state;style_control_state_init(&state,&fonts);CHECK(style_control_font_read(w,&state,passive,&cancelled) && queries==1 && normalized==1 && state.original_font==GetStockObject(DEFAULT_GUI_FONT));
    query_mode=1;CHECK(!style_control_font_read(w,&state,passive,&cancelled) && GetLastError()==ERROR_CANCELLED && normalized==1);cancelled=FALSE;
    query_mode=2;CHECK(!style_control_font_read(w,&state,passive,NULL) && GetLastError()==ERROR_ALREADY_EXISTS && normalized==1);
    CHECK(!style_instance_preflight(w,passive) && GetLastError()==ERROR_ALREADY_EXISTS);CHECK(RemoveWindowSubclass(w,passive,(UINT_PTR)passive));query_mode=0;style_control_state_dispose(&state);
    CHECK(prepare(w));f.state.original_font=GetStockObject(DEFAULT_GUI_FONT);const style_control_setup_options options={0};count=0;
    CHECK(style_control_bindings_setup(w,&f.instance,&f.state,&f.bindings,&options) && sets==1 && count==1 && events[0]=='F');
    detach_set=TRUE;CHECK(!style_control_bindings_setup(w,&f.instance,&f.state,&f.bindings,&options) && GetLastError()==ERROR_CANCELLED && f.instance.detached && !bad);detach_set=FALSE;
    style_control_bindings_dispose(&f.bindings,&f.state);style_control_state_dispose(&f.state);style_instance_release(&f.instance);CHECK(f.disposed==1);
    for(unsigned terminal=0;terminal<3;++terminal){
        child_success=terminal==2;CHECK(prepare(w));f.list=CreateWindowW(L"LISTBOX",L"",WS_CHILD,0,50,80,30,parent,NULL,module,NULL);CHECK(f.list);
        f.child=(style_control_state){.owner=&f.instance};f.bindings.combo.list=f.list;f.bindings.list_theme_owned=TRUE;
        CHECK(style_control_notify_bind(w,&f.bindings.track,passive,&f) && style_control_notify_bind(w,&f.bindings.toolbar,toolbar,&f));
        const style_control_updown_ops updown={NULL,supported,paint};style_control_updown_register(&f.bindings.updown,w,module,&f,pin,unpin,&updown);
        f.state.original_font=GetStockObject(DEFAULT_GUI_FONT);f.state.font=f.state.original_font;f.state.suppression=1;f.bindings.scroll.track.active=TRUE;
        f.bindings.scroll.resources.timer=SetTimer(w,101,5000,NULL);CHECK(f.bindings.scroll.resources.timer);
        CHECK(style_instance_detach(&f.instance));count=0;sets=0;
        const style_control_binding_ops ops={&f,passive,toolbar,lookup,remove_list,terminal_list,theme_remove,theme_terminal};
        style_control_retired result=style_control_bindings_release(w,&f.instance,&f.state,&f.bindings,terminal,FALSE,&ops);
        CHECK(result.result==(!terminal || child_success) && result.colors_changed==!terminal && !bad && !f.bindings.combo.list && !f.bindings.updown.window && !f.bindings.track.parent && !f.bindings.toolbar.parent);
        if(terminal){CHECK(count==2 && !memcmp(events,"tl",2) && !sets && f.bindings.scroll.track.active);}
        else CHECK(count==4 && !memcmp(events,"LTFR",4) && sets==1 && !f.bindings.scroll.track.active);
        CHECK(!f.bindings.scroll.resources.timer);HDC dc=GetDC(w);RECT rect={0,0,17,17};CHECK(dc && !style_control_updown_draw_frame(dc,&rect,DFC_SCROLL,0,(void *)(UINT_PTR)main,resolve) && ReleaseDC(w,dc));
        style_control_bindings_dispose(&f.bindings,&f.state);style_control_state_dispose(&f.state);style_instance_release(&f.instance);CHECK(DestroyWindow(f.list));
    }
    CHECK(!theme_cases(parent));
    CHECK(f.disposed==4 && !bad && RemoveWindowSubclass(w,observe,9) && DestroyWindow(parent));
    puts("BINDINGS PASS native_setup=1 list_theme=12 lease_release=1 cancel_phase=1 detach=1 font_reentry=1 cancel=1 preflight=1 zero_data=1 thread=1 owned_notify=1 release_order=1 terminal=1 child_failure=1 timer=1 no_style=1 cleanup=1");return 0;
}
