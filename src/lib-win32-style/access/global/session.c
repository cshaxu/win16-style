#include "session.h"
#include "lib-win32/process-global/identity.h"
#include "lib-win32/process-global/module.h"
#include <stdint.h>
#include <wchar.h>
#include <stdlib.h>
#include <string.h>

BOOL style_global_session_name(const style_global_namespace *space,WCHAR *name,DWORD capacity)
{
    DWORD session=0;ULONGLONG hash=0;
    if(!name || capacity<STYLE_GLOBAL_NAME_CAP){SetLastError(ERROR_INSUFFICIENT_BUFFER);return FALSE;}
    if(!win32_process_global_desktop_key(&session,&hash))return FALSE;
    if(swprintf(name,capacity,L"%ls.%lu.%016llx",space->prefix,session,(unsigned long long)hash)<0){SetLastError(ERROR_INSUFFICIENT_BUFFER);return FALSE;}
    return TRUE;
}
BOOL style_global_owner_alive(const style_global_namespace *space,const style_global_session *s)
{
    BOOL result;
    if(!s || s->magic!=space->magic || s->version!=space->version || !s->owner_pid || !s->cookie)return FALSE;
    HANDLE process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION|SYNCHRONIZE,FALSE,s->owner_pid);
    if(!process)return FALSE;
    result=win32_process_global_alive(process,s->owner_birth_low,s->owner_birth_high);
    CloseHandle(process);return result;
}
BOOL style_global_session_matches(const style_global_namespace *space,const style_global_session *s,DWORD cookie,DWORD style)
{
    return s && cookie && s->cookie==cookie && s->phase==STYLE_GLOBAL_ACTIVE && s->style==style &&
           style_global_owner_alive(space,s);
}
static BOOL policy(const style_global_namespace *space,DWORD cookie,DWORD style,BOOL match_style)
{
    DWORD saved=GetLastError();BOOL active=FALSE;WCHAR name[STYLE_GLOBAL_NAME_CAP];
    if(cookie && style_global_session_name(space,name,ARRAYSIZE(name))){
        HANDLE mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
        style_global_session *s=mapping?(style_global_session *)MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*s)):NULL;
        if(s && (match_style?style_global_session_matches(space,s,cookie,style):
                 (s->cookie==cookie && s->phase==STYLE_GLOBAL_ACTIVE && style_global_owner_alive(space,s))))active=TRUE;
        if(s)UnmapViewOfFile(s);
        if(mapping)CloseHandle(mapping);
    }
    SetLastError(saved);return active;
}
BOOL style_global_policy_active(const style_global_namespace *space,DWORD cookie){return policy(space,cookie,0,FALSE);}
BOOL style_global_policy_style(const style_global_namespace *space,DWORD cookie,DWORD style){return policy(space,cookie,style,TRUE);}
BOOL style_global_owner_matches(style_global_namespace *space,const style_global_session *s,HMODULE module)
{
    BOOL ok=FALSE;DWORD size=32752;WCHAR *owner=(WCHAR *)calloc(size,sizeof(WCHAR));
    WCHAR *dll=(WCHAR *)calloc(size,sizeof(WCHAR));HANDLE process=NULL;
    if(!s || !module || !owner || !dll)goto done;
    process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,s->owner_pid);
    if(!process || !QueryFullProcessImageNameW(process,0,owner,&size))goto done;
    WCHAR *owner_name=wcsrchr(owner,L'\\');
    if(!owner_name || _wcsicmp(owner_name+1,space->launcher))goto done;
    DWORD length=GetModuleFileNameW(module,dll,32752);
    if(!length || length>=32752)goto done;
    WCHAR *slash=wcsrchr(dll,L'\\');if(!slash || (size_t)(slash-dll)+wcslen(space->launcher)+1>=32752)goto done;
    wcscpy(slash+1,space->launcher);ok=!lstrcmpiW(owner,dll);
    if(!ok){
        AcquireSRWLockExclusive(&space->owner.lock);
        if(space->owner.cookie==s->cookie && space->owner.pid==s->owner_pid && space->owner.module==module)ok=TRUE;
        else{
            DWORD n=GetModuleFileNameW(module,dll,32752);WCHAR *owner_slash=wcsrchr(owner,L'\\');
            if(n && n<32752 && owner_slash && (size_t)(owner_slash-owner)+max((size_t)16,wcslen(space->module)+1)<32752){
                wcscpy(owner_slash+1,space->module);
                ok=win32_process_global_files_equal(dll,owner);
                if(ok){space->owner.cookie=s->cookie;space->owner.pid=s->owner_pid;space->owner.module=module;}
            }
        }
        ReleaseSRWLockExclusive(&space->owner.lock);
    }
done:
    if(process)CloseHandle(process);
    free(owner);free(dll);return ok;
}
