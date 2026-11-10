#include <windows.h>
#include <wchar.h>

int wmain(int argc,WCHAR **argv)
{
    const WCHAR *style=sizeof(void *)==8?L"ntw2ks64.dll":L"ntw2ks32.dll";
    const WCHAR *shared=sizeof(void *)==8?L"ntwshk64.dll":L"ntwshk32.dll";
    if(!GetModuleHandleW(style) || !GetModuleHandleW(shared))return 1;
    if(argc==2){
        STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child={0};DWORD code=1;
        if(!CreateProcessW(argv[1],NULL,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&startup,&child))return 2;
        DWORD wait=WaitForSingleObject(child.hProcess,10000);
        if(wait!=WAIT_OBJECT_0){TerminateProcess(child.hProcess,ERROR_TIMEOUT);WaitForSingleObject(child.hProcess,5000);}
        else if(!GetExitCodeProcess(child.hProcess,&code))code=1;
        CloseHandle(child.hThread);CloseHandle(child.hProcess);return code?3:0;
    }
    return argc==1?0:4;
}
