#include "ntw16s-dll/global/session.h"
#include "ntw16s-dll/global/window.h"
#include "ntw16s-dll/global/vb_text.h"
#include "ntw16s-dll/global/policy.h"
#include "ntw16s-dll/global/manager.h"
#include "ntw16s-dll/global/process.h"
#include "ntw16s-dll/global/late.h"
#include "ntw16s-dll/attachment.h"
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

static BOOL eligible(HWND w)
{
    WCHAR name[64];DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
    if(!GetClassNameW(w,name,ARRAYSIZE(name)))return FALSE;
    if(style&WS_CHILD)return TRUE;
    return (style&WS_CAPTION)==WS_CAPTION && !(GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_TOOLWINDOW|WS_EX_LAYERED));
}
typedef struct child_action {LPARAM action;DWORD style;} child_action;
static BOOL CALLBACK children(HWND w,LPARAM value)
{
    const child_action *action=(const child_action *)value;
    DWORD process=0;DWORD thread=GetWindowThreadProcessId(w,&process);
    if(process==GetCurrentProcessId() && thread==GetCurrentThreadId()){
        if(action->style!=NTW16S_GLOBAL_STYLE_16)return TRUE;
        if(action->action==NTW16S_GLOBAL_APPLY && eligible(w))(void)ntw16s_global_window_apply_style(w,action->style);
        else if(action->action==NTW16S_GLOBAL_REMOVE)(void)ntw16s_global_window_remove_style(w,action->style);
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
                       (LPCWSTR)(void *)&Ntw16sGlobalHook,&module) &&
                   GetModuleHandleW(sizeof(void*)==8?L"ntw16s64.dll":L"ntw16s32.dll")==module &&
                   ntw16s_global_owner_matches(s,module)){
                    LONG phase=s->phase;BOOL control_ok=message->message==control && (DWORD)message->wParam==s->cookie;
                    if(phase==NTW16S_GLOBAL_ACTIVE && (visible || (control_ok && message->lParam==NTW16S_GLOBAL_APPLY)) && ntw16s_global_late_schedule(s->cookie)){
                        HMODULE pinned=NULL;
                        if(eligible(message->hwnd) && GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
                                (LPCWSTR)(void *)&Ntw16sGlobalHook,&pinned)){
                            child_action action={NTW16S_GLOBAL_APPLY,s->style};
                            BOOL attached=ntw16s_global_window_apply_style(message->hwnd,s->style),text_changed=FALSE;
                            if(s->style==NTW16S_GLOBAL_STYLE_16 && attached && !ntw16s_global_vb_text(message->hwnd,&text_changed) && text_changed)
                                RedrawWindow(message->hwnd,NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN);
                            EnumChildWindows(message->hwnd,children,(LPARAM)&action);
                            if(s->phase!=NTW16S_GLOBAL_ACTIVE){
                                action.action=NTW16S_GLOBAL_REMOVE;EnumChildWindows(message->hwnd,children,(LPARAM)&action);
                                (void)ntw16s_global_window_remove_style(message->hwnd,s->style);
                            }
                        }
                    }else if(phase==NTW16S_GLOBAL_STOPPING && control_ok && message->lParam==NTW16S_GLOBAL_REMOVE){
                        child_action action={NTW16S_GLOBAL_REMOVE,s->style};
                        EnumChildWindows(message->hwnd,children,(LPARAM)&action);
                        (void)ntw16s_global_window_remove_style(message->hwnd,s->style);
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
    DWORD error=ERROR_SUCCESS;
    WCHAR name[NTW16S_GLOBAL_NAME_CAP],ready_name[NTW16S_GLOBAL_NAME_CAP+16],stop_name[NTW16S_GLOBAL_NAME_CAP+16];
    if(!ntw16s_global_name(name,ARRAYSIZE(name))){ExitProcess(GetLastError());return;}
    if(swprintf(ready_name,ARRAYSIZE(ready_name),L"%ls.ready",name)<0 || swprintf(stop_name,ARRAYSIZE(stop_name),L"%ls.stop",name)<0){ExitProcess(ERROR_INSUFFICIENT_BUFFER);return;}
    HANDLE mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name),ready=NULL,stop=NULL,owner=NULL;HHOOK hook=NULL;
    ntw16s_global_manager *manager=NULL;
    ntw16s_global_session *s=mapping?(ntw16s_global_session *)MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*s)):NULL;
    if(!s){error=GetLastError();if(!error)error=ERROR_INVALID_HANDLE;goto done;}
    if(!ntw16s_global_owner_alive(s)){error=ERROR_INVALID_STATE;goto done;}
    owner=OpenProcess(SYNCHRONIZE|PROCESS_QUERY_LIMITED_INFORMATION,FALSE,s->owner_pid);if(!owner){error=GetLastError();goto done;}
    ready=OpenEventW(EVENT_MODIFY_STATE,FALSE,ready_name);if(!ready){error=GetLastError();goto done;}
    stop=OpenEventW(SYNCHRONIZE,FALSE,stop_name);if(!stop){error=GetLastError();goto done;}
    HMODULE module=NULL;if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (LPCWSTR)(void *)&Ntw16sGlobalHook,&module)){error=GetLastError();goto done;}
    hook=SetWindowsHookExW(WH_CALLWNDPROC,Ntw16sGlobalHook,module,0);
    if(!hook && GetLastError()!=ERROR_MOD_NOT_FOUND){error=GetLastError();goto done;}
    WCHAR path[32768];DWORD n=GetModuleFileNameW(module,path,ARRAYSIZE(path));
    WORD machine=sizeof(void*)==8?IMAGE_FILE_MACHINE_AMD64:IMAGE_FILE_MACHINE_I386;
    if(!n || n>=ARRAYSIZE(path)){error=n?ERROR_INSUFFICIENT_BUFFER:GetLastError();goto done;}
    error=ntw16s_global_manager_start(path,machine,&manager);if(error)goto done;
    DWORD idle=ntw16s_global_manager_wait_idle(manager,2000);
    if(idle==ERROR_TIMEOUT)OutputDebugStringW(L"run16s: global 32-bit startup attachment still pending\n");
    else if(idle){error=idle;goto done;}
    if(!SetEvent(ready)){error=GetLastError();goto done;}
    HANDLE waits[2]={stop,owner};
    for(;;){DWORD status=MsgWaitForMultipleObjects(2,waits,FALSE,INFINITE,QS_ALLINPUT);
        if(status==WAIT_OBJECT_0 || status==WAIT_OBJECT_0+1)break;
        if(status!=WAIT_OBJECT_0+2)break;
        MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}
    }
done:
    if(hook)UnhookWindowsHookEx(hook);
    if(manager){DWORD stopped=ntw16s_global_manager_stop(manager,5000);if(stopped && !error)error=stopped;}
    if(owner && WaitForSingleObject(owner,0)==WAIT_OBJECT_0 && ntw16s_global_process_stop_peer(owner,s->owner_pid))
        OutputDebugStringW(L"run16s: global owner trace cleanup failed\n");
    if(owner)CloseHandle(owner);
    if(stop)CloseHandle(stop);
    if(ready)CloseHandle(ready);
    if(s)UnmapViewOfFile(s);
    if(mapping)CloseHandle(mapping);
    if(error)ExitProcess(error);
}
