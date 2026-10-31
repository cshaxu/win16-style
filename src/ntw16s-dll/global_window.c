/* Global-mode window attachment, confined to the target GUI thread.
 * This layer does not install desktop hooks or patch process entry points. */
#include "global_window.h"
#include "attachment.h"
#include <commctrl.h>
#include <stdlib.h>

typedef enum global_kind { GLOBAL_PENDING, GLOBAL_ATTACHMENT } global_kind;
typedef struct global_window { unsigned calls; BOOL detached, removing; global_kind kind; } global_window;
static LRESULT CALLBACK global_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);

static BOOL local(HWND w)
{
    DWORD process=0;DWORD thread=GetWindowThreadProcessId(w,&process);
    if(!thread || process!=GetCurrentProcessId() || thread!=GetCurrentThreadId()){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    return TRUE;
}
static global_window *lookup(HWND w)
{
    DWORD_PTR value=0;
    return GetWindowSubclass(w,global_proc,(UINT_PTR)global_proc,&value)?(global_window *)value:NULL;
}
static void release(global_window *s)
{if(!--s->calls && s->detached)free(s);}
static void detach(HWND w,global_window *s)
{
    if(s->detached)return;
    RemoveWindowSubclass(w,global_proc,(UINT_PTR)global_proc);
    s->detached=TRUE;
}
static LRESULT CALLBACK global_proc(HWND w,UINT message,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    global_window *s=(global_window *)data;LRESULT result;(void)id;
    ++s->calls;
    if(message==WM_NCDESTROY)detach(w,s);
    result=DefSubclassProc(w,message,a,b);
    release(s);return result;
}
BOOL ntw16s_global_window_attached(HWND w)
{
    global_window *s;if(!local(w))return FALSE;
    s=lookup(w);return s && !s->detached && s->kind!=GLOBAL_PENDING;
}
BOOL ntw16s_global_window_apply(HWND w)
{
    global_window *s;BOOL installed=FALSE;DWORD error;
    if(!local(w))return FALSE;
    if((s=lookup(w))!=NULL){if(!s->detached && s->kind!=GLOBAL_PENDING)return TRUE;SetLastError(ERROR_BUSY);return FALSE;}
    if(Ntw16sWindowAttached(w))return TRUE;
    s=(global_window *)calloc(1,sizeof(*s));if(!s){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    s->calls=1;
    if(!SetWindowSubclass(w,global_proc,(UINT_PTR)global_proc,(DWORD_PTR)s)){error=GetLastError();free(s);SetLastError(error);return FALSE;}
    installed=Ntw16sWindowAttach(w);
    if(installed)s->kind=GLOBAL_ATTACHMENT;
    error=installed?ERROR_SUCCESS:GetLastError();
    if(s->detached){installed=FALSE;error=ERROR_INVALID_WINDOW_HANDLE;}
    if(!installed && !s->detached)detach(w,s);
    release(s);SetLastError(error);return installed;
}
BOOL ntw16s_global_window_remove(HWND w)
{
    global_window *s;BOOL removed=FALSE;DWORD error;
    if(!local(w))return FALSE;
    s=lookup(w);if(!s || s->detached){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    if(s->removing || s->kind==GLOBAL_PENDING){SetLastError(ERROR_BUSY);return FALSE;}
    ++s->calls;s->removing=TRUE;
    if(s->kind==GLOBAL_ATTACHMENT)removed=Ntw16sWindowRemove(w);
    error=removed?ERROR_SUCCESS:GetLastError();
    if(removed && !s->detached)detach(w,s);
    if(!s->detached)s->removing=FALSE;
    release(s);SetLastError(error);return removed;
}
