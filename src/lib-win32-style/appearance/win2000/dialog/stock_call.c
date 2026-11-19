#define _WIN32_WINNT 0x0A00
#include "stock_call.h"
#include "color.h"
#include "find.h"
#include "lib-win32-style/application/dialog/classic_print_contract.h"
#include "lib-win32-style/application/dialog/message_observer.h"
#include "lib-win32-style/application/dialog/observation.h"
#include "lib-win32/process-hook/unwind.h"

typedef enum { STOCK_PLAIN, STOCK_FONT, STOCK_FONT_WITH_CALLBACK, STOCK_COLOR } stock_layout;
typedef struct stock_scope {
    style_dialog_observation dialog;
    const win2k_stock_windows *windows;
    stock_layout layout;
    struct stock_scope *previous;
} stock_scope;
static _Thread_local stock_scope *active_stock;
static LRESULT CALLBACK observe_stock(int code,WPARAM wp,LPARAM lp)
{
    DWORD incoming=GetLastError();
    if(active_stock && code>=0 && lp){
        const CWPRETSTRUCT *event=(const CWPRETSTRUCT *)lp;
        HWND window=event->hwnd;
        if(event->message==WM_INITDIALOG)active_stock->windows->initialized(window);
        else if(active_stock->layout==STOCK_COLOR && event->message==WM_COMMAND &&
                active_stock->windows->attached(window))win2k_color_restore_expanded_client(event);
        else if((active_stock->layout==STOCK_FONT || active_stock->layout==STOCK_FONT_WITH_CALLBACK) &&
                event->message==WM_WINDOWPOSCHANGED &&
                event->lParam && (((const WINDOWPOS *)event->lParam)->flags&SWP_SHOWWINDOW) &&
                active_stock->windows->attached(window) && style_classic_font_template(window)){
            BOOL callback=active_stock->layout==STOCK_FONT_WITH_CALLBACK;
            win2k_restore_stock_dialog_client(window,callback?287:0,callback?196:0);
        }
    }
    SetLastError(incoming);
    return CallNextHookEx(NULL,code,wp,lp);
}
static BOOL begin_stock(stock_scope *scope,const win2k_stock_windows *windows,stock_layout layout)
{
    scope->windows=windows;
    scope->layout=layout;
    scope->previous=active_stock;
    if(!style_dialog_observation_begin(&scope->dialog,NULL,observe_stock,FALSE))return FALSE;
    active_stock=scope;
    return TRUE;
}
static void end_stock(void *data)
{
    stock_scope *scope=data;
    style_dialog_observation_end(&scope->dialog);
    active_stock=scope->previous;
}
static UINT_PTR CALLBACK classic_print(HWND window,UINT message,WPARAM wp,LPARAM lp)
{(void)window;(void)message;(void)wp;(void)lp;return 0;}

static stock_layout font_layout(DWORD flags)
{
    if(flags&(CF_ENABLETEMPLATE|CF_ENABLETEMPLATEHANDLE))return STOCK_PLAIN;
    return flags&CF_ENABLEHOOK?STOCK_FONT_WITH_CALLBACK:STOCK_FONT;
}

#define STOCK_BOOL(NAME,TYPE,LAYOUT) \
PROCESS_HOOK_UNWIND_FUNCTION BOOL NAME(TYPE *parameters,BOOL (WINAPI *native)(TYPE *),const win2k_stock_windows *windows) \
{ \
    stock_scope scope;process_hook_unwind_scope unwind; \
    DWORD incoming=GetLastError(); \
    if(!begin_stock(&scope,windows,LAYOUT))return FALSE; \
    PROCESS_HOOK_UNWIND_BEGIN(unwind,end_stock,&scope); \
    SetLastError(incoming);BOOL result=native(parameters);DWORD error=GetLastError(); \
    PROCESS_HOOK_UNWIND_END(unwind); \
    SetLastError(error);return result; \
}
#define STOCK_PRINT(S,TYPE) \
PROCESS_HOOK_UNWIND_FUNCTION BOOL win2k_print_##S(TYPE *parameters,BOOL (WINAPI *native)(TYPE *),const win2k_stock_windows *windows) \
{ \
    stock_scope scope;process_hook_unwind_scope unwind;TYPE request; \
    BOOL adapted=style_classic_print_prepare_##S(parameters,&request,STYLE_CLASSIC_PRINT_FLAGS,classic_print); \
    DWORD incoming=GetLastError(); \
    if(!begin_stock(&scope,windows,STOCK_PLAIN))return FALSE; \
    PROCESS_HOOK_UNWIND_BEGIN(unwind,end_stock,&scope); \
    SetLastError(incoming);BOOL result=native(adapted?&request:parameters);DWORD error=GetLastError(); \
    PROCESS_HOOK_UNWIND_END(unwind); \
    if(adapted)style_classic_print_result_##S(parameters,&request); \
    SetLastError(error);return result; \
}
#define STOCK_FIND(S,TYPE) \
PROCESS_HOOK_UNWIND_FUNCTION HWND win2k_find_##S(TYPE *parameters,HWND (WINAPI *native)(TYPE *),const win2k_stock_windows *windows,BOOL compact) \
{ \
    stock_scope scope;process_hook_unwind_scope unwind;DWORD incoming=GetLastError(); \
    if(!begin_stock(&scope,windows,STOCK_PLAIN))return NULL; \
    PROCESS_HOOK_UNWIND_BEGIN(unwind,end_stock,&scope); \
    SetLastError(incoming);HWND result=native(parameters);DWORD error=GetLastError(); \
    PROCESS_HOOK_UNWIND_END(unwind); \
    if(result){ \
        windows->initialized(result); \
        if(compact && !(parameters->Flags&(FR_ENABLEHOOK|FR_ENABLETEMPLATE|FR_ENABLETEMPLATEHANDLE))) \
            win2k_find_compact(result); \
    } \
    SetLastError(error);return result; \
}
#define STOCK_CALLS(S,PRINT,SETUP,FONT,COLOR,FIND) \
STOCK_PRINT(S,PRINT) \
STOCK_BOOL(win2k_page_setup_##S,SETUP,STOCK_PLAIN) \
STOCK_BOOL(win2k_font_##S,FONT,parameters && parameters->lStructSize==sizeof(*parameters)? \
    font_layout(parameters->Flags):STOCK_PLAIN) \
STOCK_BOOL(win2k_color_##S,COLOR,parameters && parameters->lStructSize==sizeof(*parameters) && \
    !(parameters->Flags&(CC_ENABLETEMPLATE|CC_ENABLETEMPLATEHANDLE))?STOCK_COLOR:STOCK_PLAIN) \
STOCK_FIND(S,FIND)
STOCK_CALLS(a,PRINTDLGA,PAGESETUPDLGA,CHOOSEFONTA,CHOOSECOLORA,FINDREPLACEA)
STOCK_CALLS(w,PRINTDLGW,PAGESETUPDLGW,CHOOSEFONTW,CHOOSECOLORW,FINDREPLACEW)
