#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#define QUERY (WM_APP+85)
#define EXTRA (WM_APP+86)
static _Thread_local HWND root,button;
static const WCHAR cls[]=L"Win16GlobalThreads";
typedef BOOL (__cdecl *attached_fn)(HWND);
typedef struct worker {HANDLE ready;WCHAR title[96];} worker;
static LRESULT CALLBACK proc(HWND w,UINT m,WPARAM a,LPARAM b)
{
 if(m==QUERY){HMODULE lib=GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");attached_fn fn=lib?(attached_fn)(void *)GetProcAddress(lib,"Ntw16sWindowAttached"):NULL;HWND child=GetDlgItem(w,1);return fn?((fn(w)?1:0)|(child&&fn(child)?2:0)):0;}
 if(m==EXTRA){WCHAR title[128];GetWindowTextW(w,title,ARRAYSIZE(title));size_t n=wcslen(title);if(n+6>=ARRAYSIZE(title))return 0;wcscat(title,L"Extra");HWND extra=CreateWindowW(cls,title,WS_OVERLAPPEDWINDOW,100,100,250,160,NULL,NULL,GetModuleHandleW(NULL),NULL);if(!extra)return 0;if(!CreateWindowW(L"BUTTON",L"Action",WS_CHILD|WS_VISIBLE,10,10,90,25,extra,(HMENU)1,GetModuleHandleW(NULL),NULL)){DestroyWindow(extra);return 0;}ShowWindow(extra,SW_SHOW);return 1;}
 if(m==WM_CLOSE){DestroyWindow(w);return 0;}
 if(m==WM_DESTROY&&w==root){PostQuitMessage(0);return 0;}
 return DefWindowProcW(w,m,a,b);
}
static DWORD WINAPI run_thread(void *value)
{worker *work=(worker *)value;root=CreateWindowW(cls,work->title,WS_OVERLAPPEDWINDOW,50,50,300,200,NULL,NULL,GetModuleHandleW(NULL),NULL);if(!root){SetEvent(work->ready);return 2;}button=CreateWindowW(L"BUTTON",L"Action",WS_CHILD|WS_VISIBLE,10,10,90,25,root,(HMENU)1,GetModuleHandleW(NULL),NULL);if(!button){DestroyWindow(root);SetEvent(work->ready);return 3;}ShowWindow(root,SW_SHOW);SetEvent(work->ready);MSG m;while(GetMessageW(&m,NULL,0,0)>0){TranslateMessage(&m);DispatchMessageW(&m);}return 0;}
int wmain(int argc,WCHAR **argv)
{
 if(argc!=2)return 2;
 WNDCLASSW c={0};c.hInstance=GetModuleHandleW(NULL);c.lpfnWndProc=proc;c.lpszClassName=cls;if(!RegisterClassW(&c))return 3;
 worker work[2]={0};HANDLE threads[2]={0};int result=0;
 for(unsigned i=0;i<2;++i){work[i].ready=CreateEventW(NULL,TRUE,FALSE,NULL);if(!work[i].ready||swprintf(work[i].title,ARRAYSIZE(work[i].title),L"%ls-%u",argv[1],i)<0){result=4;break;}threads[i]=CreateThread(NULL,0,run_thread,&work[i],0,NULL);if(!threads[i]||WaitForSingleObject(work[i].ready,5000)!=WAIT_OBJECT_0){result=5;break;}}
 if(!result){for(unsigned i=0;i<2;++i)if(WaitForSingleObject(threads[i],INFINITE)!=WAIT_OBJECT_0)result=6;}
 for(unsigned i=0;i<2;++i){if(threads[i])CloseHandle(threads[i]);if(work[i].ready)CloseHandle(work[i].ready);}return result;
}
