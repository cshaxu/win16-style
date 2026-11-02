#include "tray.h"
#include "global.h"
#include "ntw16s-dll/global_session.h"
#include <stdio.h>
#include <wchar.h>

typedef struct controller {
    HANDLE parent,worker;run16s_tray *tray;
    BOOL enabled,busy,stopping;
    WCHAR session_name[NTW16S_GLOBAL_NAME_CAP],request_name[NTW16S_GLOBAL_NAME_CAP+16];
} controller;
static DWORD WINAPI worker_main(void *value)
{controller *state=(controller *)value;return (DWORD)run16s_global_with_parent(state->parent);}
static BOOL start_worker(controller *state)
{
    state->worker=CreateThread(NULL,0,worker_main,state,0,NULL);
    if(!state->worker)return FALSE;
    state->enabled=TRUE;state->busy=TRUE;state->stopping=FALSE;
    run16s_tray_state(state->tray,TRUE,TRUE);return TRUE;
}
static BOOL session_ready(controller *state)
{
    HANDLE mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,state->session_name);if(!mapping)return FALSE;
    ntw16s_global_session *s=(ntw16s_global_session *)MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*s));
    BOOL ready=s && s->magic==NTW16S_GLOBAL_MAGIC && s->version==NTW16S_GLOBAL_VERSION &&
        s->owner_pid==GetCurrentProcessId() && s->phase==NTW16S_GLOBAL_ACTIVE && s->ready;
    if(s)UnmapViewOfFile(s);
    CloseHandle(mapping);return ready;
}
static void action(unsigned command,void *value)
{
    controller *state=(controller *)value;
    if(command==RUN16S_TRAY_EXIT)ExitProcess(0);
    if(command!=RUN16S_TRAY_TOGGLE || state->busy)return;
    if(!state->enabled){if(!start_worker(state))MessageBeep(MB_ICONERROR);return;}
    HANDLE request=OpenEventW(EVENT_MODIFY_STATE,FALSE,state->request_name);
    if(!request){MessageBeep(MB_ICONERROR);return;}
    BOOL sent=SetEvent(request);CloseHandle(request);
    if(!sent){MessageBeep(MB_ICONERROR);return;}
    state->stopping=TRUE;state->busy=TRUE;run16s_tray_state(state->tray,TRUE,TRUE);
}
int run16s_tray_global(HANDLE parent)
{
    controller state={0};state.parent=parent;
    if(!ntw16s_global_name(state.session_name,ARRAYSIZE(state.session_name)) ||
       swprintf(state.request_name,ARRAYSIZE(state.request_name),L"%ls.request",state.session_name)<0){
        fwprintf(stderr,L"run16s: cannot prepare the tray session\n");return 10;
    }
    /* Group 40's 16x16 artwork has a gray base; its 32x32 artwork is the
     * standalone Windows flag used by the original tray prototype. */
    HICON icon=(HICON)LoadImageW(GetModuleHandleW(NULL),MAKEINTRESOURCEW(101),IMAGE_ICON,
        32,32,LR_DEFAULTCOLOR);
    if(!icon){fwprintf(stderr,L"run16s: cannot load tray icon\n");return 10;}
    DWORD error=run16s_tray_create(icon,action,&state,&state.tray);
    if(error){fwprintf(stderr,L"run16s: cannot create tray icon (%lu)\n",error);DestroyIcon(icon);return 10;}
    int result=10;
    for(;;){
        HANDLE waits[2];DWORD count=0;if(state.worker)waits[count++]=state.worker;if(parent)waits[count++]=parent;
        DWORD status=MsgWaitForMultipleObjects(count,waits,FALSE,200,QS_ALLINPUT);
        if(state.worker && status==WAIT_OBJECT_0){
            DWORD code=10;GetExitCodeThread(state.worker,&code);CloseHandle(state.worker);state.worker=NULL;
            if(!state.stopping){result=(int)code;break;}
            state.stopping=FALSE;state.enabled=FALSE;state.busy=FALSE;run16s_tray_state(state.tray,FALSE,FALSE);
        }else if(parent && status==WAIT_OBJECT_0+(state.worker?1:0)){
            if(!state.worker){result=0;break;}
        }else if(status==WAIT_OBJECT_0+count){MSG message;while(PeekMessageW(&message,NULL,0,0,PM_REMOVE)){TranslateMessage(&message);DispatchMessageW(&message);}}
        else if(status!=WAIT_TIMEOUT){result=10;break;}
        if(state.worker && state.busy && !state.stopping && session_ready(&state)){
            state.busy=FALSE;run16s_tray_state(state.tray,TRUE,FALSE);
        }
    }
    if(state.worker){HANDLE request=OpenEventW(EVENT_MODIFY_STATE,FALSE,state.request_name);
        if(request){SetEvent(request);CloseHandle(request);}
        if(WaitForSingleObject(state.worker,10000)==WAIT_OBJECT_0)CloseHandle(state.worker);
        else ExitProcess((UINT)result);
    }
    run16s_tray_destroy(state.tray);DestroyIcon(icon);return result;
}
