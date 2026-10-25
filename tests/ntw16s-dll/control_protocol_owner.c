#include <windows.h>
#include <commctrl.h>
#include <stdio.h>
#include <stdlib.h>
#include "isolated_process.h"
static unsigned allocations;static BOOL fail_next;
static void *tracked_calloc(size_t n,size_t size)
{if(fail_next){fail_next=FALSE;return NULL;}void *p=calloc(n,size);if(p)++allocations;return p;}
static void tracked_free(void *p){if(p){if(!allocations)abort();--allocations;free(p);}}
#define calloc tracked_calloc
#define free tracked_free
#include "../../src/ntw16s-dll/control_protocol.c"
#undef calloc
#undef free
#define CHECK(x) do{if(!(x)){fprintf(stderr,"PROTOCOL_OWNER_FAIL line=%u error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static LRESULT CALLBACK button(HWND w,UINT m,WPARAM a,LPARAM b)
{
    if(m==WM_GETDLGCODE)return DLGC_BUTTON;
    if(m==BCM_GETIDEALSIZE){
        if(GetDlgCtrlID(w)==4){DestroyWindow(w);return FALSE;}
        if(GetDlgCtrlID(w)==2)return FALSE;
        SIZE *size=(SIZE*)b;size->cx=GetDlgCtrlID(w)==3?-1:60;size->cy=20;return TRUE;
    }
    if(m==BM_GETSTATE || m==BM_GETCHECK)return 0;
    return DefWindowProcW(w,m,a,b);
}
static int worker(int argc,WCHAR **argv)
{
    (void)argc;(void)argv;HINSTANCE instance=GetModuleHandleW(NULL);
    WNDCLASSEXW native={.cbSize=sizeof(native)};CHECK(GetClassInfoExW(NULL,L"BUTTON",&native));
    WNDCLASSW wc={.hInstance=instance,.lpfnWndProc=button,.cbWndExtra=native.cbWndExtra,.lpszClassName=L"TButton"};CHECK(RegisterClassW(&wc));
    wc.lpfnWndProc=DefWindowProcW;wc.lpszClassName=L"TComboBox";CHECK(RegisterClassW(&wc));
    HWND parent=CreateWindowW(L"STATIC",L"owner",WS_POPUP,0,0,200,100,NULL,NULL,instance,NULL);CHECK(parent);
    for(unsigned i=1;i<=4;++i){
        HWND w=CreateWindowW(L"TButton",L"test",WS_CHILD,0,0,100,30,parent,(HMENU)(UINT_PTR)i,instance,NULL);CHECK(w);
        ntw16s_control_protocol *b=ntw16s_control_protocol_bind(w);
        CHECK((b!=NULL)==(i==1));
        if(b){CHECK(allocations==1 && ntw16s_control_protocol_is(w,L"BUTTON") && !ntw16s_control_protocol_is(w,L"EDIT"));
            RemovePropW(w,key);CHECK(!ntw16s_control_protocol_is(w,L"BUTTON"));
            ntw16s_control_protocol_release(b);CHECK(!GetPropW(w,key));}
        CHECK(!allocations && !bindings);if(IsWindow(w))CHECK(DestroyWindow(w));
    }
    HWND w=CreateWindowW(L"TButton",L"oom",WS_CHILD,0,0,100,30,parent,(HMENU)1,instance,NULL);CHECK(w);
    fail_next=TRUE;SetLastError(0);CHECK(!ntw16s_control_protocol_bind(w) && GetLastError()==ERROR_NOT_ENOUGH_MEMORY && !bindings && !allocations);
    CHECK(SetPropW(w,key,(HANDLE)(UINT_PTR)123));CHECK(!ntw16s_control_protocol_bind(w) && !ntw16s_control_protocol_is(w,L"BUTTON"));
    CHECK(RemovePropW(w,key)==(HANDLE)(UINT_PTR)123 && DestroyWindow(w));
    w=CreateWindowW(L"TComboBox",L"fake",WS_CHILD,0,0,100,30,parent,NULL,instance,NULL);CHECK(w);
    CHECK(!ntw16s_control_protocol_bind(w) && !allocations && !bindings && !GetPropW(w,key));
    CHECK(DestroyWindow(w) && DestroyWindow(parent));
    printf("PROTOCOL_OWNER_OK bits=%u allocations=0 bindings=0 invalid=3 destroyed_query=1 oom=1 foreign_property=1 combo_impostor=1\n",(unsigned)(sizeof(void*)*8));return 0;
}
int wmain(int argc,WCHAR **argv){return isolated_process(argc,argv,worker);}
