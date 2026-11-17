#include "lib-win32-style/application/control/report.h"
#include "lib-win32-style/application/control/control_scope.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"REPORT FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static style_instance instance;
static style_control_runtime runtime;
static unsigned mode,installs,removes,failed;
static LRESULT CALLBACK notify(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{(void)id;(void)data;return DefSubclassProc(w,m,a,b);}
static BOOL install(HWND w,BOOL tree,void *context,const BOOL *cancelled)
{
    (void)w;++installs;if(!tree || context!=&runtime || cancelled!=&instance.detached || !runtime.protocol || !runtime.report.binding.attempted)failed=1;
    if(mode==2)instance.detached=TRUE;
    return mode==0;
}
static BOOL remove_report(HWND w)
{(void)w;++removes;style_control_report_unbind(&runtime,notify,&runtime);style_control_runtime_dispose(&runtime);return TRUE;}
int main(void)
{
    INITCOMMONCONTROLSEX init={sizeof(init),ICC_TREEVIEW_CLASSES};CHECK(InitCommonControlsEx(&init));
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,300,200,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(parent);
    HWND tree=CreateWindowW(WC_TREEVIEWW,L"",WS_CHILD,0,0,250,150,parent,NULL,GetModuleHandleW(NULL),NULL);CHECK(tree);
    style_control_report_bind(tree,&instance,&runtime,notify,&runtime);
    CHECK(runtime.protocol && runtime.report.window==tree && runtime.report.parent==parent && runtime.report.module && runtime.report.tree && !installs);
    CHECK(style_control_subclass_owned(parent,notify,(UINT_PTR)tree,&runtime));
    style_control_report_unbind(&runtime,notify,&instance);CHECK(style_control_subclass_owned(parent,notify,(UINT_PTR)tree,&runtime));runtime.report.parent=parent;
    style_control_report_unbind(&runtime,notify,&runtime);CHECK(!runtime.report.parent && !style_control_subclass_owned(parent,notify,(UINT_PTR)tree,&runtime));
    style_control_runtime_dispose(&runtime);CHECK(DestroyWindow(tree));
    WNDCLASSEXW cls={.cbSize=sizeof(cls)};CHECK(GetClassInfoExW(NULL,WC_TREEVIEWW,&cls));
    cls.hInstance=GetModuleHandleW(NULL);cls.lpszClassName=L"TTreeView";cls.style&=~CS_GLOBALCLASS;CHECK(RegisterClassExW(&cls));
    const style_control_report_ops ops={&runtime,notify,install,remove_report};
    for(mode=0;mode<=3;++mode){
        instance=(style_instance){0};runtime=(style_control_runtime){.report={.binding={.candidate=TRUE,.parent=mode==3?NULL:parent,.witness=cls.hInstance},.tree_role=TRUE}};installs=removes=0;
        tree=CreateWindowW(cls.lpszClassName,L"",WS_CHILD,0,0,250,150,parent,NULL,cls.hInstance,NULL);CHECK(tree);
        style_control_report_settle(tree,WM_PRINTCLIENT,&instance,&runtime,&ops);CHECK(!runtime.report.binding.attempted && !runtime.protocol);
        SetLastError(0x5678);style_control_report_settle(tree,WM_PAINT,&instance,&runtime,&ops);CHECK(GetLastError()==0x5678 && !failed);
        CHECK(installs==(mode<3?1u:0u) && removes==(mode==1?1u:0u));
        CHECK(runtime.report.binding.installed==(mode==0));
        CHECK((runtime.protocol!=NULL)==(mode==0 || mode==2));
        if(mode==1 || mode==3)CHECK(!runtime.report.binding.attempted && !runtime.report.binding.witness);
        style_control_report_unbind(&runtime,notify,&runtime);style_control_runtime_dispose(&runtime);CHECK(DestroyWindow(tree));
    }
    CHECK(DestroyWindow(parent));CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance));
    puts("REPORT PASS native_no_style=1 parent_subclass=1 owner=1 runtime=1 failed_apply_cleanup=1 detached_apply_owner=1 new_witness=1 last_error=1 dispose=1");return 0;
}
