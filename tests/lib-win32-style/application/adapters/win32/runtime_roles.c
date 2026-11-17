#include "lib-win32-style/application/adapters/win32/runtime_roles.h"
#include "lib-win32-style/application/control/runtime.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"RUNTIME_ROLES FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct node {style_instance instance;unsigned id;} node;
static unsigned released[4],release_count,removes;
static style_instance consumer;static BOOL fail_remove;
static void disposed(void *context){node *n=context;released[release_count++]=n->id;}
static LRESULT CALLBACK subclass(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{(void)id;(void)data;return DefSubclassProc(w,m,a,b);}
static LRESULT CALLBACK native(HWND w,UINT m,WPARAM a,LPARAM b){return DefWindowProcW(w,m,a,b);}
static BOOL host_attached(HWND w){return style_instance_lookup(w,subclass)!=NULL;}
static BOOL remove_role(HWND w){(void)w;++removes;if(fail_remove){SetLastError(0x72);return FALSE;}consumer.detached=TRUE;return TRUE;}
int main(void)
{
    HMODULE module=GetModuleHandleW(NULL);static const WCHAR *names[]={L"TValueListEditor",L"TInplaceEditList",L"TDBNavigator",L"TDBLookupComboBox",L"TPopupDataList"};
    for(unsigned i=0;i<ARRAYSIZE(names);++i){WNDCLASSW cls={.hInstance=module,.lpfnWndProc=native,.lpszClassName=names[i]};CHECK(RegisterClassW(&cls));}
    HWND parent=CreateWindowW(names[0],L"",WS_POPUP,0,0,300,200,NULL,NULL,module,NULL);CHECK(parent);
    HWND other=CreateWindowW(names[0],L"",WS_POPUP,0,0,300,200,NULL,NULL,module,NULL);CHECK(other);
    node popup={.id=1},mdi={.id=2};style_instance_init(&popup.instance,&popup,disposed);style_instance_init(&mdi.instance,&mdi,disposed);
    style_instance_retain(&popup.instance);style_instance_retain(&mdi.instance);
    style_runtime_references references={0};style_runtime_reference_init(&references.popup,TRUE,parent,parent,module,&popup.instance);style_runtime_reference_init(&references.mdi,TRUE,parent,other,module,&mdi.instance);
    CHECK(popup.instance.calls==2 && mdi.instance.calls==2);
    popup.instance.detached=mdi.instance.detached=TRUE;style_instance_release(&popup.instance);style_instance_release(&mdi.instance);CHECK(!release_count);
    /* Mirrors the product: retained links survive protocol/style retirement. */
    style_control_runtime runtime={0};runtime.roles.references=references;references=(style_runtime_references){0};
    style_control_runtime_dispose(&runtime);CHECK(!release_count && runtime.roles.references.popup.held==&popup.instance && runtime.roles.references.mdi.held==&mdi.instance);
    style_runtime_references transferred=runtime.roles.references;runtime.roles.references=(style_runtime_references){0};style_runtime_references_release(&transferred);
    CHECK(release_count==2 && released[0]==1 && released[1]==2 && !transferred.popup.held && !transferred.mdi.held);
    style_runtime_references_release(&transferred);CHECK(release_count==2);
    HWND source=CreateWindowExW(WS_EX_CLIENTEDGE,names[3],L"",WS_CHILD,0,0,200,40,parent,NULL,module,NULL);CHECK(source);
    HWND list=CreateWindowW(names[4],L"",WS_POPUP|WS_BORDER,0,0,200,100,parent,NULL,module,NULL);CHECK(list);
    node link={.id=3};style_instance_init(&link.instance,&link,disposed);style_instance_retain(&link.instance);CHECK(style_instance_attach(&link.instance,source,subclass,NULL));
    consumer=(style_instance){.procedure=subclass};style_runtime_reference_init(&references.popup,TRUE,parent,source,module,&link.instance);
    CHECK(style_runtime_reference_matches(list,&consumer,&references.popup,FALSE,host_attached));
    CHECK(style_instance_detach(&link.instance));CHECK(!style_runtime_reference_matches(list,&consumer,&references.popup,FALSE,host_attached));
    style_instance_release(&link.instance);CHECK(release_count==2 && link.instance.calls==1);
    node replacement={.id=4};style_instance_init(&replacement.instance,&replacement,disposed);style_instance_retain(&replacement.instance);
    CHECK(style_instance_attach(&replacement.instance,source,subclass,NULL));CHECK(!style_runtime_reference_matches(list,&consumer,&references.popup,FALSE,host_attached));
    CHECK(style_instance_detach(&replacement.instance));style_instance_release(&replacement.instance);CHECK(release_count==3 && released[2]==4 && link.instance.calls==1);
    style_runtime_references_release(&references);CHECK(release_count==4 && released[3]==3);
    CHECK(DestroyWindow(list) && DestroyWindow(source));
    for(unsigned role=0;role<2;++role){
        HWND child=CreateWindowW(names[role?2:1],L"",WS_CHILD,0,0,200,100,parent,NULL,module,NULL);CHECK(child);
        runtime=(style_control_runtime){0};consumer=(style_instance){.procedure=subclass};removes=0;
        style_runtime_child_state *state=role?&runtime.roles.nav:&runtime.roles.inplace;*state=(style_runtime_child_state){.candidate=TRUE,.owner=module,.parent=parent};
        style_runtime_roles_guard(child,WM_NULL,&consumer,&runtime,FALSE,host_attached,remove_role);CHECK(!removes);
        CHECK(SetParent(child,other));style_runtime_roles_guard(child,WM_NCDESTROY,&consumer,&runtime,FALSE,host_attached,remove_role);CHECK(!removes);
        style_runtime_roles_guard(child,WM_NULL,&consumer,&runtime,FALSE,host_attached,remove_role);CHECK(removes==1 && consumer.detached);
        style_runtime_roles_guard(child,WM_NULL,&consumer,&runtime,FALSE,host_attached,remove_role);CHECK(removes==1);
        CHECK(DestroyWindow(child));
    }
    HWND owned=CreateWindowW(names[1],L"",WS_CHILD,0,0,80,40,parent,NULL,module,NULL);CHECK(owned);
    runtime=(style_control_runtime){0};runtime.report.parent=parent;consumer=(style_instance){0};removes=0;
    style_runtime_roles_guard(owned,WM_NULL,&consumer,&runtime,TRUE,host_attached,remove_role);CHECK(!removes);
    CHECK(SetParent(owned,other));
    style_runtime_roles_guard(owned,WM_NULL,&consumer,&runtime,FALSE,host_attached,remove_role);CHECK(!removes);
    style_runtime_roles_guard(owned,WM_DESTROY,&consumer,&runtime,TRUE,host_attached,remove_role);CHECK(!removes);
    style_runtime_roles_guard(owned,WM_NCDESTROY,&consumer,&runtime,TRUE,host_attached,remove_role);CHECK(!removes);
    runtime.report.parent=NULL;style_runtime_roles_guard(owned,WM_NULL,&consumer,&runtime,TRUE,host_attached,remove_role);CHECK(!removes);
    runtime.report.parent=parent;runtime.roles.inplace=(style_runtime_child_state){.candidate=TRUE,.owner=module,.parent=parent};
    fail_remove=TRUE;style_runtime_roles_guard(owned,WM_NULL,&consumer,&runtime,TRUE,host_attached,remove_role);CHECK(removes==2 && !consumer.detached && GetLastError()==0x72);
    fail_remove=FALSE;removes=0;style_runtime_roles_guard(owned,WM_NULL,&consumer,&runtime,TRUE,host_attached,remove_role);CHECK(removes==1 && consumer.detached);
    style_runtime_roles_guard(owned,WM_NULL,&consumer,&runtime,TRUE,host_attached,remove_role);CHECK(removes==1 && DestroyWindow(owned));
    HWND scroll=CreateWindowW(names[1],L"",WS_CHILD,0,0,200,100,other,NULL,module,NULL);CHECK(scroll);
    runtime=(style_control_runtime){0};runtime.scroll.candidate=TRUE;runtime.scroll.parent=parent;consumer=(style_instance){0};removes=0;
    style_runtime_roles_guard(scroll,WM_NCDESTROY,&consumer,&runtime,FALSE,host_attached,remove_role);CHECK(removes==1);CHECK(DestroyWindow(scroll));
    CHECK(DestroyWindow(parent) && DestroyWindow(other));for(unsigned i=0;i<ARRAYSIZE(names);++i)CHECK(UnregisterClassW(names[i],module));
    puts("RUNTIME_ROLES PASS retained=1 release_order=1 late_release=1 transfer=1 related_identity=1 detach=1 parent=1 owner_parent=1 failed_remove_continues=1 terminal=1 no_style=1 cleanup=1");return 0;
}
