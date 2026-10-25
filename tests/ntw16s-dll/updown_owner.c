#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "ntw16s-dll/updown_paint.h"
#include "ntw16s-dll/scroll_glyphs.h"
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"UPDOWN_OWNER_FAIL line=%u error=%lu %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static _Thread_local HWND scope_window;static _Thread_local HDC scope_dc;
BOOL ntw16s_native_paint_target(HDC dc,HWND *w){*w=scope_window;return dc&&scope_dc==dc;}
typedef struct owner {ntw16s_updown_paint paint;HWND window;unsigned refs,attempts,errors;BOOL alive,retire;} owner;
static BOOL acquire(void *value){owner *s=value;++s->attempts;if(!s->alive || !IsWindow(s->window) || GetWindowThreadProcessId(s->window,NULL)!=GetCurrentThreadId())return FALSE;++s->refs;return TRUE;}
static void release(void *value){owner *s=value;if(!s->refs){++s->errors;return;}--s->refs;if(s->retire&&!s->refs){s->alive=FALSE;ntw16s_updown_retire(&s->paint);}}
static ntw16s_updown_glyph *const glyphs[]={&scroll_uparrow,&scroll_dnarrow,&scroll_lfarrow,&scroll_rgarrow,&scroll_uparrowd,&scroll_dnarrowd,&scroll_lfarrowd,&scroll_rgarrowd,&scroll_uparrowi,&scroll_dnarrowi,&scroll_lfarrowi,&scroll_rgarrowi};
static void anchor(void){}
static void owner_bind(owner *s,HWND w,const win16_style_palette *p){ZeroMemory(s,sizeof(*s));s->window=w;s->alive=TRUE;ntw16s_updown_register(&s->paint,w,GetModuleHandleW(NULL),p,glyphs,s,acquire,release);}
static BOOL draw(HDC dc){RECT r={0,0,17,17};return ntw16s_updown_draw_frame(dc,&r,DFC_SCROLL,DFCS_SCROLLUP,(void*)(UINT_PTR)anchor);}
static DWORD WINAPI foreign_thread(void *dc){return draw((HDC)dc)?1:0;}
static int worker(int argc,WCHAR **argv)
{
    (void)argc;(void)argv;WNDCLASSW cls={.lpfnWndProc=DefWindowProcW,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"UpDownOwnerFixture"};CHECK(RegisterClassW(&cls));HWND root=CreateWindowW(cls.lpszClassName,L"owner",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,cls.hInstance,NULL);CHECK(root);
    HWND w=CreateWindowW(cls.lpszClassName,L"",WS_CHILD,0,0,17,34,root,(HMENU)1,cls.hInstance,NULL),other=CreateWindowW(cls.lpszClassName,L"",WS_CHILD,20,0,17,34,root,(HMENU)2,cls.hInstance,NULL);CHECK(w&&other);HDC source=GetDC(w);CHECK(source);
    win16_style_palette palette={.face=RGB(192,192,192),.light=RGB(255,255,255),.shadow=RGB(128,128,128),.edge=0};owner first,second;CHECK(!ntw16s_updown_api_enabled());ntw16s_updown_api_enable();CHECK(ntw16s_updown_api_enabled());owner_bind(&first,w,&palette);
    RECT rect={0,0,17,17};CHECK(draw(source));CHECK(!first.refs&&!first.errors);CHECK(!ntw16s_updown_draw_frame(source,&rect,DFC_SCROLL,DFCS_SCROLLUP,(void*)(UINT_PTR)GetCurrentProcessId));CHECK(!first.refs);
    /* Real native DC handles; repeated delete events must actually recover
     * capacity, not merely hide stale entries behind a detached HWND. */
    for(unsigned cycle=0;cycle<130;++cycle){HDC dc=CreateCompatibleDC(source);CHECK(dc);ntw16s_updown_dc_created(source,dc,(void*)(UINT_PTR)anchor);CHECK(draw(dc)&&!first.refs);CHECK(DeleteDC(dc));ntw16s_updown_dc_deleted(dc);CHECK(SaveDC(dc)==0);}
    HDC dc[33];for(unsigned i=0;i<33;++i){dc[i]=CreateCompatibleDC(source);CHECK(dc[i]);ntw16s_updown_dc_created(source,dc[i],(void*)(UINT_PTR)anchor);CHECK(draw(dc[i])==(i<32));}
    CHECK(DeleteDC(dc[0]));ntw16s_updown_dc_deleted(dc[0]);CHECK(SaveDC(dc[0])==0);dc[0]=NULL;
    puts("UPDOWN_DC_CAPACITY_OK slots=32 overflow_native=1 delete_recovers=1 cycles=130");
    scope_window=other;scope_dc=NULL;CHECK(!draw(dc[1]));HDC nested=CreateCompatibleDC(source);CHECK(nested);ntw16s_updown_dc_created(source,nested,(void*)(UINT_PTR)anchor);CHECK(!draw(nested));/* metadata can bind beneath a parent-paint barrier */
    scope_window=NULL;CHECK(draw(nested));CHECK(DeleteDC(nested));ntw16s_updown_dc_deleted(nested);CHECK(SaveDC(nested)==0);
    ntw16s_updown_dc_created(source,dc[32],(void*)(UINT_PTR)anchor);CHECK(draw(dc[32]));
    scope_window=w;scope_dc=dc[1];CHECK(draw(dc[1]));scope_window=NULL;scope_dc=NULL;
    HANDLE thread=CreateThread(NULL,0,foreign_thread,dc[1],0,NULL);CHECK(thread&&WaitForSingleObject(thread,2000)==WAIT_OBJECT_0);DWORD code=1;CHECK(GetExitCodeThread(thread,&code)&&!code&&CloseHandle(thread));
    unsigned attempts=first.attempts;first.alive=FALSE;ntw16s_updown_retire(&first.paint);owner_bind(&second,w,&palette);
    for(unsigned i=1;i<33;++i)CHECK(!draw(dc[i]));
    CHECK(first.attempts==attempts);
    HDC fresh=CreateCompatibleDC(source);CHECK(fresh);ntw16s_updown_dc_created(source,fresh,(void*)(UINT_PTR)anchor);CHECK(draw(fresh));CHECK(!second.refs&&!second.errors);
    for(unsigned i=1;i<33;++i){CHECK(DeleteDC(dc[i]));ntw16s_updown_dc_deleted(dc[i]);CHECK(SaveDC(dc[i])==0);}
    /* An invalid/deleted DC can still be named by stale application input.
     * SaveDC failure must release the exact private owner before fallback. */
    CHECK(DeleteDC(fresh));CHECK(!draw(fresh)&&!second.refs);ntw16s_updown_dc_deleted(fresh);CHECK(SaveDC(fresh)==0);
    second.retire=TRUE;CHECK(draw(source));CHECK(!second.alive&&!second.refs&&!second.errors);CHECK(!draw(source));CHECK(!first.refs&&!first.errors);
    CHECK(ReleaseDC(w,source)&&DestroyWindow(w)&&DestroyWindow(other)&&DestroyWindow(root));
    printf("UPDOWN_OWNER_OK bits=%u exact_refs_zero=1 retire_clears=1 nested_barrier=1 foreign_caller_thread_rejected=1 owned_dc_handles_released=165\n",(unsigned)sizeof(void*)*8);return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
