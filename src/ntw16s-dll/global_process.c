#include "global_process.h"
#include <evntrace.h>
#include <evntcons.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

/* Process IDs cannot be reused while our peer process handle is open. The
 * birth time in the name also lets a future invocation recognise an orphan
 * when both controller processes terminated before either could clean up. */
static BOOL process_birth(HANDLE process,DWORD *high,DWORD *low)
{
    FILETIME created,exited,kernel,user;
    if(!GetProcessTimes(process,&created,&exited,&kernel,&user))return FALSE;
    *high=created.dwHighDateTime;*low=created.dwLowDateTime;return TRUE;
}
BOOL ntw16s_global_process_parse_name(const WCHAR *name,DWORD *pid,DWORD *high,DWORD *low)
{
    WCHAR expected[96];DWORD p=0,h=0,l=0;int used=0;
    if(!name || !pid || !high || !low || swscanf(name,L"Win16S-ETW-%lu-%8lx%8lx%n",&p,&h,&l,&used)!=3 ||
       !p || !used || name[used] || swprintf(expected,ARRAYSIZE(expected),L"Win16S-ETW-%lu-%08lx%08lx",p,h,l)<0 ||
       wcscmp(name,expected))return FALSE;
    *pid=p;*high=h;*low=l;return TRUE;
}
static void recover_orphans(void)
{
    enum {capacity=256,name_capacity=128};
    EVENT_TRACE_PROPERTIES *entries[capacity]={0};ULONG count=0;
    size_t bytes=sizeof(EVENT_TRACE_PROPERTIES)+name_capacity*sizeof(WCHAR);
    for(unsigned i=0;i<capacity;++i){
        entries[i]=(EVENT_TRACE_PROPERTIES *)calloc(1,bytes);
        if(!entries[i])goto done;
        entries[i]->Wnode.BufferSize=(ULONG)bytes;entries[i]->LoggerNameOffset=sizeof(EVENT_TRACE_PROPERTIES);
    }
    if(QueryAllTracesW(entries,capacity,&count)!=ERROR_SUCCESS)goto done;
    for(ULONG i=0;i<count && i<capacity;++i){
        WCHAR *name=(WCHAR *)((BYTE *)entries[i]+entries[i]->LoggerNameOffset);
        DWORD pid=0,high=0,low=0;
        if(!ntw16s_global_process_parse_name(name,&pid,&high,&low) || pid==GetCurrentProcessId())continue;
        HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,pid);
        BOOL orphan=FALSE;
        if(process){
            DWORD actual_high=0,actual_low=0;
            if(process_birth(process,&actual_high,&actual_low))
                orphan=actual_high!=high || actual_low!=low || WaitForSingleObject(process,0)==WAIT_OBJECT_0;
            CloseHandle(process);
        }else orphan=GetLastError()==ERROR_INVALID_PARAMETER;
        if(orphan)(void)ControlTraceW(0,name,entries[i],EVENT_TRACE_CONTROL_STOP);
    }
done:
    for(unsigned i=0;i<capacity;++i)free(entries[i]);
}
DWORD ntw16s_global_process_stop_peer(HANDLE process,DWORD pid)
{
    WCHAR name[96];size_t bytes=sizeof(EVENT_TRACE_PROPERTIES)+sizeof(name);
    DWORD high=0,low=0,error=ERROR_SUCCESS;
    if(!process || !pid || GetProcessId(process)!=pid || WaitForSingleObject(process,0)!=WAIT_OBJECT_0)return ERROR_INVALID_PARAMETER;
    if(!process_birth(process,&high,&low))return GetLastError();
    if(!pid || swprintf(name,ARRAYSIZE(name),L"Win16S-ETW-%lu-%08lx%08lx",pid,high,low)<0)return ERROR_INVALID_PARAMETER;
    EVENT_TRACE_PROPERTIES *p=(EVENT_TRACE_PROPERTIES *)calloc(1,bytes);
    if(!p)return ERROR_NOT_ENOUGH_MEMORY;
    p->Wnode.BufferSize=(ULONG)bytes;p->LoggerNameOffset=sizeof(*p);
    error=ControlTraceW(0,name,p,EVENT_TRACE_CONTROL_STOP);free(p);
    return error==ERROR_WMI_INSTANCE_NOT_FOUND || error==ERROR_ACTIVE_CONNECTIONS?ERROR_SUCCESS:error;
}
DWORD ntw16s_global_process_event_pid(const EVENT_RECORD *record)
{return win32_process_global_monitor_event_pid(record);}
DWORD ntw16s_global_process_start(ntw16s_global_process_callback callback,void *context,ntw16s_global_process **result)
{
    WCHAR name[96];DWORD high=0,low=0;
    if(result)*result=NULL;
    if(!callback || !result)return ERROR_INVALID_PARAMETER;
    if(!process_birth(GetCurrentProcess(),&high,&low))return GetLastError();
    if(swprintf(name,ARRAYSIZE(name),L"Win16S-ETW-%lu-%08lx%08lx",GetCurrentProcessId(),high,low)<0)return ERROR_INVALID_NAME;
    recover_orphans();
    return win32_process_global_monitor_start(name,callback,context,result);
}
DWORD ntw16s_global_process_stop(ntw16s_global_process *monitor)
{return win32_process_global_monitor_stop(monitor);}
