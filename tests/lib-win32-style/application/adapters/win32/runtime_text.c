#include "lib-win32-style/application/adapters/win32/runtime_text.h"
#include "lib-win32-style/application/lifecycle/dispatch.h"
#include "lib-win32-style/application/control/runtime.h"
#include <commctrl.h>
#include <stdio.h>
#include "../../../../ntw16s-dll/exception_probe.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"RUNTIME_TEXT FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static HWND root,barrier;
static unsigned bad,labels,normal,exceptional,roles;
static BOOL include_text(HWND w,UINT m)
{
    if(m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT){HWND native=NULL;if(style_runtime_text_native_target(NULL,&native) || native!=w)++bad;return TRUE;}
    return FALSE;
}
static style_runtime_text_role role(HWND w,void *context)
{
    style_runtime_text_scope *s=context;++roles;
    if(style_runtime_text_current()!=s || !s->active)++bad;
    return (style_runtime_text_role){w==barrier?NULL:w,w==barrier};
}
static BOOL label(HWND w){(void)w;++labels;return TRUE;}
static DWORD WINAPI foreign(void *context)
{
    (void)context;HWND w=(HWND)(UINT_PTR)1;
    return style_runtime_text_current() || style_runtime_text_active() || style_runtime_text_native_target(NULL,&w) || w?1:0;
}
static LRESULT CALLBACK native(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_APP+1){SetLastError(0x4567);return 73;}
    if(m==WM_APP+2){RaiseException(NTW16S_TEST_EXCEPTION_CODE,0,0,NULL);return 0;}
    if(m==WM_APP+4)return SendMessageW(barrier,WM_APP+2,0,0);
    return DefWindowProcW(w,m,a,b);
}
static void finish(void *context,BOOL completed)
{style_runtime_text_scope *s=context;if(style_runtime_text_current()!=s)++bad;style_runtime_text_end(s);if(completed)++normal;else ++exceptional;SetLastError(0x7777);}
static LRESULT CALLBACK observe(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;
    if(m==WM_APP+1 || m==WM_APP+2 || m==WM_APP+4){
        style_runtime_text_scope scope;const style_runtime_text_ops policy={&scope,include_text,role};
        style_runtime_text_begin(&scope,w,WM_PAINT,NULL,&policy);
        const style_dispatch_ops ops={NULL,&scope,NULL,finish};return style_dispatch_message(w,m,a,b,&ops);
    }
    return DefSubclassProc(w,m,a,b);
}
int main(void)
{
    HINSTANCE module=GetModuleHandleW(NULL);WNDCLASSW cls={.hInstance=module,.lpfnWndProc=native,.lpszClassName=L"RuntimeTextScope"};CHECK(RegisterClassW(&cls));
    root=CreateWindowW(cls.lpszClassName,L"",WS_POPUP,0,0,100,100,NULL,NULL,module,NULL);CHECK(root);
    barrier=CreateWindowW(cls.lpszClassName,L"",WS_CHILD,0,0,50,50,root,NULL,module,NULL);CHECK(barrier);
    HDC dc=GetDC(root),other=GetDC(barrier);CHECK(dc && other);
    style_runtime_text_scope outer,nested;int token=1;const style_runtime_text_ops outer_ops={&outer,include_text,role},nested_ops={&nested,include_text,role};
    CHECK(!style_runtime_text_current() && !style_runtime_text_active() && style_runtime_text_direct_allowed());
    CHECK(style_runtime_text_label_caller((const void *)(UINT_PTR)1));
    CHECK(!style_runtime_text_caller((const void *)(UINT_PTR)1));
    CHECK(style_runtime_text_caller((const void *)(UINT_PTR)GetProcAddress(GetModuleHandleW(L"USER32.dll"),"MessageBoxW")));
    style_runtime_text_begin(&outer,root,WM_PAINT,&token,&outer_ops);CHECK(!bad && outer.context==&token && outer.window==root && roles==1);
    outer.native_dc=dc;HWND w=NULL;CHECK(style_runtime_text_native_target(dc,&w) && w==root && style_runtime_text_active());
    style_runtime_text_report_enable(&outer,root,NULL,NULL,label);CHECK(!labels && !outer.report);
    style_instance instance={0};style_control_runtime runtime={0};runtime.report.module=module;runtime.report.tree=TRUE;
    style_runtime_text_prepare_report(&outer,root,(WPARAM)dc,&instance,&runtime,TRUE,NULL,NULL,label);
    CHECK(outer.native_dc==dc && !outer.report && !labels);
    style_runtime_text_prepare_report(&outer,root,(WPARAM)dc,&instance,&runtime,FALSE,NULL,NULL,label);
    CHECK(outer.report && labels==1);outer.report=FALSE;labels=0;
    instance.detached=TRUE;style_runtime_text_prepare_report(&outer,root,(WPARAM)dc,&instance,&runtime,FALSE,NULL,NULL,label);
    CHECK(!outer.report && !labels);instance.detached=FALSE;
    BOOL cancelled=FALSE;style_runtime_text_report_enable(&outer,root,module,&cancelled,label);CHECK(labels==1 && outer.report && !style_runtime_text_direct_allowed());
    style_runtime_text_report_bind(barrier,other);CHECK(!style_runtime_text_report_target(other,&w) && !w);
    style_runtime_text_report_bind(root,dc);CHECK(style_runtime_text_report_target(dc,&w) && w==root && style_runtime_text_report_caller(module));
    CHECK(!style_runtime_text_report_caller(GetModuleHandleW(L"USER32.dll")));
    CHECK(style_runtime_text_caller((const void *)(UINT_PTR)main));
    CHECK(!style_runtime_text_caller((const void *)(UINT_PTR)1));
    CHECK(!style_runtime_text_label_caller((const void *)(UINT_PTR)1));
    CHECK(!style_runtime_text_label_caller((const void *)(UINT_PTR)main));
    cancelled=TRUE;CHECK(style_runtime_text_report_active() && !style_runtime_text_active() && !style_runtime_text_report_target(dc,&w) && !w && !style_runtime_text_direct_allowed());cancelled=FALSE;
    style_runtime_text_begin(&nested,barrier,WM_PRINTCLIENT,NULL,&nested_ops);CHECK(!style_runtime_text_active() && !style_runtime_text_report_active() && !style_runtime_text_direct_allowed());
    CHECK(!style_runtime_text_native_target(dc,&w) && w==barrier);style_runtime_text_end(&nested);CHECK(style_runtime_text_current()==&outer && style_runtime_text_native_target(dc,&w) && w==root);
    style_runtime_text_begin(&nested,barrier,WM_ERASEBKGND,NULL,&nested_ops);CHECK(!nested.active && nested.native_active && style_runtime_text_current()==&outer && !style_runtime_text_native_target(dc,&w) && w==barrier);
    style_runtime_text_end(&nested);style_runtime_text_begin(&nested,barrier,WM_APP+99,NULL,&nested_ops);CHECK(!nested.active && !nested.native_active && style_runtime_text_current()==&outer);style_runtime_text_end(&nested);
    HANDLE thread=CreateThread(NULL,0,foreign,NULL,0,NULL);CHECK(thread && WaitForSingleObject(thread,2000)==WAIT_OBJECT_0);DWORD code=1;CHECK(GetExitCodeThread(thread,&code) && !code && CloseHandle(thread));
    style_runtime_text_end(&outer);CHECK(!style_runtime_text_current() && !style_runtime_text_native_target(dc,&w) && !w && !bad);
    CHECK(SetWindowSubclass(root,observe,1,0) && SetWindowSubclass(barrier,observe,1,0));SetLastError(0x1234);CHECK(SendMessageW(root,WM_APP+1,0,0)==73 && GetLastError()==0x4567 && normal==1 && !style_runtime_text_current());
    CHECK(test_unwind(root,WM_APP+2,0,0) && exceptional==1 && !style_runtime_text_current() && !style_runtime_text_native_target(dc,&w) && !w);
    CHECK(test_unwind(root,WM_APP+4,0,0) && exceptional==3 && !style_runtime_text_current() && !bad);
    CHECK(SendMessageW(root,WM_APP+1,0,0)==73 && normal==2 && !style_runtime_text_current() && !bad);
    CHECK(RemoveWindowSubclass(root,observe,1) && RemoveWindowSubclass(barrier,observe,1));CHECK(ReleaseDC(root,dc) && ReleaseDC(barrier,other) && DestroyWindow(root) && UnregisterClassW(cls.lpszClassName,module));
    puts("RUNTIME_TEXT PASS native_first=1 text_publish=1 barrier=1 report=1 cancel=1 module=1 thread=1 context=1 native_unwind=1 nested_unwind=1 later_call=1 no_style=1 cleanup=1");return 0;
}
