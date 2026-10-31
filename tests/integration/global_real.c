#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdint.h>
#include <wchar.h>
#include "ntw16s-dll/global_session.h"
static DWORD target_pid;
static HWND target_window;
static BOOL CALLBACK find_window(HWND w,LPARAM unused)
{DWORD pid=0;(void)unused;GetWindowThreadProcessId(w,&pid);if(pid==target_pid && IsWindowVisible(w) && GetWindow(w,GW_OWNER)==NULL){target_window=w;return FALSE;}return TRUE;}
static HWND wait_window(PROCESS_INFORMATION *p)
{target_pid=p->dwProcessId;for(unsigned i=0;i<120;++i){target_window=NULL;EnumWindows(find_window,0);if(target_window)return target_window;if(WaitForSingleObject(p->hProcess,0)==WAIT_OBJECT_0)break;Sleep(50);}return NULL;}
static BOOL launch(const WCHAR *image,const WCHAR *argument,PROCESS_INFORMATION *p)
{WCHAR command[32768];STARTUPINFOW startup={.cb=sizeof(startup)};if(swprintf(command,ARRAYSIZE(command),L"\"%ls\" %ls",image,argument?argument:L"")<0)return FALSE;return CreateProcessW(image,command,NULL,NULL,TRUE,0,NULL,NULL,&startup,p);}
static uint64_t screenshot(HWND w,const WCHAR *directory,const WCHAR *label)
{RECT r;if(!GetWindowRect(w,&r))return 0;int x=r.right-r.left,y=r.bottom-r.top;if(x<=0 || y<=0 || x>8192 || y>8192)return 0;
 HDC screen=GetDC(NULL),memory=CreateCompatibleDC(screen);BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=x;info.bmiHeader.biHeight=y;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;BYTE *pixels=NULL;
 HBITMAP bitmap=CreateDIBSection(memory,&info,DIB_RGB_COLORS,(void **)&pixels,NULL,0);if(!screen || !memory || !bitmap || !pixels){if(bitmap)DeleteObject(bitmap);if(memory)DeleteDC(memory);if(screen)ReleaseDC(NULL,screen);return 0;}
 HGDIOBJ old=SelectObject(memory,bitmap);BOOL ok=PrintWindow(w,memory,0);GdiFlush();uint64_t hash=UINT64_C(1469598103934665603);
 if(ok){for(int row=1;row<min(y,40);++row)for(int col=1;col<min(x,300);++col){DWORD color=((DWORD *)pixels)[(y-1-row)*x+col];hash=(hash^color)*UINT64_C(1099511628211);}
  WCHAR path[32768];if(swprintf(path,ARRAYSIZE(path),L"%ls\\%ls.bmp",directory,label)>0){FILE *f=_wfopen(path,L"wb");if(f){DWORD bytes=(DWORD)(x*y*4);BITMAPFILEHEADER hdr={0};hdr.bfType=0x4d42;hdr.bfOffBits=sizeof(hdr)+sizeof(info.bmiHeader);hdr.bfSize=hdr.bfOffBits+bytes;fwrite(&hdr,sizeof(hdr),1,f);fwrite(&info.bmiHeader,sizeof(info.bmiHeader),1,f);fwrite(pixels,bytes,1,f);fclose(f);}}}
 SelectObject(memory,old);DeleteObject(bitmap);DeleteDC(memory);ReleaseDC(NULL,screen);return ok?hash:0;}
static BOOL module(DWORD pid)
{HANDLE snap=CreateToolhelp32Snapshot(TH32CS_SNAPMODULE|TH32CS_SNAPMODULE32,pid);if(snap==INVALID_HANDLE_VALUE)return FALSE;MODULEENTRY32W e={.dwSize=sizeof(e)};BOOL found=FALSE;if(Module32FirstW(snap,&e))do{if(!_wcsicmp(e.szModule,L"ntw16s32.dll")||!_wcsicmp(e.szModule,L"ntw16s64.dll")){printf("GLOBAL_REAL_DLL pid=%lu path=%ls\n",pid,e.szExePath);found=TRUE;break;}}while(Module32NextW(snap,&e));CloseHandle(snap);return found;}
static void close_process(PROCESS_INFORMATION *p,HWND w)
{if(w)PostMessageW(w,WM_CLOSE,0,0);if(p->hProcess){if(WaitForSingleObject(p->hProcess,3000)!=WAIT_OBJECT_0){TerminateProcess(p->hProcess,ERROR_TIMEOUT);WaitForSingleObject(p->hProcess,1000);}CloseHandle(p->hThread);CloseHandle(p->hProcess);}}
int wmain(int argc,WCHAR **argv)
{if(argc!=4)return 2;WCHAR desk[128],name[NTW16S_GLOBAL_NAME_CAP],request[NTW16S_GLOBAL_NAME_CAP+16];DWORD n=0;
 if(!GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desk,sizeof(desk),&n)||wcsncmp(desk,L"Win16S-Dialog-",14))return 3;
 if(!ntw16s_global_name(name,ARRAYSIZE(name))||swprintf(request,ARRAYSIZE(request),L"%ls.request",name)<0)return 4;
 PROCESS_INFORMATION old={0},fresh={0},manager={0};HWND a=NULL,b=NULL;HANDLE map=NULL,event=NULL;ntw16s_global_session *session=NULL;BOOL pass=FALSE;uint64_t before=0,styled=0,after=0;
 if(!launch(argv[2],NULL,&old)||!(a=wait_window(&old)))goto done;
 before=screenshot(a,argv[3],L"before");
 if(!launch(argv[1],L"--global",&manager))goto done;
 for(unsigned i=0;i<200;++i){map=OpenFileMappingW(FILE_MAP_READ,FALSE,name);if(map){session=MapViewOfFile(map,FILE_MAP_READ,0,0,sizeof(*session));if(session&&session->owner_pid==manager.dwProcessId&&session->phase==NTW16S_GLOBAL_ACTIVE)break;if(session)UnmapViewOfFile(session);session=NULL;CloseHandle(map);map=NULL;}if(WaitForSingleObject(manager.hProcess,0)==WAIT_OBJECT_0)break;Sleep(50);}
 if(!session||session->phase!=NTW16S_GLOBAL_ACTIVE)goto done;
 for(unsigned i=0;i<100;++i){styled=screenshot(a,argv[3],L"styled");if(styled&&styled!=before)break;Sleep(30);}
 if(!styled||styled==before||!module(old.dwProcessId))goto done;
 if(!launch(argv[2],NULL,&fresh)||!(b=wait_window(&fresh)))goto done;
 for(unsigned i=0;i<100 && !module(fresh.dwProcessId);++i)Sleep(30);
 if(!module(fresh.dwProcessId)||!screenshot(b,argv[3],L"new"))goto done;
 event=OpenEventW(EVENT_MODIFY_STATE,FALSE,request);if(!event||!SetEvent(event)||WaitForSingleObject(manager.hProcess,12000)!=WAIT_OBJECT_0)goto done;
 DWORD code=99;if(!GetExitCodeProcess(manager.hProcess,&code)||code)goto done;after=screenshot(a,argv[3],L"restored");
 if(!after || WaitForSingleObject(old.hProcess,0)!=WAIT_TIMEOUT || WaitForSingleObject(fresh.hProcess,0)!=WAIT_TIMEOUT)goto done;
 printf("GLOBAL REAL PASS old=%lu new=%lu before=%016llx styled=%016llx restored=%016llx\n",old.dwProcessId,fresh.dwProcessId,(unsigned long long)before,(unsigned long long)styled,(unsigned long long)after);pass=TRUE;
 done:if(!pass)printf("GLOBAL REAL FAIL error=%lu old=%lu new=%lu\n",GetLastError(),old.dwProcessId,fresh.dwProcessId);
 if(manager.hProcess&&WaitForSingleObject(manager.hProcess,0)!=WAIT_OBJECT_0){if(!event)event=OpenEventW(EVENT_MODIFY_STATE,FALSE,request);if(event)SetEvent(event);if(WaitForSingleObject(manager.hProcess,5000)!=WAIT_OBJECT_0){TerminateProcess(manager.hProcess,ERROR_TIMEOUT);WaitForSingleObject(manager.hProcess,1000);}}
 if(event)CloseHandle(event);
 if(session)UnmapViewOfFile(session);
 if(map)CloseHandle(map);
 if(manager.hThread)CloseHandle(manager.hThread);
 if(manager.hProcess)CloseHandle(manager.hProcess);
 close_process(&fresh,b);close_process(&old,a);return pass?0:1;}
