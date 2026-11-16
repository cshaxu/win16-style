#include "lib-win32-style/access/global/session.h"
#include "lib-win32/process-global/identity.h"
#include <stdio.h>
#include <wchar.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"session line=%d error=%lu\n",__LINE__,GetLastError());result=1;goto done;}}while(0)
int main(void)
{
    int result=0;HANDLE mapping=NULL;style_global_session *session=NULL;
    WCHAR prefix[80],other_prefix[80],name[STYLE_GLOBAL_NAME_CAP],other_name[STYLE_GLOBAL_NAME_CAP],image[32752];
    CHECK(GetModuleFileNameW(NULL,image,ARRAYSIZE(image)));
    PCWSTR base=wcsrchr(image,L'\\');CHECK(base);++base;
    swprintf(prefix,ARRAYSIZE(prefix),L"Local\\StyleSessionTest.%lu",GetCurrentProcessId());
    swprintf(other_prefix,ARRAYSIZE(other_prefix),L"Local\\StyleSessionOther.%lu",GetCurrentProcessId());
    style_global_namespace space={.prefix=prefix,.launcher=base,.module=base,.magic=0x13572468,.version=9};
    style_global_namespace other={.prefix=other_prefix,.launcher=base,.module=base,.magic=0x13572468,.version=9};
    CHECK(!style_global_session_name(&space,name,1) && GetLastError()==ERROR_INSUFFICIENT_BUFFER);
    CHECK(style_global_session_name(&space,name,ARRAYSIZE(name)) && style_global_session_name(&other,other_name,ARRAYSIZE(other_name)));
    CHECK(wcscmp(name,other_name));
    mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(*session),name);
    CHECK(mapping && GetLastError()!=ERROR_ALREADY_EXISTS);
    session=MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(*session));CHECK(session);
    *session=(style_global_session){.magic=space.magic,.version=space.version,.owner_pid=GetCurrentProcessId(),
        .cookie=7,.phase=STYLE_GLOBAL_ACTIVE,.ready=TRUE,.style=42};
    CHECK(win32_process_global_birth(GetCurrentProcess(),&session->owner_birth_low,&session->owner_birth_high));
    CHECK(style_global_owner_alive(&space,session) && style_global_owner_matches(&space,session,GetModuleHandleW(NULL)));
    CHECK(style_global_session_matches(&space,session,7,42) && !style_global_session_matches(&space,session,8,42));
    DWORD before=0,after=0;CHECK(style_global_policy_active(&space,7));
    CHECK(GetProcessHandleCount(GetCurrentProcess(),&before));
    for(unsigned i=0;i<100;++i){
        SetLastError(0x4411);
        CHECK(style_global_policy_style(&space,7,42) && !style_global_policy_style(&space,7,99) &&
            !style_global_policy_active(&other,7) && GetLastError()==0x4411);
    }
    CHECK(GetProcessHandleCount(GetCurrentProcess(),&after) && before==after);
    session->owner_birth_low^=1;CHECK(!style_global_owner_alive(&space,session));session->owner_birth_low^=1;
    session->version^=1;CHECK(!style_global_policy_active(&space,7));session->version^=1;
    session->phase=STYLE_GLOBAL_STOPPING;CHECK(!style_global_policy_active(&space,7));
    session->phase=STYLE_GLOBAL_ACTIVE;session->owner_pid=0;CHECK(!style_global_policy_active(&space,7));
 done:
    if(session)UnmapViewOfFile(session);
    if(mapping)CloseHandle(mapping);
    if(!result){
        if(style_global_policy_active(&space,7))return 2;
        puts("GLOBAL_SESSION_OK namespaces=1 opaque_style=42 owner_birth=1 version=1 cookie=1 phase=1 last_error=1 handles=1 cleanup=1");
    }
    return result;
}
