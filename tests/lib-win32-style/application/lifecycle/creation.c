#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "lib-win32-style/application/lifecycle/creation.h"

static void *records[256];static unsigned live;
static BOOL fail_next,fail_return_hook;
static void *scope_calloc(size_t n,size_t size)
{
    if(fail_next){fail_next=FALSE;return NULL;}
    void *p=calloc(n,size);if(p){if(live==256)abort();records[live++]=p;}return p;
}
static void scope_free(void *p)
{
    if(!p)return;
    for(unsigned i=0;i<live;++i)if(records[i]==p){records[i]=records[--live];free(p);return;}
    abort();
}
static HHOOK WINAPI scope_set_hook(int type,HOOKPROC callback,HINSTANCE module,DWORD thread)
{
    if(type==WH_CALLWNDPROCRET && fail_return_hook){fail_return_hook=FALSE;SetLastError(ERROR_NOT_ENOUGH_MEMORY);return NULL;}
    return SetWindowsHookExW(type,callback,module,thread);
}
#define calloc scope_calloc
#define free scope_free
#define SetWindowsHookExW scope_set_hook
#include "lib-win32-style/application/lifecycle/creation.c"
#undef calloc
#undef free
#undef SetWindowsHookExW

static const WCHAR class_name[]=L"StyleCreationOwnedTest";
static const WCHAR owned_dialog_property[]=L"Style.Creation.OwnedDialog";
/* USER may create IME/helper windows on this GUI thread. The synthetic
 * renderer selects only this fixture's explicit targets; counts are not
 * assertions about the OS's private window creation. */
static BOOL own_target(HWND window)
{WCHAR name[64];return GetPropW(window,owned_dialog_property)!=NULL ||
    (GetClassNameW(window,name,ARRAYSIZE(name)) && !lstrcmpW(name,class_name));}
static unsigned mode,unsupported_count,dialog_count;
static BOOL prepare(HWND window,DWORD style)
{(void)style;if(own_target(window) && mode==1){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}return TRUE;}
static BOOL install(HWND window,DWORD style)
{
    (void)style;if(!own_target(window))return TRUE;
    if(mode>=2){SetLastError(mode==2?ERROR_NOT_SUPPORTED:ERROR_ACCESS_DENIED);return FALSE;}
    return SetPropW(window,L"Style.Creation.Test",(HANDLE)(UINT_PTR)1);
}
static BOOL attached(HWND window){return GetPropW(window,L"Style.Creation.Test")!=NULL;}
static void cancel(HWND window){(void)window;}
static void unsupported_window(HWND window){(void)window;++unsupported_count;}
static void initialized(HWND window){if(IsWindow(window))++dialog_count;}
static const style_creation_ops operations={prepare,install,attached,cancel,unsupported_window,FALSE};
static HWND create(void)
{return CreateWindowW(class_name,L"creation",WS_OVERLAPPEDWINDOW,0,0,120,80,NULL,NULL,GetModuleHandleW(NULL),NULL);}
static INT_PTR CALLBACK dialog(HWND window,UINT message,WPARAM wp,LPARAM lp)
{(void)wp;(void)lp;if(message==WM_INITDIALOG)SetPropW(window,owned_dialog_property,(HANDLE)(UINT_PTR)1);return FALSE;}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CREATION_FAIL line=%d error=%lu live=%u\n",__LINE__,GetLastError(),live);return 1;}}while(0)
int main(void)
{
    WNDCLASSW cls={.lpfnWndProc=DefWindowProcW,.hInstance=GetModuleHandleW(NULL),.lpszClassName=class_name};
    CHECK(RegisterClassW(&cls));
    HWND warm=create();CHECK(warm && DestroyWindow(warm));
    fail_next=TRUE;CHECK(!style_creation_begin(TRUE,FALSE,&operations) && !live);
    style_creation_scope *outer=style_creation_begin(TRUE,FALSE,&operations);CHECK(outer);
    HWND a=create();CHECK(a && attached(a));
    style_creation_scope *inner=style_creation_begin(FALSE,FALSE,&operations);CHECK(inner);
    HWND b=create();CHECK(b && !attached(b));
    style_creation_result result={0};CHECK(style_creation_end(inner,&result) && !result.installed && !result.failure);
    inner=style_creation_begin(TRUE,FALSE,&operations);CHECK(inner);
    HWND c=create();CHECK(c && attached(c));
    CHECK(!style_creation_end(outer,NULL) && GetLastError()==ERROR_INVALID_STATE);
    CHECK(style_creation_end(inner,&result) && result.installed==1 && !result.failure);
    HWND d=create();CHECK(d && attached(d));
    CHECK(style_creation_end(outer,&result) && result.installed==2 && !result.failure);
    CHECK(!live && !current && !creation_before_hook && !creation_return_hook && attached(a) && attached(c));
    CHECK(DestroyWindow(a) && DestroyWindow(b) && DestroyWindow(c) && DestroyWindow(d));
    for(mode=1;mode<4;++mode){
        outer=style_creation_begin(TRUE,FALSE,&operations);CHECK(outer);
        HWND window=create();CHECK(mode==1?!window:window!=NULL);
        CHECK(style_creation_end(outer,&result));
        if(mode==2)CHECK(result.unsupported==1 && !result.failure && unsupported_count==1);
        else CHECK(result.failure==ERROR_ACCESS_DENIED);
        if(window)CHECK(!attached(window) && DestroyWindow(window));
        CHECK(!live && !current && !creation_before_hook && !creation_return_hook);
    }
    mode=0;outer=style_creation_begin(TRUE,FALSE,&operations);CHECK(outer);
    fail_next=TRUE;CHECK(!create());
    CHECK(style_creation_end(outer,&result) && result.failure==ERROR_NOT_ENOUGH_MEMORY && !live);
    style_creation_ops subclass=operations;subclass.subclass_dialogs=TRUE;
    outer=style_creation_begin(TRUE,FALSE,&subclass);CHECK(outer && creation_return_hook);
    inner=style_creation_begin(TRUE,TRUE,&subclass);CHECK(inner && !creation_return_hook);
    struct {DLGTEMPLATE d;WORD menu,cls,title;} t={.d={.style=WS_POPUP|WS_CAPTION|WS_SYSMENU,.cx=80,.cy=40}};
    CHECK(!style_creation_dialog_observer(initialized));
    HWND window=CreateDialogIndirectParamW(cls.hInstance,&t.d,NULL,dialog,0);CHECK(window && attached(window) && dialog_count==1);
    CHECK(style_creation_dialog_observer(NULL)==initialized);
    CHECK(style_creation_end(inner,&result) && result.installed==1 && !result.failure && creation_return_hook);
    CHECK(style_creation_end(outer,&result) && !result.installed && !result.failure && !live && DestroyWindow(window));
    outer=style_creation_begin(TRUE,FALSE,&subclass);CHECK(outer && creation_return_hook);
    inner=style_creation_begin(TRUE,TRUE,&subclass);CHECK(inner && !creation_return_hook);
    fail_return_hook=TRUE;
    CHECK(style_creation_end(inner,&result) && result.failure==ERROR_NOT_ENOUGH_MEMORY && !creation_return_hook);
    CHECK(style_creation_end(outer,&result) && result.failure==ERROR_NOT_ENOUGH_MEMORY && !live && !current && !creation_before_hook);
    outer=style_creation_begin(TRUE,FALSE,&subclass);CHECK(outer && creation_return_hook);
    CHECK(style_creation_end(outer,NULL) && !live && !current && !creation_before_hook && !creation_return_hook);
    CHECK(UnregisterClassW(class_name,cls.hInstance));
    printf("CREATION_PUBLIC_OK bits=%u nested=1 barrier=1 order=1 prepare_failure=1 unsupported=1 install_failure=1 oom=1 subclass=1 restore_failure=1 allocations=0\n",(unsigned)(sizeof(void *)*8));return 0;
}
