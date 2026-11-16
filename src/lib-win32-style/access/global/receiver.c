#include "receiver.h"
#include <stdlib.h>

static _Thread_local BOOL entering;

/* Eligibility is cached per configured receiver before any window action. */
static BOOL CALLBACK check_process(PINIT_ONCE once,void *parameter,void **context)
{
    WCHAR *image=(WCHAR *)calloc(32768,sizeof(WCHAR)),windows[MAX_PATH];
    DWORD n=image?GetModuleFileNameW(NULL,image,32768):0;
    UINT wn=GetWindowsDirectoryW(windows,ARRAYSIZE(windows));
    (void)once;(void)context;
    if(!n || n>=32768 || !wn || wn>=ARRAYSIZE(windows)){free(image);return TRUE;}
    style_global_receiver *state=parameter;
    state->allowed=state->ops->allowed(image,windows);
    free(image);return TRUE;
}

LRESULT style_global_receiver_handle(style_global_receiver *state,int code,WPARAM a,LPARAM b)
{
    DWORD saved=GetLastError();
    if(code>=0 && b && !entering && InitOnceExecuteOnce(&state->once,check_process,state,NULL) && state->allowed){
        const CWPSTRUCT *message=(const CWPSTRUCT *)b;DWORD process=0;
        DWORD thread=GetWindowThreadProcessId(message->hwnd,&process);
        UINT control=RegisterWindowMessageW(state->ops->control);
        BOOL visible=message->message==WM_SHOWWINDOW && message->wParam;
        if(process==GetCurrentProcessId() && thread==GetCurrentThreadId() && control &&
           ((visible && state->ops->eligible(message->hwnd)) || message->message==control)){
            WCHAR name[STYLE_GLOBAL_NAME_CAP];entering=TRUE;
            if(style_global_session_name(state->ops->space,name,ARRAYSIZE(name))){
                HANDLE mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,name);
                style_global_session *s=mapping?(style_global_session *)MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*s)):NULL;
                HMODULE module=NULL;
                if(s && s->owner_pid!=process && s->host_pid!=process && style_global_owner_alive(state->ops->space,s) &&
                   GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                       (LPCWSTR)(void *)&style_global_receiver_handle,&module) &&
                   GetModuleHandleW(state->ops->space->module)==module &&
                   style_global_owner_matches(state->ops->space,s,module)){
                    LONG phase=s->phase;BOOL control_ok=message->message==control && (DWORD)message->wParam==s->cookie;
                    if(phase==STYLE_GLOBAL_ACTIVE && (visible || (control_ok && message->lParam==state->ops->apply)) && style_global_late_schedule(state->ops->late,s->cookie)){
                        HMODULE pinned=NULL;
                        if(state->ops->eligible(message->hwnd) && GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,
                                (LPCWSTR)(void *)&style_global_receiver_handle,&pinned)){
                            state->ops->change(message->hwnd,s->style,TRUE,state->ops->context);
                            if(s->phase!=STYLE_GLOBAL_ACTIVE){
                                state->ops->change(message->hwnd,s->style,FALSE,state->ops->context);
                            }
                        }
                    }else if(phase==STYLE_GLOBAL_STOPPING && control_ok && message->lParam==state->ops->remove){
                        state->ops->change(message->hwnd,s->style,FALSE,state->ops->context);
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
