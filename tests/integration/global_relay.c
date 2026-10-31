#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global_session.h"
int wmain(int argc,WCHAR **argv)
{if(argc!=4)return 2;WCHAR desk[128],name[NTW16S_GLOBAL_NAME_CAP],request[NTW16S_GLOBAL_NAME_CAP+16],cmd[8192];DWORD len=0;
 if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desk,sizeof(desk),&len)||wcsncmp(desk,L"Win16S-Dialog-",14))return 3;
 if(!ntw16s_global_name(name,ARRAYSIZE(name))||swprintf(request,ARRAYSIZE(request),L"%ls.request",name)<0)return 4;
 STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION manager={0},target={0};HANDLE map=NULL,event=NULL;ntw16s_global_session *s=NULL;BOOL pass=FALSE;DWORD code=99;
 swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" --global",argv[1]);if(!CreateProcessW(argv[1],cmd,NULL,NULL,TRUE,0,NULL,NULL,&si,&manager))goto done;
 for(unsigned i=0;i<200;++i){map=OpenFileMappingW(FILE_MAP_READ,FALSE,name);if(map){s=MapViewOfFile(map,FILE_MAP_READ,0,0,sizeof(*s));if(s&&s->owner_pid==manager.dwProcessId&&s->phase==NTW16S_GLOBAL_ACTIVE&&s->ready)break;if(s)UnmapViewOfFile(s);s=NULL;CloseHandle(map);map=NULL;}if(WaitForSingleObject(manager.hProcess,0)==WAIT_OBJECT_0)break;Sleep(50);}
 if(!s||s->phase!=NTW16S_GLOBAL_ACTIVE||!s->ready)goto done;
 swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" %ls",argv[2],argv[3]);if(!CreateProcessW(argv[2],cmd,NULL,NULL,TRUE,0,NULL,NULL,&si,&target))goto done;
 if(WaitForSingleObject(target.hProcess,12000)!=WAIT_OBJECT_0||!GetExitCodeProcess(target.hProcess,&code)||code)goto done;
 printf("GLOBAL RELAY PASS target_exit=0\n");pass=TRUE;
 done:if(!pass)printf("GLOBAL RELAY FAIL target_exit=%lu error=%lu\n",code,GetLastError());
 if(target.hProcess){if(WaitForSingleObject(target.hProcess,0)!=WAIT_OBJECT_0){TerminateProcess(target.hProcess,ERROR_TIMEOUT);WaitForSingleObject(target.hProcess,1000);}CloseHandle(target.hThread);CloseHandle(target.hProcess);}
 if(manager.hProcess&&WaitForSingleObject(manager.hProcess,0)!=WAIT_OBJECT_0){event=OpenEventW(EVENT_MODIFY_STATE,FALSE,request);if(event)SetEvent(event);if(WaitForSingleObject(manager.hProcess,6000)!=WAIT_OBJECT_0){TerminateProcess(manager.hProcess,ERROR_TIMEOUT);WaitForSingleObject(manager.hProcess,1000);pass=FALSE;}}
 if(event)CloseHandle(event);
 if(s)UnmapViewOfFile(s);
 if(map)CloseHandle(map);
 if(manager.hThread)CloseHandle(manager.hThread);
 if(manager.hProcess)CloseHandle(manager.hProcess);
 return pass?0:1;}
