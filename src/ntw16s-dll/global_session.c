#include "global_session.h"
#include <stdint.h>
#include <wchar.h>

BOOL ntw16s_global_name(WCHAR *name,DWORD capacity)
{
    WCHAR desktop[256];DWORD needed=0,session=0;uint64_t hash=UINT64_C(1469598103934665603);
    if(!name || capacity<NTW16S_GLOBAL_NAME_CAP){SetLastError(ERROR_INSUFFICIENT_BUFFER);return FALSE;}
    if(!ProcessIdToSessionId(GetCurrentProcessId(),&session))return FALSE;
    HDESK desk=GetThreadDesktop(GetCurrentThreadId());
    if(!desk || !GetUserObjectInformationW(desk,UOI_NAME,desktop,sizeof(desktop),&needed))return FALSE;
    desktop[ARRAYSIZE(desktop)-1]=0;CharUpperBuffW(desktop,(DWORD)wcslen(desktop));
    for(const WCHAR *p=desktop;*p;++p){hash^=(uint16_t)*p;hash*=UINT64_C(1099511628211);}
    if(swprintf(name,capacity,L"Local\\Win16Style.Global.%lu.%016llx",session,(unsigned long long)hash)<0){SetLastError(ERROR_INSUFFICIENT_BUFFER);return FALSE;}
    return TRUE;
}
BOOL ntw16s_global_birth(HANDLE process,DWORD *low,DWORD *high)
{
    FILETIME creation,exit,kernel,user;
    if(!process || !low || !high){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    if(!GetProcessTimes(process,&creation,&exit,&kernel,&user))return FALSE;
    *low=creation.dwLowDateTime;*high=creation.dwHighDateTime;return TRUE;
}
BOOL ntw16s_global_owner_alive(const ntw16s_global_session *s)
{
    DWORD low=0,high=0;BOOL result=FALSE;
    if(!s || s->magic!=NTW16S_GLOBAL_MAGIC || s->version!=NTW16S_GLOBAL_VERSION || !s->owner_pid || !s->cookie)return FALSE;
    HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,s->owner_pid);
    if(!process)return FALSE;
    if(WaitForSingleObject(process,0)==WAIT_TIMEOUT && ntw16s_global_birth(process,&low,&high) &&
       low==s->owner_birth_low && high==s->owner_birth_high)result=TRUE;
    CloseHandle(process);return result;
}
BOOL ntw16s_global_policy_active(DWORD cookie)
{
    DWORD saved=GetLastError();BOOL active=FALSE;WCHAR name[NTW16S_GLOBAL_NAME_CAP];
    if(cookie && ntw16s_global_name(name,ARRAYSIZE(name))){
        HANDLE mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
        ntw16s_global_session *s=mapping?(ntw16s_global_session *)MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*s)):NULL;
        if(s && s->cookie==cookie && s->phase==NTW16S_GLOBAL_ACTIVE && ntw16s_global_owner_alive(s))active=TRUE;
        if(s)UnmapViewOfFile(s);
        if(mapping)CloseHandle(mapping);
    }
    SetLastError(saved);return active;
}
