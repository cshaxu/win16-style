#include "tracking.h"

typedef struct tracking_api {
    __typeof__(&SetWindowsHookExW) hook;
    __typeof__(&UnhookWindowsHookEx) unhook;
    __typeof__(&CallNextHookEx) chain;
    __typeof__(&GetWindowThreadProcessId) thread;
} tracking_api;
static tracking_api gui;
static INIT_ONCE once=INIT_ONCE_STATIC_INIT;
static BOOL CALLBACK initialize(PINIT_ONCE init,void *argument,void **context)
{
    (void)init;(void)argument;(void)context;
    HMODULE user=LoadLibraryW(L"user32.dll");if(!user)return FALSE;
    gui.hook=(void *)GetProcAddress(user,"SetWindowsHookExW");
    gui.unhook=(void *)GetProcAddress(user,"UnhookWindowsHookEx");
    gui.chain=(void *)GetProcAddress(user,"CallNextHookEx");
    gui.thread=(void *)GetProcAddress(user,"GetWindowThreadProcessId");
    if(gui.hook && gui.unhook && gui.chain && gui.thread)return TRUE;
    FreeLibrary(user);ZeroMemory(&gui,sizeof(gui));SetLastError(ERROR_PROC_NOT_FOUND);return FALSE;
}
static _Thread_local style_menu_tracking *current;
static _Thread_local HHOOK messages,menus;
static LRESULT CALLBACK message_filter(int code,WPARAM wp,LPARAM lp)
{
    style_menu_tracking *frame=current;
    if(code==HC_ACTION && (wp&PM_REMOVE) && lp && frame && frame->kind==WH_GETMESSAGE)
        frame->filter(frame,(MSG *)lp);
    return gui.chain(NULL,code,wp,lp);
}
static LRESULT CALLBACK menu_filter(int code,WPARAM wp,LPARAM lp)
{
    style_menu_tracking *frame=current;
    if(code==MSGF_MENU && lp && frame && frame->kind==WH_MSGFILTER)frame->filter(frame,(MSG *)lp);
    return gui.chain(NULL,code,wp,lp);
}
style_menu_tracking *style_menu_tracking_current(void){return current;}
BOOL style_menu_tracking_begin(style_menu_tracking *frame,int kind,void (*filter)(style_menu_tracking *,MSG *))
{
    if(!frame || !filter || (kind!=WH_GETMESSAGE && kind!=WH_MSGFILTER)){
        SetLastError(ERROR_INVALID_PARAMETER);return FALSE;
    }
    for(style_menu_tracking *p=current;p;p=p->previous)if(p==frame){SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
    if(!InitOnceExecuteOnce(&once,initialize,NULL,NULL))return FALSE;
    if(gui.thread(frame->window,NULL)!=GetCurrentThreadId()){SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;}
    HHOOK *hook=kind==WH_GETMESSAGE?&messages:&menus;
    if(!*hook){*hook=gui.hook(kind,kind==WH_GETMESSAGE?message_filter:menu_filter,NULL,GetCurrentThreadId());if(!*hook)return FALSE;}
    frame->kind=kind;frame->filter=filter;frame->previous=current;current=frame;return TRUE;
}
BOOL style_menu_tracking_end(style_menu_tracking *frame)
{
    style_menu_tracking **link=&current;
    while(*link && *link!=frame)link=&(*link)->previous;
    if(!frame || !*link){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    int kind=frame->kind;*link=frame->previous;frame->previous=NULL;frame->filter=NULL;
    for(style_menu_tracking *p=current;p;p=p->previous)if(p->kind==kind)return TRUE;
    HHOOK *hook=kind==WH_GETMESSAGE?&messages:&menus;
    HHOOK old=*hook;*hook=NULL;gui.unhook(old);return TRUE;
}
void style_menu_tracking_update(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    style_menu_tracking *frame=current;if(!frame || frame->window!=window)return;
    if(frame->kind==WH_GETMESSAGE){if(message==WM_MENUSELECT && lp)frame->active=(HMENU)lp;}
    else if(message==WM_INITMENUPOPUP)frame->active=(HMENU)wp;
    else if(message==WM_UNINITMENUPOPUP && frame->active==(HMENU)wp)frame->active=frame->root;
}
