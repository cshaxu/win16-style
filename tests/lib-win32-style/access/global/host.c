#include "lib-win32-style/access/global/host.h"
#include "lib-win32/process-global/identity.h"
#include <stdio.h>
#include <wchar.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"host line=%d error=%lu\n",__LINE__,GetLastError());result=1;goto done;}}while(0)
int main(void)
{
    int result=0;WCHAR prefix[80],name[STYLE_GLOBAL_NAME_CAP];HANDLE mapping=NULL;style_global_session *session=NULL;
    swprintf(prefix,ARRAYSIZE(prefix),L"Local\\StyleHostTest.%lu",GetCurrentProcessId());
    style_global_namespace space={.prefix=prefix,.launcher=L"owned.exe",.module=L"owned.dll",.magic=71,.version=2};
    const style_global_start_config start={&space,"OwnedWorker"};
    const style_global_attach_config attach={.start=&start};
    const style_global_manager_config manager={.attach=&attach};
    const style_global_host_spec spec={.manager=&manager};
    style_global_manager *retained=NULL;DWORD before=0,after=0;
    CHECK(style_global_session_name(&space,name,ARRAYSIZE(name)));
    CHECK(style_global_host_run(&spec,&retained)==ERROR_FILE_NOT_FOUND && !retained);
    mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(*session),name);
    CHECK(mapping && GetLastError()!=ERROR_ALREADY_EXISTS);
    session=MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(*session));CHECK(session);
    *session=(style_global_session){.magic=71,.version=1,.owner_pid=GetCurrentProcessId(),.cookie=991};
    CHECK(win32_process_global_birth(GetCurrentProcess(),&session->owner_birth_low,&session->owner_birth_high));
    CHECK(GetProcessHandleCount(GetCurrentProcess(),&before));
    for(unsigned i=0;i<50;++i)CHECK(style_global_host_run(&spec,&retained)==ERROR_INVALID_STATE && !retained);
    session->version=2;session->owner_birth_low^=1;
    CHECK(style_global_host_run(&spec,&retained)==ERROR_INVALID_STATE && !retained);
    CHECK(GetProcessHandleCount(GetCurrentProcess(),&after) && before==after);
 done:
    if(session)UnmapViewOfFile(session);
    if(mapping)CloseHandle(mapping);
    if(!result)puts("GLOBAL_HOST_OK missing=1 protocol=1 owner_birth=1 repeated=50 handles=1 no_hook=1");
    return result;
}
