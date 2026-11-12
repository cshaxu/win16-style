#include "ntw16s-dll/global/session.h"
#include <stdint.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>

BOOL ntw16s_global_name(WCHAR *name,DWORD capacity)
{
    DWORD session=0;ULONGLONG hash=0;
    if(!name || capacity<NTW16S_GLOBAL_NAME_CAP){SetLastError(ERROR_INSUFFICIENT_BUFFER);return FALSE;}
    if(!win32_process_global_desktop_key(&session,&hash))return FALSE;
    if(swprintf(name,capacity,L"Local\\Win16Style.Global.%lu.%016llx",session,(unsigned long long)hash)<0){SetLastError(ERROR_INSUFFICIENT_BUFFER);return FALSE;}
    return TRUE;
}
BOOL ntw16s_global_owner_alive(const ntw16s_global_session *s)
{
    BOOL result;
    if(!s || s->magic!=NTW16S_GLOBAL_MAGIC || s->version!=NTW16S_GLOBAL_VERSION || !s->owner_pid || !s->cookie)return FALSE;
    HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,s->owner_pid);
    if(!process)return FALSE;
    result=win32_process_global_alive(process,s->owner_birth_low,s->owner_birth_high);
    CloseHandle(process);return result;
}
BOOL ntw16s_global_session_matches(const ntw16s_global_session *s,DWORD cookie,DWORD style)
{
    return s && cookie && s->cookie==cookie && s->phase==NTW16S_GLOBAL_ACTIVE && s->style==style &&
           ntw16s_global_owner_alive(s);
}
static BOOL policy(DWORD cookie,DWORD style,BOOL match_style)
{
    DWORD saved=GetLastError();BOOL active=FALSE;WCHAR name[NTW16S_GLOBAL_NAME_CAP];
    if(cookie && ntw16s_global_name(name,ARRAYSIZE(name))){
        HANDLE mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
        ntw16s_global_session *s=mapping?(ntw16s_global_session *)MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*s)):NULL;
        if(s && (match_style?ntw16s_global_session_matches(s,cookie,style):
                 (s->cookie==cookie && s->phase==NTW16S_GLOBAL_ACTIVE && ntw16s_global_owner_alive(s))))active=TRUE;
        if(s)UnmapViewOfFile(s);
        if(mapping)CloseHandle(mapping);
    }
    SetLastError(saved);return active;
}
BOOL ntw16s_global_policy_active(DWORD cookie){return policy(cookie,0,FALSE);}
BOOL ntw16s_global_policy_style(DWORD cookie,DWORD style){return policy(cookie,style,TRUE);}
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
                ok=win32_process_global_files_equal(dll,owner);
                if(ok){cached_cookie=s->cookie;cached_pid=s->owner_pid;cached_module=module;}
            }
        }
        ReleaseSRWLockExclusive(&lock);
    }
done:
    if(process)CloseHandle(process);
    free(owner);free(dll);return ok;
}
