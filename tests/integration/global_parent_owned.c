#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "run16s-exe/global.h"
#include "ntw16s-dll/global/session.h"

static int witness(void)
{
    WCHAR name[NTW16S_GLOBAL_NAME_CAP];
    if(!ntw16s_global_name(name,ARRAYSIZE(name)))return 2;
    ULONGLONG until=GetTickCount64()+10000;
    while(GetTickCount64()<until){
        HANDLE map=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
        ntw16s_global_session *s=map?(void *)MapViewOfFile(map,FILE_MAP_READ,0,0,sizeof(*s)):NULL;
        BOOL ready=s && s->phase==NTW16S_GLOBAL_ACTIVE && s->ready;
        if(s)UnmapViewOfFile(s);
        if(map)CloseHandle(map);
        if(ready)return 0;
        Sleep(20);
    }
    return 3;
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !wcscmp(argv[1],L"witness"))return witness();
    WCHAR desk[128],self[32768],command[32790],name[NTW16S_GLOBAL_NAME_CAP];DWORD needed=0;
    if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desk,sizeof(desk),&needed) ||
       wcsncmp(desk,L"Win16S-Dialog-",14) || !ntw16s_global_name(name,ARRAYSIZE(name)))return 4;
    DWORD n=GetModuleFileNameW(NULL,self,ARRAYSIZE(self));
    if(!n || n>=ARRAYSIZE(self) || swprintf(command,ARRAYSIZE(command),L"\"%ls\" witness",self)<0)return 5;
    STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION pi={0};
    if(!CreateProcessW(self,command,NULL,NULL,FALSE,CREATE_NO_WINDOW,NULL,NULL,&si,&pi))return 6;
    ULONGLONG start=GetTickCount64();int result=run16s_global_with_parent(pi.hProcess);
    DWORD waited=WaitForSingleObject(pi.hProcess,3000),code=999;
    if(waited==WAIT_OBJECT_0)GetExitCodeProcess(pi.hProcess,&code);
    else{TerminateProcess(pi.hProcess,ERROR_TIMEOUT);WaitForSingleObject(pi.hProcess,1000);}
    HANDLE mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);DWORD open_error=mapping?0:GetLastError();
    BOOL gone=!mapping && open_error==ERROR_FILE_NOT_FOUND;
    if(mapping)CloseHandle(mapping);
    CloseHandle(pi.hThread);CloseHandle(pi.hProcess);
    printf("GLOBAL PARENT OWNER result=%d witness=%lu gone=%d elapsed=%llu\n",result,code,gone,(unsigned long long)(GetTickCount64()-start));
    return !result && !code && gone?0:7;
}
