#include "lib-win32-style/application/window/install.h"
#include <stdlib.h>
#include <stdio.h>

typedef struct fixture {style_instance instance;BOOL cancelled;unsigned mode;} fixture;
static unsigned allocated,disposed,setups,rollbacks;
static BOOL pinned;
static fixture *replacement;
static LRESULT CALLBACK procedure(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static fixture *create(unsigned mode)
{
    fixture *s=calloc(1,sizeof(*s));
    if(s){++allocated;s->mode=mode;}return s;
}
static void dispose(void *context)
{++disposed;free(context);SetLastError(666);}
static fixture *prepare(unsigned mode)
{fixture *s=create(mode);if(s)style_instance_init(&s->instance,s,dispose);return s;}
static BOOL setup(HWND window,void *context)
{
    fixture *s=context;++setups;unsigned before=disposed;
    if(s->mode==1){SetLastError(4501);return FALSE;}
    if(s->mode==2){DestroyWindow(window);pinned=disposed==before;return TRUE;}
    if(s->mode==3){
        s->cancelled=TRUE;
        if(!style_instance_detach(&s->instance))return FALSE;
        replacement=prepare(0);
        if(!replacement || !style_frame_install(&replacement->instance,window,procedure,NULL,NULL))return FALSE;
        pinned=disposed==before;
    }
    return TRUE;
}
static void rollback(HWND window,void *context)
{
    fixture *s=context;++rollbacks;
    pinned=s->instance.calls>0 && style_instance_lookup(window,procedure)==&s->instance;
    SetLastError(999);
}
static LRESULT CALLBACK procedure(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR ref)
{
    (void)id;style_instance *instance=(style_instance *)ref;style_instance_retain(instance);
    if(message==WM_NCDESTROY)style_instance_detach(instance);
    LRESULT result=DefSubclassProc(window,message,wp,lp);DWORD error=GetLastError();
    style_instance_release(instance);SetLastError(error);return result;
}
static void remove_instance(style_instance *instance)
{style_instance_retain(instance);style_instance_detach(instance);style_instance_release(instance);}
#define CHECK(test) do {if(!(test)){failure=__LINE__;goto done;}} while(0)
int main(void)
{
    int failure=0;HWND window=NULL;HINSTANCE module=GetModuleHandleW(NULL);
    WNDCLASSW cls={.hInstance=module,.lpfnWndProc=DefWindowProcW,.lpszClassName=L"SharedFrameInstall"};
    CHECK(RegisterClassW(&cls));
    window=CreateWindowW(cls.lpszClassName,L"Install transaction",WS_OVERLAPPEDWINDOW,20,20,200,100,NULL,NULL,module,NULL);CHECK(window);
    fixture *s=prepare(0);CHECK(s);
    const int owner=1;style_frame_install_ops ops={s,&s->cancelled,setup,rollback};
    CHECK(style_frame_install(&s->instance,window,procedure,&owner,&ops));
    CHECK(s->instance.owner==&owner && s->instance.calls==0 && setups==1 && allocated==1 && disposed==0);
    remove_instance(&s->instance);CHECK(allocated==disposed);
    s=prepare(0);CHECK(s);s->cancelled=TRUE;ops=(style_frame_install_ops){s,&s->cancelled,setup,rollback};
    CHECK(!style_frame_install(&s->instance,window,procedure,NULL,&ops) && GetLastError()==ERROR_CANCELLED && setups==1 && allocated==disposed);
    style_instance foreign={0};CHECK(SetWindowSubclass(window,procedure,(UINT_PTR)procedure,(DWORD_PTR)&foreign));
    s=prepare(0);CHECK(s);
    CHECK(!style_frame_install(&s->instance,window,procedure,NULL,NULL) && GetLastError()==ERROR_ALREADY_EXISTS && allocated==disposed);
    CHECK(RemoveWindowSubclass(window,procedure,(UINT_PTR)procedure));
    s=prepare(1);CHECK(s);ops=(style_frame_install_ops){s,&s->cancelled,setup,rollback};
    CHECK(!style_frame_install(&s->instance,window,procedure,NULL,&ops) && GetLastError()==4501 && pinned && rollbacks==1 && allocated==disposed);
    CHECK(!style_instance_lookup(window,procedure));
    s=prepare(2);CHECK(s);ops=(style_frame_install_ops){s,&s->cancelled,setup,rollback};pinned=FALSE;
    CHECK(!style_frame_install(&s->instance,window,procedure,NULL,&ops) && GetLastError()==ERROR_INVALID_WINDOW_HANDLE && pinned && allocated==disposed);
    window=NULL;
    window=CreateWindowW(cls.lpszClassName,L"Replacement",WS_OVERLAPPEDWINDOW,20,20,200,100,NULL,NULL,module,NULL);CHECK(window);
    s=prepare(3);CHECK(s);ops=(style_frame_install_ops){s,&s->cancelled,setup,rollback};pinned=FALSE;
    CHECK(!style_frame_install(&s->instance,window,procedure,NULL,&ops) && GetLastError()==ERROR_CANCELLED && pinned);
    CHECK(replacement && style_instance_lookup(window,procedure)==&replacement->instance && !replacement->instance.detached && allocated==disposed+1);
    remove_instance(&replacement->instance);replacement=NULL;CHECK(allocated==disposed);
done:
    if(window)DestroyWindow(window);
    UnregisterClassW(cls.lpszClassName,module);
    printf("SHARED FRAME INSTALL bits=%u failure=%d allocated=%u disposed=%u setup=%u rollback=%u\n",
        (unsigned)(sizeof(void *)*8),failure,allocated,disposed,setups,rollbacks);
    return failure?1:0;
}
