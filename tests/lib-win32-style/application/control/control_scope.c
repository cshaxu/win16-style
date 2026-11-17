#include "lib-win32-style/application/control/control_scope.h"
#include <stdio.h>

static style_control_scope_manager manager;
static LRESULT CALLBACK subclass(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{(void)id;(void)data;return DefSubclassProc(window,message,wp,lp);}
static unsigned created;
static BOOL created_and_end(style_control_scope *scope,const CWPRETSTRUCT *message,BOOL edit,void *context)
{
    (void)message;(void)edit;(void)context;
    ++created;
    if(style_control_scope_end(scope,NULL,&manager))*(int *)style_control_scope_data(scope)=17;
    return TRUE;
}
static unsigned install_calls,install_mode;
static BOOL counted_install(style_control_scope *scope,const CWPRETSTRUCT *message,BOOL edit,void *context)
{
    (void)scope;(void)message;(void)context;
    if(edit)return FALSE;
    ++install_calls;
    if(install_mode){SetLastError(install_mode==1?ERROR_NOT_SUPPORTED:ERROR_ACCESS_DENIED);return FALSE;}
    return TRUE;
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
    style_control_scope *outer=style_control_scope_begin(root,sizeof(int),&manager,observer);CHECK(outer && manager.hook);
    *(int *)style_control_scope_data(outer)=42;
    style_control_scope *inner=style_control_scope_begin(root,sizeof(int),&manager,observer);CHECK(inner && manager.hook);
    CHECK(style_control_scope_find(&manager,child)==inner && style_control_scope_contains(&manager,outer));
    CHECK(!style_control_scope_end((style_control_scope *)(UINT_PTR)1,NULL,&manager) && GetLastError()==ERROR_INVALID_PARAMETER);
    CWPRETSTRUCT message={.hwnd=child,.message=WM_CREATE,.lResult=-1};
    style_control_scope_observe(&manager,&message,created_and_end,NULL);CHECK(!created && style_control_scope_find(&manager,child)==inner);
    message.lResult=0;style_control_scope_observe(&manager,&message,created_and_end,NULL);
    CHECK(created==1 && manager.hook && style_control_scope_find(&manager,child)==outer);
    CHECK(*(int *)style_control_scope_data(outer)==42);
    message.hwnd=root;message.message=WM_NCDESTROY;
    style_control_scope_observe(&manager,&message,created_and_end,NULL);CHECK(!style_control_scope_find(&manager,child));
    CHECK(style_control_scope_end(outer,NULL,&manager) && !manager.scopes && !manager.hook);
    CHECK(!style_control_scope_begin(NULL,0,&manager,observer) && GetLastError()==ERROR_INVALID_THREAD_ID);
    outer=style_control_scope_begin(root,0,&manager,observer);CHECK(outer);
    message=(CWPRETSTRUCT){.hwnd=child,.message=WM_CREATE};
    for(install_mode=0;install_mode<3;++install_mode)
        style_control_scope_observe(&manager,&message,counted_install,NULL);
    HWND combo=CreateWindowW(L"COMBOBOX",L"",WS_CHILD|CBS_DROPDOWN,0,0,40,40,root,NULL,NULL,NULL);CHECK(combo);
    COMBOBOXINFO info={.cbSize=sizeof(info)};CHECK(GetComboBoxInfo(combo,&info) && info.hwndItem!=combo);
    message.hwnd=info.hwndItem;
    style_control_scope_observe(&manager,&message,counted_install,NULL);CHECK(install_calls==3);
    message.hwnd=root;style_control_scope_observe(&manager,&message,counted_install,NULL);CHECK(install_calls==3);
    style_control_scope_counts counts={0};
    CHECK(style_control_scope_end(outer,&counts,&manager) && !manager.scopes && !manager.hook);
    CHECK(counts.installed==1 && counts.unsupported==1 && counts.failed==1);
    DestroyWindow(root);puts("CONTROL_SCOPE_COMMON_OK nested=1 retained=1 root=1 invalid=1 counts=1 combo_internal=1");return 0;
}
