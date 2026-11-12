#include <windows.h>
#include <commdlg.h>
#include <uxtheme.h>
#include <stdint.h>
#include <wchar.h>
#include "lib-win32/process-hook/startup_hooks.h"
#include "lib-win32/process-hook/unwind.h"
#include "lib-style/win2k-window/frame.h"
#include "lib-style/common/global_window.h"
#include "lib-style/common/message_observer.h"
#include "lib-style/common/dialog_hook.h"
#include "lib-style/common/theme_state.h"
#include "lib-win32/style/window_identity.h"

static __typeof__(&CreateWindowExW) create_W;
static __typeof__(&CreateWindowExA) create_A;
static __typeof__(&SetWindowTheme) real_theme;
static void attach_frame(HWND window)
{if(window && !style_global_window_managed(window))win2k_frame_attach(window);}

static int control_owner;
static HRESULT native_theme(HWND window,LPCWSTR application,LPCWSTR ids)
{return real_theme?real_theme(window,application,ids):SetWindowTheme(window,application,ids);}
static HRESULT WINAPI hook_theme(HWND window,LPCWSTR application,LPCWSTR ids)
{return style_theme_intercept(window,application,ids,native_theme);}
static BOOL control_local(HWND window)
{
    if(win32_style_window_current_thread(window))return TRUE;
    SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
}
BOOL ntw2ks_control_attached(HWND window)
{return control_local(window) && style_theme_owned(window,&control_owner);}
BOOL ntw2ks_control_attach(HWND window)
{
    if(!control_local(window))return FALSE;
    if(!(GetWindowLongPtrW(window,GWL_STYLE)&WS_CHILD)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(ntw2ks_control_attached(window))return TRUE;
    if(style_theme_retained(window)){SetLastError(ERROR_BUSY);return FALSE;}
    WCHAR name[32];
    if(!GetClassNameW(window,name,ARRAYSIZE(name)))return FALSE;
    static const WCHAR *const classes[]={L"Button",L"Edit",L"ComboBox",L"ListBox",L"ScrollBar"};
    for(unsigned i=0;i<ARRAYSIZE(classes);++i){
        if(!_wcsicmp(name,classes[i])){
            if(!style_theme_capture(window))return FALSE;
            if(style_theme_suppress(window,&control_owner,native_theme))return TRUE;
            (void)style_theme_terminal(window,&control_owner);return FALSE;
        }
    }
    SetLastError(ERROR_NOT_SUPPORTED);return FALSE;
}
BOOL ntw2ks_control_remove(HWND window)
{
    if(!control_local(window))return FALSE;
    if(!ntw2ks_control_attached(window)){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    style_theme_remove(window,&control_owner,native_theme);return TRUE;
}

#define WINDOW_HOOK(S,T) \
static HWND WINAPI hook_##S(DWORD ex,T cls,T text,DWORD style,int x,int y,int width,int height,HWND parent,HMENU menu,HINSTANCE instance,void *data) \
{ HWND result=create_##S(ex,cls,text,style,x,y,width,height,parent,menu,instance,data);DWORD error=GetLastError(); \
  if(style&WS_CHILD)(void)ntw2ks_control_attach(result);else if(result)attach_frame(result);SetLastError(error);return result; }
WINDOW_HOOK(W,LPCWSTR)
WINDOW_HOOK(A,LPCSTR)

/* Dialog manager windows can bypass the exported CreateWindowEx entry points. */
static LRESULT CALLBACK dialog_created(int code,WPARAM wp,LPARAM lp)
{
    DWORD incoming=GetLastError();
    const CWPRETSTRUCT *event=style_message_dialog_initialized(code,lp);
    if(event)attach_frame(event->hwnd);
    SetLastError(incoming);return CallNextHookEx(NULL,code,wp,lp);
}
static void dialog_scope_end(void *context)
{
    HHOOK hook=*(HHOOK *)context;
    style_message_observer_end(hook);
}
#define CLASSIC_DIALOG_BEGIN(state,incoming) ((void)(incoming),*(state)=style_message_observer_begin(dialog_created),TRUE)
#define CLASSIC_DIALOG_PREPARE(state,incoming) ((void)(state),SetLastError(incoming))
#define CLASSIC_DIALOG_FINISH(state,modal,result,error,fail) do { \
    (void)(state);(void)(error);if(!(modal) && (result))attach_frame((HWND)(result)); \
} while(0)
#define DIALOG_HOOK(NAME,TYPE,RET,MODAL,FAIL) \
    STYLE_DIALOG_HOOK(NAME,(HINSTANCE instance,TYPE name,HWND parent,DLGPROC proc,LPARAM param),(instance,name,parent,proc,param),RET,FAIL,MODAL,HHOOK,CLASSIC_DIALOG_BEGIN,CLASSIC_DIALOG_PREPARE,dialog_scope_end,CLASSIC_DIALOG_FINISH)
DIALOG_HOOK(CreateDialogParamA,LPCSTR,HWND,FALSE,NULL)
DIALOG_HOOK(CreateDialogParamW,LPCWSTR,HWND,FALSE,NULL)
DIALOG_HOOK(CreateDialogIndirectParamA,LPCDLGTEMPLATEA,HWND,FALSE,NULL)
DIALOG_HOOK(CreateDialogIndirectParamW,LPCDLGTEMPLATEW,HWND,FALSE,NULL)
DIALOG_HOOK(DialogBoxParamA,LPCSTR,INT_PTR,TRUE,-1)
DIALOG_HOOK(DialogBoxParamW,LPCWSTR,INT_PTR,TRUE,-1)
DIALOG_HOOK(DialogBoxIndirectParamA,LPCDLGTEMPLATEA,INT_PTR,TRUE,-1)
DIALOG_HOOK(DialogBoxIndirectParamW,LPCDLGTEMPLATEW,INT_PTR,TRUE,-1)

/* Message boxes may create dialogs internally without calling exported dialog APIs. */
#define CLASSIC_PARAM_FINISH(state,modeless,result,error,fail) do { \
    (void)(state);(void)(error);if((modeless) && (result))attach_frame((HWND)(uintptr_t)(result)); \
} while(0)
#define MESSAGE_HOOK(NAME,ARGS,CALL) \
    STYLE_DIALOG_HOOK(NAME,ARGS,CALL,int,0,FALSE,HHOOK,CLASSIC_DIALOG_BEGIN,CLASSIC_DIALOG_PREPARE,dialog_scope_end,CLASSIC_PARAM_FINISH)
MESSAGE_HOOK(MessageBoxA,(HWND owner,LPCSTR text,LPCSTR title,UINT type),(owner,text,title,type))
MESSAGE_HOOK(MessageBoxW,(HWND owner,LPCWSTR text,LPCWSTR title,UINT type),(owner,text,title,type))
MESSAGE_HOOK(MessageBoxExA,(HWND owner,LPCSTR text,LPCSTR title,UINT type,WORD language),(owner,text,title,type,language))
MESSAGE_HOOK(MessageBoxExW,(HWND owner,LPCWSTR text,LPCWSTR title,UINT type,WORD language),(owner,text,title,type,language))
MESSAGE_HOOK(MessageBoxIndirectA,(const MSGBOXPARAMSA *parameters),(parameters))
MESSAGE_HOOK(MessageBoxIndirectW,(const MSGBOXPARAMSW *parameters),(parameters))

/* Find and replace create modeless dialogs inside COMDLG32. */
#define FIND_HOOK(NAME,TYPE) \
    STYLE_DIALOG_HOOK(NAME,(TYPE *parameters),(parameters),HWND,NULL,TRUE,HHOOK,CLASSIC_DIALOG_BEGIN,CLASSIC_DIALOG_PREPARE,dialog_scope_end,CLASSIC_PARAM_FINISH)
FIND_HOOK(FindTextA,FINDREPLACEA)
FIND_HOOK(FindTextW,FINDREPLACEW)
FIND_HOOK(ReplaceTextA,FINDREPLACEA)
FIND_HOOK(ReplaceTextW,FINDREPLACEW)

/* Common dialogs can create their window without exported dialog calls. */
#define COMMON_MODAL_HOOK(NAME,TYPE) \
    STYLE_DIALOG_HOOK(NAME,(TYPE *parameters),(parameters),BOOL,FALSE,FALSE,HHOOK,CLASSIC_DIALOG_BEGIN,CLASSIC_DIALOG_PREPARE,dialog_scope_end,CLASSIC_PARAM_FINISH)
COMMON_MODAL_HOOK(GetOpenFileNameA,OPENFILENAMEA)
COMMON_MODAL_HOOK(GetOpenFileNameW,OPENFILENAMEW)
COMMON_MODAL_HOOK(GetSaveFileNameA,OPENFILENAMEA)
COMMON_MODAL_HOOK(GetSaveFileNameW,OPENFILENAMEW)
COMMON_MODAL_HOOK(ChooseColorA,CHOOSECOLORA)
COMMON_MODAL_HOOK(ChooseColorW,CHOOSECOLORW)
COMMON_MODAL_HOOK(PageSetupDlgA,PAGESETUPDLGA)
COMMON_MODAL_HOOK(PageSetupDlgW,PAGESETUPDLGW)

DWORD ntw2ks_controls_attach_using(DWORD (*attach)(void *,void *))
{
    if(!attach)return ERROR_INVALID_PARAMETER;
    create_W=CreateWindowExW;create_A=CreateWindowExA;real_theme=SetWindowTheme;
    DWORD error=attach(&create_W,(void *)hook_W);
    if(!error)error=attach(&create_A,(void *)hook_A);
    if(!error)error=attach(&real_theme,(void *)hook_theme);
#define ATTACH_DIALOG(NAME) do { if(!error){real_##NAME=NAME;error=attach(&real_##NAME,(void *)hook_##NAME);} } while(0)
    ATTACH_DIALOG(CreateDialogParamA);ATTACH_DIALOG(CreateDialogParamW);
    ATTACH_DIALOG(CreateDialogIndirectParamA);ATTACH_DIALOG(CreateDialogIndirectParamW);
    ATTACH_DIALOG(DialogBoxParamA);ATTACH_DIALOG(DialogBoxParamW);
    ATTACH_DIALOG(DialogBoxIndirectParamA);ATTACH_DIALOG(DialogBoxIndirectParamW);
    ATTACH_DIALOG(MessageBoxA);ATTACH_DIALOG(MessageBoxW);
    ATTACH_DIALOG(MessageBoxExA);ATTACH_DIALOG(MessageBoxExW);
    ATTACH_DIALOG(MessageBoxIndirectA);ATTACH_DIALOG(MessageBoxIndirectW);
    ATTACH_DIALOG(FindTextA);ATTACH_DIALOG(FindTextW);
    ATTACH_DIALOG(ReplaceTextA);ATTACH_DIALOG(ReplaceTextW);
    ATTACH_DIALOG(GetOpenFileNameA);ATTACH_DIALOG(GetOpenFileNameW);
    ATTACH_DIALOG(GetSaveFileNameA);ATTACH_DIALOG(GetSaveFileNameW);
    ATTACH_DIALOG(ChooseColorA);ATTACH_DIALOG(ChooseColorW);
    ATTACH_DIALOG(PageSetupDlgA);ATTACH_DIALOG(PageSetupDlgW);
#undef ATTACH_DIALOG
    return error;
}

DWORD ntw2ks_controls_attach(void)
{return ntw2ks_controls_attach_using(process_hook_startup_attach);}
