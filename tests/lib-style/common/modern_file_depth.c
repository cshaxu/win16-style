#include <windows.h>
#include <objbase.h>
#include <stdio.h>
#include "lib-style/common/modern_file_dialog.h"

static int calls,uninitializes,failed,init_failure;
static HRESULT WINAPI fake_initialize(LPVOID reserved,DWORD flags)
{(void)reserved;(void)flags;return init_failure?RPC_E_CHANGED_MODE:S_OK;}
static void WINAPI fake_uninitialize(void){++uninitializes;}
static DWORD WINAPI other_thread(void *unused)
{(void)unused;return style_dialog_modern_active()?1:0;}
static HRESULT WINAPI fake_create(REFCLSID cls,LPUNKNOWN outer,DWORD context,REFIID iid,LPVOID *out)
{
    (void)cls;(void)outer;(void)context;(void)iid;(void)out;
    if(!style_dialog_modern_active())failed=1;
    if(++calls==1){
        HANDLE thread=CreateThread(NULL,0,other_thread,NULL,0,NULL);DWORD code=1;
        if(!thread || WaitForSingleObject(thread,3000)!=WAIT_OBJECT_0 || !GetExitCodeThread(thread,&code) || code)failed=1;
        if(thread)CloseHandle(thread);
        WCHAR path[4];
        if(style_dialog_show_modern(NULL,1,path,4,NULL)!=E_FAIL || !style_dialog_modern_active())failed=1;
    }
    return E_FAIL;
}
#define CoInitializeEx fake_initialize
#define CoUninitialize fake_uninitialize
#define CoCreateInstance fake_create
#include "lib-style/common/modern_file_dialog.c"
#undef CoInitializeEx
#undef CoUninitialize
#undef CoCreateInstance
int main(void)
{
    WCHAR path[4]=L"x";
    if(style_dialog_modern_active() || style_dialog_show_modern(NULL,1,path,4,NULL)!=E_FAIL ||
       failed || calls!=2 || uninitializes!=2 || style_dialog_modern_active() || path[0])return 1;
    init_failure=1;
    if(style_dialog_show_modern(NULL,1,path,4,NULL)!=RPC_E_CHANGED_MODE ||
       calls!=2 || uninitializes!=2 || style_dialog_modern_active())return 2;
    puts("MODERN_DIALOG_DEPTH_OK nesting=1 thread_local=1 failure_cleanup=1");return 0;
}
