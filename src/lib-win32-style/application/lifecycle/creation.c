#include "creation.h"
#include "creation_pending.h"
#include "../dialog/message_observer.h"
#include "../control/control_class.h"
#include <commctrl.h>
#include <stdlib.h>
#define class_is style_control_class_is

typedef win32_style_creation_pending pending_window;
struct style_creation_scope {
    struct style_creation_scope *previous;
    DWORD thread;
    BOOL enabled,owns_return_hook,suspended_return_hook;
    pending_window *pending;
    const style_creation_ops *ops;
    style_creation_result result;
};
static _Thread_local style_creation_scope *current;
static _Thread_local HHOOK creation_return_hook,creation_before_hook;
static _Thread_local void (*creation_dialog_observer)(HWND);
static LRESULT CALLBACK creation_return(int,WPARAM,LPARAM);
static LRESULT creation_observe(int,WPARAM,LPARAM,BOOL);
void (*style_creation_dialog_observer(void (*observer)(HWND)))(HWND)
{
    void (*previous)(HWND)=creation_dialog_observer;
    creation_dialog_observer=observer;return previous;
}
static LRESULT CALLBACK creation_subclass(HWND w,UINT message,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    (void)id;(void)data;
    LRESULT result=DefSubclassProc(w,message,a,b);
    if(message==WM_CREATE || message==WM_INITDIALOG || message==WM_SHOWWINDOW ||
        message==WM_WINDOWPOSCHANGED || message==WM_NCDESTROY){
        CWPRETSTRUCT returned={.lResult=result,.lParam=b,.wParam=a,.message=message,.hwnd=w};
        creation_observe(0,0,(LPARAM)&returned,TRUE);
    }
    if(message==WM_INITDIALOG && IsWindow(w) && creation_dialog_observer){
        DWORD error=GetLastError();creation_dialog_observer(w);SetLastError(error);
    }
    if(message==WM_NCDESTROY || message==WM_INITDIALOG ||
        (message==WM_CREATE && !style_control_class_is(w,L"#32770")))
        RemoveWindowSubclass(w,creation_subclass,(UINT_PTR)creation_subclass);
    return result;
}
static LRESULT CALLBACK creation_before(int code,WPARAM p,LPARAM l)
{
    style_creation_scope *scope=current;DWORD saved=GetLastError();
    if(code==HCBT_CREATEWND && scope && scope->enabled){
        const CBT_CREATEWNDW *create=(const CBT_CREATEWNDW *)l;
        pending_window *pending=win32_style_creation_add(&scope->pending,(HWND)p,(DWORD)create->lpcs->style);
        if(!pending){scope->result.failure=ERROR_NOT_ENOUGH_MEMORY;return 1;}
        if(scope->ops->subclass_dialogs && !creation_return_hook && !SetWindowSubclass((HWND)p,creation_subclass,(UINT_PTR)creation_subclass,0)){
            scope->result.failure=GetLastError()?GetLastError():ERROR_GEN_FAILURE;return 1;
        }

        if(!scope->ops->prepare((HWND)p,pending->style)){
            scope->result.failure=GetLastError();return 1;
        }
    }
    SetLastError(saved);return CallNextHookEx(NULL,code,p,l);
}
static LRESULT creation_observe(int code,WPARAM p,LPARAM l,BOOL synthetic)
{
    LRESULT result=synthetic?0:CallNextHookEx(NULL,code,p,l);DWORD saved=GetLastError();
    style_creation_scope *scope=current;
    if(code>=0 && scope && scope->enabled){
        const CWPRETSTRUCT *m=(const CWPRETSTRUCT *)l;
        BOOL created=m->message==WM_CREATE && m->lResult!=-1;
        /* Dialog managers finish layout and format their caption during
         * WM_INITDIALOG. Attach after that native initialization (ShellAbout
         * also formats its caption there); never rewrite application text.
         * Keep its pending style until the real dialog initialization exits. */
        if(created && class_is(m->hwnd,L"#32770")){
            SetLastError(saved);return result;
        }
        if(m->message==WM_NCDESTROY){pending_window *pending=win32_style_creation_take(&scope->pending,m->hwnd);scope->ops->cancel(m->hwnd);free(pending);}
        /* Some dialogs show themselves and run a nested loop inside
         * WM_INITDIALOG. Its return may be delayed until the application exits.
         * A completed first-show notification is also a safe boundary, but
         * only for a captioned top-level dialog owned by this creation chain.
         * Mark its pending node before installation can reenter USER. */
        BOOL first_show=(m->message==WM_SHOWWINDOW && m->wParam) ||
            (m->message==WM_WINDOWPOSCHANGED && m->lParam &&
             (((const WINDOWPOS *)m->lParam)->flags&SWP_SHOWWINDOW));
        style_creation_scope *owner=scope;
        if(first_show){
            DWORD style=(DWORD)GetWindowLongPtrW(m->hwnd,GWL_STYLE);
            first_show=(style&WS_VISIBLE) && !(style&WS_CHILD) &&
                (style&WS_CAPTION)==WS_CAPTION && class_is(m->hwnd,L"#32770");
            if(first_show){
                for(;owner;owner=owner->previous){
                    pending_window *pending=win32_style_creation_find(owner->pending,m->hwnd);
                    if(owner->enabled && pending && !pending->settled)break;
                }
                first_show=owner!=NULL;
            }
        }
        if((created || m->message==WM_INITDIALOG || first_show) && IsWindow(m->hwnd)){
            if(!first_show)owner=scope;
            pending_window *pending;
            if(first_show){
                pending=win32_style_creation_find(owner->pending,m->hwnd);
                if(pending)pending->settled=TRUE;
            }else pending=win32_style_creation_take(&owner->pending,m->hwnd);
            /* A first-show attachment has already completed this creation.
             * In particular, do not undo an explicit removal by attaching
             * again when the delayed initializer finally returns. */
            if(!first_show && pending && pending->settled && pending->installed){free(pending);SetLastError(saved);return result;}
            DWORD style=created?(DWORD)((const CREATESTRUCTW *)m->lParam)->style:
                (DWORD)GetWindowLongPtrW(m->hwnd,GWL_STYLE);
            if(pending){style=pending->style;if(!first_show)free(pending);}
            BOOL already=owner->ops->attached(m->hwnd);
            BOOL installed=owner->ops->install(m->hwnd,style);
            if(installed){
                if(first_show && win32_style_creation_contains(owner->pending,pending,m->hwnd))pending->installed=TRUE;
                if(!already && owner->ops->attached(m->hwnd))++owner->result.installed;
            }else{
                DWORD error=GetLastError();
                if(error==ERROR_NOT_SUPPORTED){
                    ++owner->result.unsupported;owner->ops->unsupported(m->hwnd);
                }
                else if(!owner->result.failure)owner->result.failure=error?error:ERROR_GEN_FAILURE;
            }
            if(owner->result.failure && m->message==WM_INITDIALOG)
                EndDialog(m->hwnd,-1);
        }
    }
    SetLastError(saved);return result;
}
static LRESULT CALLBACK creation_return(int code,WPARAM p,LPARAM l)
{
    LRESULT result=creation_observe(code,p,l,FALSE);
    if(code>=0 && l && ((const CWPRETSTRUCT *)l)->message==WM_INITDIALOG && creation_dialog_observer){
        HWND w=((const CWPRETSTRUCT *)l)->hwnd;DWORD error=GetLastError();
        if(IsWindow(w))creation_dialog_observer(w);
        SetLastError(error);
    }
    return result;
}
style_creation_scope *style_creation_begin(BOOL enabled,BOOL dialog,const style_creation_ops *ops)
{
    style_creation_scope *scope=calloc(1,sizeof(*scope));
    if(!scope){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return NULL;}
    if(!current){
        creation_before_hook=SetWindowsHookExW(WH_CBT,creation_before,NULL,GetCurrentThreadId());
        if(!creation_before_hook){free(scope);return NULL;}
        if(!ops->subclass_dialogs){
            creation_return_hook=style_message_observer_begin(creation_return);
            if(!creation_return_hook){DWORD error=GetLastError();UnhookWindowsHookEx(creation_before_hook);creation_before_hook=NULL;free(scope);SetLastError(error);return NULL;}
        }
    }
    if(ops->subclass_dialogs){
        if(dialog && creation_return_hook){
            if(!UnhookWindowsHookEx(creation_return_hook)){DWORD error=GetLastError();free(scope);SetLastError(error);return NULL;}
            creation_return_hook=NULL;scope->suspended_return_hook=TRUE;
        }
        if(!dialog && !creation_return_hook){
            creation_return_hook=style_message_observer_begin(creation_return);
            if(!creation_return_hook){DWORD error=GetLastError();if(!current){UnhookWindowsHookEx(creation_before_hook);creation_before_hook=NULL;}free(scope);SetLastError(error);return NULL;}
            scope->owns_return_hook=TRUE;
        }
    }
    scope->thread=GetCurrentThreadId();scope->enabled=enabled;scope->ops=ops;
    scope->previous=current;current=scope;return scope;
}
BOOL style_creation_end(style_creation_scope *scope,style_creation_result *result)
{
    if(!scope || scope!=current){SetLastError(ERROR_INVALID_STATE);return FALSE;}
    if(scope->thread!=GetCurrentThreadId()){SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;}
    current=scope->previous;if(result)*result=scope->result;
    while(scope->pending){pending_window *pending=scope->pending;scope->pending=pending->next;
        if(scope->ops->subclass_dialogs && IsWindow(pending->window))RemoveWindowSubclass(pending->window,creation_subclass,(UINT_PTR)creation_subclass);
        scope->ops->cancel(pending->window);free(pending);}
    if(scope->ops->subclass_dialogs){
        if(scope->owns_return_hook){style_message_observer_end(creation_return_hook);creation_return_hook=NULL;}
        if(scope->suspended_return_hook){
            creation_return_hook=style_message_observer_begin(creation_return);
            if(!creation_return_hook){DWORD error=GetLastError()?GetLastError():ERROR_GEN_FAILURE;
                if(result && !result->failure)result->failure=error;
                if(current && !current->result.failure)current->result.failure=error;}
        }
    }
    if(!current){style_message_observer_end(creation_return_hook);creation_return_hook=NULL;
        UnhookWindowsHookEx(creation_before_hook);creation_before_hook=NULL;}
    free(scope);return TRUE;
}
