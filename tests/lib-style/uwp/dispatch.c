#include "lib-style/uwp/dispatch.h"
#include "lib-win32/style/window_dispatch.h"
#include <stdio.h>
#include <wchar.h>
static DWORD action(const style_uwp_target *target,BOOL apply,WPARAM *out,void *context)
{
    if(!target||!context)return ERROR_INVALID_PARAMETER;
    if(*(BOOL *)context)return ERROR_CANCELLED;
    *out=apply?1:2;return ERROR_SUCCESS;
}
typedef struct worker_context {HANDLE ready;HWND frame,core;} worker_context;
static DWORD WINAPI worker(void *value)
{
    worker_context *context=value;HINSTANCE instance=GetModuleHandleW(NULL);
    context->frame=CreateWindowW(L"ApplicationFrameWindow",L"",WS_POPUP,0,0,50,50,NULL,NULL,instance,NULL);
    context->core=context->frame?CreateWindowW(L"Windows.UI.Core.CoreWindow",L"",WS_CHILD,0,0,20,20,context->frame,NULL,instance,NULL):NULL;
    MSG message;PeekMessageW(&message,NULL,WM_USER,WM_USER,PM_NOREMOVE);SetEvent(context->ready);
    if(context->core)while(GetMessageW(&message,NULL,0,0)>0){TranslateMessage(&message);DispatchMessageW(&message);}
    if(context->frame)DestroyWindow(context->frame);
    return context->core?0:1;
}
int wmain(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);WCHAR path[MAX_PATH];
    WNDCLASSW frame_class={.lpfnWndProc=DefWindowProcW,.hInstance=instance,.lpszClassName=L"ApplicationFrameWindow"};
    WNDCLASSW core_class={.lpfnWndProc=DefWindowProcW,.hInstance=instance,.lpszClassName=L"Windows.UI.Core.CoreWindow"};
    if(!GetModuleFileNameW(NULL,path,MAX_PATH))return 1;
    WCHAR *slash=wcsrchr(path,L'\\');if(!slash)return 2;
    if(wcscpy_s(slash+1,MAX_PATH-(size_t)(slash+1-path),L"core-style-uwp-hook-fixture.dll"))return 3;
    HMODULE diagnostic=LoadLibraryExW(path,NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
    DWORD load_error=diagnostic?0:GetLastError();
    HOOKPROC diagnostic_proc=diagnostic?(HOOKPROC)(void *)GetProcAddress(diagnostic,"fixture_hook"):NULL;
    DWORD export_error=diagnostic_proc?0:GetLastError();
    HHOOK diagnostic_hook=diagnostic_proc?SetWindowsHookExW(WH_CALLWNDPROC,diagnostic_proc,diagnostic,GetCurrentThreadId()):NULL;
    DWORD hook_error=diagnostic_hook?0:GetLastError();
    if(diagnostic_hook)UnhookWindowsHookEx(diagnostic_hook);
    HHOOK local_hook=diagnostic_proc?SetWindowsHookExW(WH_CALLWNDPROC,diagnostic_proc,NULL,GetCurrentThreadId()):NULL;
    DWORD local_error=local_hook?0:GetLastError();
    if(local_hook)UnhookWindowsHookEx(local_hook);
    if(diagnostic)FreeLibrary(diagnostic);
    printf("UWP_DISPATCH_DIAGNOSTIC load=%lu export=%lu hook=%lu local=%lu\n",load_error,export_error,hook_error,local_error);
    if(!RegisterClassW(&frame_class)||!RegisterClassW(&core_class))return 4;
    HWND frame=CreateWindowW(frame_class.lpszClassName,L"",WS_POPUP,0,0,50,50,NULL,NULL,instance,NULL);
    HWND core=frame?CreateWindowW(core_class.lpszClassName,L"",WS_CHILD,0,0,20,20,frame,NULL,instance,NULL):NULL;
    if(!frame||!core)return 5;
    style_uwp_target target={.pid=GetCurrentProcessId(),.core=core,.frame=frame};BOOL reject=FALSE;
    WORD machine=sizeof(void *)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386;
    style_uwp_dispatch_spec spec={machine,"fixture_hook",L"StyleDispatch.Test.Message",L"StyleDispatch.Test.Result",3000,action,&reject};
    DWORD a=style_uwp_dispatch_change(&target,path,TRUE,&spec);
    DWORD b=style_uwp_dispatch_change(&target,path,FALSE,&spec);
    reject=TRUE;DWORD c=style_uwp_dispatch_change(&target,path,TRUE,&spec);reject=FALSE;
    spec.machine=machine==IMAGE_FILE_MACHINE_AMD64?IMAGE_FILE_MACHINE_I386:IMAGE_FILE_MACHINE_AMD64;
    DWORD d=style_uwp_dispatch_change(&target,path,TRUE,&spec);spec.machine=machine;
    ++target.pid;DWORD e=style_uwp_dispatch_change(&target,path,TRUE,&spec);
    DWORD f=win32_style_window_dispatch(frame,GetCurrentThreadId(),path,"fixture_hook",spec.message,spec.result_property,3,(LPARAM)core,3000);
    DWORD g=win32_style_window_dispatch(frame,GetCurrentThreadId(),path,"fixture_hook",spec.message,spec.result_property,1,0,3000);
    worker_context context={0};context.ready=CreateEventW(NULL,TRUE,FALSE,NULL);DWORD worker_id=0,worker_exit=1;
    HANDLE worker_thread=context.ready?CreateThread(NULL,0,worker,&context,0,&worker_id):NULL;
    BOOL ready=worker_thread && WaitForSingleObject(context.ready,5000)==WAIT_OBJECT_0 && context.frame && context.core;
    style_uwp_target other={.pid=GetCurrentProcessId(),.core=context.core,.frame=context.frame};
    DWORD h=ready?style_uwp_dispatch_change(&other,path,TRUE,&spec):ERROR_TIMEOUT;
    if(worker_id)PostThreadMessageW(worker_id,WM_QUIT,0,0);
    BOOL stopped=worker_thread && WaitForSingleObject(worker_thread,5000)==WAIT_OBJECT_0 &&
        GetExitCodeThread(worker_thread,&worker_exit) && worker_exit==0;
    if(worker_thread)CloseHandle(worker_thread);
    if(context.ready)CloseHandle(context.ready);
    BOOL ok=a==ERROR_SUCCESS && b==ERROR_ACCESS_DENIED && c==ERROR_CANCELLED &&
        d==ERROR_NOT_SUPPORTED && e==ERROR_INVALID_WINDOW_HANDLE && f==ERROR_INVALID_DATA &&
        g==ERROR_INVALID_WINDOW_HANDLE && h==ERROR_SUCCESS && stopped && !GetPropW(frame,spec.result_property);
    DestroyWindow(frame);UnregisterClassW(core_class.lpszClassName,instance);UnregisterClassW(frame_class.lpszClassName,instance);
    printf("UWP_DISPATCH result=%lu denied=%lu cancelled=%lu machine=%lu identity=%lu unknown=%lu invalid_core=%lu other_thread=%lu stopped=%d\n",a,b,c,d,e,f,g,h,stopped);return ok?0:6;
}
