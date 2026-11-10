#include <windows.h>
#include <string.h>

typedef BOOL (__cdecl *attached_fn)(HWND);
static LRESULT CALLBACK procedure(HWND window,UINT message,WPARAM wparam,LPARAM lparam)
{return DefWindowProcW(window,message,wparam,lparam);}

int wmain(int argc,WCHAR **argv)
{
    HMODULE style=GetModuleHandleW(sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll");
    HMODULE shared=GetModuleHandleW(sizeof(void *)==8?L"ntwshk64.dll":L"ntwshk32.dll");
    FARPROC address=style?GetProcAddress(style,"Ntw16sWindowAttached"):NULL;attached_fn attached=NULL;
    memcpy(&attached,&address,sizeof(attached));
    if(!style || !shared || !attached)return 1;
    WNDCLASSW cls={.lpfnWndProc=procedure,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"SharedRuntimeProbe"};
    if(!RegisterClassW(&cls))return 2;
    HWND window=CreateWindowW(cls.lpszClassName,L"probe",WS_OVERLAPPEDWINDOW,0,0,240,120,NULL,NULL,cls.hInstance,NULL);
    if(!window || !attached(window)){if(window)DestroyWindow(window);return 3;}
    DestroyWindow(window);UnregisterClassW(cls.lpszClassName,cls.hInstance);
    if(argc==2){
        STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child={0};DWORD code=1;
        if(!CreateProcessW(argv[1],NULL,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&startup,&child))return 4;
        DWORD wait=WaitForSingleObject(child.hProcess,10000);
        if(wait!=WAIT_OBJECT_0){TerminateProcess(child.hProcess,ERROR_TIMEOUT);WaitForSingleObject(child.hProcess,5000);}
        else if(!GetExitCodeProcess(child.hProcess,&code))code=1;
        CloseHandle(child.hThread);CloseHandle(child.hProcess);return code?5:0;
    }
    return argc==1?0:6;
}
