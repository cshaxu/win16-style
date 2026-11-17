#include <windows.h>
#include <stdio.h>
#include "../../../ntw16s-dll/exception_probe.h"
#include "lib-win32-style/application/menu/menu_owner.h"

static BOOL owners_ok=TRUE,fail_install;
static unsigned installs,removes;
static const void *last_owner;
static BOOL install(HWND w,const void *owner,const BOOL *cancelled)
{
    const style_menu_owner_scope *scope=owner;++installs;last_owner=owner;
    owners_ok=owners_ok && scope->window==w && !cancelled;
    return !fail_install;
}
static BOOL remove_owned(HWND w,const void *owner)
{
    style_menu_owner_scope *scope=(style_menu_owner_scope *)owner;++removes;
    owners_ok=owners_ok && IsWindow(w) && owner==last_owner && !scope->installed;
    style_menu_owner_scope_end(scope);SetLastError(0x8888);return TRUE;
}
static LRESULT PROCESS_HOOK_UNWIND_FUNCTION CALLBACK window_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(message!=WM_APP+1 && message!=WM_APP+2)return DefWindowProcW(window,message,wp,lp);
    style_menu_owner_scope scope={window,FALSE,remove_owned};fail_install=(wp&1)!=0;
    scope.installed=install(window,&scope,NULL);
    process_hook_unwind_scope unwind;DWORD error;LRESULT result=73;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,style_menu_owner_scope_end,&scope);
    if(wp&2)DestroyWindow(window);
    if(message==WM_APP+2)RaiseException(NTW16S_TEST_EXCEPTION_CODE,0,0,NULL);
    SetLastError(0x4321);error=GetLastError();
    PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result;
}
static int check_scope(HWND w)
{
    if(SendMessageW(w,WM_APP+1,0,0)!=73 || removes!=1 || GetLastError()!=0x4321)return 10;
    if(SendMessageW(w,WM_APP+1,1,0)!=73 || removes!=1 || installs!=2)return 11;
    if(!test_unwind(w,WM_APP+2,0,0) || removes!=2 || installs!=3)return 12;
    if(SendMessageW(w,WM_APP+1,0,0)!=73 || removes!=3 || GetLastError()!=0x4321)return 13;
    if(SendMessageW(w,WM_APP+1,2,0)!=73 || IsWindow(w) || removes!=3 || installs!=5 || !owners_ok)return 14;
    puts("MENU OWNER SCOPE PASS identity=1 normal=1 native_unwind=1 failed_install=1 destroyed=1 reentry_once=1 native_error=1");return 0;
}
int main(void)
{
    static const WCHAR name[]=L"StyleMenuOwnerTest",property[]=L"StyleMenuOwnerTest.Marker";
    static int marker,other;
    WNDCLASSW cls={0};cls.lpfnWndProc=window_proc;cls.hInstance=GetModuleHandleW(NULL);cls.lpszClassName=name;
    if(!RegisterClassW(&cls))return 1;
    HWND w=CreateWindowExW(WS_EX_TOOLWINDOW,name,L"",WS_POPUP,0,0,10,10,NULL,NULL,cls.hInstance,NULL);
    if(!w)return 2;
    int result=0;
    if(style_menu_owner_eligible(w,property,&marker))result=3;
    else if(style_menu_owner_mark(w,(const void *)window_proc,L"WrongClass",property,&marker))result=4;
    else if(style_menu_owner_mark(w,(const void *)GetProcAddress(GetModuleHandleW(L"kernel32.dll"),"GetCurrentProcessId"),name,property,&marker))result=5;
    else if(!style_menu_owner_mark(w,(const void *)window_proc,name,property,&marker))result=6;
    else if(!style_menu_owner_eligible(w,property,&marker) || style_menu_owner_eligible(w,property,&other))result=7;
    else {
        SetWindowLongPtrW(w,GWL_STYLE,WS_POPUP|WS_VISIBLE);
        if(style_menu_owner_eligible(w,property,&marker))result=8;
        SetWindowLongPtrW(w,GWL_STYLE,WS_POPUP);
        SetWindowLongPtrW(w,GWL_EXSTYLE,0);
        if(style_menu_owner_eligible(w,property,&marker))result=9;
    }
    if(!result)result=check_scope(w);
    RemovePropW(w,property);DestroyWindow(w);UnregisterClassW(name,cls.hInstance);
    return result;
}
