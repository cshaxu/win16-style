#include "global_session.h"
#include <stdint.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>

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
BOOL ntw16s_global_files_equal(const WCHAR *a,const WCHAR *b)
{
    HANDLE left=INVALID_HANDLE_VALUE,right=INVALID_HANDLE_VALUE;BOOL equal=FALSE;
    BY_HANDLE_FILE_INFORMATION li,ri;LARGE_INTEGER ls,rs;BYTE x[16384],y[16384];DWORD nx,ny;
    if(!a || !b)return FALSE;
    left=CreateFileW(a,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT,NULL);
    if(left==INVALID_HANDLE_VALUE)goto done;
    right=CreateFileW(b,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT,NULL);
    if(right==INVALID_HANDLE_VALUE)goto done;
    if(!GetFileInformationByHandle(left,&li) || !GetFileInformationByHandle(right,&ri) ||
       ((li.dwFileAttributes|ri.dwFileAttributes)&(FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_REPARSE_POINT)) ||
       !GetFileSizeEx(left,&ls) || !GetFileSizeEx(right,&rs) || ls.QuadPart<=0 || ls.QuadPart!=rs.QuadPart)goto done;
    do{
        if(!ReadFile(left,x,sizeof(x),&nx,NULL) || !ReadFile(right,y,sizeof(y),&ny,NULL) || nx!=ny || memcmp(x,y,nx))goto done;
    }while(nx);
    equal=TRUE;
done:
    if(right!=INVALID_HANDLE_VALUE)CloseHandle(right);
    if(left!=INVALID_HANDLE_VALUE)CloseHandle(left);
    return equal;
}
BOOL ntw16s_global_owner_matches(const ntw16s_global_session *s,HMODULE module)
{
    BOOL ok=FALSE;DWORD size=32752;WCHAR *owner=(WCHAR *)calloc(size,sizeof(WCHAR));
    WCHAR *dll=(WCHAR *)calloc(size,sizeof(WCHAR));HANDLE process=NULL;
    static SRWLOCK lock=SRWLOCK_INIT;static DWORD cached_cookie,cached_pid;static HMODULE cached_module;
    if(!s || !module || !owner || !dll)goto done;
    process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,s->owner_pid);
    if(!process || !QueryFullProcessImageNameW(process,0,owner,&size))goto done;
    WCHAR *owner_name=wcsrchr(owner,L'\\');
    if(!owner_name || _wcsicmp(owner_name+1,L"run16s.exe"))goto done;
    DWORD length=GetModuleFileNameW(module,dll,32752);
    if(!length || length>=32752)goto done;
    WCHAR *slash=wcsrchr(dll,L'\\');if(!slash || (size_t)(slash-dll)+11>=32752)goto done;
    wcscpy(slash+1,L"run16s.exe");ok=!lstrcmpiW(owner,dll);
    if(!ok){
        AcquireSRWLockExclusive(&lock);
        if(cached_cookie==s->cookie && cached_pid==s->owner_pid && cached_module==module)ok=TRUE;
        else{
            DWORD n=GetModuleFileNameW(module,dll,32752);WCHAR *owner_slash=wcsrchr(owner,L'\\');
            if(n && n<32752 && owner_slash && (size_t)(owner_slash-owner)+16<32752){
                wcscpy(owner_slash+1,sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");
                ok=ntw16s_global_files_equal(dll,owner);
                if(ok){cached_cookie=s->cookie;cached_pid=s->owner_pid;cached_module=module;}
            }
        }
        ReleaseSRWLockExclusive(&lock);
    }
done:
    if(process)CloseHandle(process);
    free(owner);free(dll);return ok;
}
