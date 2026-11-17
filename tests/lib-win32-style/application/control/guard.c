#include "lib-win32-style/application/control/guard.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CONTROL_GUARD FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
typedef struct fixture {
    style_instance instance;style_control_state state;HWND parent,window;
    unsigned mode,forwarded,window_removes,control_removes,disposed,policies,sources;
    BOOL dormant,fail_window,detach_native,destroy_native,valid_on_destroy,font_before_native,terminal;
} fixture;
static fixture f;
static void dispose(void *context){++((fixture *)context)->disposed;}
static HFONT source(HFONT font){(void)font;++f.sources;return GetStockObject(SYSTEM_FONT);}
static void font_release(HFONT font){(void)font;}
static const style_control_fonts fonts={.source=source,.release=font_release};
static BOOL valid(HWND w){(void)w;++f.policies;return f.mode!=1;}
static BOOL dormant(HWND w){(void)w;return f.dormant;}
static BOOL native_button(HWND w){(void)w;++f.policies;return f.mode==2;}
static BOOL unsupported(HWND w){(void)w;++f.policies;return f.mode==3;}
static BOOL remove_window(HWND w)
{(void)w;++f.window_removes;if(f.fail_window){SetLastError(ERROR_BUSY);return FALSE;}return style_instance_detach(&f.instance);}
static BOOL remove_control(HWND w){(void)w;++f.control_removes;return style_instance_detach(&f.instance);}
static const style_control_guard_ops ops={valid,dormant,native_button,unsupported,remove_window,remove_control};
static LRESULT CALLBACK application(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;
    if(m==WM_APP+1 || m==WM_SETFONT){
        ++f.forwarded;if(m==WM_SETFONT)f.font_before_native=f.state.original_font==(HFONT)GetStockObject(SYSTEM_FONT);
        if(f.detach_native)style_instance_detach(&f.instance);
        if(f.destroy_native){if(f.valid_on_destroy)f.mode=0;DestroyWindow(w);}
        if(m==WM_APP+1)return 42;
    }
    return DefSubclassProc(w,m,a,b);
}
static LRESULT CALLBACK guard(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;style_instance *instance=(void *)data;style_instance_retain(instance);LRESULT result;unsigned before=f.policies;
    BOOL handled=style_control_guard_message(w,instance,&f.state,m,a,b,&ops,&result);
    if(m==WM_NCDESTROY){f.terminal=!handled && before==f.policies;style_instance_detach(instance);}
    if(!handled)result=DefSubclassProc(w,m,a,b);
    style_instance_release(instance);return result;
}
static BOOL create(unsigned mode)
{
    f=(fixture){.mode=mode};style_instance_init(&f.instance,&f,dispose);style_control_state_init(&f.state,&fonts);style_instance_retain(&f.instance);
    f.parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,200,100,NULL,NULL,GetModuleHandleW(NULL),NULL);if(!f.parent)return FALSE;
    f.window=CreateWindowW(L"BUTTON",L"",WS_CHILD|(mode==2?BS_OWNERDRAW:BS_PUSHBUTTON),0,0,100,30,f.parent,NULL,GetModuleHandleW(NULL),NULL);if(!f.window)return FALSE;
    return SetWindowSubclass(f.window,application,1,0) && style_instance_attach(&f.instance,f.window,guard,NULL);
}
static BOOL cleanup(void)
{
    f.mode=0;if(IsWindow(f.window) && !DestroyWindow(f.window))return FALSE;
    if(!f.instance.detached)return FALSE;
    style_instance_release(&f.instance);style_control_state_dispose(&f.state);
    return f.disposed==1 && DestroyWindow(f.parent);
}
int main(void)
{
    INITCOMMONCONTROLSEX init={sizeof(init),ICC_BAR_CLASSES};CHECK(InitCommonControlsEx(&init));
    CHECK(create(0));CHECK(SendMessageW(f.window,WM_APP+1,0,0)==42 && f.forwarded==1 && !f.instance.detached);CHECK(cleanup() && f.terminal);
    CHECK(create(1));f.dormant=TRUE;CHECK(SendMessageW(f.window,WM_APP+1,0,0)==42 && f.forwarded==1 && !f.window_removes && !f.control_removes && !f.instance.detached);CHECK(cleanup());
    CHECK(create(1));f.fail_window=TRUE;CHECK(SendMessageW(f.window,WM_APP+1,0,0)==42 && f.window_removes==1 && !f.control_removes && !f.instance.detached);CHECK(cleanup());
    CHECK(create(1));CHECK(SendMessageW(f.window,WM_APP+1,0,0)==42 && f.forwarded==1 && f.window_removes==1 && !f.control_removes && f.instance.detached && f.instance.calls==1 && !f.disposed);CHECK(cleanup());
    CHECK(create(2));f.fail_window=TRUE;CHECK(SendMessageW(f.window,WM_APP+1,0,0)==42 && f.window_removes==1 && f.control_removes==1 && f.instance.detached);CHECK(cleanup());
    CHECK(create(2));CHECK(SendMessageW(f.window,WM_APP+1,0,0)==42 && f.window_removes==1 && !f.control_removes && f.instance.detached);CHECK(cleanup());
    for(unsigned mode=1;mode<=3;mode+=2){CHECK(create(mode));SendMessageW(f.window,WM_SETFONT,(WPARAM)GetStockObject(ANSI_FIXED_FONT),FALSE);CHECK(f.font_before_native && f.sources==1 && f.forwarded==1 && f.instance.detached);CHECK(mode==1?f.window_removes==1 && !f.control_removes:!f.window_removes && f.control_removes==1);CHECK(cleanup());}
    CHECK(create(2));f.detach_native=TRUE;CHECK(SendMessageW(f.window,WM_APP+1,0,0)==42 && f.instance.detached && !f.window_removes && !f.control_removes);CHECK(cleanup());
    CHECK(create(1));f.destroy_native=TRUE;LRESULT destroyed=SendMessageW(f.window,WM_APP+1,0,0);
    printf("GUARD_DESTROY invalid=1 result=%lld window=%d detached=%d outer=%u control=%u terminal=%d\n",(long long)destroyed,IsWindow(f.window),f.instance.detached,f.window_removes,f.control_removes,f.terminal);
    CHECK(destroyed==42 && !IsWindow(f.window) && f.instance.detached && f.window_removes==1 && !f.control_removes && !f.terminal);CHECK(cleanup());
    CHECK(create(1));f.destroy_native=f.valid_on_destroy=TRUE;CHECK(SendMessageW(f.window,WM_APP+1,0,0)==42 && !IsWindow(f.window) && f.instance.detached && !f.window_removes && !f.control_removes && f.terminal);CHECK(cleanup());
    puts("CONTROL_GUARD PASS native_button=1 forward_first=1 invalid_role=1 dormant=1 native_fallback=1 no_wrong_fallback=1 font_source=1 detach_reentry=1 destroy_reentry=1 terminal=1 final_release=1 cleanup=1");return 0;
}
