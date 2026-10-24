#include "menu_scope.h"
#include <commctrl.h>
#include <stdlib.h>
#include <wchar.h>

typedef struct popup popup;
struct win16_style_menu_scope {
    win16_style_palette palette;
    struct win16_style_menu_scope *previous;
    DWORD thread;
    popup *windows;
};
struct popup {
    HWND window;popup *next;win16_style_menu_scope *scope;
    win16_style_palette palette;
    unsigned calls;
    BOOL detached;
    HWND shadow;
    BOOL shadow_visible;
};
static _Thread_local win16_style_menu_scope *current;
static _Thread_local HHOOK thread_hook;
static LRESULT CALLBACK popup_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static LRESULT CALLBACK shadow_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);

static void paint_border(HWND w,popup *p,HDC target)
{
    RECT outer,client,r;HDC dc=target?target:GetWindowDC(w);int saved,u;
    if(!dc)return;
    saved=SaveDC(dc);if(!saved){if(!target)ReleaseDC(w,dc);return;}
    if(!GetWindowRect(w,&outer) || !GetClientRect(w,&client))goto done;
    MapWindowPoints(w,NULL,(POINT *)&client,2);
    OffsetRect(&client,-outer.left,-outer.top);
    SetRect(&r,0,0,outer.right-outer.left,outer.bottom-outer.top);
    ExcludeClipRect(dc,client.left,client.top,client.right,client.bottom);
    SetDCBrushColor(dc,p->palette.light);FillRect(dc,&r,(HBRUSH)GetStockObject(DC_BRUSH));
    u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));
    SetDCBrushColor(dc,p->palette.edge);
    {RECT edge=r;edge.bottom=u;FillRect(dc,&edge,(HBRUSH)GetStockObject(DC_BRUSH));
     edge=r;edge.top=edge.bottom-u;FillRect(dc,&edge,(HBRUSH)GetStockObject(DC_BRUSH));
     edge=r;edge.right=u;FillRect(dc,&edge,(HBRUSH)GetStockObject(DC_BRUSH));
     edge=r;edge.left=edge.right-u;FillRect(dc,&edge,(HBRUSH)GetStockObject(DC_BRUSH));}
done:
    RestoreDC(dc,saved);if(!target)ReleaseDC(w,dc);
}

static void detach(popup *p,BOOL terminal)
{
    popup **link=&p->scope->windows;
    while(*link && *link!=p)link=&(*link)->next;
    if(*link)*link=p->next;
    RemoveWindowSubclass(p->window,popup_proc,(UINT_PTR)popup_proc);
    p->detached=TRUE;p->scope=NULL;
    if(p->shadow){HWND shadow=p->shadow;p->shadow=NULL;
        RemoveWindowSubclass(shadow,shadow_proc,(UINT_PTR)shadow_proc);
        if(!terminal && p->shadow_visible && IsWindowVisible(p->window))ShowWindow(shadow,SW_SHOWNOACTIVATE);}
    if(!p->calls)free(p);
}

static LRESULT CALLBACK shadow_proc(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    popup *p=(popup *)data;LRESULT result;(void)id;++p->calls;
    if(m==WM_NCDESTROY){p->shadow=NULL;RemoveWindowSubclass(w,shadow_proc,(UINT_PTR)shadow_proc);}
    else if(m==WM_WINDOWPOSCHANGING && !p->detached){WINDOWPOS *position=(WINDOWPOS *)b;
        position->flags=(position->flags&~SWP_SHOWWINDOW)|SWP_HIDEWINDOW|SWP_NOACTIVATE;}
    result=DefSubclassProc(w,m,a,b);
    if(!--p->calls && p->detached)free(p);
    return result;
}

static void suppress_shadow(HWND w,UINT message,LPARAM parameter)
{
    RECT shadow;DWORD_PTR existing;
    if(!GetWindowRect(w,&shadow))return;
    for(win16_style_menu_scope *scope=current;scope;scope=scope->previous)
        for(popup *p=scope->windows;p;p=p->next){RECT menu;
            /* SysShadow has no GW_OWNER. Require the same GUI thread (the
             * thread hook), immediate native z-order pairing and matching
             * origin/expanded bounds; never hide an unrelated popup shadow. */
            if(GetWindow(w,GW_HWNDPREV)!=p->window || !GetWindowRect(p->window,&menu) ||
                shadow.left!=menu.left || shadow.top!=menu.top || shadow.right<=menu.right || shadow.bottom<=menu.bottom ||
                shadow.right-menu.right>32 || shadow.bottom-menu.bottom>32)continue;
            if(GetWindowSubclass(w,shadow_proc,(UINT_PTR)shadow_proc,&existing)){
                if((popup *)existing!=p){((popup *)existing)->shadow=NULL;
                    RemoveWindowSubclass(w,shadow_proc,(UINT_PTR)shadow_proc);}
                else return;
            }
            if(p->shadow && p->shadow!=w)continue;
            p->shadow_visible=IsWindowVisible(w) || (message==WM_WINDOWPOSCHANGING && (((WINDOWPOS *)parameter)->flags&SWP_SHOWWINDOW));
            if(!SetWindowSubclass(w,shadow_proc,(UINT_PTR)shadow_proc,(DWORD_PTR)p))return;
            p->shadow=w;++p->calls;
            if(message==WM_WINDOWPOSCHANGING){WINDOWPOS *position=(WINDOWPOS *)parameter;
                position->flags=(position->flags&~SWP_SHOWWINDOW)|SWP_HIDEWINDOW|SWP_NOACTIVATE;}
            if(IsWindowVisible(w))SetWindowPos(w,NULL,0,0,0,0,
                SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_HIDEWINDOW);
            if(!--p->calls && p->detached)free(p);
            return;
        }
}

static LRESULT CALLBACK popup_proc(HWND w,UINT m,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    popup *p=(popup *)data;LRESULT result=0;(void)id;
    ++p->calls;
    if(m==WM_NCDESTROY){detach(p,TRUE);result=DefSubclassProc(w,m,a,b);}
    else if(m==WM_NCPAINT)paint_border(w,p,NULL);
    else if(m==WM_PAINT){result=DefSubclassProc(w,m,a,b);
        if(!p->detached)paint_border(w,p,NULL);}
    else if(m==WM_PRINT){result=DefSubclassProc(w,m,a,b);
        /* Native printing may reenter the owner, end the scope or destroy
         * this HWND. Keep the node alive but never paint after detachment. */
        if(!p->detached && (b&PRF_NONCLIENT))paint_border(w,p,(HDC)a);
    }else result=DefSubclassProc(w,m,a,b);
    if(!--p->calls && p->detached)free(p);
    return result;
}

static LRESULT CALLBACK popup_message(int code,WPARAM a,LPARAM b)
{
    const CWPSTRUCT *message=(const CWPSTRUCT *)b;
    if(code>=0 && current && (message->message==WM_WINDOWPOSCHANGING || message->message==WM_WINDOWPOSCHANGED ||
        message->message==WM_NCPAINT || message->message==WM_PRINT)){
        HWND w=message->hwnd;WCHAR name[32]={0};DWORD_PTR existing;
        if(GetClassNameW(w,name,32) && !wcscmp(name,L"SysShadow"))suppress_shadow(w,message->message,message->lParam);
        else if(!wcscmp(name,L"#32768") && !GetWindowSubclass(w,popup_proc,(UINT_PTR)popup_proc,&existing)){
            popup *p=(popup *)calloc(1,sizeof(*p));
            if(p){p->window=w;p->scope=current;p->palette=current->palette;
                if(SetWindowSubclass(w,popup_proc,(UINT_PTR)popup_proc,(DWORD_PTR)p)){
                    p->next=current->windows;current->windows=p;
                }else free(p);
            }
        }
    }
    return CallNextHookEx(NULL,code,a,b);
}

win16_style_menu_scope *win16_style_menu_begin(const win16_style_palette *palette)
{
    win16_style_menu_scope *s=(win16_style_menu_scope *)calloc(1,sizeof(*s));
    if(!s)return NULL;
    if(palette)s->palette=*palette;else win16_style_default_palette(&s->palette);
    s->thread=GetCurrentThreadId();s->previous=current;
    /* One hook for the nested scope stack, never a desktop/global hook. */
    if(!current){thread_hook=SetWindowsHookExW(WH_CALLWNDPROC,popup_message,NULL,s->thread);
        if(!thread_hook){free(s);return NULL;}}
    current=s;return s;
}

BOOL win16_style_menu_end(win16_style_menu_scope *s)
{
    win16_style_menu_scope **link=&current;
    while(*link && *link!=s)link=&(*link)->previous;
    if(!s || !*link || s->thread!=GetCurrentThreadId()){
        SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    while(s->windows)detach(s->windows,FALSE);
    *link=s->previous;
    if(!current){UnhookWindowsHookEx(thread_hook);thread_hook=NULL;}
    free(s);return TRUE;
}
