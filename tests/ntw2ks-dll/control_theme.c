#include <windows.h>
#include <stdio.h>
#include <uxtheme.h>
#include <string.h>
#include "lib-style/common/theme_state.h"

typedef BOOL (__cdecl *window_op)(HWND);
typedef BOOL (*retained_op)(HWND);
typedef HRESULT (__cdecl *theme_op)(HWND,LPCWSTR,LPCWSTR);
typedef struct thread_case {HWND window;window_op attach,attached,remove;} thread_case;
static DWORD WINAPI wrong_thread(void *value)
{
    thread_case *test=value;
    if(test->attach(test->window) || GetLastError()!=ERROR_INVALID_THREAD_ID)return 1;
    if(test->attached(test->window) || GetLastError()!=ERROR_INVALID_THREAD_ID)return 2;
    if(test->remove(test->window) || GetLastError()!=ERROR_INVALID_THREAD_ID)return 3;
    return 0;
}
static HRESULT no_theme(HWND window,LPCWSTR application,LPCWSTR ids)
{(void)window;(void)application;(void)ids;return S_OK;}
static WCHAR observed[64];
static HRESULT observe_theme(HWND window,LPCWSTR application,LPCWSTR ids)
{(void)window;(void)ids;lstrcpynW(observed,application?application:L"<null>",ARRAYSIZE(observed));return S_OK;}

int main(int argc,char **argv)
{
    HMODULE common=LoadLibraryW(sizeof(void *)==8?L"ntwshk64.dll":L"ntwshk32.dll");
    HMODULE win16=LoadLibraryW(sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    HMODULE win2k=LoadLibraryW(sizeof(void *)==8?L"ntw2ks64.dll":L"ntw2ks32.dll");
    if(!common || !win16 || !win2k)return 1;
    window_op attach=(void *)GetProcAddress(win2k,"Ntw2ksWindowAttach");
    window_op attached=(void *)GetProcAddress(win2k,"Ntw2ksWindowAttached");
    window_op remove=(void *)GetProcAddress(win2k,"Ntw2ksWindowRemove");
    retained_op retained=(void *)GetProcAddress(common,"style_theme_retained");
    BOOL (*capture)(HWND)=(void *)GetProcAddress(common,"style_theme_capture");
    BOOL (*suppress)(HWND,const void *,style_native_theme_fn)=(void *)GetProcAddress(common,"style_theme_suppress");
    void (*release)(HWND,const void *,style_native_theme_fn)=(void *)GetProcAddress(common,"style_theme_remove");
    theme_op set=(void *)GetProcAddress(win16,"Ntw16sSetWindowTheme");
    if(!attach || !attached || !remove || !retained || !capture || !suppress || !release || !set)return 2;
    HWND parent=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,120,80,NULL,NULL,GetModuleHandleW(NULL),NULL);
    HWND child=parent?CreateWindowW(L"BUTTON",L"",WS_CHILD,0,0,60,30,parent,NULL,GetModuleHandleW(NULL),NULL):NULL;
    if(!child){if(parent)DestroyWindow(parent);return 3;}
    if(argc==2 && !strcmp(argv[1],"--hooked")){
        window_op old_attached=(void *)GetProcAddress(win16,"Ntw16sWindowAttached");
        window_op old_remove=(void *)GetProcAddress(win16,"Ntw16sWindowRemove");
        BOOL old=old_attached && old_remove && old_attached(child) && old_remove(child);
        BOOL changed_style=old && attach(child) && attached(child) && !old_attached(child);
        BOOL restored_style=changed_style && remove(child) && !attached(child);
        HWND probe=CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,20,20,parent,NULL,GetModuleHandleW(NULL),NULL);
        if(probe && old_attached && old_remove && old_attached(probe))old_remove(probe);
        int owner=0;
        BOOL tracked=probe && capture(probe) && suppress(probe,&owner,observe_theme) &&
            SUCCEEDED(SetWindowTheme(probe,L"Explorer",NULL));
        if(tracked)release(probe,&owner,observe_theme);
        tracked=tracked && !lstrcmpW(observed,L"Explorer");
        printf("HOOKED THEME bits=%u win16=%d win2k=%d removed=%d tracked=%d\n",
               (unsigned)(sizeof(void *)*8),old,changed_style,restored_style,tracked);
        DestroyWindow(parent);FreeLibrary(win2k);FreeLibrary(win16);FreeLibrary(common);
        return restored_style && tracked?0:5;
    }
    int foreign_owner=0;
    BOOL foreign=capture(child) && suppress(child,&foreign_owner,no_theme);
    BOOL refused=foreign && !attach(child) && GetLastError()==ERROR_BUSY && retained(child);
    release(child,&foreign_owner,no_theme);
    HWND unsupported=CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,10,10,parent,NULL,GetModuleHandleW(NULL),NULL);
    BOOL rejected=unsupported && !attach(unsupported) && GetLastError()==ERROR_NOT_SUPPORTED;
    BOOL applied=attach(child),seen=attached(child),held=retained(child);
    thread_case wrong={child,attach,attached,remove};DWORD wrong_result=1;
    HANDLE thread=CreateThread(NULL,0,wrong_thread,&wrong,0,NULL);
    if(thread){if(WaitForSingleObject(thread,3000)==WAIT_OBJECT_0)GetExitCodeThread(thread,&wrong_result);CloseHandle(thread);}
    HRESULT changed=set(child,L"Explorer",NULL);
    BOOL removed=remove(child),released=!retained(child);
    BOOL repeat=!remove(child);
    printf("WIN2K CONTROL THEME bits=%u foreign=%d rejected=%d apply=%d attached=%d retained=%d wrong_thread=%lu change=%08lx remove=%d released=%d repeat=%d\n",
           (unsigned)(sizeof(void *)*8),refused,rejected,applied,seen,held,(unsigned long)wrong_result,(unsigned long)changed,removed,released,repeat);
    DestroyWindow(parent);FreeLibrary(win2k);FreeLibrary(win16);FreeLibrary(common);
    return refused && rejected && applied && seen && held && !wrong_result && SUCCEEDED(changed) && removed && released && repeat?0:4;
}
