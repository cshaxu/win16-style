#define _WIN32_WINNT 0x0a00
#include <windows.h>
#include <commdlg.h>
#include <uxtheme.h>
#include <stdint.h>
#include <wchar.h>
#include "lib-win32/process-hook/startup_hooks.h"
#include "lib-win32/process-hook/unwind.h"
#include "lib-win32-style/appearance/win2000/window/frame.h"
#include "lib-win32-style/appearance/win2000/dialog/color.h"
#include "lib-win32-style/appearance/win2000/dialog/file.h"
#include "lib-win32-style/appearance/win2000/dialog/find.h"
#include "lib-win32-style/application/lifecycle/global_window.h"
#include "lib-win32-style/application/dialog/message_observer.h"
#include "lib-win32-style/application/dialog/observation.h"
#include "lib-win32-style/application/dialog/dialog_hook.h"
#include "lib-win32-style/application/dialog/classic_com_controller.h"
#include "lib-win32-style/application/dialog/classic_file_contract.h"
#include "lib-win32-style/application/dialog/classic_print_contract.h"
#include "lib-win32-style/application/dialog/print_extended.h"
#include "lib-win32-style/application/lifecycle/theme_state.h"
#include "lib-win32-style/application/window/window_identity.h"
#include "lib-win32-style/appearance/win2000/control/control.h"

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
{return control_local(window) && (win2k_control_attached(window) || style_theme_owned(window,&control_owner));}
BOOL ntw2ks_control_attach(HWND window)
{
    if(!control_local(window))return FALSE;
    if(!(GetWindowLongPtrW(window,GWL_STYLE)&WS_CHILD)){SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    if(ntw2ks_control_attached(window))return TRUE;
    if(win2k_control_attach(window,native_theme))return TRUE;
    if(GetLastError()!=ERROR_NOT_SUPPORTED)return FALSE;
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
    if(win2k_control_attached(window))return win2k_control_remove(window);
    if(!ntw2ks_control_attached(window)){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    style_theme_remove(window,&control_owner,native_theme);return TRUE;
}

#define WINDOW_HOOK(S,T) \
static HWND WINAPI hook_##S(DWORD ex,T cls,T text,DWORD style,int x,int y,int width,int height,HWND parent,HMENU menu,HINSTANCE instance,void *data) \
{ HWND result=create_##S(ex,cls,text,style,x,y,width,height,parent,menu,instance,data);DWORD error=GetLastError(); \
  if(style&WS_CHILD)(void)ntw2ks_control_attach(result);else if(result)attach_frame(result);SetLastError(error);return result; }
WINDOW_HOOK(W,LPCWSTR)
WINDOW_HOOK(A,LPCSTR)

static BOOL CALLBACK attach_dialog_child(HWND child,LPARAM context)
{(void)context;(void)ntw2ks_control_attach(child);return TRUE;}
static void attach_dialog(HWND dialog)
{attach_frame(dialog);if(IsWindow(dialog))EnumChildWindows(dialog,attach_dialog_child,0);}

/* Dialog manager windows can bypass the exported CreateWindowEx entry points. */
static LRESULT CALLBACK dialog_created(int code,WPARAM wp,LPARAM lp)
{
    DWORD incoming=GetLastError();
    const CWPRETSTRUCT *event=style_message_dialog_initialized(code,lp);
    if(event)attach_dialog(event->hwnd);
    SetLastError(incoming);return CallNextHookEx(NULL,code,wp,lp);
}
static LRESULT CALLBACK color_dialog_created(int code,WPARAM wp,LPARAM lp)
{
    DWORD incoming=GetLastError();
    if(code>=0 && lp){
        const CWPRETSTRUCT *message=(const CWPRETSTRUCT *)lp;
        if(message->message==WM_COMMAND && win2k_frame_attached(message->hwnd))
            win2k_color_restore_expanded_client(message);
    }
    SetLastError(incoming);
    return dialog_created(code,wp,lp);
}
static LRESULT CALLBACK compact_file_created(int code,WPARAM wp,LPARAM lp)
{
    DWORD incoming=GetLastError();
    if(code>=0 && lp){
        const CWPRETSTRUCT *message=(const CWPRETSTRUCT *)lp;
        if(message->message==WM_WINDOWPOSCHANGED && message->lParam &&
           (((const WINDOWPOS *)message->lParam)->flags&SWP_SHOWWINDOW) &&
           win2k_frame_attached(message->hwnd))win2k_file_fit_compact(message->hwnd);
    }
    SetLastError(incoming);
    return dialog_created(code,wp,lp);
}
static PROCESS_HOOK_UNWIND_FUNCTION HRESULT observe_dialog(HRESULT (*show)(void *),void *context)
{
    style_dialog_observation scope;
    process_hook_unwind_scope unwind;
    DWORD incoming=GetLastError(),error;
    HRESULT result;
    if(!style_dialog_observation_begin(&scope,NULL,dialog_created,FALSE))return E_FAIL;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,style_dialog_observation_end,&scope);
    SetLastError(incoming);
    result=show(context);
    error=GetLastError();
    PROCESS_HOOK_UNWIND_END(unwind);
    SetLastError(error);
    return result;
}
static BOOL com_session_active(void){return TRUE;}
#define CLASSIC_DIALOG_BEGIN(state,incoming) ((void)(incoming),style_dialog_observation_begin(state,NULL,dialog_created,FALSE))
#define CLASSIC_DIALOG_PREPARE(state,incoming) ((void)(state),SetLastError(incoming))
#define CLASSIC_DIALOG_FINISH(state,modal,result,error,fail) do { \
    (void)(state);(void)(error);if(!(modal) && (result))attach_dialog((HWND)(result)); \
} while(0)
#define DIALOG_HOOK(NAME,TYPE,RET,MODAL,FAIL) \
    STYLE_DIALOG_HOOK(NAME,(HINSTANCE instance,TYPE name,HWND parent,DLGPROC proc,LPARAM param),(instance,name,parent,proc,param),RET,FAIL,MODAL,style_dialog_observation,CLASSIC_DIALOG_BEGIN,CLASSIC_DIALOG_PREPARE,style_dialog_observation_end,CLASSIC_DIALOG_FINISH)
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
    (void)(state);(void)(error);if((modeless) && (result))attach_dialog((HWND)(uintptr_t)(result)); \
} while(0)
#define MESSAGE_HOOK(NAME,ARGS,CALL) \
    STYLE_DIALOG_HOOK(NAME,ARGS,CALL,int,0,FALSE,style_dialog_observation,CLASSIC_DIALOG_BEGIN,CLASSIC_DIALOG_PREPARE,style_dialog_observation_end,CLASSIC_PARAM_FINISH)
MESSAGE_HOOK(MessageBoxA,(HWND owner,LPCSTR text,LPCSTR title,UINT type),(owner,text,title,type))
MESSAGE_HOOK(MessageBoxW,(HWND owner,LPCWSTR text,LPCWSTR title,UINT type),(owner,text,title,type))
MESSAGE_HOOK(MessageBoxExA,(HWND owner,LPCSTR text,LPCSTR title,UINT type,WORD language),(owner,text,title,type,language))
MESSAGE_HOOK(MessageBoxExW,(HWND owner,LPCWSTR text,LPCWSTR title,UINT type,WORD language),(owner,text,title,type,language))
MESSAGE_HOOK(MessageBoxIndirectA,(const MSGBOXPARAMSA *parameters),(parameters))
MESSAGE_HOOK(MessageBoxIndirectW,(const MSGBOXPARAMSW *parameters),(parameters))

/* Find and replace create modeless dialogs inside COMDLG32. */
#define CLASSIC_FIND_FINISH(state,modeless,result,error,fail) do { \
    CLASSIC_PARAM_FINISH(state,modeless,result,error,fail); \
    if((result) && !(parameters->Flags&(FR_ENABLEHOOK|FR_ENABLETEMPLATE|FR_ENABLETEMPLATEHANDLE))) \
        win2k_find_compact(result); \
} while(0)
#define FIND_HOOK(NAME,TYPE,FINISH) \
    STYLE_DIALOG_HOOK(NAME,(TYPE *parameters),(parameters),HWND,NULL,TRUE,style_dialog_observation,CLASSIC_DIALOG_BEGIN,CLASSIC_DIALOG_PREPARE,style_dialog_observation_end,FINISH)
FIND_HOOK(FindTextA,FINDREPLACEA,CLASSIC_FIND_FINISH)
FIND_HOOK(FindTextW,FINDREPLACEW,CLASSIC_FIND_FINISH)
FIND_HOOK(ReplaceTextA,FINDREPLACEA,CLASSIC_PARAM_FINISH)
FIND_HOOK(ReplaceTextW,FINDREPLACEW,CLASSIC_PARAM_FINISH)

/* Common dialogs can create their window without exported dialog calls. */
#define COMMON_MODAL_HOOK(NAME,TYPE) \
    STYLE_DIALOG_HOOK(NAME,(TYPE *parameters),(parameters),BOOL,FALSE,FALSE,style_dialog_observation,CLASSIC_DIALOG_BEGIN,CLASSIC_DIALOG_PREPARE,style_dialog_observation_end,CLASSIC_PARAM_FINISH)
COMMON_MODAL_HOOK(ChooseFontA,CHOOSEFONTA)
COMMON_MODAL_HOOK(ChooseFontW,CHOOSEFONTW)
COMMON_MODAL_HOOK(PageSetupDlgA,PAGESETUPDLGA)
COMMON_MODAL_HOOK(PageSetupDlgW,PAGESETUPDLGW)

#define FILE_HOOK(NAME,TYPE,SUFFIX) \
static __typeof__(&NAME) real_##NAME; \
static PROCESS_HOOK_UNWIND_FUNCTION BOOL WINAPI hook_##NAME(TYPE *parameters) \
{ \
    DWORD incoming=GetLastError(); \
    BOOL compact=parameters && parameters->lStructSize==sizeof(*parameters) && \
        win2k_file_compact(parameters->hwndOwner,parameters->Flags,parameters->FlagsEx); \
    TYPE local; \
    if(compact){local=*parameters;local.Flags|=OFN_ENABLESIZING;local.FlagsEx|=OFN_EX_NOPLACESBAR;} \
    style_dialog_observation scope;process_hook_unwind_scope unwind; \
    if(!style_dialog_observation_begin(&scope,NULL,compact?compact_file_created:dialog_created,FALSE))return FALSE; \
    PROCESS_HOOK_UNWIND_BEGIN(unwind,style_dialog_observation_end,&scope); \
    SetLastError(incoming); \
    BOOL result=real_##NAME(compact?&local:parameters);DWORD error=GetLastError(); \
    PROCESS_HOOK_UNWIND_END(unwind); \
    if(compact)style_classic_file_result_##SUFFIX(parameters,&local); \
    SetLastError(error);return result; \
}
FILE_HOOK(GetOpenFileNameA,OPENFILENAMEA,a)
FILE_HOOK(GetOpenFileNameW,OPENFILENAMEW,w)
FILE_HOOK(GetSaveFileNameA,OPENFILENAMEA,a)
FILE_HOOK(GetSaveFileNameW,OPENFILENAMEW,w)
#undef FILE_HOOK

#define CLASSIC_COLOR_BEGIN(state,incoming) ((void)(incoming),style_dialog_observation_begin(state,NULL, \
    parameters && parameters->lStructSize==sizeof(*parameters) && !(parameters->Flags&(CC_ENABLETEMPLATE|CC_ENABLETEMPLATEHANDLE)) \
        ?color_dialog_created:dialog_created,FALSE))
#define COLOR_HOOK(NAME,TYPE) \
    STYLE_DIALOG_HOOK(NAME,(TYPE *parameters),(parameters),BOOL,FALSE,FALSE,style_dialog_observation,CLASSIC_COLOR_BEGIN,CLASSIC_DIALOG_PREPARE,style_dialog_observation_end,CLASSIC_PARAM_FINISH)
COLOR_HOOK(ChooseColorA,CHOOSECOLORA)
COLOR_HOOK(ChooseColorW,CHOOSECOLORW)
#undef COLOR_HOOK
#undef CLASSIC_COLOR_BEGIN

static UINT_PTR CALLBACK print_dialog_hook(HWND window,UINT message,WPARAM a,LPARAM b)
{(void)window;(void)message;(void)a;(void)b;return 0;}
#define PRINT_HOOK(NAME,TYPE,SUFFIX) \
static __typeof__(&NAME) real_##NAME; \
static PROCESS_HOOK_UNWIND_FUNCTION BOOL WINAPI hook_##NAME(TYPE *parameters) \
{ style_dialog_observation scope;process_hook_unwind_scope unwind;TYPE native; \
  BOOL classic=style_classic_print_prepare_##SUFFIX(parameters,&native,STYLE_CLASSIC_PRINT_FLAGS,print_dialog_hook); \
  DWORD incoming=GetLastError(),error;BOOL result; \
  if(!style_dialog_observation_begin(&scope,NULL,dialog_created,FALSE))return FALSE; \
  PROCESS_HOOK_UNWIND_BEGIN(unwind,style_dialog_observation_end,&scope); \
  SetLastError(incoming);result=real_##NAME(classic?&native:parameters);error=GetLastError(); \
  PROCESS_HOOK_UNWIND_END(unwind); \
  if(classic)style_classic_print_result_##SUFFIX(parameters,&native); \
  SetLastError(error);return result; }
PRINT_HOOK(PrintDlgA,PRINTDLGA,a)
PRINT_HOOK(PrintDlgW,PRINTDLGW,w)
#undef PRINT_HOOK

/* Extended requests outside the classic bridge contract retain their property sheet. */
#define PRINT_EXTENDED_HOOK(NAME,TYPE,BRIDGE,PRINT) \
static __typeof__(&NAME) real_##NAME; \
static PROCESS_HOOK_UNWIND_FUNCTION HRESULT WINAPI hook_##NAME(TYPE *parameters) \
{ style_dialog_observation scope;process_hook_unwind_scope unwind; \
  DWORD incoming=GetLastError(),error;HRESULT result; \
  if(!style_dialog_observation_begin(&scope,NULL,dialog_created,FALSE))return E_FAIL; \
  PROCESS_HOOK_UNWIND_BEGIN(unwind,style_dialog_observation_end,&scope); \
  SetLastError(incoming);if(!BRIDGE(parameters,PRINT,&result))result=real_##NAME(parameters); \
  error=GetLastError();PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result; }
PRINT_EXTENDED_HOOK(PrintDlgExA,PRINTDLGEXA,style_print_extended_a,real_PrintDlgA)
PRINT_EXTENDED_HOOK(PrintDlgExW,PRINTDLGEXW,style_print_extended_w,real_PrintDlgW)
#undef PRINT_EXTENDED_HOOK

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
    ATTACH_DIALOG(ChooseFontA);ATTACH_DIALOG(ChooseFontW);
    ATTACH_DIALOG(ChooseColorA);ATTACH_DIALOG(ChooseColorW);
    ATTACH_DIALOG(PrintDlgA);ATTACH_DIALOG(PrintDlgW);
    ATTACH_DIALOG(PrintDlgExA);ATTACH_DIALOG(PrintDlgExW);
    ATTACH_DIALOG(PageSetupDlgA);ATTACH_DIALOG(PageSetupDlgW);
#undef ATTACH_DIALOG
    if(error)return error;
    const style_classic_com_options com={
        .session_active=com_session_active,.scope=observe_dialog,
        .property=L"Win2kStyle.ComDialog",.folder_message=L"Win2kStyle.ComSetFolder",
        .hidden_message=L"Win2kStyle.HiddenFilesRefresh",.explorer_files=TRUE
    };
    return style_classic_com_attach(attach,&com);
}

DWORD ntw2ks_controls_attach(void)
{return ntw2ks_controls_attach_using(process_hook_startup_attach);}
