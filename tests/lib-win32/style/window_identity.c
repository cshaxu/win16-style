#include "lib-win32/style/window_identity.h"
#include <stdio.h>
typedef struct fixture {HANDLE ready,done;HWND window;} fixture;
static LRESULT CALLBACK local_proc(HWND window,UINT message,WPARAM wp,LPARAM lp)
{return DefWindowProcW(window,message,wp,lp);}
static DWORD WINAPI worker(void *value)
{
    fixture *f=value;
    f->window=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,20,20,NULL,NULL,GetModuleHandleW(NULL),NULL);
    SetEvent(f->ready);
    WaitForSingleObject(f->done,5000);
    if(f->window)DestroyWindow(f->window);
    return 0;
}
int main(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);
    WNDCLASSW cls={.lpfnWndProc=local_proc,.hInstance=instance,.lpszClassName=L"StyleIdentityFixture"};
    if(!RegisterClassW(&cls))return 3;
    fixture f={0};f.ready=CreateEventW(NULL,TRUE,FALSE,NULL);f.done=CreateEventW(NULL,TRUE,FALSE,NULL);
    if(!f.ready || !f.done)return 1;
    HWND own=CreateWindowW(L"STATIC",L"",WS_OVERLAPPED,0,0,20,20,NULL,NULL,GetModuleHandleW(NULL),NULL);
    HWND child=own?CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,10,10,own,NULL,GetModuleHandleW(NULL),NULL):NULL;
    HMODULE owner=child?(HMODULE)GetClassLongPtrW(child,GCLP_HMODULE):NULL;
    HANDLE thread=CreateThread(NULL,0,worker,&f,0,NULL);
    BOOL ok=own && child && owner && thread && WaitForSingleObject(f.ready,5000)==WAIT_OBJECT_0 && f.window &&
        win32_style_window_current_thread(own) && !win32_style_window_current_thread(f.window) &&
        !win32_style_window_current_thread(NULL) &&
        win32_style_window_class(own,L"Static") && !win32_style_window_class(own,L"STATIC") &&
        !win32_style_window_class(NULL,L"Static") &&
        win32_style_window_owned_child(child,owner,own) && !win32_style_window_owned_child(child,NULL,own) &&
        !win32_style_window_owned_child(child,GetModuleHandleW(NULL),own) && !win32_style_window_owned_child(child,owner,f.window) &&
        !win32_style_window_owned_child(own,owner,own) &&
        win32_style_class_procedure_owned(instance,cls.lpszClassName) &&
        !win32_style_class_procedure_owned(instance,L"NoSuchStyleFixture") &&
        !win32_style_class_procedure_owned(instance,L"STATIC");
    SetEvent(f.done);
    if(thread){ok=WaitForSingleObject(thread,5000)==WAIT_OBJECT_0 && ok;CloseHandle(thread);}
    if(own)DestroyWindow(own);
    CloseHandle(f.ready);CloseHandle(f.done);UnregisterClassW(cls.lpszClassName,instance);
    if(!ok)return 2;
    puts("WINDOW_IDENTITY_OK own=1 other_thread=1 null=1 owned_child=1 cleanup=1");return 0;
}
