#include "lib-style/common/control_scope.h"
#include <stdio.h>

static style_control_scope *scopes;
static HHOOK hook;
static LRESULT CALLBACK subclass(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{(void)id;(void)data;return DefSubclassProc(window,message,wp,lp);}
static unsigned created;
static void created_and_end(style_control_scope *scope,const CWPRETSTRUCT *message,void *context)
{
    (void)message;(void)context;
    ++created;
    if(style_control_scope_end(scope,&scopes,&hook))*(int *)style_control_scope_data(scope)=17;
}
static LRESULT CALLBACK observer(int code,WPARAM wp,LPARAM lp)
{return CallNextHookEx(NULL,code,wp,lp);}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"scope line %d error %lu\n",__LINE__,(unsigned long)GetLastError());return 1;}}while(0)
int main(void)
{
    HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,50,50,NULL,NULL,NULL,NULL);CHECK(root);
    HWND child=CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,10,10,root,NULL,NULL,NULL);CHECK(child);
    int owner=1,other=2;
    CHECK(SetWindowSubclass(child,subclass,9,(DWORD_PTR)&owner));
    CHECK(style_control_subclass_owned(child,subclass,9,&owner) &&
          !style_control_subclass_owned(child,subclass,9,&other) &&
          !style_control_subclass_owned(child,subclass,10,&owner) &&
          !style_control_subclass_owned(NULL,subclass,9,&owner));
    style_control_subclass_remove_owned(child,subclass,9,&other);
    CHECK(style_control_subclass_owned(child,subclass,9,&owner));
    style_control_subclass_remove_owned(child,subclass,9,&owner);
    CHECK(!style_control_subclass_owned(child,subclass,9,&owner));
    style_control_scope *outer=style_control_scope_begin(root,sizeof(int),&scopes,&hook,observer);CHECK(outer && hook);
    *(int *)style_control_scope_data(outer)=42;
    style_control_scope *inner=style_control_scope_begin(root,sizeof(int),&scopes,&hook,observer);CHECK(inner && hook);
    CHECK(style_control_scope_find(scopes,child)==inner && style_control_scope_contains(scopes,outer));
    CHECK(!style_control_scope_end((style_control_scope *)(UINT_PTR)1,&scopes,&hook) && GetLastError()==ERROR_INVALID_PARAMETER);
    CWPRETSTRUCT message={.hwnd=child,.message=WM_CREATE,.lResult=-1};
    style_control_scope_observe(scopes,&message,created_and_end,NULL);CHECK(!created && style_control_scope_find(scopes,child)==inner);
    message.lResult=0;style_control_scope_observe(scopes,&message,created_and_end,NULL);
    CHECK(created==1 && hook && style_control_scope_find(scopes,child)==outer);
    CHECK(*(int *)style_control_scope_data(outer)==42);
    message.hwnd=root;message.message=WM_NCDESTROY;
    style_control_scope_observe(scopes,&message,created_and_end,NULL);CHECK(!style_control_scope_find(scopes,child));
    CHECK(style_control_scope_end(outer,&scopes,&hook) && !scopes && !hook);
    CHECK(!style_control_scope_begin(NULL,0,&scopes,&hook,observer) && GetLastError()==ERROR_INVALID_THREAD_ID);
    DestroyWindow(root);puts("CONTROL_SCOPE_COMMON_OK nested=1 retained=1 root=1 invalid=1");return 0;
}
