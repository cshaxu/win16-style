#include "vb_text.h"
#include "control_class.h"
#include "ntw16s.h"
#include "native_api.h"
#include "theme.h"
#include <commctrl.h>
#include <stdlib.h>

/* This coordinator composes the one existing renderer. It observes only
 * creation inside explicit caller scopes; it never scans a desktop or infers
 * task policy from an owner, title, executable name or a process-wide switch. */
typedef struct pending_window {struct pending_window *next;HWND window;DWORD style;BOOL settled,installed;} pending_window;
struct ntw16s_creation_scope {
    struct ntw16s_creation_scope *previous;
    DWORD thread;
    BOOL enabled;
    pending_window *pending;
    ntw16s_creation_result result;
};
typedef struct attachment {
    unsigned calls;
    BOOL detached,tree;
    HWND themed,report_window,report_parent;
    HMODULE report_module;
    ntw16s_vb_fonts fonts;
    ntw16s_control_protocol *protocol;
} attachment;
static _Thread_local ntw16s_creation_scope *current;
static _Thread_local HHOOK creation_return_hook;
static _Thread_local HHOOK creation_before_hook;
static LRESULT CALLBACK attachment_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static void report_try_bind(HWND,attachment *);

static attachment *lookup(HWND w)
{
    DWORD_PTR value;
    return GetWindowSubclass(w,attachment_proc,(UINT_PTR)attachment_proc,&value)?(attachment *)value:NULL;
}
#define class_is ntw16s_class_is
static void release(attachment *a)
{if(!--a->calls && a->detached){ntw16s_vb_fonts_clear(&a->fonts);free(a);}}
static LRESULT CALLBACK attachment_proc(HWND w,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    attachment *a=(attachment *)data;LRESULT result;(void)id;
    ++a->calls;
    if(m==WM_NCDESTROY)(void)Ntw16sWindowTerminal(w);
    ntw16s_vb_paint paint;ntw16s_vb_paint_begin(&paint,w,m,&a->fonts);
    if(paint.active && !a->detached && !a->protocol)report_try_bind(w,a);
    if(paint.active && a->report_module && !a->detached){
        DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);
        if((a->tree || ((style&LVS_TYPEMASK)==LVS_REPORT && !(style&(LVS_OWNERDRAWFIXED|LVS_OWNERDATA)) &&
            !SendMessageW(w,LVM_ISGROUPVIEWENABLED,0,0))) && !a->detached)
            ntw16s_report_paint_enable(&paint,w,a->report_module,&a->detached);
    }
    result=DefSubclassProc(w,m,p,l);
    ntw16s_vb_paint_end(&paint);
    release(a);return result;
}
/* Observe only this list's native custom-draw DC. Never request extra stages,
 * rewrite the application's result, select its font, or inspect item lParam. */
static LRESULT CALLBACK report_notify(HWND w,UINT m,WPARAM p,LPARAM l,UINT_PTR id,DWORD_PTR data)
{
    DWORD entry=GetLastError();attachment *a=(attachment *)data;BOOL draw=FALSE;HWND list=a->report_window;HDC dc=NULL;DWORD stage=0;
    ++a->calls;
    if(m==WM_NOTIFY && l){
        NMHDR *header=(NMHDR *)l;
        if(header->hwndFrom==list && header->code==NM_CUSTOMDRAW){
            NMCUSTOMDRAW *cd=(NMCUSTOMDRAW *)l;dc=cd->hdc;stage=cd->dwDrawStage;draw=TRUE;
            ntw16s_report_paint_bind(list,NULL);
        }
    }
    if(m==WM_NCDESTROY){RemoveWindowSubclass(w,report_notify,id);a->report_parent=NULL;}
    SetLastError(entry);LRESULT result=DefSubclassProc(w,m,p,l);DWORD error=GetLastError();
    if(draw && !a->detached && !(result&CDRF_SKIPDEFAULT) &&
        (stage==CDDS_PREPAINT || stage==CDDS_ITEMPREPAINT || stage==(CDDS_ITEMPREPAINT|CDDS_SUBITEM)))
        ntw16s_report_paint_bind(list,dc);
    release(a);SetLastError(error);return result;
}
/* Some frameworks set the report view after native WM_CREATE. This is still
 * our exact newly created attachment, on its GUI thread, after initialization.
 * A failed public protocol remains native; there is no existing-window scan. */
static void report_try_bind(HWND w,attachment *a)
{
    WCHAR name[64];
    if(!GetClassNameW(w,name,ARRAYSIZE(name)) ||
        (lstrcmpW(name,WC_LISTVIEWW) && lstrcmpW(name,L"TListView") &&
         lstrcmpW(name,WC_TREEVIEWW) && lstrcmpW(name,L"TTreeView") && lstrcmpW(name,L"TShellTreeView")))return;
    ntw16s_control_protocol *protocol=ntw16s_control_protocol_bind(w);
    if(!protocol)return;
    HMODULE module=ntw16s_control_protocol_list_module(w);
    BOOL tree=ntw16s_control_protocol_tree_module(w)!=NULL;if(tree)module=ntw16s_control_protocol_tree_module(w);
    HWND parent=GetParent(w);
    if(!a->detached && module && parent &&
        SetWindowSubclass(parent,report_notify,(UINT_PTR)w,(DWORD_PTR)a)){
        a->protocol=protocol;a->report_window=w;a->report_parent=parent;a->report_module=module;a->tree=tree;
    }else ntw16s_control_protocol_release(protocol);
}
static void report_unbind(attachment *a)
{
    DWORD_PTR value;
    if(a->report_parent && GetWindowSubclass(a->report_parent,report_notify,(UINT_PTR)a->report_window,&value) &&
        value==(DWORD_PTR)a)RemoveWindowSubclass(a->report_parent,report_notify,(UINT_PTR)a->report_window);
    a->report_parent=NULL;
}
BOOL __cdecl Ntw16sWindowAttached(HWND w)
{
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId()){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    return lookup(w)!=NULL;
}
void __cdecl Ntw16sScrollChanged(HWND w)
{
    if(GetWindowThreadProcessId(w,NULL)==GetCurrentThreadId() && lookup(w)){
        if(class_is(w,L"SCROLLBAR"))RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
        else win16_style_scrollbars_paint(w,NULL,NULL);
    }
}
BOOL __cdecl Ntw16sWindowRemove(HWND w)
{
    attachment *a;
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId()){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    a=lookup(w);if(!a){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    ++a->calls;
    RemoveWindowSubclass(w,attachment_proc,(UINT_PTR)attachment_proc);
    a->detached=TRUE;report_unbind(a);
    /* Existing owners handle reentrant destruction/removal independently. */
    win16_style_frame_remove_owned(w,a);
    win16_style_control_remove_owned(w,a);
    if(a->themed)ntw16s_theme_remove(a->themed,a);
    ntw16s_theme_remove(w,NULL);
    ntw16s_control_protocol_release(a->protocol);a->protocol=NULL;
    release(a);return TRUE;
}
BOOL __cdecl Ntw16sWindowTerminal(HWND w)
{
    attachment *a;
    DWORD saved=GetLastError(),error=ERROR_SUCCESS;
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId()){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    a=lookup(w);
    if(a){
        ++a->calls;
        if(RemoveWindowSubclass(w,attachment_proc,(UINT_PTR)attachment_proc))a->detached=TRUE;
        else error=ERROR_INVALID_STATE;
    }
    /* These leaves only retire our exact nodes. They do not restore native
     * appearance, send application messages, or call a saved procedure. */
    if(!win16_style_frame_terminal(w) && !error)error=GetLastError();
    if(!win16_style_control_terminal(w) && !error)error=GetLastError();
    if(a && a->themed && a->themed!=w &&
            !ntw16s_theme_terminal(a->themed,a) && !error)error=GetLastError();
    if(!ntw16s_theme_terminal(w,NULL) && !error)error=GetLastError();
    if(a){report_unbind(a);ntw16s_control_protocol_release(a->protocol);a->protocol=NULL;release(a);}
    SetLastError(error?error:saved);return !error;
}

static BOOL install_created(HWND w,DWORD original_style)
{
    BOOL control,edit,installed,report=FALSE,tree=FALSE;HWND parent;attachment *a;DWORD error;
    ntw16s_control_protocol *protocol=NULL;
    if(lookup(w))return TRUE;
    /* COMBOBOX installs its own internal list when construction finishes.
     * The internal EDIT also belongs to that native composite, not a frame. */
    for(parent=GetParent(w);parent;parent=GetParent(parent))
        if(class_is(parent,L"COMBOBOX"))return TRUE;
    if(class_is(w,L"ComboLBox") || class_is(w,L"#32768"))return TRUE;
    edit=(original_style&WS_CHILD) && class_is(w,L"EDIT");
    control=(original_style&WS_CHILD) && (edit || class_is(w,L"BUTTON") || class_is(w,L"STATIC") ||
        class_is(w,L"LISTBOX") || class_is(w,L"COMBOBOX") || class_is(w,L"SCROLLBAR") || class_is(w,L"SysLink"));
    if(!control && (original_style&WS_CHILD)){
        SetLastError(ERROR_SUCCESS);
        protocol=ntw16s_control_protocol_bind(w);
        if(!protocol && GetLastError()==ERROR_NOT_ENOUGH_MEMORY)return FALSE;
        report=protocol && ntw16s_control_protocol_list_module(w);
        tree=protocol && ntw16s_control_protocol_tree_module(w);
        control=protocol && !report;
    }
    /* A custom client-only child and MDICLIENT keep application painting.
     * MDI child frames have WS_CAPTION and take the ordinary frame route. */
    if(!control && !report && (original_style&WS_CHILD) && !(original_style&(WS_CAPTION|WS_THICKFRAME))){
        SetLastError(ERROR_NOT_SUPPORTED);return FALSE;
    }
    a=calloc(1,sizeof(*a));
    if(!a){ntw16s_control_protocol_release(protocol);SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    a->calls=1;a->protocol=protocol;
    if(report || tree){a->report_window=w;a->tree=tree;
        a->report_module=tree?ntw16s_control_protocol_tree_module(w):ntw16s_control_protocol_list_module(w);a->report_parent=GetParent(w);}
    if(!SetWindowSubclass(w,attachment_proc,(UINT_PTR)attachment_proc,(DWORD_PTR)a)){
        ntw16s_control_protocol_release(protocol);free(a);return FALSE;
    }
    if((report || tree) && (!a->report_parent || !SetWindowSubclass(a->report_parent,report_notify,(UINT_PTR)w,(DWORD_PTR)a))){
        DWORD error=GetLastError();Ntw16sWindowRemove(w);release(a);SetLastError(error);return FALSE;
    }
    SetLastError(ERROR_SUCCESS);
    if(protocol && class_is(w,L"COMBOBOX")){
        COMBOBOXINFO info={.cbSize=sizeof(info)};
        if(ntw16s_native_get_combo_info(w,&info) && info.hwndItem!=w && lookup(info.hwndItem))
            Ntw16sWindowRemove(info.hwndItem);
    }
    if(a->detached){release(a);SetLastError(ERROR_CANCELLED);return FALSE;}
    if(tree)installed=win16_style_tree_install_created(w,NULL,a,&a->detached);
    else if(control)installed=win16_style_control_install_created(w,edit,(original_style&WS_BORDER)!=0,
        class_is(w,L"LISTBOX") && (original_style&LBS_USETABSTOPS),NULL,a,&a->detached);
    else if(report && !(original_style&(WS_BORDER|WS_THICKFRAME|WS_CAPTION)))installed=TRUE;
    else installed=win16_style_install_created(w,NULL,a,&a->detached);
    error=GetLastError();
    if(installed && !a->detached){
        if(control){
            HWND themed=NULL;BOOL list=FALSE;
            if(class_is(w,L"COMBOBOX")){
                COMBOBOXINFO info={.cbSize=sizeof(info)};
                if(ntw16s_native_get_combo_info(w,&info)){themed=info.hwndList;list=TRUE;}
            }else if(class_is(w,L"LISTBOX")){themed=w;list=TRUE;}
            else if(edit && (original_style&ES_MULTILINE))themed=w;
            if(themed){
                a->themed=themed;
                if(!ntw16s_theme_enable(themed,a))installed=FALSE;
                else {
                    installed=list?win16_style_list_scrollbar_enable(themed):win16_style_edit_scrollbars_enable(themed);}
                error=GetLastError();
            }
        }
    }
    if(installed && !a->detached && a->themed!=w)ntw16s_theme_remove(w,NULL);
    if(a->detached){installed=FALSE;error=ERROR_CANCELLED;}
    if(!installed && !a->detached)Ntw16sWindowRemove(w);
    release(a);
    if(!installed)SetLastError(error?error:ERROR_GEN_FAILURE);
    return installed;
}
static LRESULT CALLBACK creation_before(int code,WPARAM p,LPARAM l)
{
    ntw16s_creation_scope *scope=current;DWORD saved=GetLastError();
    if(code==HCBT_CREATEWND && scope && scope->enabled){
        const CBT_CREATEWNDW *create=(const CBT_CREATEWNDW *)l;
        pending_window *pending=calloc(1,sizeof(*pending));
        if(!pending){scope->result.failure=ERROR_NOT_ENOUGH_MEMORY;return 1;}
        pending->window=(HWND)p;pending->style=(DWORD)create->lpcs->style;
        pending->next=scope->pending;scope->pending=pending;
        if(class_is((HWND)p,L"LISTBOX") || class_is((HWND)p,L"ComboLBox") ||
           (class_is((HWND)p,L"EDIT") && (pending->style&ES_MULTILINE))){
            if(!ntw16s_theme_capture((HWND)p)){
                scope->result.failure=GetLastError();return 1;
            }
        }else if((pending->style&WS_CHILD) && !ntw16s_theme_stage((HWND)p)){
            scope->result.failure=GetLastError();return 1;
        }
    }
    SetLastError(saved);return CallNextHookEx(NULL,code,p,l);
}
static pending_window *take_pending(ntw16s_creation_scope *scope,HWND w)
{
    pending_window **link=&scope->pending,*found;
    while(*link && (*link)->window!=w)link=&(*link)->next;
    found=*link;if(found)*link=found->next;return found;
}
static LRESULT CALLBACK creation_return(int code,WPARAM p,LPARAM l)
{
    LRESULT result=CallNextHookEx(NULL,code,p,l);DWORD saved=GetLastError();
    ntw16s_creation_scope *scope=current;
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
        if(m->message==WM_NCDESTROY){pending_window *pending=take_pending(scope,m->hwnd);ntw16s_theme_cancel_stage(m->hwnd);free(pending);}
        /* Some dialogs show themselves and run a nested loop inside
         * WM_INITDIALOG. Its return may be delayed until the application exits.
         * A completed first-show notification is also a safe boundary, but
         * only for a captioned top-level dialog owned by this creation chain.
         * Mark its pending node before installation can reenter USER. */
        BOOL first_show=(m->message==WM_SHOWWINDOW && m->wParam) ||
            (m->message==WM_WINDOWPOSCHANGED && m->lParam &&
             (((const WINDOWPOS *)m->lParam)->flags&SWP_SHOWWINDOW));
        ntw16s_creation_scope *owner=scope;
        if(first_show){
            DWORD style=(DWORD)GetWindowLongPtrW(m->hwnd,GWL_STYLE);
            first_show=(style&WS_VISIBLE) && !(style&WS_CHILD) &&
                (style&WS_CAPTION)==WS_CAPTION && class_is(m->hwnd,L"#32770");
            if(first_show){
                for(;owner;owner=owner->previous){
                    pending_window *pending=owner->pending;
                    while(pending && pending->window!=m->hwnd)pending=pending->next;
                    if(owner->enabled && pending && !pending->settled)break;
                }
                first_show=owner!=NULL;
            }
        }
        if((created || m->message==WM_INITDIALOG || first_show) && IsWindow(m->hwnd)){
            if(!first_show)owner=scope;
            pending_window *pending;
            if(first_show){
                pending=owner->pending;
                while(pending && pending->window!=m->hwnd)pending=pending->next;
                if(pending)pending->settled=TRUE;
            }else pending=take_pending(owner,m->hwnd);
            /* A first-show attachment has already completed this creation.
             * In particular, do not undo an explicit removal by attaching
             * again when the delayed initializer finally returns. */
            if(!first_show && pending && pending->settled && pending->installed){free(pending);SetLastError(saved);return result;}
            DWORD style=created?(DWORD)((const CREATESTRUCTW *)m->lParam)->style:
                (DWORD)GetWindowLongPtrW(m->hwnd,GWL_STYLE);
            if(pending){style=pending->style;if(!first_show)free(pending);}
            BOOL already=lookup(m->hwnd)!=NULL;
            BOOL installed=ntw16s_theme_commit(m->hwnd) && install_created(m->hwnd,style);
            if(installed){
                if(first_show){
                    for(pending_window *record=owner->pending;record;record=record->next)
                        if(record==pending && record->window==m->hwnd){record->installed=TRUE;break;}
                }
                if(!already && lookup(m->hwnd))++owner->result.installed;
            }else{
                DWORD error=GetLastError();
                if(error==ERROR_NOT_SUPPORTED){
                    ++owner->result.unsupported;ntw16s_theme_remove(m->hwnd,NULL);
                    if(class_is(m->hwnd,L"COMBOBOX")){
                        COMBOBOXINFO info={.cbSize=sizeof(info)};
                        if(ntw16s_native_get_combo_info(m->hwnd,&info))ntw16s_theme_remove(info.hwndList,NULL);
                    }
                }
                else if(!owner->result.failure)owner->result.failure=error?error:ERROR_GEN_FAILURE;
            }
            if(owner->result.failure && m->message==WM_INITDIALOG)
                EndDialog(m->hwnd,-1);
        }
    }
    SetLastError(saved);return result;
}
ntw16s_creation_scope *__cdecl Ntw16sCreationBegin(BOOL enabled)
{
    ntw16s_creation_scope *scope=calloc(1,sizeof(*scope));
    if(!scope){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return NULL;}
    if(!current){
        creation_before_hook=SetWindowsHookExW(WH_CBT,creation_before,NULL,GetCurrentThreadId());
        if(!creation_before_hook){free(scope);return NULL;}
        creation_return_hook=SetWindowsHookExW(WH_CALLWNDPROCRET,creation_return,NULL,GetCurrentThreadId());
        if(!creation_return_hook){DWORD error=GetLastError();UnhookWindowsHookEx(creation_before_hook);creation_before_hook=NULL;free(scope);SetLastError(error);return NULL;}
    }
    scope->thread=GetCurrentThreadId();scope->enabled=enabled;
    scope->previous=current;current=scope;return scope;
}
BOOL __cdecl Ntw16sCreationEnd(ntw16s_creation_scope *scope,ntw16s_creation_result *result)
{
    if(!scope || scope!=current){SetLastError(ERROR_INVALID_STATE);return FALSE;}
    if(scope->thread!=GetCurrentThreadId()){SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;}
    current=scope->previous;if(result)*result=scope->result;
    while(scope->pending){pending_window *pending=scope->pending;scope->pending=pending->next;ntw16s_theme_cancel_stage(pending->window);free(pending);}
    if(!current){UnhookWindowsHookEx(creation_return_hook);creation_return_hook=NULL;
        UnhookWindowsHookEx(creation_before_hook);creation_before_hook=NULL;}
    free(scope);return TRUE;
}
