#include "lib-win32-style/application/lifecycle/instance.h"
#include <stdio.h>

#define CHECK(x) do { if(!(x)){fprintf(stderr,"INSTANCE_FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;} } while(0)
typedef struct fixture {
    style_instance instance;
    struct fixture *successor;
    unsigned disposed;
    BOOL nested_alive;
} fixture;

static void dispose(void *context){++((fixture *)context)->disposed;}
static LRESULT CALLBACK callback(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{
    fixture *f=(fixture *)data;(void)id;style_instance_retain(&f->instance);
    LRESULT result=0;
    if(message==WM_APP){
        result=SendMessageW(window,WM_APP+1,0,0);
        f->nested_alive=f->instance.detached && !f->disposed && f->instance.calls==2;
    }else if(message==WM_APP+1){
        if(style_instance_detach(&f->instance) &&
            style_instance_attach(&f->successor->instance,window,callback,f))result=42;
    }else {
        if(message==WM_NCDESTROY)style_instance_detach(&f->instance);
        result=DefSubclassProc(window,message,wp,lp);
    }
    style_instance_release(&f->instance);return result;
}

int main(void)
{
    HWND window=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,100,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(window);
    fixture first={0},second={0};
    style_instance_init(&first.instance,&first,dispose);
    style_instance_init(&second.instance,&second,dispose);
    first.successor=&second;style_instance_retain(&first.instance);
    CHECK(style_instance_attach(&first.instance,window,callback,&first));
    CHECK(style_instance_lookup(window,callback)==&first.instance && first.instance.owner==&first);
    CHECK(style_instance_acquire_owned(&first.instance) && first.instance.calls==2);
    style_instance_return_owned(&first.instance);CHECK(first.instance.calls==1);
    CHECK(!style_instance_attach(&second.instance,window,callback,NULL) && GetLastError()==ERROR_ALREADY_EXISTS);
    CHECK(style_instance_lookup(window,callback)==&first.instance);
    CHECK(SendMessageW(window,WM_APP,0,0)==42 && first.nested_alive && !first.disposed);
    CHECK(style_instance_lookup(window,callback)==&second.instance && second.instance.owner==&first);
    CHECK(!style_instance_acquire_owned(&first.instance) && first.instance.calls==1);
    CHECK(style_instance_detach(&first.instance)); /* Must not remove its successor. */
    CHECK(style_instance_lookup(window,callback)==&second.instance);
    style_instance_release(&first.instance);CHECK(first.disposed==1);
    CHECK(style_instance_acquire_owned(&second.instance) && second.instance.calls==1);
    CHECK(DestroyWindow(window) && !second.disposed && !style_instance_acquire_owned(&second.instance));
    style_instance_return_owned(&second.instance);CHECK(second.disposed==1);

    window=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,100,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(window);
    fixture failed={0},foreign={0};
    style_instance_init(&failed.instance,&failed,dispose);
    style_instance_init(&foreign.instance,&foreign,dispose);
    style_instance_retain(&failed.instance);
    CHECK(style_instance_attach(&failed.instance,window,callback,NULL));
    CHECK(SetWindowSubclass(window,callback,(UINT_PTR)callback,(DWORD_PTR)&foreign));
    CHECK(!style_instance_detach(&failed.instance) && GetLastError()==ERROR_INVALID_STATE && !failed.instance.detached);
    CHECK(style_instance_lookup(window,callback)==&foreign.instance);
    CHECK(!style_instance_acquire_owned(&failed.instance) && failed.instance.calls==1);
    CHECK(RemoveWindowSubclass(window,callback,(UINT_PTR)callback));
    CHECK(!style_instance_detach(&failed.instance) && !failed.instance.detached);
    style_instance_release(&failed.instance);CHECK(!failed.disposed);
    CHECK(style_instance_attach(&failed.instance,window,callback,NULL));
    style_instance_retain(&failed.instance);
    CHECK(style_instance_detach(&failed.instance));
    style_instance_release(&failed.instance);CHECK(failed.disposed==1);
    CHECK(DestroyWindow(window));
    CHECK(!style_instance_attach(&foreign.instance,NULL,callback,NULL) && !foreign.instance.detached && !foreign.disposed);
    puts("INSTANCE_OK identity=1 nested=1 successor=1 detach_failure=1 retry=1 terminal=1 owned_lease=1 replaced_identity=1 final_release=1");
    return 0;
}
