#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <wchar.h>

static BOOL frame_witness(const WCHAR *role)
{
    WNDCLASSW cls={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=DefWindowProcW,.lpszClassName=L"Win2kPropagationFrame"};
    if(!RegisterClassW(&cls))return FALSE;
    HWND w=CreateWindowW(cls.lpszClassName,L"",WS_OVERLAPPEDWINDOW,80,80,320,200,NULL,NULL,cls.hInstance,NULL);
    if(!w){UnregisterClassW(cls.lpszClassName,cls.hInstance);return FALSE;}
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
    return attached && geometry && classic && destroyed;
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
