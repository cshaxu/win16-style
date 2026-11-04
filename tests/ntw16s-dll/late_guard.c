#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include "lib-win32/process-hook/late_guard.h"
static volatile LONG entered,gate,running=1,ticks;
static HANDLE late_thread;
static HANDLE heap_start,heap_locked;
static DWORD WINAPI heap_owner(void *p)
{
    (void)p;WaitForSingleObject(heap_start,INFINITE);
    if(!HeapLock(GetProcessHeap()))return 1;
    SetEvent(heap_locked);Sleep(200);HeapUnlock(GetProcessHeap());return 0;
}
static void hold_heap_before_suspend(void){SetEvent(heap_start);WaitForSingleObject(heap_locked,2000);}
static __attribute__((noinline)) DWORD WINAPI busy(void *p)
{(void)p;entered=1;while(!gate){}return 17;}
static DWORD WINAPI counter(void *p)
{(void)p;while(InterlockedCompareExchange(&running,0,0)){InterlockedIncrement(&ticks);Sleep(1);}return 0;}
static DWORD WINAPI already_exited(void *p){(void)p;return 0;}
static void spawn_late(void){late_thread=CreateThread(NULL,0,counter,NULL,0,NULL);}
int main(void)
{
    HANDLE exited_thread=CreateThread(NULL,0,already_exited,NULL,0,NULL);
    if(!exited_thread || WaitForSingleObject(exited_thread,2000)!=WAIT_OBJECT_0)return 17;
    process_hook_late_guard exited_guard={0};DWORD exited_begin=process_hook_late_guard_begin(&exited_guard,NULL,0);
    DWORD exited_end=exited_begin?exited_begin:process_hook_late_guard_end(&exited_guard);
    CloseHandle(exited_thread);
    printf("PSS EXITED HANDLE bits=%u begin=%lu end=%lu\n",(unsigned)(sizeof(void*)*8),exited_begin,exited_end);
    if(exited_begin || exited_end)return 18;
    HANDLE busy_thread=CreateThread(NULL,0,busy,NULL,0,NULL),counter_thread=CreateThread(NULL,0,counter,NULL,0,NULL);
    if(!busy_thread || !counter_thread)return 2;
    ULONGLONG deadline=GetTickCount64()+2000;while(!entered && GetTickCount64()<deadline)Sleep(1);
    if(!entered)return 3;
    FILETIME created,exited,kernel,user;if(!GetThreadTimes(busy_thread,&created,&exited,&kernel,&user))return 15;
    DWORD pid=GetCurrentProcessId(),tid=GetThreadId(busy_thread);FILETIME stale=created;stale.dwLowDateTime^=1;
    if(process_hook_late_guard_test_identity(busy_thread,pid,tid,created) ||
       process_hook_late_guard_test_identity(busy_thread,pid^1u,tid,created)!=ERROR_RETRY ||
       process_hook_late_guard_test_identity(busy_thread,pid,tid^1u,created)!=ERROR_RETRY ||
       process_hook_late_guard_test_identity(busy_thread,pid,tid,stale)!=ERROR_RETRY)return 16;
    printf("PSS IDENTITY PASS bits=%u pid=1 tid=1 birth=1\n",(unsigned)(sizeof(void*)*8));
    process_hook_late_guard guard={0};process_hook_late_region region={(const void *)busy,256};
    process_hook_late_guard_test_fail_free(2);
    DWORD cleanup_failure=process_hook_late_guard_begin(&guard,NULL,0);
    BOOL retained=guard.snapshots!=NULL && !guard.threads;
    DWORD cleanup_retry=process_hook_late_guard_end(&guard);
    if(cleanup_failure!=ERROR_ACCESS_DENIED || !retained || cleanup_retry || guard.snapshots)return 9;
    DWORD cleanup_started=process_hook_late_guard_begin(&guard,NULL,0);if(cleanup_started)return 10;
    process_hook_late_guard_test_fail_free(1);
    DWORD post_failure=process_hook_late_guard_end(&guard);
    BOOL post_retained=guard.snapshots!=NULL && !guard.threads && !guard.suspended;
    DWORD post_retry=process_hook_late_guard_end(&guard);
    if(post_failure!=ERROR_ACCESS_DENIED || !post_retained || post_retry || guard.snapshots)return 11;
    printf("PSS CLEANUP PASS bits=%u retained=1 retry=1 resumed=1\n",(unsigned)(sizeof(void*)*8));
    process_hook_late_guard_test_fail_after(1);
    DWORD failed=process_hook_late_guard_begin(&guard,NULL,0);
    process_hook_late_guard_test_fail_after(MAXDWORD);
    LONG before_failure=InterlockedCompareExchange(&ticks,0,0);Sleep(30);
    if(failed!=ERROR_ACCESS_DENIED || guard.threads || InterlockedCompareExchange(&ticks,0,0)<=before_failure)return 7;
    process_hook_late_guard_test_after_suspend(spawn_late);
    DWORD changed=process_hook_late_guard_begin(&guard,NULL,0);
    process_hook_late_guard_test_after_suspend(NULL);
    if(changed!=ERROR_RETRY || !late_thread || guard.threads)return 8;
    DWORD rejected=process_hook_late_guard_begin(&guard,&region,1);gate=1;
    DWORD completed=WaitForSingleObject(busy_thread,2000),value=0;GetExitCodeThread(busy_thread,&value);CloseHandle(busy_thread);
    if(rejected!=ERROR_BUSY || completed!=WAIT_OBJECT_0 || value!=17 || guard.threads)return 4;
    DWORD started=process_hook_late_guard_begin(&guard,NULL,0);if(started)return 5;
    LONG before=InterlockedCompareExchange(&ticks,0,0);Sleep(30);LONG during=InterlockedCompareExchange(&ticks,0,0);
    DWORD released=process_hook_late_guard_end(&guard);Sleep(30);LONG after=InterlockedCompareExchange(&ticks,0,0);
    InterlockedExchange(&running,0);DWORD stopped=WaitForSingleObject(counter_thread,2000),late_stopped=WaitForSingleObject(late_thread,2000);CloseHandle(counter_thread);CloseHandle(late_thread);
    heap_start=CreateEventW(NULL,TRUE,FALSE,NULL);heap_locked=CreateEventW(NULL,TRUE,FALSE,NULL);
    if(!heap_start || !heap_locked)return 12;
    HANDLE owner=CreateThread(NULL,0,heap_owner,NULL,0,NULL);if(!owner)return 13;
    process_hook_late_guard_test_before_suspend(hold_heap_before_suspend);
    DWORD heap_begin=process_hook_late_guard_begin(&guard,NULL,0);
    process_hook_late_guard_test_before_suspend(NULL);
    DWORD heap_end=heap_begin?heap_begin:process_hook_late_guard_end(&guard);
    DWORD heap_wait=WaitForSingleObject(owner,2000),heap_code=1;GetExitCodeThread(owner,&heap_code);
    CloseHandle(owner);CloseHandle(heap_locked);CloseHandle(heap_start);
    if(heap_begin || heap_end || heap_wait!=WAIT_OBJECT_0 || heap_code)return 14;
    printf("PSS HEAP PASS bits=%u resumed=1\n",(unsigned)(sizeof(void*)*8));
    printf("LATE GUARD bits=%u fault=%lu changed=%lu busy=%lu completed=%lu suspended=%d resumed=%d released=%lu stopped=%lu late=%lu\n",(unsigned)(sizeof(void*)*8),failed,changed,rejected,completed,before==during,after>during,released,stopped,late_stopped);
    return started==0 && before==during && after>during && released==0 && stopped==WAIT_OBJECT_0 && late_stopped==WAIT_OBJECT_0?0:6;
}
