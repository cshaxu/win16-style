#include <windows.h>
#include <stdio.h>
#include "lib-style/uwp/host.h"

#define CHECK(x) do{if(!(x)){fprintf(stderr,"UWP HOST FAIL line=%d error=%lu\n",__LINE__,(unsigned long)GetLastError());return 1;}}while(0)
typedef struct fixture {style_uwp_host host;unsigned before,after,disposed,refreshed,messages;BOOL order,detach_on_refresh;} fixture;
static BOOL surface_message(void *context,HWND w,UINT m,WPARAM a,LPARAM b,LRESULT *result)
{
    fixture *f=context;(void)w;(void)a;(void)b;
    if(m==WM_APP){*result=44;return TRUE;}
    if(m==WM_APP+1){*result=style_uwp_host_detach(&f->host) && !f->disposed?45:0;return TRUE;}
    return FALSE;
}
static void before(void *context)
{
    fixture *f=context;DWORD_PTR data;
    ++f->before;
    f->order=f->host.detached && !GetWindowSubclass(f->host.owner,style_uwp_host_owner_proc,(UINT_PTR)style_uwp_host_owner_proc,&data) &&
        GetWindowSubclass(f->host.title,style_uwp_host_title_proc,(UINT_PTR)style_uwp_host_title_proc,&data) && style_uwp_host_detach(&f->host);
}
static void after(void *context)
{
    fixture *f=context;DWORD_PTR data;
    ++f->after;
    f->order=f->order && !GetWindowSubclass(f->host.title,style_uwp_host_title_proc,(UINT_PTR)style_uwp_host_title_proc,&data) &&
        GetWindowSubclass(f->host.surface,style_uwp_host_surface_proc,(UINT_PTR)style_uwp_host_surface_proc,&data);
}
static void dispose(void *context){++((fixture *)context)->disposed;}
static BOOL owner_message(void *context,HWND w,UINT message,WPARAM wp,LPARAM lp,LRESULT *result)
{
    fixture *f=context;(void)w;(void)wp;(void)lp;++f->messages;
    if(message==WM_APP){*result=42;return TRUE;}
    if(message==WM_APP+1){*result=style_uwp_host_detach(&f->host) && !f->disposed?43:0;return TRUE;}
    return FALSE;
}
static BOOL refresh(void *context)
{fixture *f=context;++f->refreshed;if(f->detach_on_refresh)style_uwp_host_detach(&f->host);return TRUE;}
int main(void)
{
    HWND owner=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,10,10,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(owner);
    HWND title=CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,40,20,owner,NULL,GetModuleHandleW(NULL),NULL);
    HWND surface=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,40,20,owner,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(title && surface);
    fixture f={.host={.owner=owner,.title=title,.surface=surface,
        .surface_message=surface_message,.owner_message=owner_message,.before_title_remove=before,.after_title_remove=after,.dispose=dispose,.refresh=refresh,.calls=1}};
    f.host.context=&f;
    CHECK(!style_uwp_host_detach(&f.host) && !f.host.detached && !f.before && !f.after);
    CHECK(SetWindowSubclass(owner,style_uwp_host_owner_proc,(UINT_PTR)style_uwp_host_owner_proc,(DWORD_PTR)&f.host));
    CHECK(SetWindowSubclass(title,style_uwp_host_title_proc,(UINT_PTR)style_uwp_host_title_proc,(DWORD_PTR)&f.host));
    CHECK(SetWindowSubclass(surface,style_uwp_host_surface_proc,(UINT_PTR)style_uwp_host_surface_proc,(DWORD_PTR)&f.host));
    CHECK(SendMessageW(surface,WM_APP,0,0)==44 && f.host.calls==1);
    CHECK(SendMessageW(owner,WM_APP,0,0)==42 && f.messages==1);
    SendMessageW(owner,WM_SIZE,0,0);CHECK(f.refreshed==1 && f.messages==2 && f.host.calls==1);
    SendMessageW(title,WM_SHOWWINDOW,TRUE,0);CHECK(f.refreshed==2 && f.host.calls==1);
    style_uwp_host_retain(&f.host);
    f.detach_on_refresh=TRUE;SendMessageW(title,WM_SHOWWINDOW,TRUE,0);
    CHECK(f.host.detached && f.refreshed==3 && f.order && f.before==1 && f.after==1 && !IsWindow(surface));
    CHECK(style_uwp_host_detach(&f.host) && f.before==1 && f.after==1);
    style_uwp_host_release(&f.host);CHECK(!f.disposed);
    style_uwp_host_release(&f.host);CHECK(f.disposed==1);
    CHECK(DestroyWindow(owner));
    owner=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,10,10,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(owner);
    title=CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,40,20,owner,NULL,GetModuleHandleW(NULL),NULL);
    surface=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,40,20,owner,NULL,GetModuleHandleW(NULL),NULL);
    CHECK(title && surface);
    fixture second={.host={.owner=owner,.title=title,.surface=surface,.surface_message=surface_message,
        .owner_message=owner_message,.before_title_remove=before,.after_title_remove=after,.dispose=dispose,.refresh=refresh}};
    second.host.context=&second;
    CHECK(SetWindowSubclass(owner,style_uwp_host_owner_proc,(UINT_PTR)style_uwp_host_owner_proc,(DWORD_PTR)&second.host));
    CHECK(SetWindowSubclass(title,style_uwp_host_title_proc,(UINT_PTR)style_uwp_host_title_proc,(DWORD_PTR)&second.host));
    CHECK(SetWindowSubclass(surface,style_uwp_host_surface_proc,(UINT_PTR)style_uwp_host_surface_proc,(DWORD_PTR)&second.host));
    CHECK(SendMessageW(owner,WM_APP+1,0,0)==43 && second.order && second.host.detached && second.disposed==1 && !IsWindow(surface));
    CHECK(DestroyWindow(owner));
    for(unsigned i=0;i<2;++i){
        owner=CreateWindowW(L"STATIC",L"",WS_OVERLAPPEDWINDOW,10,10,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);
        CHECK(owner);
        surface=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,40,20,owner,NULL,GetModuleHandleW(NULL),NULL);
        CHECK(surface);
        fixture f3={.host={.owner=owner,.surface=surface,.surface_message=surface_message,.dispose=dispose,.calls=1}};
        f3.host.context=&f3;
        CHECK(SetWindowSubclass(owner,style_uwp_host_owner_proc,(UINT_PTR)style_uwp_host_owner_proc,(DWORD_PTR)&f3.host));
        CHECK(SetWindowSubclass(surface,style_uwp_host_surface_proc,(UINT_PTR)style_uwp_host_surface_proc,(DWORD_PTR)&f3.host));
        if(i==0)CHECK(SendMessageW(surface,WM_APP+1,0,0)==45);
        else CHECK(DestroyWindow(surface));
        CHECK(f3.host.detached && !f3.host.surface && !f3.disposed && !IsWindow(surface));
        style_uwp_host_release(&f3.host);CHECK(f3.disposed==1);
        CHECK(DestroyWindow(owner));
    }
    puts("UWP HOST PASS owner_failure=1 ordering=1 title_reentry=1 owner_reentry=1 title_refresh=1 surface=1 surface_reentry=1 surface_destroy=1 deferred_dispose=1");return 0;
}
