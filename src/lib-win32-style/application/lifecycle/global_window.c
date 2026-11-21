#include "global_window.h"
#include "lib-win32-style/application/window/window_identity.h"
#include <commctrl.h>
#include <stdlib.h>

typedef struct global_window {
    unsigned calls;
    DWORD profile;
    BOOL detached,removing,installed;
    const style_global_window_ops *ops;
} global_window;
static LRESULT CALLBACK global_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static const WCHAR managed_property[]=L"StyleCommon.GlobalWindow";

typedef struct child_visit {BOOL (*visit)(HWND,void *);void *context;} child_visit;
static BOOL CALLBACK visit_child(HWND window,LPARAM parameter)
{
    const child_visit *visitor=(const child_visit *)parameter;
    return !win32_style_window_current_thread(window) || visitor->visit(window,visitor->context);
}
BOOL style_global_window_visit_children(HWND parent,BOOL (*visit)(HWND,void *),void *context)
{
    if(!visit){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    child_visit visitor={visit,context};return EnumChildWindows(parent,visit_child,(LPARAM)&visitor);
}

BOOL style_global_window_managed(HWND window)
{return GetPropW(window,managed_property)!=NULL;}

static BOOL local(HWND window)
{
    if(!win32_style_window_current_thread(window)){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    return TRUE;
}
static global_window *lookup(HWND window)
{
    DWORD_PTR value=0;
    return GetWindowSubclass(window,global_proc,(UINT_PTR)global_proc,&value)?(global_window *)value:NULL;
}
static void release(global_window *state)
{if(!--state->calls && state->detached)free(state);}
static void detach(HWND window,global_window *state)
{
    if(state->detached)return;
    if(GetPropW(window,managed_property)==(HANDLE)state)RemovePropW(window,managed_property);
    RemoveWindowSubclass(window,global_proc,(UINT_PTR)global_proc);state->detached=TRUE;
}
static LRESULT CALLBACK global_proc(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{
    global_window *state=(global_window *)data;const style_global_window_ops *ops=state->ops;
    LRESULT result;(void)id;++state->calls;
    if(message==RegisterWindowMessageW(ops->control) && lp==ops->query && !state->detached &&
       state->installed && ops->active((DWORD)wp) && ops->attached(window)){
        release(state);return ops->acknowledgement;
    }
    if(message==WM_NCDESTROY)detach(window,state);
    result=DefSubclassProc(window,message,wp,lp);release(state);return result;
}
BOOL style_global_window_attached(HWND window,const style_global_window_ops *ops)
{
    if(!local(window))return FALSE;
    global_window *state=lookup(window);
    return state && state->ops==ops && !state->detached && state->installed;
}
static BOOL valid_ops(const style_global_window_ops *ops)
{return ops && ops->attached && ops->apply && ops->remove && ops->active && ops->control;}

static BOOL apply_window(HWND window,const style_global_window_ops *ops,DWORD profile,
    style_global_window_apply_profile apply_profile)
{
    if(!local(window))return FALSE;
    if(!valid_ops(ops)){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    global_window *state=lookup(window);
    if(state){if(state->ops==ops && !state->detached && state->installed)return TRUE;SetLastError(ERROR_BUSY);return FALSE;}
    if(style_global_window_managed(window)){SetLastError(ERROR_BUSY);return FALSE;}
    BOOL already=ops->attached(window);state=(global_window *)calloc(1,sizeof(*state));
    if(!state){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    state->calls=1;state->ops=ops;state->profile=profile;
    if(!SetWindowSubclass(window,global_proc,(UINT_PTR)global_proc,(DWORD_PTR)state)){
        DWORD error=GetLastError();free(state);SetLastError(error);return FALSE;
    }
    if(!SetPropW(window,managed_property,(HANDLE)state)){
        DWORD error=GetLastError();detach(window,state);release(state);SetLastError(error);return FALSE;
    }
    BOOL installed=already || (apply_profile?apply_profile(window,ops->apply,profile):ops->apply(window));
    if(installed)state->installed=TRUE;
    DWORD error=installed?ERROR_SUCCESS:GetLastError();
    if(state->detached){installed=FALSE;error=ERROR_INVALID_WINDOW_HANDLE;}
    if(!installed && !state->detached)detach(window,state);
    release(state);SetLastError(error);return installed;
}
BOOL style_global_window_apply(HWND window,const style_global_window_ops *ops)
{return apply_window(window,ops,0,NULL);}
BOOL style_global_window_remove(HWND window,const style_global_window_ops *ops)
{
    if(!local(window))return FALSE;
    if(!ops || !ops->attached || !ops->remove){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    global_window *state=lookup(window);
    if(!state){if(ops->attached(window))return ops->remove(window);SetLastError(ERROR_NOT_FOUND);return FALSE;}
    if(state->ops!=ops || state->detached){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    if(state->removing || !state->installed){SetLastError(ERROR_BUSY);return FALSE;}
    ++state->calls;state->removing=TRUE;
    BOOL removed=ops->remove(window);DWORD error=removed?ERROR_SUCCESS:GetLastError();
    if(removed && !state->detached)detach(window,state);
    if(!state->detached)state->removing=FALSE;
    release(state);SetLastError(error);return removed;
}
DWORD style_global_window_profile(HWND window)
{
    if(!local(window))return 0;
    global_window *state=lookup(window);
    return state && state->installed && !state->detached?state->profile:0;
}
BOOL style_global_window_remove_current(HWND window)
{
    if(!local(window))return FALSE;
    global_window *state=lookup(window);
    if(!state){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    return style_global_window_remove(window,state->ops);
}
BOOL style_global_window_transition(HWND window,const style_global_window_ops *selected,
    const style_global_window_ops *unmanaged_other,DWORD profile,BOOL refresh_direct,
    style_global_window_apply_profile apply_profile)
{
    if(!local(window))return FALSE;
    if(!valid_ops(selected) || (unmanaged_other && (!unmanaged_other->attached || !unmanaged_other->apply || !unmanaged_other->remove))){
        SetLastError(ERROR_INVALID_PARAMETER);return FALSE;
    }
    global_window *state=lookup(window);
    BOOL managed=state!=NULL;
    DWORD prior=managed?state->profile:0;
    const style_global_window_ops *previous=NULL;
    if(managed){
        if(state->ops!=selected || (prior!=profile && (prior || refresh_direct)))
            previous=state->ops;
    }else{
        if(refresh_direct && selected->attached(window) && !style_global_window_remove(window,selected))return FALSE;
        if(unmanaged_other && unmanaged_other!=selected && unmanaged_other->attached(window))previous=unmanaged_other;
    }

    if(previous && !style_global_window_remove(window,previous))return FALSE;
    if(apply_window(window,selected,profile,apply_profile)){
        state=lookup(window);
        if(state)state->profile=profile;
        return TRUE;
    }
    DWORD error=GetLastError();
    if(managed && previous)(void)apply_window(window,previous,prior,apply_profile);
    else if(previous)(void)previous->apply(window);
    SetLastError(error);return FALSE;
}
