#include "late_guard.h"
#include <tlhelp32.h>
#include <stdint.h>
#ifdef NTW16S_LATE_GUARD_TEST
static DWORD fail_after=MAXDWORD;
static void (*after_suspend)(void);
void ntw16s_late_guard_test_fail_after(DWORD count){fail_after=count;}
void ntw16s_late_guard_test_after_suspend(void (*callback)(void)){after_suspend=callback;}
#endif
static DWORD verify_threads(const ntw16s_late_guard *guard,DWORD pid,DWORD self)
{
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);
    if(snapshot==INVALID_HANDLE_VALUE)return GetLastError();
    THREADENTRY32 e={.dwSize=sizeof(e)};DWORD error=ERROR_SUCCESS,found=0;
    if(Thread32First(snapshot,&e)){
        do{
            if(e.th32OwnerProcessID==pid && e.th32ThreadID!=self){
                DWORD i=0;while(i<guard->count && GetThreadId(guard->threads[i])!=e.th32ThreadID)++i;
                if(i==guard->count){error=ERROR_RETRY;break;}++found;
            }
            e.dwSize=sizeof(e);
        }while(Thread32Next(snapshot,&e));
        if(!error && GetLastError()!=ERROR_NO_MORE_FILES)error=GetLastError();
    }else if(GetLastError()!=ERROR_NO_MORE_FILES)error=GetLastError();
    if(!error && found!=guard->count)error=ERROR_RETRY;
    CloseHandle(snapshot);return error;
}
DWORD ntw16s_late_guard_end(ntw16s_late_guard *guard)
{
    DWORD error=ERROR_SUCCESS;
    if(!guard)return ERROR_INVALID_PARAMETER;
    for(DWORD i=0;i<guard->suspended;++i){
        if(guard->threads[i] && ResumeThread(guard->threads[i])==(DWORD)-1){if(!error)error=GetLastError();}
        else if(guard->threads[i]){CloseHandle(guard->threads[i]);guard->threads[i]=NULL;}
    }
    if(error)return error;
    for(DWORD i=guard->suspended;i<guard->count;++i)if(guard->threads[i])CloseHandle(guard->threads[i]);
    if(guard->threads)HeapFree(GetProcessHeap(),0,guard->threads);
    ZeroMemory(guard,sizeof(*guard));return ERROR_SUCCESS;
}
DWORD ntw16s_late_guard_begin(ntw16s_late_guard *guard,const ntw16s_late_region *regions,DWORD count)
{
    if(!guard || guard->threads || guard->count || guard->suspended || (count&&!regions))return ERROR_INVALID_PARAMETER;
    for(DWORD i=0;i<count;++i)if(!regions[i].address || !regions[i].bytes || (uintptr_t)regions[i].address>UINTPTR_MAX-regions[i].bytes)return ERROR_INVALID_PARAMETER;
    HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);if(snapshot==INVALID_HANDLE_VALUE)return GetLastError();
    THREADENTRY32 e={.dwSize=sizeof(e)};DWORD capacity=0,error=ERROR_SUCCESS,pid=GetCurrentProcessId(),self=GetCurrentThreadId();
    if(Thread32First(snapshot,&e)){
        do{if(e.th32OwnerProcessID==pid && e.th32ThreadID!=self)++capacity;e.dwSize=sizeof(e);}while(Thread32Next(snapshot,&e));
        if(GetLastError()!=ERROR_NO_MORE_FILES){error=GetLastError();goto done;}
    }
    else if(GetLastError()!=ERROR_NO_MORE_FILES){error=GetLastError();goto done;}
    if(!capacity)goto verify;
    guard->threads=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(SIZE_T)capacity*sizeof(HANDLE));if(!guard->threads){error=ERROR_NOT_ENOUGH_MEMORY;goto done;}
    e.dwSize=sizeof(e);if(!Thread32First(snapshot,&e)){error=GetLastError();goto done;}
    do{if(e.th32OwnerProcessID==pid && e.th32ThreadID!=self){
        if(guard->count==capacity){error=ERROR_MORE_DATA;goto done;}
        HANDLE h=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT|THREAD_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,e.th32ThreadID);
        if(!h){error=GetLastError();goto done;}guard->threads[guard->count++]=h;
    }e.dwSize=sizeof(e);}while(Thread32Next(snapshot,&e));
    if(GetLastError()!=ERROR_NO_MORE_FILES){error=GetLastError();goto done;}
    for(DWORD i=0;i<guard->count;++i){
#ifdef NTW16S_LATE_GUARD_TEST
        if(i==fail_after){error=ERROR_ACCESS_DENIED;goto done;}
#endif
        if(SuspendThread(guard->threads[i])==(DWORD)-1){error=GetLastError();goto done;}++guard->suspended;
    }
    for(DWORD i=0;i<guard->suspended;++i){CONTEXT context={0};context.ContextFlags=CONTEXT_CONTROL;
        if(!GetThreadContext(guard->threads[i],&context)){error=GetLastError();goto done;}
#ifdef _WIN64
        uintptr_t ip=(uintptr_t)context.Rip;
#else
        uintptr_t ip=(uintptr_t)context.Eip;
#endif
        for(DWORD j=0;j<count;++j){uintptr_t start=(uintptr_t)regions[j].address;if(ip>=start && ip-start<regions[j].bytes){error=ERROR_BUSY;goto done;}}
    }
 verify:
#ifdef NTW16S_LATE_GUARD_TEST
    if(after_suspend)after_suspend();
#endif
    error=verify_threads(guard,pid,self);
 done:
    CloseHandle(snapshot);
    if(error){DWORD cleanup=ntw16s_late_guard_end(guard);if(cleanup)return cleanup;}
    return error;
}
