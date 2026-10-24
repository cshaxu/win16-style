#define COBJMACROS
#define _WIN32_WINNT 0x0a00
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <wchar.h>
#include <shellapi.h>
#include <string.h>
static LRESULT CALLBACK procedure(HWND w,UINT m,WPARAM p,LPARAM l)
{if(m==WM_DESTROY){PostQuitMessage(0);return 0;}return DefWindowProcW(w,m,p,l);}
int wmain(int argc,WCHAR **argv)
{
    if(argc<3)return 2;
    HANDLE token=NULL;TOKEN_ELEVATION elevation={0};DWORD size=0;
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token))return 3;
    BOOL ok=GetTokenInformation(token,TokenElevation,&elevation,sizeof(elevation),&size);CloseHandle(token);
    if(!ok||!elevation.TokenIsElevated)return 4;
    DWORD integrity=0;HANDLE integrity_token=NULL;
    if(OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&integrity_token)){
        DWORD needed=0;GetTokenInformation(integrity_token,TokenIntegrityLevel,NULL,0,&needed);
        TOKEN_MANDATORY_LABEL *label=HeapAlloc(GetProcessHeap(),0,needed);
        if(label&&GetTokenInformation(integrity_token,TokenIntegrityLevel,label,needed,&needed))integrity=*GetSidSubAuthority(label->Label.Sid,*GetSidSubAuthorityCount(label->Label.Sid)-1);
        if(label)HeapFree(GetProcessHeap(),0,label);
        CloseHandle(integrity_token);
    }
    WNDCLASSW cls={.lpfnWndProc=procedure,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"Win16StyleOwnedUacFixture",.hbrBackground=(HBRUSH)(COLOR_WINDOW+1)};
    if(!RegisterClassW(&cls))return 5;
    HWND window=CreateWindowExW(0,cls.lpszClassName,L"Win16 Style UAC integration",WS_OVERLAPPEDWINDOW|WS_VISIBLE,100,100,560,300,NULL,NULL,cls.hInstance,NULL);
    if(!window)return 6;
    PCWSTR dll_name=sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll";
    HMODULE module=GetModuleHandleW(dll_name);
    typedef BOOL (__cdecl *attached_fn)(HWND);
    FARPROC symbol=module?GetProcAddress(module,"Ntw16sWindowAttached"):NULL;
    attached_fn attached=NULL;memcpy(&attached,&symbol,sizeof(attached));
    BOOL styled=attached&&attached(window);
    WCHAR image[32768]={0},dll[32768]={0},cwd[32768]={0};GetModuleFileNameW(NULL,image,32768);GetCurrentDirectoryW(32768,cwd);
    if(module)GetModuleFileNameW(module,dll,32768);
    DWORD parent=0;HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,0);PROCESSENTRY32W pe={.dwSize=sizeof(pe)};
    if(snapshot!=INVALID_HANDLE_VALUE){for(BOOL more=Process32FirstW(snapshot,&pe);more;more=Process32NextW(snapshot,&pe))if(pe.th32ProcessID==GetCurrentProcessId()){parent=pe.th32ParentProcessID;break;}CloseHandle(snapshot);}
    FILE *file=_wfopen(argv[1],L"w, ccs=UTF-8");if(!file){DestroyWindow(window);return 7;}
    fwprintf(file,L"UAC TARGET %ls\nnonce=%ls\npid=%lu\nparent=%lu\nbits=%u\nelevated=%lu\nimage=%ls\ndll=%ls\ncwd=%ls\nargc=%d\n",styled?L"PASS":L"FAIL",argv[2],GetCurrentProcessId(),parent,(unsigned)(sizeof(void*)*8),elevation.TokenIsElevated,image,dll,cwd,argc);
    for(int i=3;i<argc;i++)fwprintf(file,L"arg%d=%ls\n",i-3,argv[i]);
    fwprintf(file,L"integrity=%lu\ndpi=%u\n",integrity,GetDpiForWindow(window));
    WCHAR env[256]={0};GetEnvironmentVariableW(L"WIN16STYLE_UAC_MARKER",env,256);fwprintf(file,L"environment_marker=%ls\n",env);fclose(file);
    /* Bounded self-owned UI; no user action or external process killing needed. */
    ULONGLONG deadline=GetTickCount64()+600;MSG message;
    while(GetTickCount64()<deadline){while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}MsgWaitForMultipleObjects(0,NULL,FALSE,20,QS_ALLINPUT);}
    WCHAR capture[32768];swprintf(capture,32768,L"%ls.bmp",argv[1]);RECT r;GetWindowRect(window,&r);
    int width=r.right-r.left,height=r.bottom-r.top;HDC screen=GetDC(NULL),memory=CreateCompatibleDC(screen);
    BITMAPINFO bitmap_info={.bmiHeader={.biSize=sizeof(BITMAPINFOHEADER),.biWidth=width,.biHeight=-height,.biPlanes=1,.biBitCount=32,.biCompression=BI_RGB}};
    void *pixels=NULL;HBITMAP bitmap=CreateDIBSection(screen,&bitmap_info,DIB_RGB_COLORS,&pixels,NULL,0);
    if(bitmap){HGDIOBJ old=SelectObject(memory,bitmap);BOOL drawn=PrintWindow(window,memory,2);DWORD bytes=(DWORD)width*(DWORD)height*4,written=0;
        BITMAPFILEHEADER header={.bfType=0x4d42,.bfSize=sizeof(header)+sizeof(BITMAPINFOHEADER)+bytes,.bfOffBits=sizeof(header)+sizeof(BITMAPINFOHEADER)};
        HANDLE output=CreateFileW(capture,GENERIC_WRITE,0,NULL,CREATE_NEW,0,NULL);
        if(output!=INVALID_HANDLE_VALUE){WriteFile(output,&header,sizeof(header),&written,NULL);WriteFile(output,&bitmap_info.bmiHeader,sizeof(BITMAPINFOHEADER),&written,NULL);WriteFile(output,pixels,bytes,&written,NULL);CloseHandle(output);}else styled=FALSE;
        if(!drawn)styled=FALSE;
        SelectObject(memory,old);DeleteObject(bitmap);
    }else styled=FALSE;
    DeleteDC(memory);ReleaseDC(NULL,screen);
    if(wcsncmp(argv[2],L"child-",6)){
        WCHAR command_line[32768],child_report[32768];swprintf(child_report,32768,L"%ls.child",argv[1]);
        swprintf(command_line,32768,L"\"%ls\" \"%ls\" child-%ls",image,child_report,argv[2]);
        PROCESS_INFORMATION child={0};STARTUPINFOW startup={.cb=sizeof(startup)};
        if(CreateProcessW(image,command_line,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&startup,&child)){
            DWORD status=WaitForSingleObject(child.hProcess,5000),code=STILL_ACTIVE;GetExitCodeProcess(child.hProcess,&code);
            if(status!=WAIT_OBJECT_0){TerminateProcess(child.hProcess,ERROR_TIMEOUT);WaitForSingleObject(child.hProcess,2000);}
            CloseHandle(child.hThread);CloseHandle(child.hProcess);if(status!=WAIT_OBJECT_0||code!=73)styled=FALSE;
        }else styled=FALSE;
    }
    DestroyWindow(window);UnregisterClassW(cls.lpszClassName,cls.hInstance);
    WCHAR done[32768];swprintf(done,32768,L"%ls.done",argv[1]);FILE *complete=_wfopen(done,L"wb");
    if(complete){fputs(styled?"PASS\n":"FAIL\n",complete);fclose(complete);}else styled=FALSE;
    return styled?73:8;
}
#ifdef UAC_GUI
int WINAPI wWinMain(HINSTANCE instance,HINSTANCE previous,PWSTR command,int show)
{(void)instance;(void)previous;(void)command;(void)show;int argc=0;WCHAR **argv=CommandLineToArgvW(GetCommandLineW(),&argc);if(!argv)return 2;int result=wmain(argc,argv);LocalFree(argv);return result;}
#endif
