#include <windows.h>
#include <stdio.h>
#include <string.h>
#include "lib-win32/process-global/load.h"

typedef struct reader {HANDLE go;const char *memory;} reader;
static DWORD WINAPI read_later(void *value)
{reader *r=value;return WaitForSingleObject(r->go,3000)!=WAIT_OBJECT_0 || strcmp(r->memory,"still available")?1:0;}
int main(void)
{
    win32_process_global_load ticket={0};HANDLE watch=NULL;
    ticket.remote_path=VirtualAlloc(NULL,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    if(!ticket.remote_path)return 2;
    strcpy((char *)ticket.remote_path,"still available");void *memory=ticket.remote_path;
    if(!DuplicateHandle(GetCurrentProcess(),GetCurrentProcess(),GetCurrentProcess(),&ticket.process,0,FALSE,DUPLICATE_SAME_ACCESS))return 2;
    HANDLE go=CreateEventW(NULL,TRUE,FALSE,NULL);if(!go)return 2;
    reader data={go,(const char *)ticket.remote_path};
    ticket.thread=CreateThread(NULL,0,read_later,&data,0,NULL);if(!ticket.thread)return 2;
    if(!DuplicateHandle(GetCurrentProcess(),ticket.thread,GetCurrentProcess(),&watch,0,FALSE,DUPLICATE_SAME_ACCESS))return 2;
    DWORD busy=win32_process_global_load_discard(&ticket),abandoned=win32_process_global_load_abandon(&ticket);
    BOOL cleared=!ticket.process && !ticket.thread && !ticket.remote_path && !ticket.path;
    SetEvent(go);DWORD code=1,wait=WaitForSingleObject(watch,3000);
    if(wait==WAIT_OBJECT_0)GetExitCodeThread(watch,&code);
    CloseHandle(watch);CloseHandle(go);BOOL freed=VirtualFree(memory,0,MEM_RELEASE);
    printf("LOAD ABANDON bits=%u busy=%lu abandon=%lu cleared=%d wait=%lu reader=%lu freed=%d\n",
           (unsigned)(sizeof(void *)*8),busy,abandoned,cleared,wait,code,freed);
    return busy==ERROR_BUSY && !abandoned && cleared && wait==WAIT_OBJECT_0 && !code && freed?0:1;
}
