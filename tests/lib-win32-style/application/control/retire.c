#include "lib-win32-style/application/control/retire.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"RETIRE line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct fixture {style_instance instance;style_control_state state;BOOL recursive,destroy,fail_child;} fixture;
static unsigned disposed,finished,released,terminal_releases,colors;
static BOOL pinned,ordered,nested_ok;
static const style_control_retire_ops operations;
static void dispose(void *context)
{fixture *f=context;++disposed;style_control_state_dispose(&f->state);free(f);}
static void finish(HWND window,void *context)
{
    fixture *f=context;++finished;
    if(f->recursive)nested_ok=style_control_retire(window,&f->instance,&f->state,FALSE,f,&operations);
    if(f->destroy)DestroyWindow(window);
    pinned=f->instance.calls>0;
}
static style_control_retired release(HWND w,void *context,BOOL terminal)
{
    (void)w;fixture *f=context;++released;if(terminal)++terminal_releases;
    pinned=f->instance.calls>0;
    return (style_control_retired){!f->fail_child,!terminal};
}
static const style_control_retire_ops operations={finish,release};
static LRESULT CALLBACK procedure(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;fixture *f=(fixture *)data;style_instance_retain(&f->instance);
    if(m==WM_NCDESTROY)style_control_retire(w,&f->instance,&f->state,TRUE,f,&operations);
    LRESULT result=DefSubclassProc(w,m,a,b);style_instance_release(&f->instance);return result;
}
static LRESULT CALLBACK owner(HWND w,UINT m,WPARAM a,LPARAM b)
{if(m==WM_SYSCOLORCHANGE){++colors;ordered=disposed>0;}return DefWindowProcW(w,m,a,b);}
static fixture *attach(HWND w)
{
    fixture *f=calloc(1,sizeof(*f));if(!f)return NULL;
    style_instance_init(&f->instance,f,dispose);
    if(!style_instance_attach(&f->instance,w,procedure,NULL)){free(f);return NULL;}return f;
}
int main(void)
{
    WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"PublicControlRetire",.lpfnWndProc=owner};CHECK(RegisterClassW(&cls));
    HWND window=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,cls.hInstance,NULL);CHECK(window);
    fixture *f=attach(window);CHECK(f);f->recursive=TRUE;
    CHECK(style_control_retire(window,&f->instance,&f->state,FALSE,f,&operations));
    CHECK(disposed==1 && finished==1 && released==1 && nested_ok && pinned && colors==1 && ordered && !style_instance_lookup(window,procedure));
    f=attach(window);CHECK(f);CHECK(RemoveWindowSubclass(window,procedure,(UINT_PTR)procedure));fixture *replacement=attach(window);CHECK(replacement);
    unsigned prior=disposed,before=released;
    CHECK(!style_control_retire(window,&f->instance,&f->state,FALSE,f,&operations));
    CHECK(!f->instance.detached && !f->state.removing && !f->instance.calls && disposed==prior && released==before);
    CHECK(style_instance_lookup(window,procedure)==&replacement->instance);
    style_instance_retain(&f->instance);f->instance.detached=TRUE;style_instance_release(&f->instance);
    CHECK(style_control_retire(window,&replacement->instance,&replacement->state,FALSE,replacement,&operations));
    f=attach(window);CHECK(f);f->fail_child=TRUE;before=finished;
    CHECK(!style_control_retire(window,&f->instance,&f->state,TRUE,f,&operations));
    CHECK(!style_instance_lookup(window,procedure) && finished==before && terminal_releases==1);
    f=attach(window);CHECK(f);f->destroy=TRUE;prior=disposed;before=terminal_releases;
    CHECK(style_control_retire(window,&f->instance,&f->state,FALSE,f,&operations));
    CHECK(!IsWindow(window) && disposed==prior+1 && terminal_releases==before+1 && pinned);
    CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance));
    puts("RETIRE recursive removal, callback destruction, detach-failure ownership, terminal child failure and disposal-before-refresh PASS");return 0;
}
