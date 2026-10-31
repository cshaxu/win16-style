#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include "ntw16s-dll/injection/late_patch.h"
#include "ntw16s-dll/injection/minhook/src/buffer.h"
typedef int (*sample_fn)(int);
static sample_fn original_a,original_b;static sample_fn volatile call_a,call_b;
static volatile LONG running=1,errors,calls;
static __attribute__((noinline)) int sample_a(int n){return n+7;}
static __attribute__((noinline)) int sample_b(int n){return n+9;}
static int detour_a(int n){return original_a(n)+100;}
static int detour_b(int n){return original_b(n)+200;}
static DWORD WINAPI worker(void *p){(void)p;while(InterlockedCompareExchange(&running,0,0)){int a=call_a(5),b=call_b(5);if((a!=12&&a!=112)||(b!=14&&b!=214))InterlockedIncrement(&errors);InterlockedIncrement(&calls);}return 0;}
static DWORD attempt(ntw16s_late_guard *g,ntw16s_late_patch **items)
{DWORD error=ERROR_BUSY;for(unsigned i=0;i<100&&(error==ERROR_BUSY||error==ERROR_RETRY);++i){error=ntw16s_late_patch_install_batch(g,items,2);if(error==ERROR_BUSY||error==ERROR_RETRY)Sleep(1);}return error;}
int main(void)
{
 InitializeBuffer();original_a=sample_a;original_b=sample_b;call_a=sample_a;call_b=sample_b;
 HANDLE thread=CreateThread(NULL,0,worker,NULL,0,NULL);if(!thread)return 2;while(!InterlockedCompareExchange(&calls,0,0))Sleep(1);
 ntw16s_late_patch a={0},b={0};ntw16s_late_patch *items[]={&a,&b};ntw16s_late_guard guard={0};
 if(ntw16s_late_patch_prepare(&a,&original_a,(void *)detour_a)||ntw16s_late_patch_prepare(&b,&original_b,(void *)detour_b))return 3;
 MEMORY_BASIC_INFORMATION before={0},after={0};if(!VirtualQuery((void *)sample_a,&before,sizeof(before)))return 8;
 b.backup[0]^=1;DWORD conflict=attempt(&guard,items);b.backup[0]^=1;
 if(conflict!=ERROR_REVISION_MISMATCH||a.installed||b.installed||original_a!=sample_a||original_b!=sample_b||call_a(5)!=12||call_b(5)!=14)return 4;
 ntw16s_late_patch_test_fail_protect_after(1);DWORD fault=attempt(&guard,items);ntw16s_late_patch_test_fail_protect_after(MAXDWORD);
 if(!VirtualQuery((void *)sample_a,&after,sizeof(after)) || before.Protect!=after.Protect)return 9;
 if(fault!=ERROR_ACCESS_DENIED||a.installed||b.installed||original_a!=sample_a||original_b!=sample_b||call_a(5)!=12||call_b(5)!=14)return 5;
 DWORD result=attempt(&guard,items);if(result||!a.installed||!b.installed||call_a(5)!=112||call_b(5)!=214||original_a(5)!=12||original_b(5)!=14)return 6;
 Sleep(20);InterlockedExchange(&running,0);DWORD wait=WaitForSingleObject(thread,2000);CloseHandle(thread);
 printf("LATE BATCH bits=%u conflict=%lu fault=%lu protection=restored installed=2 originals=2 errors=%ld joined=%d\n",(unsigned)(sizeof(void*)*8),conflict,fault,errors,wait==WAIT_OBJECT_0);
 return wait==WAIT_OBJECT_0&&!errors?0:7;
}
