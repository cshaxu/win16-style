#include "console.h"
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <stdint.h>
void win32_console_rebind_stderr(void)
{
    /* A process may have stale CRT handles after attaching to a console.
     * Keep stderr open if binding fails. */
    HANDLE console=CreateFileW(L"CONOUT$",GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,
        NULL,OPEN_EXISTING,0,NULL);
    if(console==INVALID_HANDLE_VALUE)return;
    int fd=_open_osfhandle((intptr_t)console,_O_WRONLY|_O_TEXT);
    if(fd<0){CloseHandle(console);return;}
    if(fd!=2){(void)_dup2(fd,2);_close(fd);}
    clearerr(stderr);
}

