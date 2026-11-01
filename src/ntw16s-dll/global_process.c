#include "global_process.h"
#include <evntrace.h>
#include <evntcons.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

/* The logger is private to this manager. The system process provider is
 * asynchronous: even a successful notification can arrive after exit. */
static const GUID process_guid={0x3d6fa8d0,0xfe05,0x11d0,{0x9d,0xda,0x00,0xc0,0x4f,0xd7,0xba,0x7c}};
struct ntw16s_global_process {
    WCHAR name[96];TRACEHANDLE session,trace;EVENT_TRACE_PROPERTIES *properties;
    HANDLE consumer,flusher,stop;ntw16s_global_process_callback callback;void *context;
};
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
{
    if(!record || !record->UserData || memcmp(&record->EventHeader.ProviderId,&process_guid,sizeof(process_guid)) ||
       record->EventHeader.EventDescriptor.Opcode!=1)return 0;
    unsigned offset=(record->EventHeader.Flags&EVENT_HEADER_FLAG_32_BIT_HEADER)?4u:8u;
    if(record->UserDataLength<offset+sizeof(DWORD))return 0;
    DWORD pid=0;memcpy(&pid,(const BYTE *)record->UserData+offset,sizeof(pid));
    return pid;
}
static VOID WINAPI event_record(PEVENT_RECORD record)
{
    ntw16s_global_process *m=(ntw16s_global_process *)record->UserContext;
    DWORD pid=ntw16s_global_process_event_pid(record);
    if(!m)return;
    if(pid && WaitForSingleObject(m->stop,0)==WAIT_TIMEOUT)m->callback(pid,m->context);
}
static DWORD WINAPI consume(void *parameter)
{ntw16s_global_process *m=(ntw16s_global_process *)parameter;return ProcessTrace(&m->trace,1,NULL,NULL);}
static DWORD WINAPI flush(void *parameter)
{
    ntw16s_global_process *m=(ntw16s_global_process *)parameter;
    size_t bytes=sizeof(EVENT_TRACE_PROPERTIES)+sizeof(m->name);
    EVENT_TRACE_PROPERTIES *p=(EVENT_TRACE_PROPERTIES *)calloc(1,bytes);
    if(!p)return ERROR_NOT_ENOUGH_MEMORY;
    p->Wnode.BufferSize=(ULONG)bytes;p->LoggerNameOffset=sizeof(*p);
    while(WaitForSingleObject(m->stop,100)==WAIT_TIMEOUT)
        (void)ControlTraceW(m->session,m->name,p,EVENT_TRACE_CONTROL_FLUSH);
    free(p);return ERROR_SUCCESS;
}
DWORD ntw16s_global_process_stop(ntw16s_global_process *m)
{
    if(!m)return ERROR_INVALID_PARAMETER;
    if(m->stop)SetEvent(m->stop);
    if(m->flusher){
        DWORD wait=WaitForSingleObject(m->flusher,5000);
        if(wait!=WAIT_OBJECT_0)return wait==WAIT_TIMEOUT?ERROR_TIMEOUT:GetLastError();
        CloseHandle(m->flusher);m->flusher=NULL;
    }
    ULONG error=ERROR_SUCCESS;
    if(m->session){
        error=ControlTraceW(m->session,m->name,m->properties,EVENT_TRACE_CONTROL_STOP);
        if(error && error!=ERROR_WMI_INSTANCE_NOT_FOUND && error!=ERROR_ACTIVE_CONNECTIONS)return error;
        m->session=0;
        error=ERROR_SUCCESS;
    }
    if(m->consumer){
        DWORD wait=WaitForSingleObject(m->consumer,5000);
        if(wait!=WAIT_OBJECT_0)return wait==WAIT_TIMEOUT?ERROR_TIMEOUT:GetLastError();
        CloseHandle(m->consumer);m->consumer=NULL;
    }
    if(m->trace!=INVALID_PROCESSTRACE_HANDLE)CloseTrace(m->trace);
    if(m->stop)CloseHandle(m->stop);
    free(m->properties);free(m);return error;
}
DWORD ntw16s_global_process_start(ntw16s_global_process_callback callback,void *context,
                                 ntw16s_global_process **result)
{
    if(result)*result=NULL;
    if(!callback || !result)return ERROR_INVALID_PARAMETER;
    ntw16s_global_process *m=(ntw16s_global_process *)calloc(1,sizeof(*m));
    if(!m)return ERROR_NOT_ENOUGH_MEMORY;
    m->trace=INVALID_PROCESSTRACE_HANDLE;m->callback=callback;m->context=context;
    DWORD high=0,low=0;if(!process_birth(GetCurrentProcess(),&high,&low)){DWORD error=GetLastError();free(m);return error;}
    if(swprintf(m->name,ARRAYSIZE(m->name),L"Win16S-ETW-%lu-%08lx%08lx",GetCurrentProcessId(),high,low)<0){free(m);return ERROR_INVALID_NAME;}
    recover_orphans();
    size_t bytes=sizeof(EVENT_TRACE_PROPERTIES)+sizeof(m->name);
    m->properties=(EVENT_TRACE_PROPERTIES *)calloc(1,bytes);if(!m->properties){free(m);return ERROR_NOT_ENOUGH_MEMORY;}
    m->properties->Wnode.BufferSize=(ULONG)bytes;m->properties->Wnode.Flags=WNODE_FLAG_TRACED_GUID;
    m->properties->Wnode.ClientContext=1;m->properties->LogFileMode=EVENT_TRACE_REAL_TIME_MODE|EVENT_TRACE_SYSTEM_LOGGER_MODE;
    m->properties->EnableFlags=EVENT_TRACE_FLAG_PROCESS;m->properties->LoggerNameOffset=sizeof(*m->properties);
    m->properties->FlushTimer=1;
    ULONG error=StartTraceW(&m->session,m->name,m->properties);if(error)goto fail;
    m->stop=CreateEventW(NULL,TRUE,FALSE,NULL);if(!m->stop){error=GetLastError();goto fail;}
    EVENT_TRACE_LOGFILEW log={0};log.LoggerName=m->name;log.ProcessTraceMode=PROCESS_TRACE_MODE_REAL_TIME|PROCESS_TRACE_MODE_EVENT_RECORD;
    log.EventRecordCallback=event_record;log.Context=m;
    m->trace=OpenTraceW(&log);if(m->trace==INVALID_PROCESSTRACE_HANDLE){error=GetLastError();goto fail;}
    m->consumer=CreateThread(NULL,0,consume,m,0,NULL);if(!m->consumer){error=GetLastError();goto fail;}
    m->flusher=CreateThread(NULL,0,flush,m,0,NULL);if(!m->flusher){error=GetLastError();goto fail;}
    *result=m;return ERROR_SUCCESS;
fail:
    {DWORD cleanup=ntw16s_global_process_stop(m);if(cleanup){*result=m;return cleanup;}return error;}
}
