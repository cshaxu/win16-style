#include "global_session.h"
#include "global_window.h"
#include "global_vb_text.h"
#include "global_policy.h"
#include "global_manager.h"
#include "attachment.h"
#include <commctrl.h>
#include <stdlib.h>
#include <wchar.h>

static _Thread_local BOOL entering;
LRESULT CALLBACK Ntw16sGlobalHook(int code,WPARAM a,LPARAM b);

/* Do not style shared Windows shell or UWP host processes.  This is checked
 * inside the target, before any window is attached. */
static INIT_ONCE process_once=INIT_ONCE_STATIC_INIT;
static BOOL process_allowed;
static BOOL CALLBACK check_process(PINIT_ONCE once,void *parameter,void **context)
{
    WCHAR *image=(WCHAR *)calloc(32768,sizeof(WCHAR)),windows[MAX_PATH];
    DWORD n=image?GetModuleFileNameW(NULL,image,32768):0;
    UINT wn=GetWindowsDirectoryW(windows,ARRAYSIZE(windows));
    (void)once;(void)parameter;(void)context;
    if(!n || n>=32768 || !wn || wn>=ARRAYSIZE(windows)){free(image);return TRUE;}
    process_allowed=ntw16s_global_image_allowed(image,windows);
    free(image);return TRUE;
}

static BOOL owner_matches(const ntw16s_global_session *s,HMODULE module)
{
    BOOL ok=FALSE;DWORD size=32752;WCHAR *owner=(WCHAR *)calloc(size,sizeof(WCHAR));
    WCHAR *dll=(WCHAR *)calloc(size,sizeof(WCHAR));HANDLE process=NULL;
    if(!owner || !dll)goto done;
    process=OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION,FALSE,s->owner_pid);
    if(!process || !QueryFullProcessImageNameW(process,0,owner,&size))goto done;
    DWORD length=GetModuleFileNameW(module,dll,32752);
    if(!length || length>=32752)goto done;
    WCHAR *slash=wcsrchr(dll,L'\\');if(!slash || (size_t)(slash-dll)+11>=32752)goto done;
    wcscpy(slash+1,L"run16s.exe");ok=!lstrcmpiW(owner,dll);
done:
    if(process)CloseHandle(process);
    free(owner);free(dll);return ok;
}
static BOOL eligible(HWND w)
{
    WCHAR name[64];DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    if(!GetClassNameW(w,name,ARRAYSIZE(name)))return FALSE;
    if(style&WS_CHILD)return TRUE;
    return (style&WS_CAPTION)==WS_CAPTION && !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_TOOLWINDOW|WS_EX_LAYERED));
}
static BOOL CALLBACK children(HWND w,LPARAM value)
{
    DWORD process=0;DWORD thread=GetWindowThreadProcessId(w,&process);
    if(process==GetCurrentProcessId() && thread==GetCurrentThreadId()){
        if(value==NTW16S_GLOBAL_APPLY && eligible(w))(void)ntw16s_global_window_apply(w);
        else if(value==NTW16S_GLOBAL_REMOVE)(void)ntw16s_global_window_remove(w);
    }
    return TRUE;
}
LRESULT CALLBACK Ntw16sGlobalHook(int code,WPARAM a,LPARAM b)
{
    DWORD saved=GetLastError();
    if(code>=0 && b && !entering && InitOnceExecuteOnce(&process_once,check_process,NULL,NULL) && process_allowed){
        const CWPSTRUCT *message=(const CWPSTRUCT *)b;DWORD process=0;
        DWORD thread=GetWindowThreadProcessId(message->hwnd,&process);
        UINT control=RegisterWindowMessageW(NTW16S_GLOBAL_CONTROL);
        BOOL visible=message->message==WM_SHOWWINDOW && message->wParam;
        if(process==GetCurrentProcessId() && thread==GetCurrentThreadId() && control &&
           ((visible && eligible(message->hwnd)) || message->message==control)){
            WCHAR name[NTW16S_GLOBAL_NAME_CAP];entering=TRUE;
            if(ntw16s_global_name(name,ARRAYSIZE(name))){
                HANDLE mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
                ntw16s_global_session *s=mapping?(ntw16s_global_session *)MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*s)):NULL;
                HMODULE module=NULL;
                if(s && s->owner_pid!=process && s->host_pid!=process && ntw16s_global_owner_alive(s) &&
                   GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)(void *)&Ntw16sGlobalHook,&module) && owner_matches(s,module)){
                    LONG phase=s->phase;BOOL control_ok=message->message==control && (DWORD)message->wParam==s->cookie;
                    if(phase==NTW16S_GLOBAL_ACTIVE && (visible || (control_ok && message->lParam==NTW16S_GLOBAL_APPLY))){
                        HMODULE pinned=NULL;
                        if(eligible(message->hwnd) && GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
                                (LPCWSTR)(void *)&Ntw16sGlobalHook,&pinned)){
                            BOOL attached=ntw16s_global_window_apply(message->hwnd),text_changed=FALSE;
                            if(attached && !ntw16s_global_vb_text(message->hwnd,&text_changed) && text_changed)
                                RedrawWindow(message->hwnd,NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN);
                            EnumChildWindows(message->hwnd,children,NTW16S_GLOBAL_APPLY);
                            if(s->phase!=NTW16S_GLOBAL_ACTIVE){
                                EnumChildWindows(message->hwnd,children,NTW16S_GLOBAL_REMOVE);
                                (void)ntw16s_global_window_remove(message->hwnd);
                            }
                        }
                    }else if(phase==NTW16S_GLOBAL_STOPPING && control_ok && message->lParam==NTW16S_GLOBAL_REMOVE){
                        EnumChildWindows(message->hwnd,children,NTW16S_GLOBAL_REMOVE);
                        (void)ntw16s_global_window_remove(message->hwnd);
                    }
                }
                if(s)UnmapViewOfFile(s);
                if(mapping)CloseHandle(mapping);
            }
            entering=FALSE;
        }
    }
    LRESULT result=CallNextHookEx(NULL,code,a,b);SetLastError(saved);return result;
}

void CALLBACK Ntw16sGlobalHost(HWND window,HINSTANCE instance,LPSTR command,int show)
{
    (void)window;(void)instance;(void)command;(void)show;
    WCHAR name[NTW16S_GLOBAL_NAME_CAP],ready_name[NTW16S_GLOBAL_NAME_CAP+16],stop_name[NTW16S_GLOBAL_NAME_CAP+16];
    if(!ntw16s_global_name(name,ARRAYSIZE(name)))return;
    if(swprintf(ready_name,ARRAYSIZE(ready_name),L"%ls.ready",name)<0 || swprintf(stop_name,ARRAYSIZE(stop_name),L"%ls.stop",name)<0)return;
    HANDLE mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name),ready=NULL,stop=NULL,owner=NULL;HHOOK hook=NULL;
    ntw16s_global_manager *manager=NULL;
    ntw16s_global_session *s=mapping?(ntw16s_global_session *)MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*s)):NULL;
    if(!s || !ntw16s_global_owner_alive(s))goto done;
    owner=OpenProcess(SYNCHRONIZE,FALSE,s->owner_pid);ready=OpenEventW(EVENT_MODIFY_STATE,FALSE,ready_name);
    stop=OpenEventW(SYNCHRONIZE,FALSE,stop_name);if(!owner || !ready || !stop)goto done;
    HMODULE module=NULL;if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCWSTR)(void *)&Ntw16sGlobalHook,&module))goto done;
    hook=SetWindowsHookExW(WH_CALLWNDPROC,Ntw16sGlobalHook,module,0);
    if(!hook){
        if(GetLastError()!=ERROR_MOD_NOT_FOUND)goto done;
        WCHAR path[32768];DWORD n=GetModuleFileNameW(module,path,ARRAYSIZE(path));
        WORD machine=sizeof(void*)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386;
        if(!n || n>=ARRAYSIZE(path) || ntw16s_global_manager_start(path,machine,&manager))goto done;
    }
    SetEvent(ready);
    HANDLE waits[2]={stop,owner};
    for(;;){DWORD status=MsgWaitForMultipleObjects(2,waits,FALSE,INFINITE,QS_ALLINPUT);
        if(status==WAIT_OBJECT_0 || status==WAIT_OBJECT_0+1)break;
        if(status!=WAIT_OBJECT_0+2)break;
        MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
    }
done:
    if(hook)UnhookWindowsHookEx(hook);
    if(manager)(void)ntw16s_global_manager_stop(manager,5000);
    if(owner)CloseHandle(owner);
    if(stop)CloseHandle(stop);
    if(ready)CloseHandle(ready);
    if(s)UnmapViewOfFile(s);
    if(mapping)CloseHandle(mapping);
}
