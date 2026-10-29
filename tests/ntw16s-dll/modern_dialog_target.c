/* Owned-process probe of the explicit modern-dialog ABI, including cancellation. */
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
typedef HRESULT (__cdecl *show_fn)(HWND,UINT,PWSTR,DWORD);
typedef BOOL (__cdecl *attached_fn)(HWND);
static DWORD gui_thread;
static volatile LONG seen, modern_child, frame, scheduled;
static attached_fn attached;
static BOOL CALLBACK child_scan(HWND w,LPARAM arg)
{
    (void)arg;WCHAR cls[128];
    if(GetClassNameW(w,cls,ARRAYSIZE(cls)) &&
       (!lstrcmpiW(cls,L"DirectUIHWND") || !lstrcmpiW(cls,L"DUIViewWndClassName")))
        InterlockedExchange(&modern_child,1);
    return TRUE;
}
static void CALLBACK finish(HWND w,UINT message,UINT_PTR id,DWORD time)
{
    (void)message;(void)time;KillTimer(w,id);
    InterlockedExchange(&frame,attached && attached(w));
    InterlockedExchange(&seen,1);PostMessageW(w,WM_CLOSE,0,0);
}
static BOOL CALLBACK window_scan(HWND w,LPARAM arg)
{
    (void)arg;WCHAR cls[128];
    if(!IsWindowVisible(w) || !GetClassNameW(w,cls,ARRAYSIZE(cls)) || lstrcmpW(cls,L"#32770"))return TRUE;
    EnumChildWindows(w,child_scan,0);
    if(!modern_child)return TRUE;
    if(InterlockedExchange(&scheduled,1))return FALSE;
    WCHAR title[128];GetWindowTextW(w,title,ARRAYSIZE(title));
    printf("MODERN_WINDOW class=%ls title=%ls style=%08lx ex=%08lx\n",cls,title,(unsigned long)GetWindowLongW(w,GWL_STYLE),(unsigned long)GetWindowLongW(w,GWL_EXSTYLE));
    SetTimer(w,931,200,finish);
    return FALSE;
}
static DWORD WINAPI observe(void *arg)
{
    (void)arg;
    for(unsigned i=0;i<200 && !seen;++i){EnumThreadWindows(gui_thread,window_scan,0);if(!seen)Sleep(50);}
    if(!seen)PostThreadMessageW(gui_thread,WM_QUIT,0,0);
    return 0;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc!=2)return 2;
    UINT kind=(UINT)wcstoul(argv[1],NULL,10);
    HMODULE module=GetModuleHandleW(sizeof(void*)==4?L"ntw16s32.dll":L"ntw16s64.dll");
    if(!module)return 3;
    FARPROC p=GetProcAddress(module,"Ntw16sShowModernFileDialog");show_fn show=NULL;memcpy(&show,&p,sizeof(show));
    p=GetProcAddress(module,"Ntw16sWindowAttached");memcpy(&attached,&p,sizeof(attached));
    if(!show || !attached)return 4;
    WCHAR path[1024]=L"unchanged";
    if(show(NULL,0,path,ARRAYSIZE(path))!=E_INVALIDARG || show(NULL,1,NULL,0)!=E_INVALIDARG)return 5;
    gui_thread=GetCurrentThreadId();HANDLE worker=CreateThread(NULL,0,observe,NULL,0,NULL);if(!worker)return 6;
    HRESULT hr=show(NULL,kind,path,ARRAYSIZE(path));
    DWORD wait=WaitForSingleObject(worker,11000);CloseHandle(worker);
    printf("MODERN_API bits=%u kind=%u seen=%ld modern=%ld frame=%ld hr=%08lx path_empty=%d wait=%lu\n",
        (unsigned)(sizeof(void*)*8),kind,seen,modern_child,frame,(unsigned long)hr,path[0]==0,wait);
    return wait==WAIT_OBJECT_0 && seen && modern_child && frame && path[0]==0 && hr==HRESULT_FROM_WIN32(ERROR_CANCELLED)?0:1;
}
