#include "lib-win32/process-global/load.h"
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"load_finish line=%d error=%lu\n",__LINE__,GetLastError());result=1;goto done;}}while(0)
typedef struct fixture {HANDLE release;DWORD error;unsigned calls;} fixture;
static DWORD WINAPI worker(void *context)
{return WaitForSingleObject(((fixture *)context)->release,3000)==WAIT_OBJECT_0?0:ERROR_TIMEOUT;}
static DWORD resolve(HANDLE process,DWORD pid,const WCHAR *path,void **address,void *value)
{
    fixture *f=value;++f->calls;
    if(GetProcessId(process)!=pid || wcscmp(path,L"owned.dll"))return ERROR_INVALID_DATA;
    *address=(void *)(ULONG_PTR)73;return f->error;
}
static BOOL begin(win32_process_global_load *ticket,fixture *f)
{
    if(!DuplicateHandle(GetCurrentProcess(),GetCurrentProcess(),GetCurrentProcess(),&ticket->process,0,FALSE,DUPLICATE_SAME_ACCESS))return FALSE;
    ticket->pid=GetCurrentProcessId();ticket->path=_wcsdup(L"owned.dll");
    ticket->remote_path=VirtualAllocEx(ticket->process,NULL,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);
    ticket->thread=CreateThread(NULL,0,worker,f,0,NULL);
    return ticket->path && ticket->remote_path && ticket->thread;
}
int main(void)
{
    int result=0;fixture f={0};win32_process_global_load ticket={0};void *address=(void *)(ULONG_PTR)99;
    f.release=CreateEventW(NULL,TRUE,FALSE,NULL);CHECK(f.release);
    CHECK(begin(&ticket,&f));
    CHECK(win32_process_global_load_finish_using(&ticket,0,resolve,&f,&address)==ERROR_TIMEOUT && !address);
    CHECK(ticket.remote_path && ticket.path && !f.calls);
    SetEvent(f.release);CHECK(WaitForSingleObject(ticket.thread,3000)==WAIT_OBJECT_0);
    f.error=ERROR_ACCESS_DENIED;
    CHECK(win32_process_global_load_finish_using(&ticket,0,resolve,&f,&address)==ERROR_ACCESS_DENIED);
    CHECK(!address && !ticket.process && !ticket.thread && !ticket.path && !ticket.remote_path && f.calls==1);
    ResetEvent(f.release);f.error=0;CHECK(begin(&ticket,&f));SetEvent(f.release);
    CHECK(WaitForSingleObject(ticket.thread,3000)==WAIT_OBJECT_0);
    CHECK(!win32_process_global_load_finish_using(&ticket,0,resolve,&f,&address));
    CHECK(address==(void *)(ULONG_PTR)73 && f.calls==2 && !ticket.process && !ticket.path);
 done:
    if(f.release)SetEvent(f.release);
    if(ticket.thread && WaitForSingleObject(ticket.thread,4000)!=WAIT_OBJECT_0)ExitProcess(2);
    if(win32_process_global_load_discard(&ticket))ExitProcess(3);
    if(f.release)CloseHandle(f.release);
    if(!result)puts("PROCESS_GLOBAL_LOAD_FINISH_OK context=1 timeout_retains=1 error_bubbles=1 success=1 cleanup=1");
    return result;
}
