#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include "ntw16s-dll/injection/late_guard.h"
static volatile LONG entered,gate,running=1,ticks;
static __attribute__((noinline)) DWORD WINAPI busy(void *p)
{(void)p;entered=1;while(!gate){}return 17;}
static DWORD WINAPI counter(void *p)
{(void)p;while(InterlockedCompareExchange(&running,0,0)){InterlockedIncrement(&ticks);Sleep(1);}return 0;}
int main(void)
{
    HANDLE busy_thread=CreateThread(NULL,0,busy,NULL,0,NULL),counter_thread=CreateThread(NULL,0,counter,NULL,0,NULL);
    if(!busy_thread || !counter_thread)return 2;
    ULONGLONG deadline=GetTickCount64()+2000;while(!entered && GetTickCount64()<deadline)Sleep(1);
    if(!entered)return 3;
    ntw16s_late_guard guard={0};ntw16s_late_region region={(const void *)busy,256};
    ntw16s_late_guard_test_fail_after(1);
    DWORD failed=ntw16s_late_guard_begin(&guard,NULL,0);
    ntw16s_late_guard_test_fail_after(MAXDWORD);
    LONG before_failure=InterlockedCompareExchange(&ticks,0,0);Sleep(30);
    if(failed!=ERROR_ACCESS_DENIED || guard.threads || InterlockedCompareExchange(&ticks,0,0)<=before_failure)return 7;
    DWORD rejected=ntw16s_late_guard_begin(&guard,&region,1);gate=1;
    DWORD completed=WaitForSingleObject(busy_thread,2000),value=0;GetExitCodeThread(busy_thread,&value);CloseHandle(busy_thread);
    if(rejected!=ERROR_BUSY || completed!=WAIT_OBJECT_0 || value!=17 || guard.threads)return 4;
    DWORD started=ntw16s_late_guard_begin(&guard,NULL,0);if(started)return 5;
    LONG before=InterlockedCompareExchange(&ticks,0,0);Sleep(30);LONG during=InterlockedCompareExchange(&ticks,0,0);
    DWORD released=ntw16s_late_guard_end(&guard);Sleep(30);LONG after=InterlockedCompareExchange(&ticks,0,0);
    InterlockedExchange(&running,0);DWORD stopped=WaitForSingleObject(counter_thread,2000);CloseHandle(counter_thread);
    printf("LATE GUARD bits=%u fault=%lu busy=%lu completed=%lu suspended=%d resumed=%d released=%lu stopped=%lu\n",(unsigned)(sizeof(void*)*8),failed,rejected,completed,before==during,after>during,released,stopped);
    return started==0 && before==during && after>during && released==0 && stopped==WAIT_OBJECT_0?0:6;
}
