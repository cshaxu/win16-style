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
int wmain(void)
{
    HINSTANCE instance=GetModuleHandleW(NULL);WCHAR path[MAX_PATH];
    WNDCLASSW frame_class={.lpfnWndProc=DefWindowProcW,.hInstance=instance,.lpszClassName=L"ApplicationFrameWindow"};
    WNDCLASSW core_class={.lpfnWndProc=DefWindowProcW,.hInstance=instance,.lpszClassName=L"Windows.UI.Core.CoreWindow"};
    if(!GetModuleFileNameW(NULL,path,MAX_PATH))return 1;
    WCHAR *slash=wcsrchr(path,L'\\');if(!slash)return 2;
    if(wcscpy_s(slash+1,MAX_PATH-(size_t)(slash+1-path),L"core-style-uwp-hook-fixture.dll"))return 3;
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
    BOOL ok=a==ERROR_SUCCESS && b==ERROR_ACCESS_DENIED && c==ERROR_CANCELLED &&
        d==ERROR_NOT_SUPPORTED && e==ERROR_INVALID_WINDOW_HANDLE && f==ERROR_INVALID_DATA &&
        g==ERROR_INVALID_WINDOW_HANDLE && !GetPropW(frame,spec.result_property);
    DestroyWindow(frame);UnregisterClassW(core_class.lpszClassName,instance);UnregisterClassW(frame_class.lpszClassName,instance);
    printf("UWP_DISPATCH result=%lu denied=%lu cancelled=%lu machine=%lu identity=%lu unknown=%lu invalid_core=%lu\n",a,b,c,d,e,f,g);return ok?0:6;
}
