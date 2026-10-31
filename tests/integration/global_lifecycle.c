#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <wchar.h>
#include "ntw16s-dll/global_session.h"
#define QUERY_STYLE (WM_APP+71)
static const WCHAR cls[]=L"Win16GlobalE2ETarget";
static BOOL launch(const WCHAR *image,const WCHAR *argument,PROCESS_INFORMATION *p)
{WCHAR cmd[8192];STARTUPINFOW s={.cb=sizeof(s)};if(swprintf(cmd,ARRAYSIZE(cmd),L"\"%ls\" %ls",image,argument)<0)return FALSE;return CreateProcessW(image,cmd,NULL,NULL,TRUE,0,NULL,NULL,&s,p);}
static HWND find(const WCHAR *title,PROCESS_INFORMATION *p)
{for(unsigned i=0;i<150;++i){HWND w=FindWindowW(cls,title);if(w&&IsWindowVisible(w))return w;if(WaitForSingleObject(p->hProcess,0)==WAIT_OBJECT_0)break;Sleep(30);}return NULL;}
static int state(HWND w)
{DWORD_PTR result=0;return SendMessageTimeoutW(w,QUERY_STYLE,0,0,SMTO_ABORTIFHUNG,500,&result)?(int)result:-1;}
static BOOL wait_state(HWND w,int expected)
{for(unsigned i=0;i<160;++i){if(state(w)==expected)return TRUE;Sleep(30);}return FALSE;}
static BOOL has_style_module(DWORD pid)
{HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);MODULEENTRY32W entry={.dwSize=sizeof(entry)};BOOL found=FALSE;if(snapshot==INVALID_HANDLE_VALUE)return FALSE;if(Module32FirstW(snapshot,&entry))do{if(!lstrcmpiW(entry.szModule,L"ntw16s32.dll")||!lstrcmpiW(entry.szModule,L"ntw16s64.dll")){printf("GLOBAL LIFECYCLE module pid=%lu path=%ls\n",pid,entry.szExePath);found=TRUE;break;}}while(Module32NextW(snapshot,&entry));CloseHandle(snapshot);return found;}
static void cleanup(PROCESS_INFORMATION *p,HWND w)
{if(w)PostMessageW(w,WM_CLOSE,0,0);if(p->hProcess){if(WaitForSingleObject(p->hProcess,2000)!=WAIT_OBJECT_0){TerminateProcess(p->hProcess,ERROR_TIMEOUT);WaitForSingleObject(p->hProcess,1000);}CloseHandle(p->hThread);CloseHandle(p->hProcess);}}
int wmain(int argc,WCHAR **argv)
{if(argc!=3 && argc!=4)return 2;WCHAR desktop[128],name[NTW16S_GLOBAL_NAME_CAP],request[NTW16S_GLOBAL_NAME_CAP+16];DWORD bytes=0;
 if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&bytes)||wcsncmp(desktop,L"Win16S-Dialog-",14))return 3;
 if(!ntw16s_global_name(name,ARRAYSIZE(name))||swprintf(request,ARRAYSIZE(request),L"%ls.request",name)<0)return 4;
 PROCESS_INFORMATION first={0},second={0},manager={0},duplicate={0},restarted={0};HWND w1=NULL,w2=NULL;HANDLE mapping=NULL,host=NULL,event=NULL;ntw16s_global_session *session=NULL;BOOL pass=FALSE;DWORD duplicate_code=99;
 if(!launch(argv[2],L"LifeBefore",&first)||!(w1=find(L"LifeBefore",&first)))goto done;
 if(!launch(argv[1],L"--global",&manager))goto done;
 for(unsigned i=0;i<200;++i){mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);if(mapping){session=MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*session));if(session&&session->owner_pid==manager.dwProcessId&&session->phase==NTW16S_GLOBAL_ACTIVE)break;if(session)UnmapViewOfFile(session);session=NULL;CloseHandle(mapping);mapping=NULL;}if(WaitForSingleObject(manager.hProcess,0)==WAIT_OBJECT_0)break;Sleep(50);}
 if(!session||session->phase!=NTW16S_GLOBAL_ACTIVE)goto done;
 for(unsigned i=0;i<100&&state(w1)!=1;++i)Sleep(30);
 if(state(w1)!=1){printf("GLOBAL LIFECYCLE initial_attach pid=%lu state=%d module=%d\n",first.dwProcessId,state(w1),has_style_module(first.dwProcessId));goto done;}
 host=OpenProcess(SYNCHRONIZE|(argc==4?PROCESS_TERMINATE:0),FALSE,session->host_pid);if(!host)goto done;
 if(argc==4){DWORD code=99;
  if(wcscmp(argv[3],L"host-crash")||!TerminateProcess(host,78)||WaitForSingleObject(host,5000)!=WAIT_OBJECT_0)goto done;
  if(WaitForSingleObject(manager.hProcess,3000)!=WAIT_OBJECT_0 || !GetExitCodeProcess(manager.hProcess,&code) || code!=10){printf("GLOBAL HOST FAILURE manager_wait_or_exit=%lu\n",code);goto done;}
  printf("GLOBAL HOST FAILURE PASS host=78 controller_exit=10 existing_alive=%d\n",WaitForSingleObject(first.hProcess,0)==WAIT_TIMEOUT);pass=TRUE;goto done;
 }
 if(!launch(argv[1],L"--global",&duplicate)||WaitForSingleObject(duplicate.hProcess,5000)!=WAIT_OBJECT_0||!GetExitCodeProcess(duplicate.hProcess,&duplicate_code)||duplicate_code!=10)goto done;
 if(!TerminateProcess(manager.hProcess,77)||WaitForSingleObject(manager.hProcess,5000)!=WAIT_OBJECT_0||WaitForSingleObject(host,5000)!=WAIT_OBJECT_0)goto done;
 if(!launch(argv[2],L"LifeAfterCrash",&second)||!(w2=find(L"LifeAfterCrash",&second)))goto done;
 if(state(w2)!=0||WaitForSingleObject(first.hProcess,0)!=WAIT_TIMEOUT)goto done;
 UnmapViewOfFile(session);session=NULL;CloseHandle(mapping);mapping=NULL;
 if(!launch(argv[1],L"--global",&restarted))goto done;
 for(unsigned i=0;i<200;++i){mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);if(mapping){session=MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*session));if(session&&session->owner_pid==restarted.dwProcessId&&session->phase==NTW16S_GLOBAL_ACTIVE)break;if(session)UnmapViewOfFile(session);session=NULL;CloseHandle(mapping);mapping=NULL;}if(WaitForSingleObject(restarted.hProcess,0)==WAIT_OBJECT_0)break;Sleep(30);}
 if(!session||session->owner_pid!=restarted.dwProcessId||session->phase!=NTW16S_GLOBAL_ACTIVE){printf("GLOBAL LIFECYCLE restart_not_active error=%lu\n",GetLastError());goto done;}
 if(!wait_state(w1,1)||!wait_state(w2,1)){printf("GLOBAL LIFECYCLE restart_attach old=%d new=%d module=%d\n",state(w1),state(w2),has_style_module(second.dwProcessId));goto done;}
 event=OpenEventW(EVENT_MODIFY_STATE,FALSE,request);
 if(!event||!SetEvent(event)||WaitForSingleObject(restarted.hProcess,10000)!=WAIT_OBJECT_0)goto done;
 DWORD restart_code=99;
 if(!GetExitCodeProcess(restarted.hProcess,&restart_code)||restart_code||!wait_state(w1,0)||!wait_state(w2,0)){
  printf("GLOBAL LIFECYCLE restart_cleanup exit=%lu old=%d new=%d\n",restart_code,state(w1),state(w2));goto done;
 }
 printf("GLOBAL LIFECYCLE PASS duplicate=10 crash=77 host_exit=1 post_crash_native=1 existing_alive=1 restart=1 restored=2\n");pass=TRUE;
 done:if(!pass)printf("GLOBAL LIFECYCLE FAIL duplicate=%lu error=%lu\n",duplicate_code,GetLastError());
 if(manager.hProcess&&WaitForSingleObject(manager.hProcess,0)!=WAIT_OBJECT_0){HANDLE event=OpenEventW(EVENT_MODIFY_STATE,FALSE,request);if(event){SetEvent(event);CloseHandle(event);}if(WaitForSingleObject(manager.hProcess,5000)!=WAIT_OBJECT_0){TerminateProcess(manager.hProcess,ERROR_TIMEOUT);WaitForSingleObject(manager.hProcess,1000);}}
 if(restarted.hProcess&&WaitForSingleObject(restarted.hProcess,0)!=WAIT_OBJECT_0){if(!event)event=OpenEventW(EVENT_MODIFY_STATE,FALSE,request);if(event)SetEvent(event);if(WaitForSingleObject(restarted.hProcess,5000)!=WAIT_OBJECT_0){TerminateProcess(restarted.hProcess,ERROR_TIMEOUT);WaitForSingleObject(restarted.hProcess,1000);}}
 if(event)CloseHandle(event);
 if(host)CloseHandle(host);
 if(session)UnmapViewOfFile(session);
 if(mapping)CloseHandle(mapping);
 cleanup(&duplicate,NULL);cleanup(&restarted,NULL);cleanup(&manager,NULL);cleanup(&second,w2);cleanup(&first,w1);return pass?0:1;}
