#include <windows.h>
#include <stdio.h>
#include <wchar.h>

static LRESULT CALLBACK procedure(HWND window,UINT message,WPARAM wparam,LPARAM lparam)
{
    if(message==WM_APP+1){
        const WCHAR *style=sizeof(void *)==8?L"ntw16s64.dll":L"ntw16s32.dll";
        const WCHAR *shared=sizeof(void *)==8?L"ntwshk64.dll":L"ntwshk32.dll";
        return GetModuleHandleW(style) && GetModuleHandleW(shared);
    }
    if(message==WM_CLOSE){DestroyWindow(window);PostQuitMessage(0);return 0;}
    return DefWindowProcW(window,message,wparam,lparam);
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==3 && !wcscmp(argv[1],L"--child")){
        WNDCLASSW cls={.lpfnWndProc=procedure,.hInstance=GetModuleHandleW(NULL),.lpszClassName=L"SharedHookProbe"};
        if(!RegisterClassW(&cls))return 1;
        HWND window=CreateWindowW(cls.lpszClassName,argv[2],WS_OVERLAPPED,0,0,100,100,NULL,NULL,cls.hInstance,NULL);
        if(!window)return 2;
        MSG message;while(GetMessageW(&message,NULL,0,0)>0){TranslateMessage(&message);DispatchMessageW(&message);}
        return 0;
    }
    if(argc!=2)return 3;
    WCHAR self[32768],title[80],command[32896];
    if(!GetModuleFileNameW(NULL,self,ARRAYSIZE(self)))return 4;
    swprintf(title,ARRAYSIZE(title),L"SharedHookProbe-%lu",GetCurrentProcessId());
    if(swprintf(command,ARRAYSIZE(command),L"\"%ls\" --child \"%ls\"",self,title)<0)return 4;
    STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child={0};
    if(!CreateProcessW(self,command,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&startup,&child))return 5;
    HWND window=NULL;for(unsigned i=0;i<100 && !window;++i){window=FindWindowW(L"SharedHookProbe",title);if(!window)Sleep(20);}
    HMODULE module=NULL;HHOOK hook=NULL;BOOL observed=FALSE;DWORD error=0;
    if(window){
        module=LoadLibraryExW(argv[1],NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_SYSTEM32);
        if(module){
            HOOKPROC callback=(HOOKPROC)(void *)GetProcAddress(module,"Ntw16sGlobalHook");
            DWORD thread=GetWindowThreadProcessId(window,NULL);
            if(callback && thread)hook=SetWindowsHookExW(WH_CALLWNDPROC,callback,module,thread);
            if(hook)observed=SendMessageW(window,WM_APP+1,0,0)!=0;
            else error=GetLastError();
        }else error=GetLastError();
    }else error=ERROR_TIMEOUT;
    if(hook)UnhookWindowsHookEx(hook);
    if(module)FreeLibrary(module);
    if(window)PostMessageW(window,WM_CLOSE,0,0);
    if(WaitForSingleObject(child.hProcess,5000)!=WAIT_OBJECT_0){TerminateProcess(child.hProcess,ERROR_TIMEOUT);WaitForSingleObject(child.hProcess,1000);}
    CloseHandle(child.hThread);CloseHandle(child.hProcess);
    printf("SHARED HOOK observed=%d error=%lu\n",observed,error);
    return observed?0:6;
}
