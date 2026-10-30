#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/injection/system_process.h"
#define CHECK(x) do {if(!(x)){printf("PRINT HOST POLICY FAIL line=%d\n",__LINE__);return 1;}} while(0)
int main(void)
{
    WCHAR root[MAX_PATH],path[MAX_PATH],cmd[MAX_PATH+32],fake[MAX_PATH];char ansi[MAX_PATH];
    UINT n=GetSystemWindowsDirectoryW(root,MAX_PATH);CHECK(n && n<MAX_PATH);
    CHECK(swprintf(path,MAX_PATH,L"%ls%lssplwow64.exe",root,root[n-1]==L'\\'?L"":L"\\")>0);
    CHECK(swprintf(cmd,MAX_PATH+32,L"\"%ls\" 8192",path)>0);
    CHECK(ntw16s_system_print_host_w(path,L"ignored"));CHECK(ntw16s_system_print_host_w(NULL,cmd));
    CHECK(WideCharToMultiByte(CP_ACP,0,path,-1,ansi,MAX_PATH,NULL,NULL)>0);
    CHECK(ntw16s_system_print_host_a(ansi,NULL));
    CHECK(swprintf(fake,MAX_PATH,L"%ls.bak",path)>0);
    CHECK(!ntw16s_system_print_host_w(fake,cmd));CHECK(!ntw16s_system_print_host_w(L"splwow64.exe",cmd));
    CHECK(!ntw16s_system_print_host_w(NULL,L"\"C:\\fake\\splwow64.exe\" 8192"));
    CHECK(!ntw16s_system_print_host_a("splwow64.exe",NULL));
    puts("PRINT HOST POLICY PASS exact quoted ansi spoof");return 0;
}
