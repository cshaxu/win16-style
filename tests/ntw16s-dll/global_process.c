#include "ntw16s-dll/global_process.h"
#include "lib-win32/process-global/recovery.h"
#include <stdio.h>
#include <string.h>

int main(void)
{
    const GUID provider={0x3d6fa8d0,0xfe05,0x11d0,{0x9d,0xda,0x00,0xc0,0x4f,0xd7,0xba,0x7c}};
    BYTE data[16]={0};DWORD pid=0x12345678;EVENT_RECORD record={0};
    record.EventHeader.ProviderId=provider;record.EventHeader.EventDescriptor.Opcode=1;
    record.UserData=data;record.UserDataLength=sizeof(data);
    memcpy(data+8,&pid,sizeof(pid));
    if(ntw16s_global_process_event_pid(&record)!=pid)return 1;
    ZeroMemory(data,sizeof(data));memcpy(data+4,&pid,sizeof(pid));
    record.EventHeader.Flags=EVENT_HEADER_FLAG_32_BIT_HEADER;
    if(ntw16s_global_process_event_pid(&record)!=pid)return 2;
    record.UserDataLength=7;if(ntw16s_global_process_event_pid(&record))return 3;
    record.UserDataLength=sizeof(data);record.EventHeader.EventDescriptor.Opcode=2;
    if(ntw16s_global_process_event_pid(&record))return 4;
    record.EventHeader.EventDescriptor.Opcode=1;record.EventHeader.ProviderId.Data1++;
    if(ntw16s_global_process_event_pid(&record) || ntw16s_global_process_event_pid(NULL))return 5;
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
    win32_process_global_recover_orphans(NULL);
    puts("GLOBAL PROCESS PARSER PASS 32=1 64=1 short=1 opcode=1 provider=1");return 0;
}
