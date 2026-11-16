#include "lib-win32-style/application/control/control_state.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CONTROL_STATE line=%d error=%lu\n",__LINE__,GetLastError());return 1;}}while(0)
static HFONT application,variant,native_font;
static unsigned leases,acquisitions,forwarded;
static BOOL invalid_release,fail_acquire,last_native,detach_query,detach_forward,fail_forward;
static style_instance *active;
static LRESULT CALLBACK procedure(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_GETFONT){if(detach_query)active->detached=TRUE;return (LRESULT)native_font;}
    if(message==WM_SETFONT){native_font=(HFONT)wp;return 0;}
    return DefWindowProcW(window,message,wp,lp);
}
static HFONT source(HFONT font){return font==variant?application:font;}
static HFONT acquire(HWND window,HFONT font,BOOL native)
{(void)window;(void)font;++acquisitions;last_native=native;if(fail_acquire)return NULL;++leases;return variant;}
static void retain(HFONT font){if(font==variant)++leases;}
static void release(HFONT font){if(font==variant){if(leases)--leases;else invalid_release=TRUE;}}
static const style_control_fonts operations={source,acquire,retain,release};
static LRESULT forward(HWND window,WPARAM count,LPARAM values)
{(void)window;(void)count;(void)values;++forwarded;if(detach_forward)active->detached=TRUE;return !fail_forward;}
int main(void)
{
    WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"PublicControlState",.lpfnWndProc=procedure};CHECK(RegisterClassW(&cls));
    HWND window=CreateWindowW(cls.lpszClassName,L"",WS_POPUP,0,0,240,100,NULL,NULL,cls.hInstance,NULL);CHECK(window);
    style_control_state prepared;style_control_state_init(&prepared,NULL);int initial[]={11,27};
    CHECK(style_control_state_prepare(window,&prepared,2,initial) && prepared.tab_count==2 && prepared.tab_positions!=initial);
    initial[0]=99;CHECK(prepared.tab_positions[0]==11 && prepared.tab_positions[1]==27);
    style_control_state_dispose(&prepared);CHECK(!prepared.protocol && !prepared.tab_positions);
    CHECK(!style_control_state_prepare(window,&prepared,-1,NULL) && GetLastError()==ERROR_INVALID_PARAMETER);
    CHECK(!style_control_state_prepare(window,&prepared,1,NULL) && GetLastError()==ERROR_INVALID_PARAMETER);
    application=(HFONT)GetStockObject(DEFAULT_GUI_FONT);variant=CreateFontW(-18,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Arial");CHECK(variant);
    style_instance instance={0};active=&instance;style_control_state state;style_control_state_init(&state,&operations);
    HFONT previous=NULL,outbound=NULL;WPARAM parameter=(WPARAM)application;
    CHECK(style_control_font_message(window,&instance,&state,&parameter,STYLE_CONTROL_FONT_REPLACE,FALSE,FALSE,&previous,&outbound));
    CHECK(parameter==(WPARAM)variant && state.original_font==application && state.font==variant && outbound==variant && !previous && leases==2);
    state.native_paints=1;style_control_font_finish(window,&instance,&state,WM_SETFONT,0,TRUE,application,previous,outbound,NULL);
    CHECK(leases==1 && !state.native_paints);
    previous=outbound=NULL;parameter=(WPARAM)variant;
    CHECK(style_control_font_message(window,&instance,&state,&parameter,STYLE_CONTROL_FONT_CACHE,FALSE,FALSE,&previous,&outbound));
    CHECK(parameter==(WPARAM)application && previous==variant && !outbound && last_native && leases==2);
    style_control_font_finish(window,&instance,&state,WM_PAINT,0,FALSE,application,previous,NULL,NULL);CHECK(leases==1);
    previous=outbound=NULL;parameter=(WPARAM)application;
    CHECK(style_control_font_message(window,&instance,&state,&parameter,STYLE_CONTROL_FONT_REPLACE,FALSE,TRUE,&previous,&outbound));
    CHECK(parameter==(WPARAM)application && previous==variant && leases==3);
    style_control_font_finish(window,&instance,&state,WM_SETFONT,0,FALSE,application,previous,outbound,NULL);CHECK(leases==1);
    fail_acquire=TRUE;previous=outbound=NULL;
    CHECK(style_control_font_message(window,&instance,&state,&parameter,STYLE_CONTROL_FONT_REPLACE,FALSE,FALSE,&previous,&outbound));
    CHECK(parameter==(WPARAM)application && state.font==application && previous==variant && leases==1);
    style_control_font_finish(window,&instance,&state,WM_SETFONT,0,FALSE,application,previous,outbound,NULL);CHECK(!leases);fail_acquire=FALSE;
    native_font=variant;detach_query=TRUE;unsigned before=acquisitions;previous=outbound=NULL;
    CHECK(!style_control_font_message(window,&instance,&state,&parameter,STYLE_CONTROL_FONT_OBSERVE,TRUE,FALSE,&previous,&outbound));
    CHECK(acquisitions==before && !previous && state.font==application);detach_query=FALSE;
    HFONT successor=(HFONT)GetStockObject(SYSTEM_FONT);
    style_control_font_finish(window,&instance,&state,WM_SETFONT,0,FALSE,successor,NULL,NULL,NULL);CHECK(native_font==successor);
    instance.detached=FALSE;state.font=application;int stops[]={4,8};
    CHECK(style_control_tabs_set(window,&instance,&state,2,(LPARAM)stops,forward) && state.tab_count==2 && state.tab_positions[0]>0);
    int *saved=state.tab_positions;before=forwarded;
    CHECK(!style_control_tabs_set(window,&instance,&state,1,0,forward) && forwarded==before && state.tab_positions==saved);
    fail_forward=TRUE;CHECK(!style_control_tabs_set(window,&instance,&state,0,0,forward) && state.tab_positions==saved);fail_forward=FALSE;
    detach_forward=TRUE;CHECK(style_control_tabs_set(window,&instance,&state,0,0,forward) && state.tab_count==2 && state.tab_positions==saved);
    style_control_state_dispose(&state);style_control_state_dispose(&state);CHECK(!state.font && !state.tab_positions && !leases && !invalid_release);
    LOGFONTW font;CHECK(GetObjectW(application,sizeof(font),&font) && DeleteObject(variant));
    CHECK(DestroyWindow(window) && UnregisterClassW(cls.lpszClassName,cls.hInstance));
    puts("CONTROL_STATE font leases/native forwarding/fallback/retired successor/tab commit/failure/reentrant cleanup PASS");return 0;
}
