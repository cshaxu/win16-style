#include "lib-win32-style/application/lifecycle/dispatch.h"
#include <commctrl.h>
#include <stdio.h>
#include "../../../ntw16s-dll/exception_probe.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"DISPATCH FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct fixture {HWND window;unsigned active,normal,exceptional,invoked;DWORD entry;BOOL invoke;} fixture;
static fixture parent,child;
static unsigned order[8],count;
static LRESULT CALLBACK native(HWND w,UINT m,WPARAM a,LPARAM b)
{
    (void)a;(void)b;
    if(m==WM_APP+1){if(w==parent.window)SendMessageW(child.window,WM_APP+3,0,0);SetLastError(0x2222);return 0x1234;}
    if(m==WM_APP+2){RaiseException(NTW16S_TEST_EXCEPTION_CODE,0,0,NULL);return 0;}
    if(m==WM_APP+3){SetLastError(0x3333);return 0x5678;}
    if(m==WM_APP+4)return SendMessageW(child.window,WM_APP+2,0,0);
    return DefWindowProcW(w,m,a,b);
}
static void finish(void *context,BOOL completed)
{
    fixture *f=context;if(completed)++f->normal;else ++f->exceptional;
    --f->active;if(count<ARRAYSIZE(order))order[count++]=f==&parent?1:2;
    SetLastError(0x7777);
}
static LRESULT forward(void *context,HWND w,UINT m,WPARAM a,LPARAM b)
{fixture *f=context;++f->invoked;f->entry=GetLastError();return DefSubclassProc(w,m,a,b);}
static LRESULT CALLBACK observe(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;fixture *f=(void *)data;
    if(m>=WM_APP+1 && m<=WM_APP+4){++f->active;const style_dispatch_ops ops={f,f,f->invoke?forward:NULL,finish};return style_dispatch_message(w,m,a,b,&ops);}
    return DefSubclassProc(w,m,a,b);
}
int main(void)
{
    INITCOMMONCONTROLSEX init={sizeof(init),ICC_BAR_CLASSES};CHECK(InitCommonControlsEx(&init));HMODULE module=GetModuleHandleW(NULL);
    WNDCLASSW cls={.hInstance=module,.lpfnWndProc=native,.lpszClassName=L"DispatchContract"};CHECK(RegisterClassW(&cls));
    parent.window=CreateWindowW(cls.lpszClassName,L"",WS_POPUP,0,0,100,100,NULL,NULL,module,NULL);CHECK(parent.window);
    child.window=CreateWindowW(cls.lpszClassName,L"",WS_CHILD,0,0,50,50,parent.window,NULL,module,NULL);CHECK(child.window);
    CHECK(SetWindowSubclass(parent.window,observe,1,(DWORD_PTR)&parent));CHECK(SetWindowSubclass(child.window,observe,1,(DWORD_PTR)&child));
    SetLastError(0x1111);CHECK(SendMessageW(parent.window,WM_APP+1,0,0)==0x1234 && GetLastError()==0x2222);
    CHECK(!parent.active && !child.active && parent.normal==1 && child.normal==1 && !parent.invoked && count==2 && order[0]==2 && order[1]==1);
    parent.invoke=child.invoke=TRUE;count=0;SetLastError(0x4444);
    CHECK(SendMessageW(parent.window,WM_APP+1,0,0)==0x1234 && GetLastError()==0x2222 && parent.entry==0x4444);
    CHECK(parent.invoked==1 && child.invoked==1 && !parent.active && !child.active && count==2 && order[0]==2 && order[1]==1);
    CHECK(test_unwind(parent.window,WM_APP+2,0,0));CHECK(parent.exceptional==1 && !parent.active && !child.active);
    unsigned before=parent.normal;SetLastError(0x5555);CHECK(SendMessageW(parent.window,WM_APP+1,0,0)==0x1234 && GetLastError()==0x2222 && parent.entry==0x5555);
    CHECK(parent.normal==before+1 && parent.exceptional==1 && !parent.active && !child.active);
    for(unsigned custom=0;custom<2;++custom){
        parent.invoke=child.invoke=custom;unsigned before_parent=parent.exceptional,before_child=child.exceptional;count=0;
        CHECK(test_unwind(parent.window,WM_APP+4,0,0));
        CHECK(!parent.active && !child.active && parent.exceptional==before_parent+1 && child.exceptional==before_child+1 && count==2 && order[0]==2 && order[1]==1);
        before=parent.normal;CHECK(SendMessageW(parent.window,WM_APP+1,0,0)==0x1234 && GetLastError()==0x2222 && parent.normal==before+1 && !parent.active && !child.active);
    }
    CHECK(RemoveWindowSubclass(child.window,observe,1));CHECK(RemoveWindowSubclass(parent.window,observe,1));CHECK(DestroyWindow(parent.window));CHECK(UnregisterClassW(cls.lpszClassName,module));
    puts("DISPATCH PASS native_subclass=1 optional_invoke=1 nested_order=1 entry_error=1 result_error=1 finish_once=1 native_unwind=1 nested_unwind=1 later_call=1 no_style=1 cleanup=1");return 0;
}
