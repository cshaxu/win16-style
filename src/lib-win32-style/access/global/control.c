#include "control.h"
#include <wchar.h>

static DWORD WINAPI worker_main(void *value)
{
    style_global_control *state=value;
    return state->ops->run(state->parent,state->style,state->context);
}
static BOOL start(style_global_control *state,DWORD style)
{
    state->style=style;
    state->worker=CreateThread(NULL,0,worker_main,state,0,NULL);
    if(!state->worker)return FALSE;
    state->enabled=FALSE;state->busy=TRUE;state->stopping=FALSE;return TRUE;
}
static BOOL ready(style_global_control *state)
{
    HANDLE mapping=OpenFileMappingW(FILE_MAP_READ,FALSE,state->session_name);if(!mapping)return FALSE;
    style_global_session *session=MapViewOfFile(mapping,FILE_MAP_READ,0,0,sizeof(*session));
    BOOL result=session && session->magic==state->space->magic && session->version==state->space->version &&
        session->owner_pid==GetCurrentProcessId() && session->style==state->style &&
        session->phase==STYLE_GLOBAL_ACTIVE && session->ready;
    if(session)UnmapViewOfFile(session);
    CloseHandle(mapping);return result;
}
static void reconcile(style_global_control *state)
{
    if(!state->worker){
        if(state->desired!=state->none && !start(state,state->desired)){
            DWORD error=GetLastError();state->desired=state->none;state->ops->failed(error,state->context);
        }
        return;
    }
    if(state->stopping || state->style==state->desired)return;
    HANDLE request=OpenEventW(EVENT_MODIFY_STATE,FALSE,state->request_name);
    if(!request)return;
    BOOL sent=SetEvent(request);DWORD error=GetLastError();CloseHandle(request);
    if(!sent){state->ops->failed(error,state->context);return;}
    state->stopping=TRUE;state->busy=TRUE;
}
BOOL style_global_control_init(style_global_control *state,HANDLE parent,DWORD initial,DWORD none,
    const style_global_namespace *space,const style_global_control_ops *ops,void *context)
{
    *state=(style_global_control){.parent=parent,.style=initial,.desired=none,.none=none,
        .space=space,.ops=ops,.context=context};
    if(!style_global_session_name(space,state->session_name,ARRAYSIZE(state->session_name)))return FALSE;
    if(swprintf(state->request_name,ARRAYSIZE(state->request_name),L"%ls.request",state->session_name)<0){
        SetLastError(ERROR_INSUFFICIENT_BUFFER);return FALSE;
    }
    return TRUE;
}
void style_global_control_select(style_global_control *state,DWORD selected)
{
    state->desired=state->desired==selected?state->none:selected;
    reconcile(state);
}
BOOL style_global_control_finished(style_global_control *state,DWORD *result)
{
    DWORD code=state->ops->failed_result;GetExitCodeThread(state->worker,&code);CloseHandle(state->worker);state->worker=NULL;
    if(!state->stopping){*result=code;return TRUE;}
    state->stopping=FALSE;state->enabled=FALSE;state->busy=FALSE;
    if(code){state->desired=state->none;state->ops->failed(code,state->context);return FALSE;}
    if(!state->parent || WaitForSingleObject(state->parent,0)==WAIT_TIMEOUT)reconcile(state);
    return FALSE;
}
void style_global_control_update(style_global_control *state)
{
    if(state->worker && state->busy && !state->stopping && ready(state)){
        state->enabled=TRUE;state->busy=FALSE;
    }
    if(state->worker && !state->stopping)reconcile(state);
}
DWORD style_global_control_close(style_global_control *state,DWORD timeout)
{
    if(!state->worker)return ERROR_SUCCESS;
    HANDLE request=OpenEventW(EVENT_MODIFY_STATE,FALSE,state->request_name);
    if(request){SetEvent(request);CloseHandle(request);}
    DWORD wait=WaitForSingleObject(state->worker,timeout);
    if(wait!=WAIT_OBJECT_0)return wait==WAIT_TIMEOUT?ERROR_TIMEOUT:GetLastError();
    CloseHandle(state->worker);state->worker=NULL;return ERROR_SUCCESS;
}
