#include "lib-win32-style/application/adapters/win32/message.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"REPORT_NOTIFY FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static style_instance instance;static style_control_runtime runtime;static style_runtime_paint_manager manager;
static style_runtime_report_notify_ops policy;
static HWND parent,child,other;static HDC dc;
static unsigned calls,prepares,bad,disposed;static BOOL expect_text,retire;static LRESULT app_result;
static void dispose(void *context){if(context!=&instance)++bad;++disposed;}
static BOOL identity(HWND window){return window==child;}
static BOOL include(HWND window,UINT message){return window==child && message==WM_PRINTCLIENT;}
static style_runtime_text_role role(HWND window,void *context){(void)context;return (style_runtime_text_role){window,FALSE};}
static void prepare(style_runtime_text_scope *scope,HWND window,void *owner)
{
    ++prepares;if(owner!=&instance || instance.calls!=2)++bad;
    const style_runtime_text_ops ops={NULL,include,role};style_runtime_text_begin(scope,window,WM_PRINTCLIENT,owner,&ops);
}
static LRESULT CALLBACK native(HWND window,UINT message,WPARAM a,LPARAM b)
{
    if(message==WM_NOTIFY || message==WM_DRAWITEM){
        HWND target=NULL;BOOL text=style_runtime_text_report_target(dc,&target);++calls;
        if(instance.calls!=2 || text!=expect_text || (text && target!=child))++bad;
        if(retire)instance.detached=TRUE;
        SetLastError(0x4567);return app_result;
    }
    return DefWindowProcW(window,message,a,b);
}
static LRESULT CALLBACK observe(HWND window,UINT message,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)data;const style_runtime_observation observation={.instance=&instance,.runtime=&runtime,.owner=&instance,.window=child};
    return style_runtime_report_notify(&manager,&observation,window,message,a,b,id,&policy);
}
int main(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);WNDCLASSW cls={.lpfnWndProc=native,.hInstance=module,.lpszClassName=L"ReportNotifyContract"};
    CHECK(RegisterClassW(&cls));parent=CreateWindowW(cls.lpszClassName,L"",WS_POPUP,0,0,240,160,NULL,NULL,module,NULL);CHECK(parent);
    child=CreateWindowW(L"LISTBOX",L"",WS_CHILD,0,0,120,80,parent,NULL,module,NULL);other=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,100,100,NULL,NULL,module,NULL);CHECK(child && other);
    dc=GetDC(child);CHECK(dc);style_instance_init(&instance,&instance,dispose);style_instance_retain(&instance);
    runtime.report.window=child;runtime.report.parent=parent;runtime.report.module=module;
    policy=(style_runtime_report_notify_ops){.notify=observe,.file_identity=identity,.button_identity=identity,.prepare_text=prepare};
    CHECK(SetWindowSubclass(parent,observe,37,0));
    style_runtime_text_scope outer;const style_runtime_text_ops ops={NULL,include,role};style_runtime_text_begin(&outer,child,WM_PRINTCLIENT,&instance,&ops);
    style_runtime_text_report_enable(&outer,child,module,NULL,identity);
    NMCUSTOMDRAW draw={.hdr={.hwndFrom=child,.code=NM_CUSTOMDRAW},.dwDrawStage=CDDS_PREPAINT,.hdc=dc};
    const DWORD stages[]={CDDS_PREPAINT,CDDS_ITEMPREPAINT,CDDS_ITEMPREPAINT|CDDS_SUBITEM};
    for(unsigned i=0;i<ARRAYSIZE(stages);++i){
        draw.dwDrawStage=stages[i];style_runtime_text_report_bind(child,dc);expect_text=FALSE;app_result=CDRF_DODEFAULT;SetLastError(0x1234);
        CHECK(SendMessageW(parent,WM_NOTIFY,0,(LPARAM)&draw)==app_result && GetLastError()==0x4567);
        HWND target=NULL;CHECK(style_runtime_text_report_target(dc,&target) && target==child && !bad && instance.calls==1);
    }
    app_result=CDRF_SKIPDEFAULT;style_runtime_text_report_bind(child,NULL);CHECK(SendMessageW(parent,WM_NOTIFY,0,(LPARAM)&draw)==app_result);
    HWND target=NULL;CHECK(!style_runtime_text_report_target(dc,&target));
    draw.dwDrawStage=CDDS_POSTPAINT;app_result=CDRF_DODEFAULT;CHECK(SendMessageW(parent,WM_NOTIFY,0,(LPARAM)&draw)==app_result && !style_runtime_text_report_target(dc,&target));
    DRAWITEMSTRUCT item={.CtlType=ODT_LISTBOX,.hwndItem=child,.hDC=dc,.itemID=0,.rcItem={0,0,100,20}};
    policy.file_owner=TRUE;expect_text=TRUE;CHECK(SendMessageW(parent,WM_DRAWITEM,0,(LPARAM)&item)==app_result && prepares==1 && !bad);
    CHECK(style_runtime_text_current()==&outer && !style_runtime_text_report_target(dc,&target));
    item.CtlType=ODT_BUTTON;expect_text=FALSE;CHECK(SendMessageW(parent,WM_DRAWITEM,0,(LPARAM)&item)==app_result && prepares==1 && !bad);
    policy.file_owner=FALSE;policy.button_owner=TRUE;expect_text=TRUE;CHECK(SendMessageW(parent,WM_DRAWITEM,0,(LPARAM)&item)==app_result && prepares==2 && !bad);
    CHECK(style_runtime_text_current()==&outer && instance.calls==1);
    CHECK(SetParent(child,other)==parent);expect_text=FALSE;CHECK(SendMessageW(parent,WM_DRAWITEM,0,(LPARAM)&item)==app_result && prepares==2 && !bad);CHECK(SetParent(child,parent)==other);
    policy.button_owner=FALSE;draw.dwDrawStage=CDDS_PREPAINT;retire=TRUE;CHECK(SendMessageW(parent,WM_NOTIFY,0,(LPARAM)&draw)==app_result && !style_runtime_text_report_target(dc,&target) && instance.detached && instance.calls==1);retire=FALSE;
    style_runtime_text_end(&outer);CHECK(!style_runtime_text_current() && !manager.checklists.current && calls==10 && !bad);
    CHECK(ReleaseDC(child,dc) && DestroyWindow(parent) && !runtime.report.parent && DestroyWindow(other));
    style_instance_release(&instance);CHECK(disposed==1 && !bad && UnregisterClassW(cls.lpszClassName,module));
    puts("REPORT_NOTIFY PASS native_result=1 native_error=1 stages=3 skip=1 file=1 button=1 wrong_type=1 reparent=1 retire=1 scope=1 last_ref=1 cleanup=1 no_style=1");return 0;
}
