#include "ntw16s-dll/global/process.h"
#include "lib-win32/process-global/recovery.h"
#include "lib-win32/process-global/identity.h"
#include <evntrace.h>
#include <evntcons.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

/* Process IDs cannot be reused while our peer process handle is open. The
 * birth time in the name also lets a future invocation recognise an orphan
 * when both controller processes terminated before either could clean up. */
BOOL ntw16s_global_process_parse_name(const WCHAR *name,DWORD *pid,DWORD *high,DWORD *low)
{
    WCHAR expected[96];DWORD p=0,h=0,l=0;int used=0;
    if(!name || !pid || !high || !low || swscanf(name,L"Win16S-ETW-%lu-%8lx%8lx%n",&p,&h,&l,&used)!=3 ||
       !p || !used || name[used] || swprintf(expected,ARRAYSIZE(expected),L"Win16S-ETW-%lu-%08lx%08lx",p,h,l)<0 ||
       wcscmp(name,expected))return FALSE;
    *pid=p;*high=h;*low=l;return TRUE;
}
DWORD ntw16s_global_process_stop_peer(HANDLE process,DWORD pid)
{
    WCHAR name[96];
    DWORD high=0,low=0;
    if(!process || !pid || GetProcessId(process)!=pid || WaitForSingleObject(process,0)!=WAIT_OBJECT_0)return ERROR_INVALID_PARAMETER;
    if(!win32_process_global_birth(process,&low,&high))return GetLastError();
    if(!pid || swprintf(name,ARRAYSIZE(name),L"Win16S-ETW-%lu-%08lx%08lx",pid,high,low)<0)return ERROR_INVALID_PARAMETER;
    return win32_process_global_trace_stop_peer(process,pid,name);
}
DWORD ntw16s_global_process_start(win32_process_global_monitor_callback callback,void *context,win32_process_global_monitor **result)
{
    WCHAR name[96];DWORD high=0,low=0;
    if(result)*result=NULL;
    if(!callback || !result)return ERROR_INVALID_PARAMETER;
    if(!win32_process_global_birth(GetCurrentProcess(),&low,&high))return GetLastError();
    if(swprintf(name,ARRAYSIZE(name),L"Win16S-ETW-%lu-%08lx%08lx",GetCurrentProcessId(),high,low)<0)return ERROR_INVALID_NAME;
    win32_process_global_recover_orphans(ntw16s_global_process_parse_name);
    return win32_process_global_monitor_start(name,callback,context,result);
}
