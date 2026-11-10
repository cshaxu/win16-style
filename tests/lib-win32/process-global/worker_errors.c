#include <windows.h>
#include <stdio.h>
static unsigned calls,fail_on,exit_on,freed;
static BOOL WINAPI free_memory(HANDLE process,LPVOID address,SIZE_T size,DWORD type)
{(void)process;(void)size;(void)type;if(address!=(void *)(UINT_PTR)4)return FALSE;++freed;return TRUE;}
static DWORD WINAPI ready_wait(DWORD n,const HANDLE *handles,BOOL all,DWORD timeout)
{(void)n;(void)handles;(void)all;(void)timeout;return WAIT_OBJECT_0;}
static DWORD WINAPI state_wait(HANDLE handle,DWORD timeout)
{(void)timeout;if(handle!=(HANDLE)(UINT_PTR)(calls==0?3:2))return WAIT_OBJECT_0;++calls;if(calls==fail_on){SetLastError(ERROR_ACCESS_DENIED);return WAIT_FAILED;}return calls==exit_on?WAIT_OBJECT_0:WAIT_TIMEOUT;}
#define VirtualFreeEx free_memory
#define WaitForMultipleObjects ready_wait
#define WaitForSingleObject state_wait
#include "../../../src/lib-win32/process-global/worker.c"
int main(void)
{
    win32_process_global_worker ticket={0};ticket.ready=(HANDLE)1;ticket.thread=(HANDLE)2;ticket.process=(HANDLE)3;ticket.remote_packet=(void *)(UINT_PTR)4;
    for(fail_on=1;fail_on<=2;++fail_on){calls=0;if(win32_process_global_worker_ready(&ticket,0)!=ERROR_ACCESS_DENIED || calls!=fail_on || freed || !ticket.remote_packet)return (int)fail_on;}
    fail_on=0;
    for(exit_on=1;exit_on<=2;++exit_on){calls=0;if(win32_process_global_worker_ready(&ticket,0)!=ERROR_INVALID_STATE || calls!=exit_on || freed || !ticket.remote_packet)return (int)exit_on+2;}
    exit_on=0;calls=0;if(win32_process_global_worker_ready(&ticket,0)!=ERROR_SUCCESS || calls!=2 || freed!=1 || ticket.remote_packet)return 5;
    puts("PROCESS_GLOBAL_WORKER_ERRORS_OK process=1 thread=1 ready=1 exited=1 ownership=1");return 0;
}
