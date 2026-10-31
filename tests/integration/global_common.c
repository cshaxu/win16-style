#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global_session.h"
#define QUERY (WM_APP+81)
#define CONTENT (WM_APP+82)
#define CREATE_LATE (WM_APP+83)
#define LATE_QUERY (WM_APP+84)
#define LATE_CLICKED (WM_APP+85)
#define VB_HASH (WM_APP+86)
#define VB_ATTACHED (WM_APP+87)
static const WCHAR cls[]=L"Win16GlobalCommon";
typedef struct target {PROCESS_INFORMATION process;HWND window;} target;
static BOOL launch(target *t,const WCHAR *image,const WCHAR *title,const WCHAR *fixture)
{WCHAR cmd[8192];STARTUPINFOW s={.cb=sizeof(s)};int n=fixture?swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" %ls \"%ls\"",image,title,fixture):swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" %ls",image,title);if(n<0||!CreateProcessW(image,cmd,NULL,NULL,TRUE,0,NULL,NULL,&s,&t->process))return FALSE;for(unsigned i=0;i<160;++i){t->window=FindWindowW(cls,title);if(t->window&&IsWindowVisible(t->window))return TRUE;if(WaitForSingleObject(t->process.hProcess,0)==WAIT_OBJECT_0)break;Sleep(30);}return FALSE;}
static int query(HWND w,UINT msg)
{DWORD_PTR result=0;return SendMessageTimeoutW(w,msg,0,0,SMTO_ABORTIFHUNG,500,&result)?(int)result:-1;}
static BOOL wait_mask(HWND w,int mask)
{for(unsigned i=0;i<160;++i){if(query(w,QUERY)==mask)return TRUE;Sleep(30);}return FALSE;}
static BOOL wait_vb(HWND w,int attached,int native,BOOL changed)
{for(unsigned i=0;i<160;++i){int a=query(w,VB_ATTACHED),h=query(w,VB_HASH);if(a==attached && h>0 && ((h!=native)==changed))return TRUE;Sleep(30);}return FALSE;}
static void cleanup(target *t)
{if(t->window)PostMessageW(t->window,WM_CLOSE,0,0);if(t->process.hProcess){if(WaitForSingleObject(t->process.hProcess,2000)!=WAIT_OBJECT_0){TerminateProcess(t->process.hProcess,ERROR_TIMEOUT);WaitForSingleObject(t->process.hProcess,1000);}CloseHandle(t->process.hThread);CloseHandle(t->process.hProcess);}}
int wmain(int argc,WCHAR **argv)
{
 if(argc!=4 && argc!=6)return 2;
 BOOL vb=argc==6;int native[2]={0};
 WCHAR desk[128],name[NTW16S_GLOBAL_NAME_CAP],request[NTW16S_GLOBAL_NAME_CAP+16],cmd[8192];DWORD size=0;
 if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desk,sizeof(desk),&size)||wcsncmp(desk,L"Win16S-Dialog-",14))return 3;
 if(!ntw16s_global_name(name,ARRAYSIZE(name))||swprintf(request,ARRAYSIZE(request),L"%ls.request",name)<0)return 4;
 target t[4]={0};PROCESS_INFORMATION manager={0};HANDLE mapping=NULL,event=NULL;ntw16s_global_session *session=NULL;BOOL pass=FALSE;
 if(!launch(&t[0],argv[2],L"Old32",vb?argv[4]:NULL)||!launch(&t[1],argv[3],L"Old64",vb?argv[5]:NULL))goto done;
 if(!wait_mask(t[0].window,0)||!wait_mask(t[1].window,0))goto done;
 if(vb)for(unsigned i=0;i<2;++i){native[i]=query(t[i].window,VB_HASH);if(native[i]<=0 || query(t[i].window,VB_ATTACHED)!=0)goto done;}
 STARTUPINFOW startup={.cb=sizeof(startup)};if(swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" --global",argv[1])<0||!CreateProcessW(argv[1],cmd,NULL,NULL,TRUE,0,NULL,NULL,&startup,&manager))goto done;
 for(unsigned i=0;i<200;++i){mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);if(mapping){session=MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*session));if(session&&session->owner_pid==manager.dwProcessId&&session->phase==NTW16S_GLOBAL_ACTIVE)break;if(session)UnmapViewOfFile(session);session=NULL;CloseHandle(mapping);mapping=NULL;}if(WaitForSingleObject(manager.hProcess,0)==WAIT_OBJECT_0)break;Sleep(50);}
 if(!session||session->phase!=NTW16S_GLOBAL_ACTIVE)goto done;
 for(unsigned i=0;i<2;++i)if(!wait_mask(t[i].window,31)){printf("GLOBAL COMMON existing=%u mask=%d\n",i,query(t[i].window,QUERY));goto done;}
 if(vb)for(unsigned i=0;i<2;++i)if(!wait_vb(t[i].window,1,native[i],TRUE)){printf("GLOBAL VB existing=%u attached=%d hash=%d native=%d\n",i,query(t[i].window,VB_ATTACHED),query(t[i].window,VB_HASH),native[i]);goto done;}
 if(!launch(&t[2],argv[2],L"New32",vb?argv[4]:NULL)||!launch(&t[3],argv[3],L"New64",vb?argv[5]:NULL))goto done;
 for(unsigned i=2;i<4;++i)if(!wait_mask(t[i].window,31)){printf("GLOBAL COMMON new=%u mask=%d\n",i,query(t[i].window,QUERY));goto done;}
 if(vb)for(unsigned i=2;i<4;++i)if(!wait_vb(t[i].window,1,native[i%2],TRUE)){printf("GLOBAL VB new=%u attached=%d hash=%d native=%d\n",i,query(t[i].window,VB_ATTACHED),query(t[i].window,VB_HASH),native[i%2]);goto done;}
 for(unsigned i=0;i<4;++i){if(query(t[i].window,CREATE_LATE)!=1)goto done;int mask=query(t[i].window,LATE_QUERY);if(mask!=3){printf("GLOBAL LATE attach=%u mask=%d\n",i,mask);goto done;}
  HWND a=GetDlgItem(t[i].window,100),b=GetDlgItem(t[i].window,101);if(!a||!b)goto done;SendMessageW(a,BM_CLICK,0,0);SendMessageW(b,BM_CLICK,0,0);if(query(t[i].window,LATE_CLICKED)!=2)goto done;}
 event=OpenEventW(EVENT_MODIFY_STATE,FALSE,request);if(!event||!SetEvent(event)||WaitForSingleObject(manager.hProcess,12000)!=WAIT_OBJECT_0)goto done;
 DWORD code=99;if(!GetExitCodeProcess(manager.hProcess,&code)||code)goto done;
 for(unsigned i=0;i<4;++i)if(!wait_mask(t[i].window,0)||query(t[i].window,CONTENT)!=1||query(t[i].window,LATE_QUERY)!=0){printf("GLOBAL COMMON restore=%u mask=%d late=%d content=%d\n",i,query(t[i].window,QUERY),query(t[i].window,LATE_QUERY),query(t[i].window,CONTENT));goto done;}
 if(vb)for(unsigned i=0;i<4;++i)if(!wait_vb(t[i].window,0,native[i%2],FALSE)){printf("GLOBAL VB restore=%u attached=%d hash=%d native=%d\n",i,query(t[i].window,VB_ATTACHED),query(t[i].window,VB_HASH),native[i%2]);goto done;}
 if(vb)printf("GLOBAL VB E2E PASS widths=2 existing=2 new=2 restored=4 pixels=4\n");
 printf("GLOBAL COMMON PASS widths=2 existing=2 new=2 controls=5 restored=4 content=4\nGLOBAL LATE PASS widths=2 targets=4 modes=2 clicked=8 restored=8\n");pass=TRUE;
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
