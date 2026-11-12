#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <uxtheme.h>
#include "lib-style/common/theme_state.h"

static WCHAR restored[64];
static HRESULT observe_theme(HWND window,LPCWSTR application,LPCWSTR ids)
{
    (void)window;(void)ids;lstrcpynW(restored,application?application:L"<null>",ARRAYSIZE(restored));return S_OK;
}

static BOOL frame_witness(const WCHAR *role)
{
    WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=DefWindowProcW,.lpszClassName=L"Win2kPropagationFrame"};
    if(!RegisterClassW(&cls))return FALSE;
    HWND w=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPEDWINDOW,80,80,320,200,NULL,NULL,cls.hInstance,NULL);
    if(!w){UnregisterClassW(cls.lpszClassName,cls.hInstance);return FALSE;}
    HMODULE product=GetModuleHandleW(sizeof(void *)==8?L"ntw2ks64.dll":L"ntw2ks32.dll");
    HMODULE common=GetModuleHandleW(sizeof(void *)==8?L"ntwshk64.dll":L"ntwshk32.dll");
    BOOL (__cdecl *attached_control)(HWND)=product?(void *)GetProcAddress(product,"Ntw2ksWindowAttached"):NULL;
    BOOL (__cdecl *remove_control)(HWND)=product?(void *)GetProcAddress(product,"Ntw2ksWindowRemove"):NULL;
    BOOL (*capture)(HWND)=common?(void *)GetProcAddress(common,"style_theme_capture"):NULL;
    BOOL (*suppress)(HWND,const void *,style_native_theme_fn)=common?(void *)GetProcAddress(common,"style_theme_suppress"):NULL;
    void (*release)(HWND,const void *,style_native_theme_fn)=common?(void *)GetProcAddress(common,"style_theme_remove"):NULL;
    HWND theme_probe=CreateWindowW(L"STATIC",L"",WS_CHILD,0,0,20,20,w,NULL,cls.hInstance,NULL);
    int theme_owner=0;
    BOOL tracked=theme_probe && capture && suppress && release && capture(theme_probe) &&
        suppress(theme_probe,&theme_owner,observe_theme) && SUCCEEDED(SetWindowTheme(theme_probe,L"Explorer",NULL));
    if(tracked)release(theme_probe,&theme_owner,observe_theme);
    tracked=tracked && !lstrcmpW(restored,L"Explorer");
    if(theme_probe)DestroyWindow(theme_probe);
    HWND control=CreateWindowW(L"BUTTON",L"Probe",WS_CHILD,0,0,50,20,w,NULL,cls.hInstance,NULL);
    BOOL control_attached=control && attached_control && attached_control(control);
    HRESULT requested=control?SetWindowTheme(control,L"Explorer",NULL):E_FAIL;
    HTHEME while_attached=control?GetWindowTheme(control):NULL;
    BOOL control_removed=control_attached && remove_control && remove_control(control) && !attached_control(control);
    HTHEME after_remove=control?GetWindowTheme(control):NULL;
    if(control)DestroyWindow(control);
    HWND destroyed_control=CreateWindowW(L"BUTTON",L"Cleanup",WS_CHILD,0,0,50,20,w,NULL,cls.hInstance,NULL);
    BOOL cleanup=destroyed_control && attached_control && attached_control(destroyed_control) &&
        DestroyWindow(destroyed_control) && !attached_control(destroyed_control);
    ShowWindow(w,SW_SHOW);UpdateWindow(w);
    TITLEBARINFOEX title={.cbSize=sizeof(title)};RECT outer={0};
    SendMessageW(w,WM_GETTITLEBARINFOEX,0,(LPARAM)&title);
    BOOL attached=GetPropW(w,L"Win2kStyle.Frame")!=NULL,measured=GetWindowRect(w,&outer);
    UINT dpi=GetDpiForWindow(w);if(!dpi)dpi=96;
    HDC dc=GetWindowDC(w);COLORREF pixel=dc?GetPixel(dc,title.rcTitleBar.left-outer.left+40,title.rcTitleBar.top-outer.top+2):CLR_INVALID;
    if(dc)ReleaseDC(w,dc);
    BOOL classic=pixel!=CLR_INVALID && ((GetBValue(pixel)>GetRValue(pixel) && GetBValue(pixel)>=90) ||
        (GetRValue(pixel)==GetGValue(pixel) && GetGValue(pixel)==GetBValue(pixel) && GetRValue(pixel)>=128 && GetRValue(pixel)<=181));
    BOOL geometry=measured && title.rcTitleBar.bottom-title.rcTitleBar.top==MulDiv(19,(int)dpi,96);
    BOOL destroyed=DestroyWindow(w);UnregisterClassW(cls.lpszClassName,cls.hInstance);
    wprintf(L"WIN2K PROPAGATION FRAME bits=%u role=%ls attached=%d geometry=%d painted=%d destroyed=%d\n",
            (unsigned)(sizeof(void*)*8),role,attached,geometry,classic,destroyed);fflush(stdout);
    wprintf(L"WIN2K PROPAGATION CONTROL bits=%u role=%ls attached=%d removed=%d cleanup=%d\n",
            (unsigned)(sizeof(void*)*8),role,control_attached,control_removed,cleanup);fflush(stdout);
    wprintf(L"WIN2K THEME REQUEST result=%08lx active=%d while=%d after=%d tracked=%d\n",
            (unsigned long)requested,IsThemeActive(),while_attached!=NULL,after_remove!=NULL,tracked);fflush(stdout);
    return attached && geometry && classic && destroyed && control_attached && control_removed && cleanup && SUCCEEDED(requested) && tracked;
}

int wmain(int argc,WCHAR **argv)
{
    const WCHAR *own=sizeof(void *)==8?L"ntw2ks64.dll":L"ntw2ks32.dll";
    const WCHAR *other=sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll";
    if(!GetModuleHandleW(own) || GetModuleHandleW(other))return 1;
    if(argc==2 && !wcscmp(argv[1],L"--frame-child"))return frame_witness(L"child")?0:5;
    if(argc==2 || (argc==3 && !wcscmp(argv[2],L"--frame"))){
        BOOL frames=argc==3;if(frames && !frame_witness(L"parent"))return 5;
        STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child={0};DWORD code=1;
        WCHAR command[32768];WCHAR *arguments=NULL;
        if(frames){if(swprintf(command,ARRAYSIZE(command),L"\"%ls\" --frame-child",argv[1])<0)return 2;arguments=command;
            startup.dwFlags=STARTF_USESTDHANDLES;startup.hStdInput=GetStdHandle(STD_INPUT_HANDLE);
            startup.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE);startup.hStdError=GetStdHandle(STD_ERROR_HANDLE);}
        if(!CreateProcessW(argv[1],arguments,NULL,NULL,frames,CREATE_NO_WINDOW,NULL,NULL,&startup,&child))return 2;
        DWORD wait=WaitForSingleObject(child.hProcess,10000);
        if(wait!=WAIT_OBJECT_0){TerminateProcess(child.hProcess,ERROR_TIMEOUT);WaitForSingleObject(child.hProcess,5000);}
        else if(!GetExitCodeProcess(child.hProcess,&code))code=1;
        CloseHandle(child.hThread);CloseHandle(child.hProcess);return code?3:0;
    }
    return argc==1?0:4;
}
