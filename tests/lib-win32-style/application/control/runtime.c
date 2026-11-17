#include "lib-win32-style/application/control/runtime.h"
#include <commctrl.h>
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"RUNTIME BINDING FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static style_instance instance;
static style_control_runtime runtime;
static unsigned mode,installs,removes,failed;
static BOOL reenter;
static style_control_runtime_ops operations;
static style_control_runtime_kind expected_kind=STYLE_RUNTIME_TAB;
static BOOL install(HWND w,style_control_runtime_kind kind,void *context,const BOOL *cancelled)
{
    (void)w;++installs;
    if(context!=&runtime || cancelled!=&instance.detached || kind!=expected_kind || !runtime.protocol || !(kind==STYLE_RUNTIME_EDIT?runtime.edit.attempted:runtime.tab.attempted))failed=1;
    if(mode==2)instance.detached=TRUE;
    SetLastError(0x1111);return mode==0;
}
static BOOL remove_style(HWND w,style_control_runtime_kind kind,const void *context)
{(void)w;(void)kind;if(context!=&runtime)failed=1;++removes;SetLastError(0x2222);return TRUE;}
static LRESULT CALLBACK query(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;LRESULT result=DefSubclassProc(w,m,a,b);
    if((m==TCM_GETITEMCOUNT || m==EM_GETLIMITTEXT) && reenter){reenter=FALSE;
        style_control_runtime_settle(w,WM_PAINT,&instance,&runtime,expected_kind,&operations);
        if(mode==3)instance.detached=TRUE;
    }
    return result;
}
int main(void)
{
    INITCOMMONCONTROLSEX init={sizeof(init),ICC_TAB_CLASSES};CHECK(InitCommonControlsEx(&init));
    WNDCLASSEXW cls={.cbSize=sizeof(cls)};CHECK(GetClassInfoExW(NULL,WC_TABCONTROLW,&cls));
    cls.hInstance=GetModuleHandleW(NULL);cls.lpszClassName=L"TPageControl";cls.style&=~CS_GLOBALCLASS;CHECK(RegisterClassExW(&cls));
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,300,200,NULL,NULL,cls.hInstance,NULL);CHECK(parent);
    operations=(style_control_runtime_ops){&runtime,install,remove_style};
    for(mode=0;mode<=4;++mode){
        instance=(style_instance){0};runtime=(style_control_runtime){.tab={.candidate=TRUE,.parent=mode==4?NULL:parent,.witness=cls.hInstance}};
        installs=removes=0;HWND tab=CreateWindowW(cls.lpszClassName,L"",WS_CHILD,0,0,250,150,parent,NULL,cls.hInstance,NULL);CHECK(tab);
        CHECK(SetWindowSubclass(tab,query,1,0));
        SetLastError(0x5678);style_control_runtime_settle(tab,WM_PRINTCLIENT,&instance,&runtime,STYLE_RUNTIME_TAB,&operations);
        CHECK(!runtime.tab.attempted && !runtime.protocol && !installs && GetLastError()==0x5678);
        reenter=TRUE;SetLastError(0x8765);style_control_runtime_settle(tab,WM_PAINT,&instance,&runtime,STYLE_RUNTIME_TAB,&operations);
        CHECK(runtime.tab.attempted && GetLastError()==0x8765 && !failed);
        CHECK(installs==(mode<3?1u:0u) && removes==(mode==1?1u:0u));
        CHECK(runtime.tab.installed==(mode==0));
        CHECK((runtime.protocol!=NULL)==(mode==0 || mode==2));
        CHECK((style_control_protocol_is(tab,WC_TABCONTROLW)!=0)==(mode==0 || mode==2));
        style_control_runtime_settle(tab,WM_PAINT,&instance,&runtime,STYLE_RUNTIME_TAB,&operations);CHECK(installs==(mode<3?1u:0u));
        style_control_runtime_dispose(&runtime);CHECK(!runtime.protocol && !style_control_protocol_is(tab,WC_TABCONTROLW));
        style_control_runtime_dispose(&runtime);CHECK(DestroyWindow(tab));
    }
    CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance));
    cls=(WNDCLASSEXW){.cbSize=sizeof(cls)};CHECK(GetClassInfoExW(NULL,L"EDIT",&cls));
    cls.hInstance=GetModuleHandleW(NULL);cls.lpszClassName=L"TEdit";cls.style&=~CS_GLOBALCLASS;CHECK(RegisterClassExW(&cls));
    expected_kind=STYLE_RUNTIME_EDIT;
    for(mode=0;mode<=3;++mode){
        instance=(style_instance){0};runtime=(style_control_runtime){.edit={.candidate=TRUE,.witness=cls.hInstance}};installs=removes=0;
        HWND edit=CreateWindowW(cls.lpszClassName,L"Text",WS_CHILD|WS_BORDER,0,0,200,30,parent,NULL,cls.hInstance,NULL);CHECK(edit);
        CHECK(SetWindowSubclass(edit,query,1,0));reenter=TRUE;SetLastError(0x7654);
        style_control_runtime_settle(edit,WM_PAINT,&instance,&runtime,STYLE_RUNTIME_EDIT,&operations);
        CHECK(GetLastError()==0x7654 && !failed && runtime.edit.attempted);
        CHECK(runtime.edit.installed==(mode==0) && installs==(mode<3?1u:0u) && removes==(mode==1?1u:0u));
        CHECK((runtime.protocol!=NULL)==(mode==0 || mode==2));
        style_control_runtime_dispose(&runtime);CHECK(DestroyWindow(edit));
    }
    instance=(style_instance){0};runtime=(style_control_runtime){.edit={.candidate=TRUE,.witness=cls.hInstance},.link=TRUE};
    HWND link=CreateWindowW(cls.lpszClassName,L"",WS_CHILD,0,0,100,25,parent,NULL,cls.hInstance,NULL);CHECK(link);
    style_control_runtime_settle(link,WM_PAINT,&instance,&runtime,STYLE_RUNTIME_EDIT,&operations);
    CHECK(!runtime.protocol && !runtime.edit.attempted && !runtime.edit.witness);
    CHECK(DestroyWindow(link));
    CHECK(DestroyWindow(parent));CHECK(UnregisterClassW(cls.lpszClassName,cls.hInstance));
    puts("RUNTIME BINDING PASS native_protocol=1 no_style=1 one_attempt=1 reentry=1 failed_apply_cleanup=1 detached_apply_owner=1 detached_bind=1 parent=1 edit=1 fresh_link_witness=1 last_error=1 dispose=1");return 0;
}
