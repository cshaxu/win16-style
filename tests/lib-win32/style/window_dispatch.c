#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "lib-win32/style/window_dispatch.h"
static const WCHAR klass[]=L"StyleDispatchOwnedTarget";
static LRESULT CALLBACK procedure(HWND w,UINT m,WPARAM a,LPARAM b)
{if(m==WM_CLOSE){DestroyWindow(w);PostQuitMessage(0);return 0;}return DefWindowProcW(w,m,a,b);}
static int child(const WCHAR *title)
{
    WNDCLASSW c={0};c.lpfnWndProc=procedure;c.hInstance=GetModuleHandleW(NULL);c.lpszClassName=klass;
    if(!RegisterClassW(&c))return 1;
    HWND w=CreateWindowW(klass,title,WS_OVERLAPPEDWINDOW,0,0,80,60,NULL,NULL,c.hInstance,NULL);
    if(!w)return 2;
    MSG message;while(GetMessageW(&message,NULL,0,0)>0){TranslateMessage(&message);DispatchMessageW(&message);}
    return 0;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==3 && !lstrcmpW(argv[1],L"--child"))return child(argv[2]);
    WCHAR image[MAX_PATH],dll[MAX_PATH],title[80],command[MAX_PATH*2];
    if(!GetModuleFileNameW(NULL,image,MAX_PATH))return 1;
    lstrcpynW(dll,image,MAX_PATH);WCHAR *slash=wcsrchr(dll,L'\\');if(!slash)return 2;
    if(wcscpy_s(slash+1,MAX_PATH-(size_t)(slash+1-dll),L"core-style-dispatch-fixture.dll"))return 2;
    swprintf(title,ARRAYSIZE(title),L"StyleDispatch-%lu",GetCurrentProcessId());
    if(swprintf(command,ARRAYSIZE(command),L"\"%ls\" --child %ls",image,title)<0)return 2;
    STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION process={0};
    if(!CreateProcessW(image,command,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&startup,&process))return 3;
    HWND window=NULL;for(unsigned i=0;i<250 && !window;++i){
        if(WaitForSingleObject(process.hProcess,0)!=WAIT_TIMEOUT)break;
        window=FindWindowW(klass,title);if(!window)Sleep(20);
    }
    DWORD pid=0,thread=window?GetWindowThreadProcessId(window,&pid):0;
    BOOL ok=window && thread && pid==process.dwProcessId;
    const WCHAR *message=L"StyleDispatch.Test.Message",*result=L"StyleDispatch.Test.Result";
    DWORD a=ERROR_INVALID_DATA,b=a,c=a,missing=a;
    if(ok){
        a=win32_style_window_dispatch(window,thread,dll,"fixture_hook",message,result,1,0,3000);
        b=win32_style_window_dispatch(window,thread,dll,"fixture_hook",message,result,2,0,3000);
        c=win32_style_window_dispatch(window,thread,dll,"fixture_hook",message,result,3,0,3000);
        missing=win32_style_window_dispatch(window,thread,dll,"fixture_missing",message,result,1,0,3000);
        ok=a==ERROR_SUCCESS && b==ERROR_ACCESS_DENIED && c==ERROR_INVALID_DATA &&
            missing==ERROR_PROC_NOT_FOUND && !GetPropW(window,result);
    }
    if(window)PostMessageW(window,WM_CLOSE,0,0);
    if(WaitForSingleObject(process.hProcess,5000)!=WAIT_OBJECT_0){TerminateProcess(process.hProcess,1);WaitForSingleObject(process.hProcess,1000);ok=FALSE;}
    CloseHandle(process.hThread);CloseHandle(process.hProcess);
    printf("STYLE DISPATCH child=%d success=%lu denied=%lu no_result=%lu missing=%lu\n",pid!=GetCurrentProcessId(),a,b,c,missing);
    return ok?0:4;
}
