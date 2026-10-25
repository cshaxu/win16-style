#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include "isolated_process.h"
static void *owned[32];static unsigned count;static BOOL fail_next;
static void *track(void *p){if(p){if(count==32)abort();owned[count++]=p;}return p;}
static void *stage_calloc(size_t n,size_t s){if(fail_next){fail_next=FALSE;return NULL;}return track(calloc(n,s));}
static WCHAR *stage_dup(const WCHAR *s){if(fail_next){fail_next=FALSE;return NULL;}return track(_wcsdup(s));}
static void stage_free(void *p){if(!p)return;for(unsigned i=0;i<count;++i)if(owned[i]==p){owned[i]=owned[--count];free(p);return;}abort();}
#define calloc stage_calloc
#define free stage_free
#define _wcsdup stage_dup
#include "../../src/ntw16s-dll/theme.c"
#undef calloc
#undef free
#undef _wcsdup
#define CHECK(x) do{if(!(x)){fprintf(stderr,"THEME STAGE FAIL line=%u error=%lu %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static WCHAR observed_app[80],observed_ids[80];static unsigned native_calls;
static HRESULT WINAPI theme_observe(HWND w,LPCWSTR app,LPCWSTR ids)
{++native_calls;lstrcpynW(observed_app,app?app:L"<null>",80);lstrcpynW(observed_ids,ids?ids:L"<null>",80);return SetWindowTheme(w,app,ids);}
static BOOL WINAPI get_scroll(HWND w,int b,LPSCROLLINFO s){return GetScrollInfo(w,b,s);}
static int WINAPI set_scroll(HWND w,int b,LPCSCROLLINFO s,BOOL r){return SetScrollInfo(w,b,s,r);}
static LRESULT CALLBACK owner_proc(HWND w,UINT m,WPARAM a,LPARAM b)
{if(m==WM_APP+7)return 0x514;return DefWindowProcW(w,m,a,b);}
static int worker(int argc,WCHAR **argv)
{
    (void)argc;(void)argv;ntw16s_native_api api={sizeof(api),get_scroll,set_scroll,theme_observe};CHECK(ntw16s_bind_native_api(&api));
    WNDCLASSW c={0};c.lpfnWndProc=owner_proc;c.hInstance=GetModuleHandleW(NULL);c.lpszClassName=L"StageOwnership";CHECK(RegisterClassW(&c));
    HWND w=CreateWindowW(c.lpszClassName,L"stage",WS_OVERLAPPEDWINDOW,0,0,200,100,NULL,NULL,c.hInstance,NULL);CHECK(w);
    WNDPROC before=(WNDPROC)GetWindowLongPtrW(w,GWLP_WNDPROC);
    CHECK(ntw16s_theme_stage(w) && count==1 && staged && (WNDPROC)GetWindowLongPtrW(w,GWLP_WNDPROC)==before);
    CHECK(SUCCEEDED(Ntw16sSetWindowTheme(w,L"Explorer",L"Window")) && count==3);
    CHECK(!wcscmp(observed_app,L"Explorer") && !wcscmp(observed_ids,L"Window"));
    fail_next=TRUE;CHECK(Ntw16sSetWindowTheme(w,L"Other",NULL)==E_OUTOFMEMORY && count==3);
    CHECK(!wcscmp(state(w)->application,L"Explorer"));
    CHECK(ntw16s_theme_commit(w) && !staged && count==3);
    const void *owner=&api;CHECK(ntw16s_theme_enable(w,owner));CHECK(!wcscmp(observed_app,L"") && !wcscmp(observed_ids,L""));
    CHECK(SUCCEEDED(Ntw16sSetWindowTheme(w,L"Other",NULL)) && count==2);
    CHECK(!wcscmp(observed_app,L"") && !wcscmp(state(w)->application,L"Other"));
    ntw16s_theme_remove(w,owner);CHECK(!state(w) && !staged && !count);
    CHECK(!wcscmp(observed_app,L"Other") && !wcscmp(observed_ids,L"<null>") && SendMessageW(w,WM_APP+7,0,0)==0x514 &&
        CallWindowProcW((WNDPROC)GetWindowLongPtrW(w,GWLP_WNDPROC),w,WM_APP+7,0,0)==CallWindowProcW(before,w,WM_APP+7,0,0));
    fail_next=TRUE;CHECK(!ntw16s_theme_stage(w) && GetLastError()==ERROR_NOT_ENOUGH_MEMORY && !count && !staged);
    CHECK(ntw16s_theme_stage(w));ntw16s_theme_cancel_stage(w);CHECK(!count && !staged && !state(w));
    CHECK(ntw16s_theme_stage(w));CHECK(ntw16s_theme_terminal(w,NULL));CHECK(!count && !staged && !state(w));
    CHECK(ntw16s_theme_stage(w) && ntw16s_theme_commit(w));CHECK(DestroyWindow(w));CHECK(!count && !staged);
    CHECK(UnregisterClassW(c.lpszClassName,c.hInstance));
    printf("THEME_STAGE_OWNERSHIP_OK bits=%u calls=%u allocations=0 early_proc_unchanged=1 commit=1 cancel=1 terminal=1 fail_alloc=1 latest_theme_restored=1\n",(unsigned)(sizeof(void *)*8),native_calls);return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
