#include "ntw16s-dll/global/process.h"
#include <evntrace.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>

static void event(DWORD pid,void *context){(void)pid;(void)context;}
static ULONG query(const WCHAR *name)
{
    BYTE buffer[sizeof(EVENT_TRACE_PROPERTIES)+192*sizeof(WCHAR)]={0};
    EVENT_TRACE_PROPERTIES *p=(EVENT_TRACE_PROPERTIES *)(void *)buffer;
    p->Wnode.BufferSize=sizeof(buffer);p->LoggerNameOffset=sizeof(*p);
    return ControlTraceW(0,name,p,EVENT_TRACE_CONTROL_QUERY);
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !wcscmp(argv[1],L"--orphan")){
        win32_process_global_monitor *m=NULL;DWORD error=ntw16s_global_process_start(event,NULL,&m);
        if(error==ERROR_ACCESS_DENIED || error==ERROR_PRIVILEGE_NOT_HELD)return 77;
        return error?2:0; /* Deliberate process exit without stopping our own trace. */
    }
    WCHAR self[32768],command[32800],name[96];STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION child={0};
    if(!GetModuleFileNameW(NULL,self,ARRAYSIZE(self)) || swprintf(command,ARRAYSIZE(command),L"\"%ls\" --orphan",self)<0)return 3;
    if(!CreateProcessW(self,command,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&child))return 4;
    DWORD wait=WaitForSingleObject(child.hProcess,5000),exitcode=99;FILETIME created,exited,kernel,user;
    if(wait!=WAIT_OBJECT_0){TerminateProcess(child.hProcess,1);WaitForSingleObject(child.hProcess,1000);}
    BOOL ok=wait==WAIT_OBJECT_0 && GetExitCodeProcess(child.hProcess,&exitcode) &&
        GetProcessTimes(child.hProcess,&created,&exited,&kernel,&user);
    if(!ok){CloseHandle(child.hThread);CloseHandle(child.hProcess);return 5;}
    if(exitcode==77){CloseHandle(child.hThread);CloseHandle(child.hProcess);puts("GLOBAL ORPHAN SKIP elevation required");return 77;}
    if(exitcode || swprintf(name,ARRAYSIZE(name),L"Win16S-ETW-%lu-%08lx%08lx",child.dwProcessId,created.dwHighDateTime,created.dwLowDateTime)<0){CloseHandle(child.hThread);CloseHandle(child.hProcess);return 6;}
    ULONG before=query(name);win32_process_global_monitor *m=NULL;DWORD start=ntw16s_global_process_start(event,NULL,&m);
    ULONG after=query(name);DWORD stop=m?win32_process_global_monitor_stop(m):0;
    /* Clean up the explicitly owned orphan if a failure prevented recovery. */
    if(after!=ERROR_WMI_INSTANCE_NOT_FOUND){BYTE buffer[sizeof(EVENT_TRACE_PROPERTIES)+192*sizeof(WCHAR)]={0};EVENT_TRACE_PROPERTIES *p=(EVENT_TRACE_PROPERTIES *)(void *)buffer;p->Wnode.BufferSize=sizeof(buffer);p->LoggerNameOffset=sizeof(*p);(void)ControlTraceW(0,name,p,EVENT_TRACE_CONTROL_STOP);}
    CloseHandle(child.hThread);CloseHandle(child.hProcess);
    printf("GLOBAL ORPHAN before=%lu start=%lu after=%lu stop=%lu\n",before,start,after,stop);
    return before==ERROR_SUCCESS && !start && after==ERROR_WMI_INSTANCE_NOT_FOUND && !stop?0:1;
}
