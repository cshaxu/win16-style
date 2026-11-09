#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "run16s-exe/console_host.h"

#define CHECK(x) do { if(!(x)){fprintf(stderr,"console host failure line=%d error=%lu\n",__LINE__,GetLastError());return 1;} } while(0)

int wmain(void)
{
    WCHAR path[MAX_PATH];UINT length=GetSystemDirectoryW(path,ARRAYSIZE(path));
    CHECK(length && length+13<ARRAYSIZE(path));
    wcscat(path,L"\\conhost.exe");
    CHECK(run16s_console_host_image(path));
    CHECK(!run16s_console_host_image(NULL));
    CHECK(!run16s_console_host_image(L"C:\\other\\conhost.exe"));
    CHECK(!run16s_console_host_image(L"C:\\Windows\\System32\\cmd.exe"));
    CHECK(run16s_console_host_attach(NULL,NULL)==ERROR_INVALID_PARAMETER);
    STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child={0};
    WCHAR command[]=L"cmd.exe /c exit";
    CHECK(CreateProcessW(NULL,command,NULL,NULL,FALSE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&startup,&child));
    CHECK(TerminateProcess(child.hProcess,0));
    CHECK(WaitForSingleObject(child.hProcess,3000)==WAIT_OBJECT_0);
    CHECK(run16s_console_host_attach(child.hProcess,L"")==ERROR_PROCESS_ABORTED);
    CloseHandle(child.hThread);CloseHandle(child.hProcess);
    return 0;
}
