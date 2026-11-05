#include "ntw16s-dll/global/process.h"
#include "lib-win32/process-global/recovery.h"
#include <stdio.h>
#include <string.h>

int main(int argc,char **argv)
{
    if(argc==2 && !strcmp(argv[1],"child"))return 0;
    const GUID provider={0x3d6fa8d0,0xfe05,0x11d0,{0x9d,0xda,0x00,0xc0,0x4f,0xd7,0xba,0x7c}};
    BYTE data[16]={0};DWORD pid=0x12345678;EVENT_RECORD record={0};
    record.EventHeader.ProviderId=provider;record.EventHeader.EventDescriptor.Opcode=1;
    record.UserData=data;record.UserDataLength=sizeof(data);
    memcpy(data+8,&pid,sizeof(pid));
    if(win32_process_global_monitor_event_pid(&record)!=pid)return 1;
    ZeroMemory(data,sizeof(data));memcpy(data+4,&pid,sizeof(pid));
    record.EventHeader.Flags=EVENT_HEADER_FLAG_32_BIT_HEADER;
    if(win32_process_global_monitor_event_pid(&record)!=pid)return 2;
    record.UserDataLength=7;if(win32_process_global_monitor_event_pid(&record))return 3;
    record.UserDataLength=sizeof(data);record.EventHeader.EventDescriptor.Opcode=2;
    if(win32_process_global_monitor_event_pid(&record))return 4;
    record.EventHeader.EventDescriptor.Opcode=1;record.EventHeader.ProviderId.Data1++;
    if(win32_process_global_monitor_event_pid(&record) || win32_process_global_monitor_event_pid(NULL))return 5;
    DWORD owner=0,high=0,low=0;
    if(!ntw16s_global_process_parse_name(L"Win16S-ETW-1234-0123456789abcdef",&owner,&high,&low) ||
       owner!=1234 || high!=0x01234567 || low!=0x89abcdef)return 6;
    if(ntw16s_global_process_parse_name(L"Win16S-ETW-01234-0123456789abcdef",&owner,&high,&low) ||
       ntw16s_global_process_parse_name(L"Win16S-ETW-1234-0123456789abcdef-extra",&owner,&high,&low) ||
       ntw16s_global_process_parse_name(L"Other-ETW-1234-0123456789abcdef",&owner,&high,&low))return 7;
    FILETIME created,exited,kernel,user;
    if(!GetProcessTimes(GetCurrentProcess(),&created,&exited,&kernel,&user))return 8;
    if(win32_process_global_trace_orphan(GetCurrentProcessId(),created.dwHighDateTime,created.dwLowDateTime))return 9;
    if(!win32_process_global_trace_orphan(GetCurrentProcessId(),created.dwHighDateTime,created.dwLowDateTime^1u))return 10;
    if(win32_process_global_trace_stop_peer(GetCurrentProcess(),GetCurrentProcessId(),L"owned-test")!=ERROR_INVALID_PARAMETER ||
       win32_process_global_trace_stop_peer(NULL,0,L"owned-test")!=ERROR_INVALID_PARAMETER)return 11;
    WCHAR self[MAX_PATH],command[MAX_PATH+32],trace[128];STARTUPINFOW startup={.cb=sizeof(startup)};PROCESS_INFORMATION child={0};
    if(!GetModuleFileNameW(NULL,self,ARRAYSIZE(self)) || swprintf(command,ARRAYSIZE(command),L"\"%ls\" child",self)<0 ||
       swprintf(trace,ARRAYSIZE(trace),L"S41-owned-%lu-%llu",GetCurrentProcessId(),(unsigned long long)GetTickCount64())<0)return 12;
    if(!CreateProcessW(self,command,NULL,NULL,FALSE,0,NULL,NULL,&startup,&child))return 13;
    if(WaitForSingleObject(child.hProcess,3000)!=WAIT_OBJECT_0){TerminateProcess(child.hProcess,ERROR_TIMEOUT);WaitForSingleObject(child.hProcess,1000);CloseHandle(child.hThread);CloseHandle(child.hProcess);return 14;}
    DWORD trace_error=win32_process_global_trace_stop_peer(child.hProcess,child.dwProcessId,trace);
    CloseHandle(child.hThread);CloseHandle(child.hProcess);
    if(trace_error==ERROR_ACCESS_DENIED || trace_error==ERROR_PRIVILEGE_NOT_HELD)printf("TRACE STOP SKIP permission=%lu\n",trace_error);
    else if(trace_error)return 15;
    else puts("TRACE STOP PASS owned_exited=1 missing_trace=1");
    win32_process_global_recover_orphans(NULL);
    puts("GLOBAL PROCESS PARSER PASS 32=1 64=1 short=1 opcode=1 provider=1");return 0;
}
