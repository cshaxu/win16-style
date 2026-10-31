#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global_session.h"
static BOOL launch(const WCHAR *image,const WCHAR *argument,PROCESS_INFORMATION *p)
{WCHAR command[32768];STARTUPINFOW si={.cb=sizeof(si)};return swprintf(command,ARRAYSIZE(command),L"\"%ls\" %ls",image,argument)>=0 && CreateProcessW(image,command,NULL,NULL,TRUE,0,NULL,NULL,&si,p);}
static int query(HWND window,UINT message,DWORD timeout)
{DWORD_PTR result=0;return SendMessageTimeoutW(window,message,0,0,SMTO_ABORTIFHUNG,timeout,&result)?(int)result:-1;}
static int run(const WCHAR *launcher,const WCHAR *target,const WCHAR *request)
{
 PROCESS_INFORMATION app={0},manager={0};HANDLE event=NULL;HWND window=NULL;int result=1;
 if(!launch(target,L"managed",&app))goto done;
 for(unsigned i=0;i<100&&!window;++i){window=FindWindowW(L"GlobalPropagationProbe",L"Owned propagation root");if(!window)Sleep(30);}
 if(!window || query(window,WM_APP+110,1000)!=0)goto done;
 for(unsigned cycle=0;cycle<2;++cycle){
  if(!launch(launcher,L"--global",&manager))goto done;
  int attached=0;for(unsigned i=0;i<200&&!attached;++i){attached=query(window,WM_APP+110,1000)==1;if(!attached)Sleep(30);}
  if(!attached)goto done;
  int active_child=query(window,WM_APP+111,8000);
  event=OpenEventW(EVENT_MODIFY_STATE,FALSE,request);if(!event||!SetEvent(event)||WaitForSingleObject(manager.hProcess,10000)!=WAIT_OBJECT_0)goto done;
  DWORD exitcode=99;if(!GetExitCodeProcess(manager.hProcess,&exitcode)||exitcode)goto done;
  int removed=query(window,WM_APP+110,1000),stopped_child=query(window,WM_APP+111,8000);
  printf("GLOBAL PROPAGATION LIFECYCLE cycle=%u attached=%d active_child=%d removed=%d stopped_child=%d\n",cycle,attached,active_child,removed,stopped_child);
  if(active_child!=0 || removed!=0 || stopped_child!=41)goto done;
  CloseHandle(event);event=NULL;CloseHandle(manager.hThread);CloseHandle(manager.hProcess);ZeroMemory(&manager,sizeof(manager));
 }
 result=0;
 done:
 if(manager.hProcess && WaitForSingleObject(manager.hProcess,0)!=WAIT_OBJECT_0){if(!event)event=OpenEventW(EVENT_MODIFY_STATE,FALSE,request);if(event)SetEvent(event);if(WaitForSingleObject(manager.hProcess,5000)!=WAIT_OBJECT_0){TerminateProcess(manager.hProcess,ERROR_TIMEOUT);WaitForSingleObject(manager.hProcess,1000);}}
 if(window)PostMessageW(window,WM_CLOSE,0,0);
 if(app.hProcess){if(WaitForSingleObject(app.hProcess,3000)!=WAIT_OBJECT_0){TerminateProcess(app.hProcess,ERROR_TIMEOUT);WaitForSingleObject(app.hProcess,1000);}CloseHandle(app.hThread);CloseHandle(app.hProcess);}
 if(event)CloseHandle(event);
 if(manager.hThread)CloseHandle(manager.hThread);
 if(manager.hProcess)CloseHandle(manager.hProcess);
 return result;
}
int wmain(int argc,WCHAR **argv)
{
 if(argc!=4)return 2;
 WCHAR desktop[128],name[NTW16S_GLOBAL_NAME_CAP],request[NTW16S_GLOBAL_NAME_CAP+16];DWORD needed=0;
 if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&needed)||wcsncmp(desktop,L"Win16S-Dialog-",14))return 3;
 if(!ntw16s_global_name(name,ARRAYSIZE(name))||swprintf(request,ARRAYSIZE(request),L"%ls.request",name)<0)return 4;
 int x86=run(argv[1],argv[2],request),x64=run(argv[1],argv[3],request);
 if(!x86&&!x64){puts("GLOBAL PROPAGATION LIFECYCLE PASS widths=2 cycles=2 active=4 stopped=4 removed=4");return 0;}
 printf("GLOBAL PROPAGATION LIFECYCLE FAIL x86=%d x64=%d error=%lu\n",x86,x64,GetLastError());return 1;
}
