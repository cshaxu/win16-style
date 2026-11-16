#include "popup.h"
#include <commctrl.h>
#include <stdlib.h>
#include <wchar.h>

/* The shared runtime also serves console launchers. Resolve GUI services on
 * first use, never in DllMain and without eager GUI imports. */
typedef struct popup_api {
    __typeof__(&GetWindowThreadProcessId) thread;
    __typeof__(&SetWindowsHookExW) hook;
    __typeof__(&UnhookWindowsHookEx) unhook;
    __typeof__(&CallNextHookEx) chain;
    __typeof__(&GetWindowSubclass) get;
    __typeof__(&SetWindowSubclass) set;
    __typeof__(&RemoveWindowSubclass) remove;
    __typeof__(&DefSubclassProc) next;
    __typeof__(&GetClassNameW) class_name;
    __typeof__(&GetWindowRect) rect;
    __typeof__(&GetWindow) window;
    __typeof__(&IsWindowVisible) visible;
    __typeof__(&ShowWindow) show;
    __typeof__(&SetWindowPos) position;
    __typeof__(&RedrawWindow) redraw;
} popup_api;
static popup_api gui;
static INIT_ONCE once=INIT_ONCE_STATIC_INIT;
static BOOL CALLBACK initialize(PINIT_ONCE init,void *argument,void **context)
{
    (void)init;(void)argument;(void)context;
    HMODULE user=LoadLibraryW(L"user32.dll"),controls=LoadLibraryW(L"comctl32.dll");
    if(user && controls){
#define USER(field,name) gui.field=(void *)GetProcAddress(user,#name)
#define CONTROL(field,name) gui.field=(void *)GetProcAddress(controls,name)
        USER(thread,GetWindowThreadProcessId);USER(hook,SetWindowsHookExW);
        USER(unhook,UnhookWindowsHookEx);USER(chain,CallNextHookEx);
        CONTROL(get,MAKEINTRESOURCEA(411));CONTROL(set,"SetWindowSubclass");
        CONTROL(remove,"RemoveWindowSubclass");CONTROL(next,"DefSubclassProc");
        USER(class_name,GetClassNameW);USER(rect,GetWindowRect);USER(window,GetWindow);
        USER(visible,IsWindowVisible);USER(show,ShowWindow);USER(position,SetWindowPos);
        USER(redraw,RedrawWindow);
#undef USER
#undef CONTROL
        if(gui.thread && gui.hook && gui.unhook && gui.chain && gui.get && gui.set &&
           gui.remove && gui.next && gui.class_name && gui.rect && gui.window &&
           gui.visible && gui.show && gui.position && gui.redraw)return TRUE;
    }
    if(controls)FreeLibrary(controls);
    if(user)FreeLibrary(user);
    ZeroMemory(&gui,sizeof(gui));SetLastError(ERROR_PROC_NOT_FOUND);return FALSE;
}
struct style_menu_popup_scope {
    HWND owner;
    const style_menu_popup_ops *ops;
    void *context;
    struct style_menu_popup_scope *previous;
    style_menu_popup *windows;
    unsigned references;
    BOOL detached;
};
struct style_menu_popup {
    HWND window,shadow;
    style_menu_popup_scope *scope;
    style_menu_popup *next;
    unsigned calls;
    ULONGLONG selection;
    BOOL detached,painting,painted,shadow_visible;
};
static _Thread_local style_menu_popup_scope *current;
static _Thread_local HHOOK observer;
static LRESULT CALLBACK popup_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static LRESULT CALLBACK shadow_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static void scope_release(style_menu_popup_scope *s)
{if(!--s->references){if(s->ops->dispose)s->ops->dispose(s->context);free(s);}}
static void popup_release(style_menu_popup *p)
{if(!--p->calls && p->detached){scope_release(p->scope);free(p);}}
HWND style_menu_popup_window(const style_menu_popup *p){return p->window;}
void *style_menu_popup_context(const style_menu_popup *p){return p->scope->context;}
BOOL style_menu_popup_live(const style_menu_popup *p){return !p->detached && !p->scope->detached;}
static void detach(style_menu_popup *p,BOOL terminal)
{
    if(p->detached)return;
    ++p->calls;
    style_menu_popup **link=&p->scope->windows;
    while(*link && *link!=p)link=&(*link)->next;
    if(*link)*link=p->next;
    p->detached=TRUE;
    gui.remove(p->window,popup_proc,(UINT_PTR)popup_proc);
    if(p->shadow){HWND shadow=p->shadow;p->shadow=NULL;
        gui.remove(shadow,shadow_proc,(UINT_PTR)shadow_proc);
        if(!terminal && p->shadow_visible && gui.visible(p->window))gui.show(shadow,SW_SHOWNOACTIVATE);
    }
    popup_release(p);
}
static void paint(style_menu_popup *p,HDC dc,BOOL window_coordinates)
{
    const style_menu_popup_ops *ops=p->scope->ops;
    if(!style_menu_popup_live(p) || p->painting || !ops->paint)return;
    p->painting=TRUE;
    BOOL painted=ops->paint(p,dc,window_coordinates);
    if(painted && !dc && style_menu_popup_live(p)){
        p->painted=ops->selection && ops->selection(p,&p->selection);
    }
    p->painting=FALSE;
}
static LRESULT CALLBACK popup_proc(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    style_menu_popup *p=(style_menu_popup *)data;LRESULT result=0;(void)id;++p->calls;
    const style_menu_popup_ops *ops=p->scope->ops;
    BOOL mouse=m==WM_MOUSEMOVE || m==WM_NCMOUSEMOVE;
    ULONGLONG before=0,after=0;
    BOOL observed=mouse && ops->selection && ops->selection(p,&before);
    if(m==WM_NCDESTROY){detach(p,TRUE);result=gui.next(w,m,a,b);}
    else if(m==WM_NCPAINT && ops->border){if(style_menu_popup_live(p))ops->border(p,NULL);}
    else {
        result=gui.next(w,m,a,b);
        if(style_menu_popup_live(p)){
            if(m==WM_PRINT){
                if((b&PRF_NONCLIENT) && ops->border)ops->border(p,(HDC)a);
                if(b&PRF_CLIENT)paint(p,(HDC)a,TRUE);
            }else if(m==WM_PAINT){if(ops->border)ops->border(p,NULL);paint(p,NULL,FALSE);}
            else {
                if(m==WM_WINDOWPOSCHANGED && b && ops->border && (((WINDOWPOS *)b)->flags&SWP_SHOWWINDOW))
                    gui.redraw(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
                if(m==WM_WINDOWPOSCHANGED || (ops->border && m==0x92) ||
                   (ops->keyboard_refresh && m==WM_KEYDOWN) ||
                   (mouse && (!observed || !ops->selection || !ops->selection(p,&after) || before!=after)))paint(p,NULL,FALSE);
            }
        }
    }
    popup_release(p);return result;
}
static LRESULT CALLBACK shadow_proc(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    style_menu_popup *p=(style_menu_popup *)data;(void)id;++p->calls;
    if(m==WM_NCDESTROY){p->shadow=NULL;gui.remove(w,shadow_proc,(UINT_PTR)shadow_proc);}
    else if(m==WM_WINDOWPOSCHANGING && style_menu_popup_live(p)){
        WINDOWPOS *position=(WINDOWPOS *)b;
        position->flags=(position->flags&~SWP_SHOWWINDOW)|SWP_HIDEWINDOW|SWP_NOACTIVATE;
    }
    LRESULT result=gui.next(w,m,a,b);popup_release(p);return result;
}
static void suppress_shadow(HWND w,UINT message,LPARAM parameter)
{
    RECT shadow;if(!gui.rect(w,&shadow))return;
    for(style_menu_popup_scope *s=current;s;s=s->previous)if(s->ops->suppress_shadow)
        for(style_menu_popup *p=s->windows;p;p=p->next){RECT menu;DWORD_PTR existing;
            /* SysShadow has no owner: require native z-order and bounded
             * matching geometry on this observer's GUI thread. */
            if(gui.window(w,GW_HWNDPREV)!=p->window || !gui.rect(p->window,&menu) ||
               shadow.left!=menu.left || shadow.top!=menu.top || shadow.right<=menu.right || shadow.bottom<=menu.bottom ||
               shadow.right-menu.right>32 || shadow.bottom-menu.bottom>32)continue;
            if(gui.get(w,shadow_proc,(UINT_PTR)shadow_proc,&existing)){
                if((style_menu_popup *)existing==p)return;
                ((style_menu_popup *)existing)->shadow=NULL;gui.remove(w,shadow_proc,(UINT_PTR)shadow_proc);
            }
            if(p->shadow && p->shadow!=w)continue;
            p->shadow_visible=gui.visible(w) || (message==WM_WINDOWPOSCHANGING && (((WINDOWPOS *)parameter)->flags&SWP_SHOWWINDOW));
            if(!gui.set(w,shadow_proc,(UINT_PTR)shadow_proc,(DWORD_PTR)p))return;
            p->shadow=w;++p->calls;
            if(message==WM_WINDOWPOSCHANGING){WINDOWPOS *position=(WINDOWPOS *)parameter;
                position->flags=(position->flags&~SWP_SHOWWINDOW)|SWP_HIDEWINDOW|SWP_NOACTIVATE;}
            if(gui.visible(w))gui.position(w,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_HIDEWINDOW);
            popup_release(p);return;
        }
}
static LRESULT CALLBACK observe(int code,WPARAM a,LPARAM b)
{
    if(code>=0 && current && b){
        const CWPSTRUCT *m=(const CWPSTRUCT *)b;
        if(m->message==WM_WINDOWPOSCHANGING || m->message==WM_WINDOWPOSCHANGED || m->message==WM_NCPAINT || m->message==WM_PRINT){
            WCHAR name[32]={0};DWORD_PTR existing;
            if(gui.class_name(m->hwnd,name,32) && !wcscmp(name,L"SysShadow"))suppress_shadow(m->hwnd,m->message,m->lParam);
            else if(!wcscmp(name,L"#32768") && !gui.get(m->hwnd,popup_proc,(UINT_PTR)popup_proc,&existing)){
                style_menu_popup_scope *s=current;++s->references;
                if(s->ops->accept(s->context,m->hwnd) && !s->detached){
                    style_menu_popup *p=calloc(1,sizeof(*p));
                    if(p){p->window=m->hwnd;p->scope=s;
                        if(gui.set(p->window,popup_proc,(UINT_PTR)popup_proc,(DWORD_PTR)p)){
                            ++s->references;p->next=s->windows;s->windows=p;
                        }else free(p);
                    }
                }
                scope_release(s);
            }
        }
    }
    return gui.chain(NULL,code,a,b);
}
style_menu_popup_scope *style_menu_popup_begin(HWND owner,const style_menu_popup_ops *ops,void *context)
{
    if(!ops || !ops->accept){SetLastError(ERROR_INVALID_PARAMETER);return NULL;}
    if(!InitOnceExecuteOnce(&once,initialize,NULL,NULL))return NULL;
    if(owner && gui.thread(owner,NULL)!=GetCurrentThreadId()){SetLastError(ERROR_INVALID_THREAD_ID);return NULL;}
    style_menu_popup_scope *s=calloc(1,sizeof(*s));if(!s){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return NULL;}
    if(!current){observer=gui.hook(WH_CALLWNDPROC,observe,NULL,GetCurrentThreadId());
        if(!observer){free(s);return NULL;}}
    s->owner=owner;s->ops=ops;s->context=context;s->references=1;s->previous=current;current=s;return s;
}
BOOL style_menu_popup_end(style_menu_popup_scope *s)
{
    style_menu_popup_scope **link=&current;
    while(*link && *link!=s)link=&(*link)->previous;
    if(!s || !*link){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    *link=s->previous;s->detached=TRUE;
    if(!current){HHOOK hook=observer;observer=NULL;gui.unhook(hook);}
    while(s->windows)detach(s->windows,FALSE);
    scope_release(s);return TRUE;
}
void *style_menu_popup_scope_context(style_menu_popup_scope *scope)
{for(style_menu_popup_scope *s=current;s;s=s->previous)if(s==scope)return s->context;return NULL;}
void style_menu_popup_idle(style_menu_popup_scope *s)
{
    style_menu_popup_scope *live=current;
    while(live && live!=s)live=live->previous;
    if(!s || !live)return;
    ++s->references;
    /* Idle painting can end the scope. Pin each node, then stop on teardown. */
    for(style_menu_popup *p=s->windows;p;){
        ++p->calls;ULONGLONG selected=0;
        if(!p->painted || !s->ops->selection || !s->ops->selection(p,&selected) || selected!=p->selection)paint(p,NULL,FALSE);
        style_menu_popup *next=p->detached?NULL:p->next;
        popup_release(p);p=next;
    }
    scope_release(s);
}
