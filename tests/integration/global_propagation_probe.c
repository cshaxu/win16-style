#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
typedef BOOL (__cdecl *attached_fn)(HWND);
static int result=1;
static ULONGLONG deadline;
static BOOL create_owned(const WCHAR *argument,DWORD *code)
{
    WCHAR image[32768],command[32780];STARTUPINFOW si={.cb=sizeof(si)};PROCESS_INFORMATION child={0};
    DWORD length=GetModuleFileNameW(NULL,image,ARRAYSIZE(image));
    if(!length || length>=ARRAYSIZE(image) || swprintf(command,ARRAYSIZE(command),L"\"%ls\" %ls",image,argument)<0)return FALSE;
    if(!CreateProcessW(image,command,NULL,NULL,TRUE,0,NULL,NULL,&si,&child))return FALSE;
    BOOL done=WaitForSingleObject(child.hProcess,5000)==WAIT_OBJECT_0;
    if(done)done=GetExitCodeProcess(child.hProcess,code);
    else {TerminateProcess(child.hProcess,ERROR_TIMEOUT);WaitForSingleObject(child.hProcess,1000);}
    CloseHandle(child.hThread);CloseHandle(child.hProcess);return done;
}
static HMODULE module(void)
{return GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll");}
static LRESULT CALLBACK proc(HWND w,UINT message,WPARAM a,LPARAM b)
{
    if(message==WM_TIMER){
        HMODULE dll=module();attached_fn attached=dll?(attached_fn)(void*)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;
        if(attached && attached(w)){
            DWORD child=99;KillTimer(w,1);BOOL ok=create_owned(L"child",&child);
            printf("GLOBAL PROPAGATION bits=%u root_attached=1 create=%d child_exit=%lu\n",(unsigned)(sizeof(void*)*8),ok,child);
            result=ok&&child==0?0:1;DestroyWindow(w);return 0;
        }
        if(GetTickCount64()>deadline){printf("GLOBAL PROPAGATION root_timeout\n");DestroyWindow(w);}return 0;
    }
    if(message==WM_DESTROY){PostQuitMessage(0);return 0;}
    return DefWindowProcW(w,message,a,b);
}
int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && (!wcscmp(argv[1],L"child") || !wcscmp(argv[1],L"grandchild"))){
        BOOL loaded=module()!=NULL;printf("PROPAGATION ENTRY bits=%u role=%ls dll=%d\n",(unsigned)(sizeof(void*)*8),argv[1],loaded);
        if(!loaded)return 41;
        if(!wcscmp(argv[1],L"child")){DWORD code=99;BOOL ok=create_owned(L"grandchild",&code);printf("PROPAGATION CHILD create=%d exit=%lu\n",ok,code);return ok&&code==0?0:42;}
        return 0;
    }
    WNDCLASSW c={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=proc,.lpszClassName=L"GlobalPropagationProbe"};
    if(!RegisterClassW(&c))return 2;
    HWND w=CreateWindowW(c.lpszClassName,L"Owned propagation root",WS_OVERLAPPEDWINDOW|WS_VISIBLE,50,50,300,180,NULL,NULL,c.hInstance,NULL);
    if(!w)return 3;
    deadline=GetTickCount64()+5000;if(!SetTimer(w,1,50,NULL)){DestroyWindow(w);return 4;}
    MSG m;while(GetMessageW(&m,NULL,0,0)>0){TranslateMessage(&m);DispatchMessageW(&m);}return result;
}
