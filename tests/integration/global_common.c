#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global_session.h"
#define QUERY (WM_APP+81)
#define CONTENT (WM_APP+82)
static const WCHAR cls[]=L"Win16GlobalCommon";
typedef struct target {PROCESS_INFORMATION process;HWND window;} target;
static BOOL launch(target *t,const WCHAR *image,const WCHAR *title)
{WCHAR cmd[8192];STARTUPINFOW s={.cb=sizeof(s)};if(swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" %ls",image,title)<0||!CreateProcessW(image,cmd,NULL,NULL,TRUE,0,NULL,NULL,&s,&t->process))return FALSE;for(unsigned i=0;i<160;++i){t->window=FindWindowW(cls,title);if(t->window&&IsWindowVisible(t->window))return TRUE;if(WaitForSingleObject(t->process.hProcess,0)==WAIT_OBJECT_0)break;Sleep(30);}return FALSE;}
static int query(HWND w,UINT msg)
{DWORD_PTR result=0;return SendMessageTimeoutW(w,msg,0,0,SMTO_ABORTIFHUNG,500,&result)?(int)result:-1;}
static BOOL wait_mask(HWND w,int mask)
{for(unsigned i=0;i<160;++i){if(query(w,QUERY)==mask)return TRUE;Sleep(30);}return FALSE;}
static void cleanup(target *t)
{if(t->window)PostMessageW(t->window,WM_CLOSE,0,0);if(t->process.hProcess){if(WaitForSingleObject(t->process.hProcess,2000)!=WAIT_OBJECT_0){TerminateProcess(t->process.hProcess,ERROR_TIMEOUT);WaitForSingleObject(t->process.hProcess,1000);}CloseHandle(t->process.hThread);CloseHandle(t->process.hProcess);}}
int wmain(int argc,WCHAR **argv)
{
 if(argc!=4)return 2;
 WCHAR desk[128],name[NTW16S_GLOBAL_NAME_CAP],request[NTW16S_GLOBAL_NAME_CAP+16],cmd[8192];DWORD size=0;
 if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desk,sizeof(desk),&size)||wcsncmp(desk,L"Win16S-Dialog-",14))return 3;
 if(!ntw16s_global_name(name,ARRAYSIZE(name))||swprintf(request,ARRAYSIZE(request),L"%ls.request",name)<0)return 4;
 target t[4]={0};PROCESS_INFORMATION manager={0};HANDLE mapping=NULL,event=NULL;ntw16s_global_session *session=NULL;BOOL pass=FALSE;
 if(!launch(&t[0],argv[2],L"Old32")||!launch(&t[1],argv[3],L"Old64"))goto done;
 if(!wait_mask(t[0].window,0)||!wait_mask(t[1].window,0))goto done;
 STARTUPINFOW startup={.cb=sizeof(startup)};if(swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" --global",argv[1])<0||!CreateProcessW(argv[1],cmd,NULL,NULL,TRUE,0,NULL,NULL,&startup,&manager))goto done;
 for(unsigned i=0;i<200;++i){mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);if(mapping){session=MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*session));if(session&&session->owner_pid==manager.dwProcessId&&session->phase==NTW16S_GLOBAL_ACTIVE)break;if(session)UnmapViewOfFile(session);session=NULL;CloseHandle(mapping);mapping=NULL;}if(WaitForSingleObject(manager.hProcess,0)==WAIT_OBJECT_0)break;Sleep(50);}
 if(!session||session->phase!=NTW16S_GLOBAL_ACTIVE)goto done;
 for(unsigned i=0;i<2;++i)if(!wait_mask(t[i].window,31)){printf("GLOBAL COMMON existing=%u mask=%d\n",i,query(t[i].window,QUERY));goto done;}
 if(!launch(&t[2],argv[2],L"New32")||!launch(&t[3],argv[3],L"New64"))goto done;
 for(unsigned i=2;i<4;++i)if(!wait_mask(t[i].window,31)){printf("GLOBAL COMMON new=%u mask=%d\n",i,query(t[i].window,QUERY));goto done;}
 event=OpenEventW(EVENT_MODIFY_STATE,FALSE,request);if(!event||!SetEvent(event)||WaitForSingleObject(manager.hProcess,12000)!=WAIT_OBJECT_0)goto done;
 DWORD code=99;if(!GetExitCodeProcess(manager.hProcess,&code)||code)goto done;
 for(unsigned i=0;i<4;++i)if(!wait_mask(t[i].window,0)||query(t[i].window,CONTENT)!=1){printf("GLOBAL COMMON restore=%u mask=%d content=%d\n",i,query(t[i].window,QUERY),query(t[i].window,CONTENT));goto done;}
 printf("GLOBAL COMMON PASS widths=2 existing=2 new=2 controls=5 restored=4 content=4\n");pass=TRUE;
 done:if(!pass)printf("GLOBAL COMMON FAIL error=%lu\n",GetLastError());
 if(manager.hProcess&&WaitForSingleObject(manager.hProcess,0)!=WAIT_OBJECT_0){if(!event)event=OpenEventW(EVENT_MODIFY_STATE,FALSE,request);if(event)SetEvent(event);if(WaitForSingleObject(manager.hProcess,5000)!=WAIT_OBJECT_0){TerminateProcess(manager.hProcess,ERROR_TIMEOUT);WaitForSingleObject(manager.hProcess,1000);}}
 if(event)CloseHandle(event);
 if(session)UnmapViewOfFile(session);
 if(mapping)CloseHandle(mapping);
 if(manager.hThread)CloseHandle(manager.hThread);
 if(manager.hProcess)CloseHandle(manager.hProcess);
 for(unsigned i=0;i<4;++i)cleanup(&t[i]);
 return pass?0:1;
}
