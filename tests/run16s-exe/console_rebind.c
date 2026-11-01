#include "run16s-exe/elevation.h"
#include <windows.h>
#include <io.h>
#include <stdio.h>
#include <wchar.h>

int main(void)
{
    FreeConsole();
    if(!AllocConsole())return 2;
    ShowWindow(GetConsoleWindow(),SW_HIDE);
    HANDLE reader=CreateFileW(L"CONOUT$",GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL,OPEN_EXISTING,0,NULL);
    if(reader==INVALID_HANDLE_VALUE){FreeConsole();return 3;}
    (void)_close(2);
    run16s_rebind_console_stderr();
    int written=fwprintf(stderr,L"WIN16S_CONSOLE_REBIND_OK"),flushed=fflush(stderr);
    WCHAR screen[1024]={0};DWORD read=0;COORD origin={0,0};
    BOOL captured=ReadConsoleOutputCharacterW(reader,screen,ARRAYSIZE(screen)-1,origin,&read) &&
        wcsstr(screen,L"WIN16S_CONSOLE_REBIND_OK")!=NULL;
    CloseHandle(reader);FreeConsole();
    /* No console is a valid best-effort case and must not abort. */
    run16s_rebind_console_stderr();
    if(written<=0 || flushed!=0 || !captured)return 4;
    puts("CONSOLE REBIND PASS restored=1 no_console=1");return 0;
}
