#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#include <windows.h>
#include <commdlg.h>
#include <uxtheme.h>
#include <stdint.h>
#include <wchar.h>
#include "lib-win32/process-hook/startup_hooks.h"
#include "lib-win32/process-hook/unwind.h"
#include "lib-win32-style/appearance/win2000/window/frame.h"
#include "lib-win32-style/application/dialog/explorer_file.h"
#include "lib-win32-style/application/dialog/print_general.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "lib-win32-style/application/dialog/message_observer.h"
#include "lib-win32-style/application/dialog/observation.h"
#include "lib-win32-style/application/dialog/dialog_hook.h"
#include "lib-win32-style/application/dialog/classic_com_controller.h"
#include "lib-win32-style/appearance/msstyles/classic_style.h"
#include "lib-win32-style/application/dialog/stock_dialog.h"
#include "lib-win32-style/application/dialog/print_extended.h"
#include "lib-win32-style/application/dialog/classic_print_contract.h"
#include "lib-win32-style/application/lifecycle/theme_state.h"
#include "ntwshk-dll/observed.h"
static __typeof__(&CreateWindowExW) create_W;
static __typeof__(&CreateWindowExA) create_A;
static __typeof__(&SetWindowTheme) real_theme;

HRESULT ntwshk_observed_native_theme(HWND window,LPCWSTR application,LPCWSTR ids)
{return real_theme?real_theme(window,application,ids):SetWindowTheme(window,application,ids);}

static HRESULT WINAPI hook_theme(HWND window,LPCWSTR application,LPCWSTR ids)
{return style_theme_intercept(window,application,ids,ntwshk_observed_native_theme);}

#define WINDOW_HOOK(S,T) \
static HWND WINAPI hook_##S(DWORD ex,T cls,T text,DWORD style,int x,int y,int width,int height,HWND parent,HMENU menu,HINSTANCE instance,void *data) \
{ HWND result=create_##S(ex,cls,text,style,x,y,width,height,parent,menu,instance,data);DWORD error=GetLastError(); \
  ntwshk_observed_window_created(result,style);SetLastError(error);return result; }
WINDOW_HOOK(W,LPCWSTR)
WINDOW_HOOK(A,LPCSTR)

/* Dialog manager windows can bypass the exported CreateWindowEx entry points. */
static LRESULT CALLBACK dialog_created(int code,WPARAM wp,LPARAM lp)
{
    DWORD incoming=GetLastError();
    const CWPRETSTRUCT *event=style_message_dialog_initialized(code,lp);
    if(event)ntwshk_observed_dialog_initialize(event->hwnd);
    SetLastError(incoming);return CallNextHookEx(NULL,code,wp,lp);
}
static HRESULT observe_dialog(HRESULT (*show)(void *),void *context)
{return style_dialog_observe(show,context,ntwshk_observed_dialog_initialize);}
static BOOL com_session_active(void){return TRUE;}
#define CLASSIC_DIALOG_BEGIN(state,incoming) ((void)(incoming),style_dialog_observation_begin(state,NULL,dialog_created,FALSE))
#define CLASSIC_DIALOG_PREPARE(state,incoming) ((void)(state),SetLastError(incoming))
#define CLASSIC_DIALOG_FINISH(state,modal,result,error,fail) do { \
    (void)(state);(void)(error);if(!(modal) && (result))ntwshk_observed_dialog_initialize((HWND)(result)); \
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
    (void)(state);(void)(error);if((modeless) && (result))ntwshk_observed_dialog_initialize((HWND)(uintptr_t)(result)); \
} while(0)
#define MESSAGE_HOOK(NAME,ARGS,CALL) \
    STYLE_DIALOG_HOOK(NAME,ARGS,CALL,int,0,FALSE,style_dialog_observation,CLASSIC_DIALOG_BEGIN,CLASSIC_DIALOG_PREPARE,style_dialog_observation_end,CLASSIC_PARAM_FINISH)
MESSAGE_HOOK(MessageBoxA,(HWND owner,LPCSTR text,LPCSTR title,UINT type),(owner,text,title,type))
MESSAGE_HOOK(MessageBoxW,(HWND owner,LPCWSTR text,LPCWSTR title,UINT type),(owner,text,title,type))
MESSAGE_HOOK(MessageBoxExA,(HWND owner,LPCSTR text,LPCSTR title,UINT type,WORD language),(owner,text,title,type,language))
MESSAGE_HOOK(MessageBoxExW,(HWND owner,LPCWSTR text,LPCWSTR title,UINT type,WORD language),(owner,text,title,type,language))
MESSAGE_HOOK(MessageBoxIndirectA,(const MSGBOXPARAMSA *parameters),(parameters))
MESSAGE_HOOK(MessageBoxIndirectW,(const MSGBOXPARAMSW *parameters),(parameters))

extern IMAGE_DOS_HEADER __ImageBase;
static HMODULE dialog_resources(void){return (HMODULE)&__ImageBase;}
static const style_dialog_windows dialog_windows={ntwshk_observed_dialog_initialize,win2k_frame_attached,
    dialog_resources,classic_theme_dialog_metrics};
#define STOCK_ENTRY(NAME,TYPE,CALL) \
static __typeof__(&NAME) real_##NAME; \
static BOOL WINAPI hook_##NAME(TYPE *parameters) \
{return CALL(parameters,real_##NAME,&dialog_windows);}
STOCK_ENTRY(ChooseFontA,CHOOSEFONTA,style_observed_font_a)
STOCK_ENTRY(ChooseFontW,CHOOSEFONTW,style_observed_font_w)
STOCK_ENTRY(ChooseColorA,CHOOSECOLORA,style_observed_color_a)
STOCK_ENTRY(ChooseColorW,CHOOSECOLORW,style_observed_color_w)
#undef STOCK_ENTRY

static enum classic_print_route print_route(void)
{return CLASSIC_PRINT_GENERAL_PAGE;}
#define PRINT_ENTRY(NAME,TYPE,SUFFIX) \
static __typeof__(&NAME) real_##NAME; \
static BOOL WINAPI hook_##NAME(TYPE *parameters) \
{ \
    enum classic_print_route route=print_route(); \
    if(route==CLASSIC_PRINT_GENERAL_PAGE)return style_observed_print_##SUFFIX(parameters,real_##NAME,&dialog_windows); \
    if(route==CLASSIC_PRINT_LEGACY_BRIDGE)return style_classic_print_call_##SUFFIX(parameters,real_##NAME); \
    return real_##NAME(parameters); \
}
#define PAGE_ENTRY(NAME,TYPE,SUFFIX) \
static __typeof__(&NAME) real_##NAME; \
static BOOL WINAPI hook_##NAME(TYPE *parameters) \
{return print_route()==CLASSIC_PRINT_GENERAL_PAGE?style_observed_page_setup_##SUFFIX(parameters,real_##NAME,&dialog_windows):real_##NAME(parameters);}
PRINT_ENTRY(PrintDlgA,PRINTDLGA,a)
PRINT_ENTRY(PrintDlgW,PRINTDLGW,w)
PAGE_ENTRY(PageSetupDlgA,PAGESETUPDLGA,a)
PAGE_ENTRY(PageSetupDlgW,PAGESETUPDLGW,w)
#undef PRINT_ENTRY
#undef PAGE_ENTRY
#define FIND_ENTRY(NAME,TYPE,CALL,COMPACT) \
static __typeof__(&NAME) real_##NAME; \
static HWND WINAPI hook_##NAME(TYPE *parameters) \
{return CALL(parameters,real_##NAME,&dialog_windows,COMPACT);}
FIND_ENTRY(FindTextA,FINDREPLACEA,style_observed_find_a,TRUE)
FIND_ENTRY(FindTextW,FINDREPLACEW,style_observed_find_w,TRUE)
FIND_ENTRY(ReplaceTextA,FINDREPLACEA,style_observed_find_a,FALSE)
FIND_ENTRY(ReplaceTextW,FINDREPLACEW,style_observed_find_w,FALSE)
#undef FIND_ENTRY

#define FILE_HOOK(NAME,TYPE,SUFFIX) \
static __typeof__(&NAME) real_##NAME; \
static BOOL WINAPI hook_##NAME(TYPE *parameters) \
{return style_explorer_file_##SUFFIX(parameters,real_##NAME,&dialog_windows);}
FILE_HOOK(GetOpenFileNameA,OPENFILENAMEA,a)
FILE_HOOK(GetOpenFileNameW,OPENFILENAMEW,w)
FILE_HOOK(GetSaveFileNameA,OPENFILENAMEA,a)
FILE_HOOK(GetSaveFileNameW,OPENFILENAMEW,w)
#undef FILE_HOOK

/* Extended requests outside the classic bridge contract retain their property sheet. */
#define PRINT_EXTENDED_HOOK(NAME,TYPE,GENERAL,BRIDGE,PRINT) \
static __typeof__(&NAME) real_##NAME; \
static PROCESS_HOOK_UNWIND_FUNCTION HRESULT WINAPI hook_##NAME(TYPE *parameters) \
{ style_dialog_observation scope;process_hook_unwind_scope unwind; \
  DWORD incoming=GetLastError(),error;HRESULT result; \
  enum classic_print_route route=print_route(); \
  if(route==CLASSIC_PRINT_NATIVE)return real_##NAME(parameters); \
  if(!style_dialog_observation_begin(&scope,NULL,dialog_created,FALSE))return E_FAIL; \
  PROCESS_HOOK_UNWIND_BEGIN(unwind,style_dialog_observation_end,&scope); \
  SetLastError(incoming);if((route!=CLASSIC_PRINT_GENERAL_PAGE || !GENERAL((HINSTANCE)&__ImageBase,parameters,&result,classic_theme_dialog_resources)) && !BRIDGE(parameters,PRINT,&result))result=real_##NAME(parameters); \
  error=GetLastError();PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result; }
PRINT_EXTENDED_HOOK(PrintDlgExA,PRINTDLGEXA,style_print_general_a,style_print_extended_a,real_PrintDlgA)
PRINT_EXTENDED_HOOK(PrintDlgExW,PRINTDLGEXW,style_print_general_w,style_print_extended_w,real_PrintDlgW)
#undef PRINT_EXTENDED_HOOK

DWORD ntw2ks_controls_attach(void)
{
    DWORD (*attach)(void *,void *)=process_hook_startup_attach;
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
