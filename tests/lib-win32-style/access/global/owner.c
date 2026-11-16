#include "lib-win32-style/access/global/owner.h"
#include <stdio.h>
#include <wchar.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"owner line=%d error=%lu\n",__LINE__,GetLastError());result=1;goto done;}}while(0)
static unsigned calls;
static DWORD paths(DWORD style,WCHAR *native,WCHAR *peer,DWORD capacity,void *context)
{
    (void)native;(void)peer;(void)capacity;(void)context;
    ++calls;return style==42?ERROR_FILE_NOT_FOUND:ERROR_INVALID_PARAMETER;
}
int main(void)
{
    int result=0;WCHAR prefix[80],name[STYLE_GLOBAL_NAME_CAP];HANDLE parent=NULL,mapping=NULL;
    style_global_session *session=NULL;style_global_manager *retained=NULL;DWORD before=0,after=0;
    swprintf(prefix,ARRAYSIZE(prefix),L"Local\\StyleOwnerTest.%lu",GetCurrentProcessId());
    style_global_namespace space={.prefix=prefix,.launcher=L"owned.exe",.module=L"owned.dll",.magic=83,.version=4};
    const style_global_start_config start={&space,"OwnedWorker"};
    const style_global_attach_config attach={.start=&start};
    const style_global_manager_config manager={.attach=&attach};
    const style_global_owner_spec spec={.manager=&manager,.paths=paths};
    CHECK(style_global_session_name(&space,name,ARRAYSIZE(name)));
    parent=CreateEventW(NULL,TRUE,TRUE,NULL);CHECK(parent);
    CHECK(!style_global_owner_run(&spec,parent,42,&retained) && !calls && !retained);
    CloseHandle(parent);parent=NULL;
    CHECK(style_global_owner_run(&spec,NULL,42,&retained)==ERROR_FILE_NOT_FOUND && calls==1 && !retained);
    CHECK(GetProcessHandleCount(GetCurrentProcess(),&before));
    for(unsigned i=0;i<20;++i)CHECK(style_global_owner_run(&spec,NULL,42,&retained)==ERROR_FILE_NOT_FOUND && !retained);
    CHECK(GetProcessHandleCount(GetCurrentProcess(),&after) && before==after && calls==21);
    HANDLE missing=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
    if(missing){CloseHandle(missing);CHECK(FALSE);}
    CHECK(GetLastError()==ERROR_FILE_NOT_FOUND);
    mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(*session),name);
    CHECK(mapping && GetLastError()!=ERROR_ALREADY_EXISTS);
    session=MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(*session));CHECK(session);
    session->cookie=991;
    CHECK(style_global_owner_run(&spec,NULL,42,&retained)==ERROR_ALREADY_EXISTS && !retained);
    CHECK(calls==21 && session->cookie==991);
 done:
    if(parent)CloseHandle(parent);
    if(session)UnmapViewOfFile(session);
    if(mapping)CloseHandle(mapping);
    if(!result)puts("GLOBAL_OWNER_OK parent_cancel=1 opaque_style=42 preflight_failure=1 repeated=20 handles=1 mapping_gone=1 collision_untouched=1 no_hook=1");
    return result;
}
