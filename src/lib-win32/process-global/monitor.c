#include "monitor.h"
#include <evntrace.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
static const GUID process_guid={0x3d6fa8d0,0xfe05,0x11d0,{0x9d,0xda,0x00,0xc0,0x4f,0xd7,0xba,0x7c}};
struct win32_process_global_monitor {
    WCHAR name[96];TRACEHANDLE session,trace;EVENT_TRACE_PROPERTIES *properties;
    HANDLE consumer,flusher,stop;win32_process_global_monitor_callback callback;void *context;
};
DWORD win32_process_global_monitor_event_pid(const EVENT_RECORD *record)
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
    win32_process_global_monitor *m=(win32_process_global_monitor *)record->UserContext;
    DWORD pid=win32_process_global_monitor_event_pid(record);
    if(!m)return;
    if(pid && WaitForSingleObject(m->stop,0)==WAIT_TIMEOUT)m->callback(pid,m->context);
}
static DWORD WINAPI consume(void *parameter)
{win32_process_global_monitor *m=(win32_process_global_monitor *)parameter;return ProcessTrace(&m->trace,1,NULL,NULL);}
static DWORD WINAPI flush(void *parameter)
{
    win32_process_global_monitor *m=(win32_process_global_monitor *)parameter;
    size_t bytes=sizeof(EVENT_TRACE_PROPERTIES)+sizeof(m->name);
    EVENT_TRACE_PROPERTIES *p=(EVENT_TRACE_PROPERTIES *)calloc(1,bytes);
    if(!p)return ERROR_NOT_ENOUGH_MEMORY;
    p->Wnode.BufferSize=(ULONG)bytes;p->LoggerNameOffset=sizeof(*p);
    while(WaitForSingleObject(m->stop,100)==WAIT_TIMEOUT)
        (void)ControlTraceW(m->session,m->name,p,EVENT_TRACE_CONTROL_FLUSH);
    free(p);return ERROR_SUCCESS;
}
DWORD win32_process_global_monitor_stop(win32_process_global_monitor *m)
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
DWORD win32_process_global_monitor_start(const WCHAR *name,win32_process_global_monitor_callback callback,void *context,
                                 win32_process_global_monitor **result)
{
    if(result)*result=NULL;
    if(!callback || !result)return ERROR_INVALID_PARAMETER;
    win32_process_global_monitor *m=(win32_process_global_monitor *)calloc(1,sizeof(*m));
    if(!m)return ERROR_NOT_ENOUGH_MEMORY;
    m->trace=INVALID_PROCESSTRACE_HANDLE;m->callback=callback;m->context=context;
    if(!name || !*name || wcslen(name)>=ARRAYSIZE(m->name)){free(m);return ERROR_INVALID_NAME;}
    wcscpy(m->name,name);
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
    {DWORD cleanup=win32_process_global_monitor_stop(m);if(cleanup){*result=m;return cleanup;}return error;}
}
