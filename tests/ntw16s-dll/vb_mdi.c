#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "isolated_process.h"
#define CHECK(x) do{if(!(x)){fprintf(stderr,"VB_MDI_FAIL line=%d error=%lu %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static HWND client;
static LRESULT CALLBACK frame_proc(HWND w,UINT m,WPARAM a,LPARAM b){return DefFrameProcW(w,client,m,a,b);}
static int child(const WCHAR *runtime,BOOL styled){
 HMODULE dll=LoadLibraryW(runtime);CHECK(dll);
 BOOL (WINAPI *reg)(void)=(void*)GetProcAddress(dll,"VbMdiRegister"), (WINAPI *release)(void)=(void*)GetProcAddress(dll,"VbMdiRelease");CHECK(reg && release && reg());
 BOOL vb5=GetModuleHandleW(L"MSVBVM50.DLL")==dll;const WCHAR *name=vb5?L"ThunderRT5Form":L"ThunderRT6FormDC";
 HMODULE style=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");
 BOOL (__cdecl *attached)(HWND)=style?(void*)GetProcAddress(style,"Ntw16sWindowAttached"):NULL;CHECK(styled?attached!=NULL:style==NULL);
 HINSTANCE inst=GetModuleHandleW(NULL);WNDCLASSW cls={.hInstance=inst,.lpfnWndProc=frame_proc,.lpszClassName=L"VBMDIModelFrame"};CHECK(RegisterClassW(&cls));
 HWND frame=CreateWindowW(cls.lpszClassName,L"MDI model",WS_OVERLAPPEDWINDOW,0,0,500,350,NULL,NULL,inst,NULL);CHECK(frame);
 CLIENTCREATESTRUCT info={.idFirstChild=100};client=CreateWindowW(L"MDICLIENT",NULL,WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN,0,0,480,300,frame,(HMENU)1,inst,&info);CHECK(client);
 HWND w=CreateMDIWindowW(name,L"Document",WS_VISIBLE|WS_OVERLAPPEDWINDOW,10,10,240,180,client,dll,0);CHECK(w && (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_MDICHILD));
 ShowWindow(frame,SW_SHOWNOACTIVATE);UpdateWindow(frame);CHECK(!styled || attached(w));
 CHECK((HWND)SendMessageW(client,WM_MDIGETACTIVE,0,0)==w);SendMessageW(client,WM_MDIMAXIMIZE,(WPARAM)w,0);CHECK(IsZoomed(w));SendMessageW(client,WM_MDIRESTORE,(WPARAM)w,0);CHECK(!IsZoomed(w));CHECK(!styled || attached(w));
 HWND wrong=CreateWindowW(name,L"wrong parent",WS_CHILD|WS_CAPTION,0,0,100,80,frame,(HMENU)2,dll,NULL);CHECK(wrong);CHECK(!styled || !attached(wrong));CHECK(DestroyWindow(wrong));
 SendMessageW(client,WM_MDIDESTROY,(WPARAM)w,0);CHECK(!IsWindow(w));CHECK(DestroyWindow(frame));client=NULL;CHECK(UnregisterClassW(cls.lpszClassName,inst) && release() && FreeLibrary(dll));
 printf("VB_MDI_OK bits=%u vb=%u styled=%u attached=%u wrong_parent_native=1 maximize_restore=1 destroyed=1\n",(unsigned)(8*sizeof(void*)),vb5?5:6,styled,styled);return 0;
}
static int worker(int argc,WCHAR **argv){CHECK(argc==3);WCHAR self[MAX_PATH],cmd[2048];CHECK(GetModuleFileNameW(NULL,self,MAX_PATH));for(unsigned mode=0;mode<2;mode++){
 if(mode)swprintf_s(cmd,2048,L"\"%ls\" --wait \"%ls\" --child \"%ls\" 1",argv[1],self,argv[2]);else swprintf_s(cmd,2048,L"\"%ls\" --child \"%ls\" 0",self,argv[2]);
 HANDLE job=CreateJobObjectW(NULL,NULL);JOBOBJECT_EXTENDED_LIMIT_INFORMATION lim={0};lim.BasicLimitInformation.LimitFlags=JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;CHECK(job && SetInformationJobObject(job,JobObjectExtendedLimitInformation,&lim,sizeof(lim)));
 STARTUPINFOW si={.cb=sizeof(si),.dwFlags=STARTF_USESTDHANDLES,.hStdInput=GetStdHandle(STD_INPUT_HANDLE),.hStdOutput=GetStdHandle(STD_OUTPUT_HANDLE),.hStdError=GetStdHandle(STD_ERROR_HANDLE)};PROCESS_INFORMATION p={0};CHECK(CreateProcessW(NULL,cmd,NULL,NULL,TRUE,CREATE_SUSPENDED|CREATE_NO_WINDOW,NULL,NULL,&si,&p));CHECK(AssignProcessToJobObject(job,p.hProcess) && ResumeThread(p.hThread)==1);DWORD wait=WaitForSingleObject(p.hProcess,10000),code=1;if(wait==WAIT_OBJECT_0)GetExitCodeProcess(p.hProcess,&code);CloseHandle(p.hThread);CloseHandle(p.hProcess);CloseHandle(job);CHECK(wait==WAIT_OBJECT_0 && code==0);
 }return 0;}
int wmain(int argc,WCHAR **argv){if(argc==4 && !wcscmp(argv[1],L"--child"))return child(argv[2],!wcscmp(argv[3],L"1"));return isolated_process(argc,argv,worker);}
