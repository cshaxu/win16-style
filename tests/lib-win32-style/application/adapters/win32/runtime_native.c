#include "lib-win32-style/application/adapters/win32/runtime_native.h"
#include "../../../../ntw16s-dll/exception_probe.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"NATIVE_CALL FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static style_runtime_native_calls calls;
static style_instance instances[2];
static style_control_runtime runtimes[2];
static HWND windows[2];
static WNDPROC native_static;
static BOOL raising,nested;
static unsigned bad,paints,barriers;
static BOOL current(HWND w,void *owner)
{return IsWindow(w) && (owner==&instances[0] || owner==&instances[1]);}
static style_runtime_observation observation(HWND w)
{unsigned i=w==windows[1]?1:0;return (style_runtime_observation){.instance=&instances[i],.runtime=&runtimes[i],.owner=&instances[i],.window=w,.current=current};}
static PROCESS_HOOK_UNWIND_FUNCTION LRESULT CALLBACK alias(HWND w,UINT m,WPARAM a,LPARAM b)
{
    style_runtime_observation paint=observation(w);style_runtime_native_call scope;process_hook_unwind_scope unwind;
    if(!style_runtime_native_call_begin(&calls,&scope,native_static,w,m,alias,&paint,NULL))return CallWindowProcW(native_static,w,m,a,b);
    PROCESS_HOOK_UNWIND_BEGIN(unwind,style_runtime_native_call_end,&scope);
    if(m==WM_PAINT){
        ++paints;if(style_runtime_native_current(&calls,&paint)!=&scope || !paint.runtime->roles.text.verified)++bad;
        if(raising)RaiseException(NTW16S_TEST_EXCEPTION_CODE,0,0,NULL);
        if(!nested){nested=TRUE;style_runtime_native_call barrier,inner;
            if(!style_runtime_native_call_begin(&calls,&barrier,DefWindowProcW,w,WM_APP,alias,&paint,NULL) || !barrier.active || barrier.instance || style_runtime_native_current(&calls,&paint))++bad;
            else ++barriers;
            style_runtime_native_call_end(&barrier);if(calls.current!=&scope)++bad;
            style_runtime_observation other=observation(windows[1]);
            if(!style_runtime_native_call_begin(&calls,&inner,native_static,windows[1],WM_PAINT,alias,&other,NULL) || style_runtime_native_current(&calls,&other)!=&inner)++bad;
            CallWindowProcW(native_static,windows[1],WM_PAINT,0,0);style_runtime_native_call_end(&inner);if(calls.current!=&scope)++bad;
            paint.instance->detached=TRUE;if(style_runtime_native_current(&calls,&paint))++bad;paint.instance->detached=FALSE;
            style_runtime_observation wrong=paint;wrong.window=windows[1];if(style_runtime_native_current(&calls,&wrong) || style_runtime_native_current(&calls,NULL))++bad;
        }
    }else if(style_runtime_native_current(&calls,&paint))++bad;
    LRESULT result=CallWindowProcW(native_static,w,m,a,b);
    PROCESS_HOOK_UNWIND_END(unwind);return result;
}
int main(void)
{
    HMODULE module=GetModuleHandleW(NULL);WNDCLASSEXW cls={.cbSize=sizeof(cls)};CHECK(GetClassInfoExW(NULL,L"STATIC",&cls));native_static=cls.lpfnWndProc;
    cls.hInstance=module;cls.lpfnWndProc=alias;cls.lpszClassName=L"NativeCallAlias";cls.style&=~CS_GLOBALCLASS;CHECK(RegisterClassExW(&cls));
    for(unsigned i=0;i<2;++i){windows[i]=CreateWindowW(cls.lpszClassName,L"Native scope",WS_POPUP,0,0,160,40,NULL,NULL,module,NULL);CHECK(windows[i]);}
    style_runtime_native_call ignored;style_runtime_observation paint=observation(windows[0]);
    CHECK(!style_runtime_native_call_begin(&calls,&ignored,DefWindowProcW,windows[0],WM_PAINT,alias,&paint,NULL) && !ignored.active && !calls.current);
    SendMessageW(windows[0],WM_PAINT,0,0);CHECK(!bad && paints>=1 && barriers==1 && !calls.current && runtimes[0].roles.text.verified && runtimes[1].roles.text.verified);
    unsigned before=paints;raising=TRUE;CHECK(test_unwind(windows[0],WM_PAINT,0,0) && !calls.current && paints==before+1);raising=FALSE;
    SendMessageW(windows[0],WM_PAINT,0,0);CHECK(!bad && !calls.current && paints==before+2);
    CHECK(DestroyWindow(windows[1]) && DestroyWindow(windows[0]));CHECK(UnregisterClassW(cls.lpszClassName,module));
    puts("NATIVE_CALL PASS native_static=1 witness=1 no_match=1 barrier=1 nested_verified=1 detached=1 current_window=1 native_unwind=1 later_call=1 no_style=1 cleanup=1");return 0;
}
