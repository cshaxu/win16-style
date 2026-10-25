#include "theme.h"
#include "native_api.h"
#include <commctrl.h>
#include <uxtheme.h>
#include <stdlib.h>
#include <wchar.h>

typedef struct theme_state {
    struct theme_state *next;HWND window;BOOL staged;
    WCHAR *application,*ids;
    unsigned calls,epoch;
    BOOL active,detached;
    const void *owner;
} theme_state;
static _Thread_local theme_state *staged;
static LRESULT CALLBACK theme_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static theme_state *state(HWND w)
{
    DWORD_PTR data;
    if(GetWindowSubclass(w,theme_proc,(UINT_PTR)theme_proc,&data))return (theme_state *)data;
    for(theme_state *s=staged;s;s=s->next)if(s->window==w)return s;
    return NULL;
}
static void release(theme_state *s)
{if(!--s->calls && s->detached){free(s->application);free(s->ids);free(s);}}
static HRESULT native_theme(HWND w,LPCWSTR application,LPCWSTR ids)
{return ntw16s_native_set_window_theme(w,application,ids);}
static LRESULT CALLBACK theme_proc(HWND w,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    theme_state *s=(theme_state *)data;LRESULT result;(void)id;++s->calls;
    if(m==WM_NCDESTROY)(void)ntw16s_theme_terminal(w,NULL);
    result=DefSubclassProc(w,m,p,l);
    release(s);return result;
}
BOOL ntw16s_theme_capture(HWND w)
{
    theme_state *s;
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId()){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    if(state(w))return TRUE;
    s=calloc(1,sizeof(*s));if(!s){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    if(!SetWindowSubclass(w,theme_proc,(UINT_PTR)theme_proc,(DWORD_PTR)s)){free(s);return FALSE;}
    return TRUE;
}
/* A framework may replace its InitWndProc during WM_NCCREATE. Do not put a
 * native subclass in that temporary chain just to record early theme calls. */
BOOL ntw16s_theme_stage(HWND w)
{
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId()){SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;}
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
BOOL ntw16s_theme_commit(HWND w)
{
    theme_state *s=state(w);if(!s || !s->staged)return TRUE;
    if(!SetWindowSubclass(w,theme_proc,(UINT_PTR)theme_proc,(DWORD_PTR)s))return FALSE;
    unstage(s);return TRUE;
}
void ntw16s_theme_cancel_stage(HWND w)
{
    theme_state *s=state(w);if(!s || !s->staged)return;
    unstage(s);++s->calls;s->detached=TRUE;release(s);
}
BOOL ntw16s_theme_terminal(HWND w,const void *owner)
{
    theme_state *s;
    s=state(w);if(!s || (owner && s->owner!=owner))return TRUE;
    if(s->staged)unstage(s);
    else if(!RemoveWindowSubclass(w,theme_proc,(UINT_PTR)theme_proc)){
        SetLastError(ERROR_INVALID_STATE);return FALSE;
    }
    ++s->calls;s->detached=TRUE;release(s);
    return TRUE;
}
BOOL ntw16s_theme_enable(HWND w,const void *owner)
{
    theme_state *s=state(w);HRESULT result;
    if(!s){SetLastError(ERROR_INVALID_STATE);return FALSE;}
    ++s->calls;s->owner=owner;s->active=TRUE;result=native_theme(w,L"",L"");
    if(s->detached)result=HRESULT_FROM_WIN32(ERROR_CANCELLED);
    else if(FAILED(result))s->active=FALSE;
    release(s);
    if(FAILED(result)){SetLastError(HRESULT_CODE(result));return FALSE;}
    return TRUE;
}
BOOL ntw16s_theme_retained(HWND w)
{theme_state *s=state(w);return s && s->active;}
void ntw16s_theme_remove(HWND w,const void *owner)
{
    theme_state *s=state(w);if(!s || s->owner!=owner)return;
    ++s->calls;if(s->staged)unstage(s);else RemoveWindowSubclass(w,theme_proc,(UINT_PTR)theme_proc);s->detached=TRUE;
    if(s->active)native_theme(w,s->application,s->ids);
    release(s);
}
HRESULT __cdecl Ntw16sSetWindowTheme(HWND w,LPCWSTR application,LPCWSTR ids)
{
    theme_state *s;WCHAR *app_copy=NULL,*id_copy=NULL;HRESULT result;unsigned epoch;
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId() || !(s=state(w)))
        return native_theme(w,application,ids);
    if(application && !(app_copy=_wcsdup(application)))return E_OUTOFMEMORY;
    if(ids && !(id_copy=_wcsdup(ids))){free(app_copy);return E_OUTOFMEMORY;}
    ++s->calls;epoch=++s->epoch;
    result=native_theme(w,s->active?L"":application,s->active?L"":ids);
    if(SUCCEEDED(result) && !s->detached && s->epoch==epoch){
        free(s->application);free(s->ids);s->application=app_copy;s->ids=id_copy;
        app_copy=id_copy=NULL;
    }
    free(app_copy);free(id_copy);release(s);return result;
}
