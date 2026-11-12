#include "lib-style/common/control_scope.h"
#include <stdio.h>

static style_control_scope *scopes;
static HHOOK hook;
static LRESULT CALLBACK observer(int code,WPARAM wp,LPARAM lp)
{return CallNextHookEx(NULL,code,wp,lp);}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"scope line %d error %lu\n",__LINE__,(unsigned long)GetLastError());return 1;}}while(0)
int main(void)
{
    HWND root=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,50,50,NULL,NULL,NULL,NULL);CHECK(root);
    HWND child=CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,10,10,root,NULL,NULL,NULL);CHECK(child);
    style_control_scope *outer=style_control_scope_begin(root,sizeof(int),&scopes,&hook,observer);CHECK(outer && hook);
    *(int *)style_control_scope_data(outer)=42;
    style_control_scope *inner=style_control_scope_begin(root,sizeof(int),&scopes,&hook,observer);CHECK(inner && hook);
    CHECK(style_control_scope_find(scopes,child)==inner && style_control_scope_contains(scopes,outer));
    CHECK(!style_control_scope_end((style_control_scope *)(UINT_PTR)1,&scopes,&hook) && GetLastError()==ERROR_INVALID_PARAMETER);
    style_control_scope_retain(inner);
    CHECK(style_control_scope_end(inner,&scopes,&hook) && hook && style_control_scope_find(scopes,child)==outer);
    CHECK(*(int *)style_control_scope_data(outer)==42);
    style_control_scope_release(inner);
    style_control_scope_root_destroyed(scopes,root);CHECK(!style_control_scope_find(scopes,child));
    CHECK(style_control_scope_end(outer,&scopes,&hook) && !scopes && !hook);
    CHECK(!style_control_scope_begin(NULL,0,&scopes,&hook,observer) && GetLastError()==ERROR_INVALID_THREAD_ID);
    DestroyWindow(root);puts("CONTROL_SCOPE_COMMON_OK nested=1 retained=1 root=1 invalid=1");return 0;
}
