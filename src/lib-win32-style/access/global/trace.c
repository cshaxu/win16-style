#include "trace.h"
#include "lib-win32/process-global/recovery.h"
#include "lib-win32/process-global/identity.h"
#include <wchar.h>

/* Process IDs cannot be reused while our peer process handle is open. The
 * birth time in the name also lets a future invocation recognise an orphan
 * when both controller processes terminated before either could clean up. */
BOOL style_global_trace_parse(const WCHAR *prefix,const WCHAR *name,DWORD *pid,DWORD *high,DWORD *low)
{
    WCHAR expected[96];DWORD p=0,h=0,l=0;int used=0;
    if(!prefix || !*prefix || !name || !pid || !high || !low)return FALSE;
    size_t length=wcslen(prefix);
    if(wcsncmp(name,prefix,length) || swscanf(name+length,L"%lu-%8lx%8lx%n",&p,&h,&l,&used)!=3 ||
       !p || !used || name[length+used] || swprintf(expected,ARRAYSIZE(expected),L"%ls%lu-%08lx%08lx",prefix,p,h,l)<0 ||
       wcscmp(name,expected))return FALSE;
    *pid=p;*high=h;*low=l;return TRUE;
}
DWORD style_global_trace_stop_peer(const WCHAR *prefix,HANDLE process,DWORD pid)
{
    WCHAR name[96];
    DWORD high=0,low=0;
    if(!prefix || !*prefix || !process || !pid || GetProcessId(process)!=pid || WaitForSingleObject(process,0)!=WAIT_OBJECT_0)return ERROR_INVALID_PARAMETER;
    if(!win32_process_global_birth(process,&low,&high))return GetLastError();
    if(!pid || swprintf(name,ARRAYSIZE(name),L"%ls%lu-%08lx%08lx",prefix,pid,high,low)<0)return ERROR_INVALID_PARAMETER;
    return win32_process_global_trace_stop_peer(process,pid,name);
}
static BOOL parse(const WCHAR *name,DWORD *pid,DWORD *high,DWORD *low,void *context)
{return style_global_trace_parse(context,name,pid,high,low);}
DWORD style_global_trace_start(const WCHAR *prefix,win32_process_global_monitor_callback callback,void *context,win32_process_global_monitor **result)
{
    WCHAR name[96];DWORD high=0,low=0;
    if(result)*result=NULL;
    if(!prefix || !*prefix || !callback || !result)return ERROR_INVALID_PARAMETER;
    if(!win32_process_global_birth(GetCurrentProcess(),&low,&high))return GetLastError();
    if(swprintf(name,ARRAYSIZE(name),L"%ls%lu-%08lx%08lx",prefix,GetCurrentProcessId(),high,low)<0)return ERROR_INVALID_NAME;
    win32_process_global_recover_orphans_using(parse,(void *)prefix);
    return win32_process_global_monitor_start(name,callback,context,result);
}
