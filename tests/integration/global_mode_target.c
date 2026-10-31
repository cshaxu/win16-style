#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <wchar.h>
#define QUERY_STYLE (WM_APP+71)
#define CREATE_EXTRA (WM_APP+72)
#define CREATE_PROCESS (WM_APP+73)
#define RESET_CLICKS (WM_APP+76)
#define QUERY_CLICKS (WM_APP+77)
#define QUERY_DPI (WM_APP+78)
static HINSTANCE instance;
static unsigned clicks;
static const WCHAR cls[]=L"Win16GlobalE2ETarget";
typedef BOOL (__cdecl *hit_fn)(HWND,LPARAM,LRESULT *);
static LRESULT CALLBACK window_proc(HWND w,UINT message,WPARAM a,LPARAM b)
{
    if(message==QUERY_STYLE){
        HMODULE dll=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");
        hit_fn hit=dll?(hit_fn)(void *)GetProcAddress(dll,"win16_style_hit_test"):NULL;
        RECT r;LRESULT result=0;if(!hit || !GetWindowRect(w,&r))return 0;
        return hit(w,MAKELPARAM((r.left+r.right)/2,(r.top+r.bottom)/2),&result)?1:0;
    }
    if(message==CREATE_EXTRA){
        WCHAR title[80];swprintf(title,ARRAYSIZE(title),L"Win16GlobalExtra-%lu",GetCurrentProcessId());
        HWND child=CreateWindowW(cls,title,WS_OVERLAPPEDWINDOW,90,90,300,180,NULL,NULL,instance,NULL);
        if(child)ShowWindow(child,SW_SHOW);
        return child!=NULL;
    }
    if(message==CREATE_PROCESS){
        WCHAR image[32768],command[33000];DWORD n=GetModuleFileNameW(NULL,image,ARRAYSIZE(image));
        if(!n || n>=ARRAYSIZE(image) || swprintf(command,ARRAYSIZE(command),L"\"%ls\" Win16GlobalChild-%lu",image,GetCurrentProcessId())<0)return 0;
        STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child={0};
        if(!CreateProcessW(image,command,NULL,NULL,FALSE,0,NULL,NULL,&startup,&child))return 0;
        CloseHandle(child.hThread);CloseHandle(child.hProcess);return 1;
    }
    if(message==RESET_CLICKS){clicks=0;return 1;}
    if(message==QUERY_CLICKS)return clicks;
    if(message==QUERY_DPI)return GetDpiForWindow(w);
    if(message==WM_COMMAND && LOWORD(a)==1 && HIWORD(a)==BN_CLICKED){++clicks;return 0;}
    if(message==WM_CLOSE){DestroyWindow(w);return 0;}
    if(message==WM_DESTROY){PostQuitMessage(0);return 0;}
    return DefWindowProcW(w,message,a,b);
}
int wmain(int argc,WCHAR **argv)
{
    if(argc!=2)return 2;
    WCHAR option[4];BOOL unaware=GetEnvironmentVariableW(L"WIN16S_TEST_DPI_UNAWARE",option,ARRAYSIZE(option))==1 && option[0]==L'1';
    if(!SetProcessDpiAwarenessContext(unaware?DPI_AWARENESS_CONTEXT_UNAWARE:DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2))return 7;
    instance=GetModuleHandleW(NULL);WNDCLASSW c={0};
    c.hInstance=instance;c.lpfnWndProc=window_proc;c.lpszClassName=cls;
    if(!RegisterClassW(&c))return 3;
    HWND w=CreateWindowW(cls,argv[1],WS_OVERLAPPEDWINDOW,50,50,400,260,NULL,NULL,instance,NULL);if(!w)return 4;
    if(!CreateWindowW(L"BUTTON",L"Action",WS_CHILD|WS_VISIBLE|BS_PUSHBUTTON,20,20,100,28,w,(HMENU)1,instance,NULL))return 5;
    if(!CreateWindowW(L"EDIT",L"Original text",WS_CHILD|WS_VISIBLE|WS_BORDER,20,60,180,28,w,(HMENU)2,instance,NULL))return 6;
    ShowWindow(w,SW_SHOW);UpdateWindow(w);
    MSG m;while(GetMessageW(&m,NULL,0,0)>0){TranslateMessage(&m);DispatchMessageW(&m);}return 0;
}
