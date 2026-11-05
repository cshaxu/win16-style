#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <tlhelp32.h>
#include "ntw16s-dll/global_session.h"
typedef BOOL (__cdecl *attached_fn)(HWND);
static int result=1;
static BOOL managed;
static HWND root_window;
static ULONGLONG deadline;
static HMODULE module(void);
/* Optional owned scratch trace survives invalid or non-inherited stdout. */
static void headless_trace(const char *stage)
{
    WCHAR path[32768];DWORD n=GetEnvironmentVariableW(L"WIN16S_TEST_HEADLESS_TRACE",path,ARRAYSIZE(path));
    if(!n || n>=ARRAYSIZE(path))return;
    HANDLE file=CreateFileW(path,FILE_APPEND_DATA,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
    if(file==INVALID_HANDLE_VALUE)return;
    char line[128];int length=snprintf(line,sizeof(line),"pid=%lu bits=%u stage=%s\r\n",GetCurrentProcessId(),(unsigned)(sizeof(void*)*8),stage);
    if(length>0 && (size_t)length<sizeof(line)){DWORD written;(void)WriteFile(file,line,(DWORD)length,&written,NULL);}
    CloseHandle(file);
}
/* Event-discovered targets may map the DLL before the late hook transaction
 * has completed. Observe the target worker's actual ready event, not merely
 * the module list, before testing post-initialization child propagation. */
static BOOL wait_global_ready(DWORD timeout)
{
    WCHAR name[NTW16S_GLOBAL_NAME_CAP],ready[NTW16S_GLOBAL_NAME_CAP+64];
    if(!ntw16s_global_name(name,ARRAYSIZE(name)))return FALSE;
    ULONGLONG end=GetTickCount64()+timeout;
    while(GetTickCount64()<end){
        HANDLE mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
        ntw16s_global_session *session=mapping?(ntw16s_global_session *)MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*session)):NULL;
        DWORD cookie=session?session->cookie:0;
        if(session)UnmapViewOfFile(session);
        if(mapping)CloseHandle(mapping);
        HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);THREADENTRY32 entry={.dwSize=sizeof(entry)};
        BOOL found=FALSE;
        if(cookie && snapshot!=INVALID_HANDLE_VALUE && Thread32First(snapshot,&entry))do{
            if(entry.th32OwnerProcessID!=GetCurrentProcessId())continue;
            if(swprintf(ready,ARRAYSIZE(ready),L"%ls.thread.%lu.%lu.%lu",name,GetCurrentProcessId(),entry.th32ThreadID,cookie)<0)continue;
            HANDLE event=OpenEventW(SYNCHRONIZE,FALSE,ready);
            if(event){found=WaitForSingleObject(event,0)==WAIT_OBJECT_0;CloseHandle(event);if(found)break;}
        }while(Thread32Next(snapshot,&entry));
        if(snapshot!=INVALID_HANDLE_VALUE)CloseHandle(snapshot);
        if(found)return TRUE;
        Sleep(20);
    }
    WCHAR desktop[256]=L"",session_name[NTW16S_GLOBAL_NAME_CAP]=L"";DWORD needed=0,open_error=0;
    (void)GetUserObjectInformationW(GetThreadDesktop(GetCurrentThreadId()),UOI_NAME,desktop,sizeof(desktop),&needed);
    if(ntw16s_global_name(session_name,ARRAYSIZE(session_name))){HANDLE mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,session_name);
        if(!mapping)open_error=GetLastError();else CloseHandle(mapping);}
    printf("HEADLESS READY DIAG bits=%u desktop=%ls mapping_error=%lu module=%d\n",(unsigned)(sizeof(void*)*8),desktop,open_error,module()!=NULL);
    return FALSE;
}
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
    if(message==WM_APP+110){HMODULE dll=module();attached_fn attached=dll?(attached_fn)(void*)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;return attached&&attached(w);}
    if(message==WM_APP+111){DWORD code=99;return create_owned(L"grandchild",&code)?(LRESULT)code:-1;}
    if(message==WM_APP+112){
        HWND created=CreateWindowW(L"GlobalPropagationProbe",L"Owned late window",WS_OVERLAPPEDWINDOW,70,70,200,120,NULL,NULL,GetModuleHandleW(NULL),NULL);
        if(!created)return -1;
        ShowWindow(created,SW_SHOW);
        HMODULE dll=module();attached_fn attached=dll?(attached_fn)(void*)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;
        int styled=attached&&attached(created);DestroyWindow(created);return styled;
    }
    if(message==WM_CLOSE){DestroyWindow(w);return 0;}
    if(message==WM_TIMER){
        HMODULE dll=module();attached_fn attached=dll?(attached_fn)(void*)GetProcAddress(dll,"Ntw16sWindowAttached"):NULL;
        if(attached && attached(w)){
            DWORD child=99;KillTimer(w,1);BOOL ok=create_owned(L"child",&child);
            printf("GLOBAL PROPAGATION bits=%u root_attached=1 create=%d child_exit=%lu\n",(unsigned)(sizeof(void*)*8),ok,child);
            result=ok&&child==0?0:1;DestroyWindow(w);return 0;
        }
        if(GetTickCount64()>deadline){printf("GLOBAL PROPAGATION root_timeout\n");DestroyWindow(w);}return 0;
    }
    if(message==WM_DESTROY && w==root_window){PostQuitMessage(0);return 0;}
    return DefWindowProcW(w,message,a,b);
}
int wmain(int argc,WCHAR **argv)
{
    HMODULE local_module=NULL;HHOOK local_hook=NULL;
    if(argc==3 && !wcscmp(argv[1],L"localhook")){
        local_module=LoadLibraryW(argv[2]);if(!local_module)return 45;
        HOOKPROC callback=(HOOKPROC)(void *)GetProcAddress(local_module,"Ntw16sGlobalHook");
        if(!callback || !(local_hook=SetWindowsHookExW(WH_CALLWNDPROC,callback,NULL,GetCurrentThreadId())))return 46;
    }
    if(argc==4 && !wcscmp(argv[1],L"wait")){
        if(!module())return 41;
        HANDLE ready=OpenEventW(EVENT_MODIFY_STATE,FALSE,argv[2]),go=OpenEventW(SYNCHRONIZE,FALSE,argv[3]);
        if(!ready || !go)return 43;
        BOOL signaled=SetEvent(ready) && WaitForSingleObject(go,5000)==WAIT_OBJECT_0;DWORD code=99;
        BOOL made=signaled && create_owned(L"grandchild",&code);
        printf("PROPAGATION DESCENDANT stopped=%d create=%d exit=%lu\n",signaled,made,code);
        CloseHandle(ready);CloseHandle(go);return made&&code==41?0:44;
    }
    if(argc==2 && !wcscmp(argv[1],L"headless-child")){
        headless_trace("child-main");
        BOOL loaded=module()!=NULL;
        printf("HEADLESS CHILD bits=%u dll=%d\n",(unsigned)(sizeof(void*)*8),loaded);
        return loaded?0:41;
    }
    if(argc==2 && (!wcscmp(argv[1],L"headless") || !wcscmp(argv[1],L"headless-event"))){
        headless_trace("main");
        printf("HEADLESS START bits=%u pid=%lu\n",(unsigned)(sizeof(void*)*8),GetCurrentProcessId());fflush(stdout);
        if(!wcscmp(argv[1],L"headless-event")){
            BOOL ready=wait_global_ready(4000);
            printf("HEADLESS READY bits=%u ready=%d\n",(unsigned)(sizeof(void*)*8),ready);fflush(stdout);
            if(!ready){headless_trace("ready-timeout");return 43;}
            headless_trace("ready");
        }
        ULONGLONG until=GetTickCount64()+1000;
        while(!module() && GetTickCount64()<until)Sleep(20);
        BOOL loaded=module()!=NULL;DWORD code=99;
        headless_trace(loaded?"before-child-loaded":"before-child-unloaded");
        printf("HEADLESS BEFORE CHILD bits=%u dll=%d\n",(unsigned)(sizeof(void*)*8),loaded);fflush(stdout);
        BOOL made=create_owned(L"headless-child",&code);
        headless_trace(made?"after-child-created":"after-child-failed");
        printf("HEADLESS ROOT bits=%u dll=%d child=%d child_exit=%lu\n",(unsigned)(sizeof(void*)*8),loaded,made,code);
        return loaded && made && code==0?0:1;
    }
    if(argc==2 && !wcscmp(argv[1],L"headless-to-gui")){
        BOOL loaded=module()!=NULL;DWORD code=99;
        BOOL made=create_owned(L"normal",&code);
        printf("HEADLESS TO GUI bits=%u initial_dll=%d create=%d gui_exit=%lu\n",
            (unsigned)(sizeof(void*)*8),loaded,made,code);
        return !loaded && made && code==0?0:1;
    }
    if(argc==2 && (!wcscmp(argv[1],L"child") || !wcscmp(argv[1],L"grandchild"))){
        BOOL loaded=module()!=NULL;printf("PROPAGATION ENTRY bits=%u role=%ls dll=%d\n",(unsigned)(sizeof(void*)*8),argv[1],loaded);
        if(!loaded)return 41;
        WCHAR only[2];
        if(GetEnvironmentVariableW(L"WIN16S_TEST_PROPAGATION_ONLY",only,ARRAYSIZE(only))){
            WNDCLASSW klass={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=DefWindowProcW,.lpszClassName=L"PropagationOnlyOwned"};
            if(!RegisterClassW(&klass))return 45;
            HWND w=CreateWindowW(klass.lpszClassName,L"Propagation only",WS_OVERLAPPEDWINDOW|WS_VISIBLE,20,20,160,100,NULL,NULL,klass.hInstance,NULL);
            attached_fn attached=(attached_fn)(void*)GetProcAddress(module(),"Ntw16sWindowAttached");
            BOOL styled=w && attached && attached(w);BOOL valid=w && attached;
            if(w)DestroyWindow(w);
            UnregisterClassW(klass.lpszClassName,klass.hInstance);
            printf("PROPAGATION ONLY WINDOW bits=%u role=%ls attached=%d\n",(unsigned)(sizeof(void*)*8),argv[1],styled);
            if(!valid || styled)return 46;
        }
        if(!wcscmp(argv[1],L"child")){DWORD code=99;BOOL ok=create_owned(L"grandchild",&code);printf("PROPAGATION CHILD create=%d exit=%lu\n",ok,code);return ok&&code==0?0:42;}
        return 0;
    }
    WCHAR only_mode[2];
    if(argc==1 && GetEnvironmentVariableW(L"WIN16S_TEST_PROPAGATION_ONLY",only_mode,ARRAYSIZE(only_mode))){
        DWORD code=99;BOOL made=create_owned(L"child",&code);
        printf("PROPAGATION ONLY ROOT create=%d child_exit=%lu\n",made,code);
        return made && code==0?0:47;
    }
    managed=argc==2 && !wcscmp(argv[1],L"managed");
    WNDCLASSW c={.hInstance=GetModuleHandleW(NULL),.lpfnWndProc=proc,.lpszClassName=L"GlobalPropagationProbe"};
    if(!RegisterClassW(&c))return 2;
    HWND w=CreateWindowW(c.lpszClassName,L"Owned propagation root",WS_OVERLAPPEDWINDOW|WS_VISIBLE,50,50,300,180,NULL,NULL,c.hInstance,NULL);
    if(!w)return 3;
    root_window=w;
    deadline=GetTickCount64()+5000;if(!managed && !SetTimer(w,1,50,NULL)){DestroyWindow(w);return 4;}
    MSG m;while(GetMessageW(&m,NULL,0,0)>0){TranslateMessage(&m);DispatchMessageW(&m);}
    if(local_hook)UnhookWindowsHookEx(local_hook);
    if(local_module)FreeLibrary(local_module);
    return managed?0:result;
}
