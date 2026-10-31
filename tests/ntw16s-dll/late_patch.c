#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include "ntw16s-dll/injection/late_patch.h"
#include "ntw16s-dll/injection/minhook/src/buffer.h"
typedef int (*sample_fn)(int);
static sample_fn original;
static sample_fn volatile call_sample;
static volatile LONG running=1,errors,calls,hits;
static __attribute__((noinline)) int sample(int n){return n+7;}
static int detour(int n){InterlockedIncrement(&hits);return original(n)+100;}
static DWORD WINAPI worker(void *p){(void)p;while(InterlockedCompareExchange(&running,0,0)){int n=call_sample(5);if(n!=12 && n!=112)InterlockedIncrement(&errors);InterlockedIncrement(&calls);}return 0;}
int main(void)
{
    InitializeBuffer();original=sample;call_sample=sample;HANDLE thread=CreateThread(NULL,0,worker,NULL,0,NULL);if(!thread)return 2;
    while(!InterlockedCompareExchange(&calls,0,0))Sleep(1);
    ntw16s_late_patch p={0};DWORD prepared=ntw16s_late_patch_prepare(&p,&original,(void *)detour);if(prepared)return 3;
    ntw16s_late_guard_test_fail_after(0);DWORD failed=ntw16s_late_patch_install(&p);ntw16s_late_guard_test_fail_after(MAXDWORD);
    if(failed!=ERROR_ACCESS_DENIED || p.installed || original!=sample || call_sample(5)!=12)return 4;
    p.backup[0]^=1;DWORD conflict=ERROR_BUSY;
    for(unsigned i=0;i<100 && (conflict==ERROR_BUSY || conflict==ERROR_RETRY);++i){conflict=ntw16s_late_patch_install(&p);if(conflict==ERROR_BUSY || conflict==ERROR_RETRY)Sleep(1);}
    p.backup[0]^=1;
    if(conflict!=ERROR_REVISION_MISMATCH || p.installed || original!=sample || call_sample(5)!=12)return 5;
    DWORD installed=ERROR_BUSY;for(unsigned i=0;i<100 && (installed==ERROR_BUSY || installed==ERROR_RETRY);++i){installed=ntw16s_late_patch_install(&p);if(installed)Sleep(1);}
    int styled=call_sample(5),native=original(5);
    if(installed || !p.installed || styled!=112 || native!=12){printf("LATE PATCH install_error=%lu installed=%d styled=%d native=%d\n",installed,p.installed,styled,native);return 6;}
    Sleep(30);InterlockedExchange(&running,0);DWORD wait=WaitForSingleObject(thread,2000);CloseHandle(thread);
    printf("LATE PATCH bits=%u fault=%lu conflict=%lu installed=%d original=%d hits=%ld errors=%ld joined=%d\n",(unsigned)(sizeof(void*)*8),failed,conflict,p.installed,original(5),hits,errors,wait==WAIT_OBJECT_0);
    return wait==WAIT_OBJECT_0 && hits>0 && errors==0?0:7;
}
