#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global_session.h"
#define QUERY (WM_APP+85)
#define EXTRA (WM_APP+86)
static const WCHAR cls[]=L"Win16GlobalThreads";
static BOOL launch(const WCHAR *image,const WCHAR *argument,PROCESS_INFORMATION *p)
{WCHAR cmd[8192];STARTUPINFOW s={.cb=sizeof(s)};return swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" %ls",image,argument)>=0&&CreateProcessW(image,cmd,NULL,NULL,TRUE,0,NULL,NULL,&s,p);}
static HWND find(const WCHAR *title)
{for(unsigned i=0;i<160;++i){HWND w=FindWindowW(cls,title);if(w&&IsWindowVisible(w))return w;Sleep(30);}return NULL;}
static int query(HWND w,UINT msg)
{DWORD_PTR v=0;return SendMessageTimeoutW(w,msg,0,0,SMTO_ABORTIFHUNG,1000,&v)?(int)v:-1;}
static BOOL wait_mask(HWND w,int mask)
{for(unsigned i=0;i<160;++i){if(query(w,QUERY)==mask)return TRUE;Sleep(30);}return FALSE;}
static BOOL resources(HANDLE p,DWORD *handles,DWORD *gdi)
{*gdi=GetGuiResources(p,GR_GDIOBJECTS);return *gdi!=0 && GetProcessHandleCount(p,handles)!=FALSE && *handles!=0;}
int wmain(int argc,WCHAR **argv)
{
 if(argc!=4)return 2;
 WCHAR desk[128],name[NTW16S_GLOBAL_NAME_CAP],request[NTW16S_GLOBAL_NAME_CAP+16];DWORD n=0;
 if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desk,sizeof(desk),&n)||wcsncmp(desk,L"Win16S-Dialog-",14))return 3;
 if(!ntw16s_global_name(name,ARRAYSIZE(name))||swprintf(request,ARRAYSIZE(request),L"%ls.request",name)<0)return 4;
 PROCESS_INFORMATION targets[2]={0},manager={0};HWND windows[8]={0};HANDLE mapping=NULL,event=NULL;ntw16s_global_session *session=NULL;BOOL pass=FALSE;DWORD handles[2][2]={0},gdi[2][2]={0};
 if(!launch(argv[2],L"Threads32",&targets[0])||!launch(argv[3],L"Threads64",&targets[1]))goto done;
 for(unsigned p=0;p<2;++p)for(unsigned i=0;i<2;++i){WCHAR title[100];swprintf(title,ARRAYSIZE(title),L"Threads%u-%u",p?64:32,i);windows[p*4+i]=find(title);if(!windows[p*4+i]||!wait_mask(windows[p*4+i],0))goto done;}
 for(unsigned cycle=0;cycle<3;++cycle){
  if(!launch(argv[1],L"--global",&manager))goto done;
  for(unsigned i=0;i<200;++i){mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);if(mapping){session=MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*session));if(session&&session->owner_pid==manager.dwProcessId&&session->phase==NTW16S_GLOBAL_ACTIVE)break;if(session)UnmapViewOfFile(session);session=NULL;CloseHandle(mapping);mapping=NULL;}if(WaitForSingleObject(manager.hProcess,0)==WAIT_OBJECT_0)break;Sleep(30);}
  if(!session||session->phase!=NTW16S_GLOBAL_ACTIVE)goto done;
  for(unsigned p=0;p<2;++p)for(unsigned i=0;i<2;++i)if(!wait_mask(windows[p*4+i],3)){printf("GLOBAL THREADS root_failed cycle=%u p=%u i=%u state=%d\n",cycle,p,i,query(windows[p*4+i],QUERY));goto done;}
  if(!cycle){for(unsigned p=0;p<2;++p)for(unsigned i=0;i<2;++i){WCHAR title[100];if(query(windows[p*4+i],EXTRA)!=1)goto done;swprintf(title,ARRAYSIZE(title),L"Threads%u-%uExtra",p?64:32,i);windows[p*4+2+i]=find(title);if(!windows[p*4+2+i]||!wait_mask(windows[p*4+2+i],3))goto done;}}
  for(unsigned i=0;i<8;++i)if(!wait_mask(windows[i],3)){printf("GLOBAL THREADS new_failed cycle=%u i=%u state=%d\n",cycle,i,query(windows[i],QUERY));goto done;}
  event=OpenEventW(EVENT_MODIFY_STATE,FALSE,request);if(!event||!SetEvent(event)||WaitForSingleObject(manager.hProcess,12000)!=WAIT_OBJECT_0)goto done;
  DWORD exitcode=99;if(!GetExitCodeProcess(manager.hProcess,&exitcode)||exitcode)goto done;
  for(unsigned i=0;i<8;++i)if(!wait_mask(windows[i],0)){printf("GLOBAL THREADS remove_failed cycle=%u i=%u state=%d\n",cycle,i,query(windows[i],QUERY));goto done;}
  if(cycle==0||cycle==2)for(unsigned p=0;p<2;++p)if(!resources(targets[p].hProcess,&handles[cycle?1:0][p],&gdi[cycle?1:0][p]))goto done;
  CloseHandle(event);event=NULL;UnmapViewOfFile(session);session=NULL;CloseHandle(mapping);mapping=NULL;CloseHandle(manager.hThread);CloseHandle(manager.hProcess);ZeroMemory(&manager,sizeof(manager));
 }
 for(unsigned p=0;p<2;++p)if(handles[1][p]>handles[0][p]+2||gdi[1][p]>gdi[0][p]+2){printf("GLOBAL THREADS resource_growth p=%u handles=%lu,%lu gdi=%lu,%lu\n",p,handles[0][p],handles[1][p],gdi[0][p],gdi[1][p]);goto done;}
 printf("GLOBAL THREADS PASS widths=2 threads=4 old=4 new=4 cycles=3 handles=%lu,%lu/%lu,%lu gdi=%lu,%lu/%lu,%lu\n",handles[0][0],handles[1][0],handles[0][1],handles[1][1],gdi[0][0],gdi[1][0],gdi[0][1],gdi[1][1]);pass=TRUE;
 done:if(!pass)printf("GLOBAL THREADS FAIL error=%lu\n",GetLastError());
 if(manager.hProcess&&WaitForSingleObject(manager.hProcess,0)!=WAIT_OBJECT_0){if(!event)event=OpenEventW(EVENT_MODIFY_STATE,FALSE,request);if(event)SetEvent(event);if(WaitForSingleObject(manager.hProcess,5000)!=WAIT_OBJECT_0){TerminateProcess(manager.hProcess,ERROR_TIMEOUT);WaitForSingleObject(manager.hProcess,1000);}}
 if(event)CloseHandle(event);
 if(session)UnmapViewOfFile(session);
 if(mapping)CloseHandle(mapping);
 if(manager.hThread)CloseHandle(manager.hThread);
 if(manager.hProcess)CloseHandle(manager.hProcess);
 for(unsigned i=0;i<8;++i)if(windows[i])PostMessageW(windows[i],WM_CLOSE,0,0);
 for(unsigned i=0;i<2;++i)if(targets[i].hProcess){if(WaitForSingleObject(targets[i].hProcess,3000)!=WAIT_OBJECT_0){TerminateProcess(targets[i].hProcess,ERROR_TIMEOUT);WaitForSingleObject(targets[i].hProcess,1000);}CloseHandle(targets[i].hThread);CloseHandle(targets[i].hProcess);}
 return pass?0:1;
}
