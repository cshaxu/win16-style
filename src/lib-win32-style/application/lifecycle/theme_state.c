#include "theme_state.h"
#include <commctrl.h>
#include <uxtheme.h>
#include <stdlib.h>
#include <wchar.h>
#include "lib-win32/process-hook/unwind.h"

/* Resolve GUI services only when a style operation actually needs them.
 * The process helper can consequently remain usable in non-GUI processes. */
typedef struct theme_gui_api {
    __typeof__(&GetWindowThreadProcessId) thread;
    __typeof__(&GetWindowSubclass) get;
    __typeof__(&SetWindowSubclass) set;
    __typeof__(&RemoveWindowSubclass) remove;
    __typeof__(&DefSubclassProc) next;
} theme_gui_api;
static INIT_ONCE gui_once=INIT_ONCE_STATIC_INIT;
static theme_gui_api gui;
static BOOL CALLBACK initialize_gui(PINIT_ONCE once,void *parameter,void **context)
{
    (void)once;(void)parameter;(void)context;
    HMODULE user=LoadLibraryW(L"user32.dll"),controls=LoadLibraryW(L"comctl32.dll");
    if(user && controls){
        gui.thread=(void *)GetProcAddress(user,"GetWindowThreadProcessId");
        gui.get=(void *)GetProcAddress(controls,MAKEINTRESOURCEA(411));
        gui.set=(void *)GetProcAddress(controls,"SetWindowSubclass");
        gui.remove=(void *)GetProcAddress(controls,"RemoveWindowSubclass");
        gui.next=(void *)GetProcAddress(controls,"DefSubclassProc");
        if(gui.thread && gui.get && gui.set && gui.remove && gui.next)return TRUE;
    }
    if(controls)FreeLibrary(controls);
    if(user)FreeLibrary(user);
    ZeroMemory(&gui,sizeof(gui));SetLastError(ERROR_PROC_NOT_FOUND);return FALSE;
}
static BOOL theme_gui_ready(void)
{return InitOnceExecuteOnce(&gui_once,initialize_gui,NULL,NULL);}

typedef struct theme_state {
    struct theme_state *next;HWND window;BOOL staged;
    WCHAR *application,*ids;
    unsigned calls,epoch;
    BOOL active,detached;
    const void *owner;
} theme_state;
static _Thread_local theme_state *staged;
static _Thread_local HWND forwarding_suppression;
typedef struct theme_intercept_frame {
    struct theme_intercept_frame *previous;
    HWND window;
    style_native_theme_fn native;
    BOOL root;
} theme_intercept_frame;
static _Thread_local theme_intercept_frame *intercepts;
static LRESULT CALLBACK theme_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static theme_state *state(HWND w)
{
    DWORD_PTR data;
    if(!theme_gui_ready())return NULL;
    if(gui.get(w,theme_proc,(UINT_PTR)theme_proc,&data))return (theme_state *)data;
    for(theme_state *s=staged;s;s=s->next)if(s->window==w)return s;
    return NULL;
}
static void release(theme_state *s)
{if(!--s->calls && s->detached){free(s->application);free(s->ids);free(s);}}
static LRESULT CALLBACK theme_proc(HWND w,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    theme_state *s=(theme_state *)data;LRESULT result;(void)id;++s->calls;
    if(m==WM_NCDESTROY)(void)style_theme_terminal(w,NULL);
    result=gui.next(w,m,p,l);
    release(s);return result;
}
BOOL style_theme_capture(HWND w)
{
    theme_state *s;
    if(!theme_gui_ready())return FALSE;
    if(gui.thread(w,NULL)!=GetCurrentThreadId()){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    if(state(w))return TRUE;
    s=calloc(1,sizeof(*s));if(!s){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    if(!gui.set(w,theme_proc,(UINT_PTR)theme_proc,(DWORD_PTR)s)){free(s);return FALSE;}
    return TRUE;
}
/* A framework may replace its InitWndProc during WM_NCCREATE. Do not put a
 * native subclass in that temporary chain just to record early theme calls. */
BOOL style_theme_stage(HWND w)
{
    if(!theme_gui_ready())return FALSE;
    if(gui.thread(w,NULL)!=GetCurrentThreadId()){SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;}
    if(state(w))return TRUE;
    theme_state *s=calloc(1,sizeof(*s));if(!s){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    s->staged=TRUE;s->window=w;s->next=staged;staged=s;return TRUE;
}
static void unstage(theme_state *s)
{
    theme_state **link=&staged;while(*link && *link!=s)link=&(*link)->next;
    if(*link)*link=s->next;
    s->next=NULL;s->staged=FALSE;
}
BOOL style_theme_commit(HWND w)
{
    theme_state *s=state(w);if(!s || !s->staged)return TRUE;
    if(!gui.set(w,theme_proc,(UINT_PTR)theme_proc,(DWORD_PTR)s))return FALSE;
    unstage(s);return TRUE;
}
void style_theme_cancel_stage(HWND w)
{
    theme_state *s=state(w);if(!s || !s->staged)return;
    unstage(s);++s->calls;s->detached=TRUE;release(s);
}
BOOL style_theme_terminal(HWND w,const void *owner)
{
    theme_state *s;
    s=state(w);if(!s || (owner && s->owner!=owner))return TRUE;
    if(s->staged)unstage(s);
    else if(!gui.remove(w,theme_proc,(UINT_PTR)theme_proc)){
        SetLastError(ERROR_INVALID_STATE);return FALSE;
    }
    ++s->calls;s->detached=TRUE;release(s);
    return TRUE;
}
typedef struct theme_suppress_scope {theme_state *state;HWND previous;BOOL succeeded;} theme_suppress_scope;
static void suppress_end(void *context)
{
    theme_suppress_scope *scope=context;forwarding_suppression=scope->previous;
    if(!scope->state->detached && !scope->succeeded)scope->state->active=FALSE;
    release(scope->state);
}
PROCESS_HOOK_UNWIND_FUNCTION BOOL style_theme_suppress(HWND w,const void *owner,style_native_theme_fn native_theme)
{
    theme_state *s=state(w);HRESULT result=E_FAIL;process_hook_unwind_scope unwind;
    if(!s){SetLastError(ERROR_INVALID_STATE);return FALSE;}
    ++s->calls;s->owner=owner;s->active=TRUE;
    theme_suppress_scope scope={s,forwarding_suppression,FALSE};forwarding_suppression=w;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,suppress_end,&scope);
    {result=native_theme(w,L"",L"");
     if(s->detached)result=HRESULT_FROM_WIN32(ERROR_CANCELLED);
     scope.succeeded=SUCCEEDED(result);}
    PROCESS_HOOK_UNWIND_END(unwind);
    if(FAILED(result)){SetLastError(HRESULT_CODE(result));return FALSE;}
    return TRUE;
}
BOOL style_theme_retained(HWND w)
{theme_state *s=state(w);return s && s->active;}
BOOL style_theme_owned(HWND w,const void *owner)
{theme_state *s=state(w);return owner && s && s->active && s->owner==owner;}
static void release_theme(void *context){release(context);}
PROCESS_HOOK_UNWIND_FUNCTION void style_theme_remove(HWND w,const void *owner,style_native_theme_fn native_theme)
{
    theme_state *s=state(w);process_hook_unwind_scope unwind;if(!s || s->owner!=owner)return;
    ++s->calls;if(s->staged)unstage(s);else gui.remove(w,theme_proc,(UINT_PTR)theme_proc);s->detached=TRUE;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,release_theme,s);
    {if(s->active)native_theme(w,s->application,s->ids);}
    PROCESS_HOOK_UNWIND_END(unwind);
}
typedef struct theme_set_scope {theme_state *state;WCHAR *application,*ids;} theme_set_scope;
static void set_end(void *context)
{
    theme_set_scope *scope=context;
    free(scope->application);free(scope->ids);release(scope->state);
}
PROCESS_HOOK_UNWIND_FUNCTION HRESULT style_theme_set(HWND w,LPCWSTR application,LPCWSTR ids,style_native_theme_fn native_theme)
{
    theme_state *s;WCHAR *app_copy=NULL,*id_copy=NULL;HRESULT result=E_FAIL;unsigned epoch;process_hook_unwind_scope unwind;
    if(!theme_gui_ready() || gui.thread(w,NULL)!=GetCurrentThreadId() || !(s=state(w)))
        return native_theme(w,application,ids);
    if(application && !(app_copy=_wcsdup(application)))return E_OUTOFMEMORY;
    if(ids && !(id_copy=_wcsdup(ids))){free(app_copy);return E_OUTOFMEMORY;}
    ++s->calls;epoch=++s->epoch;theme_set_scope scope={s,app_copy,id_copy};
    PROCESS_HOOK_UNWIND_BEGIN(unwind,set_end,&scope);
    {result=native_theme(w,s->active?L"":application,s->active?L"":ids);
     if(SUCCEEDED(result) && !s->detached && s->epoch==epoch){
        free(s->application);free(s->ids);s->application=scope.application;s->ids=scope.ids;
        scope.application=scope.ids=NULL;
     }}
    PROCESS_HOOK_UNWIND_END(unwind);return result;
}
static void intercept_end(void *context)
{theme_intercept_frame *frame=context;intercepts=frame->previous;}
PROCESS_HOOK_UNWIND_FUNCTION HRESULT style_theme_intercept(HWND w,LPCWSTR application,LPCWSTR ids,style_native_theme_fn native_theme)
{
    BOOL prior=FALSE,reentry=FALSE;
    for(theme_intercept_frame *p=intercepts;p;p=p->previous){
        if(p->window==w){
            prior=TRUE;
            if(p->native==native_theme){reentry=TRUE;break;}
            if(p->root)break;
        }
    }
    BOOL forwarded=(prior && !reentry) || (!prior && forwarding_suppression==w);
    if(!prior && forwarding_suppression==w)forwarding_suppression=NULL;
    theme_intercept_frame frame={intercepts,w,native_theme,!prior || reentry};process_hook_unwind_scope unwind;HRESULT result=E_FAIL;
    intercepts=&frame;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,intercept_end,&frame);
    {result=forwarded?native_theme(w,application,ids):style_theme_set(w,application,ids,native_theme);}
    PROCESS_HOOK_UNWIND_END(unwind);return result;
}
