#include "lib-win32-style/access/global/trace.h"
#include <stdio.h>
int main(void)
{
    DWORD pid=0,high=0,low=0;
    if(!style_global_trace_parse(L"OwnedA-",L"OwnedA-42-0123456789abcdef",&pid,&high,&low) ||
       pid!=42 || high!=0x01234567 || low!=0x89abcdef)return 1;
    if(!style_global_trace_parse(L"OwnedB-",L"OwnedB-42-0123456789abcdef",&pid,&high,&low))return 2;
    const WCHAR *bad[]={L"OwnedB-42-0123456789abcdef",L"OwnedA-042-0123456789abcdef",
        L"OwnedA-42-0123456789ABCDEF",L"OwnedA-0-0123456789abcdef",L"OwnedA-42-0123456789abcdef-extra"};
    for(unsigned i=0;i<ARRAYSIZE(bad);++i)if(style_global_trace_parse(L"OwnedA-",bad[i],&pid,&high,&low))return 3;
    if(style_global_trace_parse(L"",L"42-0123456789abcdef",&pid,&high,&low) ||
       style_global_trace_parse(NULL,L"42-0123456789abcdef",&pid,&high,&low))return 4;
    if(style_global_trace_stop_peer(L"OwnedA-",GetCurrentProcess(),GetCurrentProcessId())!=ERROR_INVALID_PARAMETER)return 5;
    puts("GLOBAL_TRACE_OK namespaces=2 canonical=1 isolated=1 invalid=1 live_peer_rejected=1 no_etw=1");return 0;
}
