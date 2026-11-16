#include "lib-win32-style/access/global/receiver.h"
#include "lib-win32/process-global/identity.h"
#include <stdio.h>
#include <wchar.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"receiver line=%d error=%lu\n",__LINE__,GetLastError());result=1;goto done;}}while(0)
static unsigned applies,removes,allowed_calls;
static BOOL stop_during_apply;
static style_global_session *session;
static style_global_receiver *current;
static UINT control;
static BOOL allowed(const WCHAR *image,const WCHAR *windows)
{++allowed_calls;return image && *image && windows && *windows;}
static BOOL denied(const WCHAR *image,const WCHAR *windows)
{(void)image;(void)windows;++allowed_calls;return FALSE;}
static BOOL eligible(HWND window){return IsWindow(window);}
static void change(HWND window,DWORD style,BOOL apply,void *context)
{
    (void)context;
    if(style!=42)ExitProcess(9);
    if(apply){
        ++applies;CWPSTRUCT nested={.hwnd=window,.message=control,.wParam=991,.lParam=10};
        style_global_receiver_handle(current,0,0,(LPARAM)&nested);
        if(stop_during_apply)InterlockedExchange(&session->phase,STYLE_GLOBAL_STOPPING);
    }else ++removes;
    SetLastError(0x7777);
}
static int target(style_global_namespace *space,const WCHAR *name)
{
    int result=0;HANDLE mapping=OpenFileMappingW(FILE_MAP_ALL_ACCESS,FALSE,name);HWND window=NULL;
    session=mapping?MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(*session)):NULL;CHECK(session);
    style_global_late late={0};style_global_late_mark(&late,991);
    const style_global_receiver_ops ops={space,&late,L"StyleReceiverTest.Control",10,20,allowed,eligible,change,NULL};
    style_global_receiver receiver={.ops=&ops};current=&receiver;control=RegisterWindowMessageW(ops.control);CHECK(control);
    window=CreateWindowW(L"STATIC",L"owned",WS_OVERLAPPEDWINDOW,0,0,100,80,NULL,NULL,GetModuleHandleW(NULL),NULL);CHECK(window);
    CWPSTRUCT message={.hwnd=window,.message=control,.wParam=991,.lParam=10};
    SetLastError(0x1234);style_global_receiver_handle(&receiver,0,0,(LPARAM)&message);
    CHECK(applies==1 && !removes && allowed_calls==1 && GetLastError()==0x1234);
    message.wParam=992;style_global_receiver_handle(&receiver,0,0,(LPARAM)&message);CHECK(applies==1);
    message.wParam=991;style_global_receiver_handle(&receiver,-1,0,(LPARAM)&message);CHECK(applies==1);
    stop_during_apply=TRUE;style_global_receiver_handle(&receiver,0,0,(LPARAM)&message);
    CHECK(applies==2 && removes==1 && allowed_calls==1);
    message.lParam=20;style_global_receiver_handle(&receiver,0,0,(LPARAM)&message);CHECK(removes==2);
    message.lParam=10;style_global_receiver_handle(&receiver,0,0,(LPARAM)&message);CHECK(applies==2);
    style_global_receiver_ops deny_ops=ops;deny_ops.allowed=denied;
    style_global_receiver deny_receiver={.ops=&deny_ops};
    InterlockedExchange(&session->phase,STYLE_GLOBAL_ACTIVE);
    style_global_receiver_handle(&deny_receiver,0,0,(LPARAM)&message);
    style_global_receiver_handle(&deny_receiver,0,0,(LPARAM)&message);
    CHECK(applies==2 && removes==2 && allowed_calls==2);
 done:
    if(window)DestroyWindow(window);
    if(session)UnmapViewOfFile(session);
    if(mapping)CloseHandle(mapping);
    if(!result)puts("GLOBAL_RECEIVER_OK direct=1 no_style=1 owner=1 cookie=1 once=1 reentry=1 stop_race=1 remove=1 last_error=1");
    return result;
}
int wmain(int argc,WCHAR **argv)
{
    (void)argv;
    WCHAR self[MAX_PATH],prefix[80],name[STYLE_GLOBAL_NAME_CAP],command[MAX_PATH+32];
    if(!GetModuleFileNameW(NULL,self,ARRAYSIZE(self)))return 2;
    WCHAR *base=wcsrchr(self,L'\\');if(!base)return 3;++base;
    if(argc==2){if(!GetEnvironmentVariableW(L"STYLE_RECEIVER_PREFIX",prefix,ARRAYSIZE(prefix)))return 4;}
    else{swprintf(prefix,ARRAYSIZE(prefix),L"Local\\StyleReceiverTest.%lu",GetCurrentProcessId());SetEnvironmentVariableW(L"STYLE_RECEIVER_PREFIX",prefix);}
    style_global_namespace space={.prefix=prefix,.launcher=base,.module=base,.magic=71,.version=2};
    if(!style_global_session_name(&space,name,ARRAYSIZE(name)))return 5;
    if(argc==2)return target(&space,name);
    int result=0;HANDLE mapping=NULL;style_global_session *owned=NULL;PROCESS_INFORMATION child={0};
    mapping=CreateFileMappingW(INVALID_HANDLE_VALUE,NULL,PAGE_READWRITE,0,sizeof(*owned),name);
    CHECK(mapping && GetLastError()!=ERROR_ALREADY_EXISTS);
    owned=MapViewOfFile(mapping,FILE_MAP_ALL_ACCESS,0,0,sizeof(*owned));CHECK(owned);
    *owned=(style_global_session){.magic=71,.version=2,.owner_pid=GetCurrentProcessId(),.cookie=991,.phase=STYLE_GLOBAL_ACTIVE,.style=42};
    CHECK(win32_process_global_birth(GetCurrentProcess(),&owned->owner_birth_low,&owned->owner_birth_high));
    swprintf(command,ARRAYSIZE(command),L"\"%ls\" target",self);STARTUPINFOW startup={.cb=sizeof(startup)};
    CHECK(CreateProcessW(self,command,NULL,NULL,FALSE,0,NULL,NULL,&startup,&child));
    CHECK(WaitForSingleObject(child.hProcess,10000)==WAIT_OBJECT_0);
    DWORD code=1;CHECK(GetExitCodeProcess(child.hProcess,&code) && !code);
 done:
    if(child.hProcess){if(WaitForSingleObject(child.hProcess,0)!=WAIT_OBJECT_0){TerminateProcess(child.hProcess,ERROR_TIMEOUT);WaitForSingleObject(child.hProcess,1000);}CloseHandle(child.hProcess);}
    if(child.hThread)CloseHandle(child.hThread);
    if(owned)UnmapViewOfFile(owned);
    if(mapping)CloseHandle(mapping);
    return result;
}
