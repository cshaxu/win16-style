#include "lib-win32-style/application/graphics/text_metrics.h"
#include "lib-win32-style/application/menu/menu_owner.h"
#include "lib-win32-style/application/dialog/classic_print_contract.h"
#include "lib-win32-style/application/dialog/classic_dialog_controls.h"
#include "lib-win32-style/application/dialog/message_observer.h"
#include "lib-win32-style/application/dialog/dialog_hook.h"
#include "lib-win32-style/application/lifecycle/creation_call.h"
#include "lib-win32-style/application/dialog/observation.h"
#include "intercept.h"
#include "ntwshk-dll/observed.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "../../lib-win32/process-hook/startup_hooks.h"
#include "../../lib-win32/process-hook/unwind.h"
#include "lib-win32-style/application/lifecycle/theme_track.h"
#include "lib-win32-style/application/lifecycle/theme_state.h"
#include "lib-win32-style/application/dialog/print_extended.h"
#include "lib-win32-style/application/dialog/print_general.h"
#include "lib-win32-style/application/dialog/stock_dialog.h"
#include "ntw16s-dll/global/window.h"
#include "ntw16s-dll/global/session.h"
#include "ntw16s-dll/ntw16s.h"
#include "ntw16s-dll/attachment.h"
#include "ntw16s-dll/native_api.h"
#include "lib-win32-style/application/adapters/win32/runtime_text.h"
#include "lib-win32-style/application/control/updown.h"
#include "ntw16s-dll/controls.h"
#include "lib-win32-style/application/adapters/win32/runtime_native.h"
#include "ntw16s-dll/status_theme.h"
#include "ntw16s-dll/win16_style.h"
#include <wchar.h>
#include <uxtheme.h>
#include <commdlg.h>
#include <commctrl.h>
#include <vssym32.h>
#include "lib-win32-style/appearance/msstyles/raster_font.h"
#include "lib-win32-style/application/graphics/font_scope.h"
#include "lib-win32-style/application/graphics/font_cache.h"
#include "lib-win32-style/application/control/month.h"
#include "lib-win32-style/appearance/win3x/control/special_paint.h"
#include "lib-win32-style/application/adapters/win32/checklist.h"
#include "lib-win32-style/application/adapters/win32/frame_paint.h"
HFONT ntw16s_month_text_borrow(HTHEME,HDC,int,int,DWORD,DWORD,const RECT *,const void *);
BOOL ntw16s_month_text_room(HTHEME,HDC,int,int,DWORD,DWORD,const RECT *,int,const void *);
typedef struct month_text_room_context {HTHEME theme;HDC dc;int part,state;DWORD flags,flags2;const RECT *rect;const void *caller;} month_text_room_context;
static BOOL month_text_room(void *value,int width)
{month_text_room_context *c=value;return ntw16s_month_text_room(c->theme,c->dc,c->part,c->state,c->flags,c->flags2,c->rect,width,c->caller);}
static void release_system_font(void *context,HFONT font)
{(void)context;style_raster_font_release(font);}

/* Private, synchronous coordinator scope; not part of the public ABI. */
void ntw16s_status_frame(HDC,const RECT *,const RECT *,const void *);
BOOL ntw16s_controlbar_edge_active(void);
void ntw16s_controlbar_edge(HDC,const RECT *,UINT,UINT,const void *);
void ntw16s_bit_theme_frame(HDC,int,const RECT *,const RECT *,const void *);
void ntw16s_nav_theme(HDC,int,int,const RECT *,const RECT *,const void *);
void ntw16s_inplace_button(HDC,int,int,const RECT *,const RECT *,const void *);
void ntw16s_inplace_dropdown(HDC,int,int,const RECT *,const RECT *,const void *);
BOOL ntw16s_inplace_paint(HDC);
void ntw16s_status_text_frame(HDC,const RECT *,UINT,const void *);
void ntw16s_status_icon_frame(HDC,int,int,int,int,HICON,const void *);
void ntw16s_status_hooks_enable(void);
void ntw16s_spin_frame(HDC,const RECT *,UINT,UINT,const void *);
void ntw16s_month_arrow(HTHEME,HDC,int,int,const RECT *,const RECT *,const void *);

/* The original trampoline leaves are bound privately to this same DLL.
 * No renderer LoadLibrary/GetProcAddress, GUI allocation or binding in DllMain. */
static LONG style_ready;
static __typeof__(&GetScrollInfo) real_get_scroll;
static __typeof__(&SetScrollInfo) real_set_scroll;
static __typeof__(&SetWindowTheme) real_set_theme;
static BOOL CALLBACK bind_titles(PINIT_ONCE once,void *parameter,void **context)
{
    (void)once;(void)parameter;(void)context;
    ntw16s_native_api api={sizeof(api),real_get_scroll,real_set_scroll,real_set_theme};
    if(!ntw16s_bind_native_api(&api))return FALSE;
    style_control_updown_api_enable();
    ntw16s_status_hooks_enable();
    InterlockedExchange(&style_ready,1);return TRUE;
}
static BOOL creation_active(void)
{
    if(!(__atomic_load_n(&ntw16s_process_context.flags,__ATOMIC_ACQUIRE)&NTWSHK_POLICY_GLOBAL_SESSION))
        return TRUE;
    DWORD cookie=__atomic_load_n(&ntw16s_process_context.session_cookie,__ATOMIC_ACQUIRE),style;
    classic_theme_selection selection;
    return style_global_policy_current(&ntw16s_global_namespace,cookie,&style) &&
        classic_theme_decode_selection(style,&selection) &&
        classic_theme_selection_layout(selection)==CLASSIC_WINDOW_PROVIDER_FULL_CLIENT;
}
static style_creation_context title_context={.once=INIT_ONCE_STATIC_INIT,.prepare=bind_titles,
    .active=creation_active,.operations=&ntw16s_creation_ops};
static const WCHAR menu_owner_property[]=L"Win16Style.T18.MenuOwner";
static int menu_owner_marker;
static void mark_menu_owner(HWND w,const void *caller)
{(void)style_menu_owner_mark(w,caller,L"TPUtilWindow",menu_owner_property,(HANDLE)&menu_owner_marker);}

typedef style_creation_call title_scope;
#define begin_titles(scope,modal) style_creation_call_begin(scope,modal,&title_context)
#define end_titles style_creation_call_end

/* Shell COM dialogs can create their window without passing the exported
 * CreateWindow entry points. Keep attachment active during their modal Show. */
HRESULT ntw16s_dialog_style_scope(HRESULT (*callback)(void *),void *context)
{
#ifdef NTWSHK_UNIFIED
    if(ntw16s_dialog_policy().window_provider==CLASSIC_WINDOW_PROVIDER_NONCLIENT)
        return style_dialog_observe(callback,context,ntw16s_observed_dialog_initialize);
#endif
    return style_creation_call_invoke(&title_context,callback,context);
}

/* The observed route attaches after USER returns. The creation route keeps
 * its existing pre-create transaction. Both routes own the same trampolines. */
static void (*startup_window_created)(HWND,DWORD);

HRESULT ntwshk_observed_native_theme(HWND window,LPCWSTR application,LPCWSTR ids)
{
    return startup_window_created?real_set_theme(window,application,ids):SetWindowTheme(window,application,ids);
}

#define WINDOW_HOOK(S,T) \
static __typeof__(&CreateWindowEx##S) create_##S; \
static PROCESS_HOOK_UNWIND_FUNCTION HWND WINAPI window_##S(DWORD ex,T cls,T text,DWORD style,int x,int y,int cx,int cy,HWND parent,HMENU menu,HINSTANCE instance,void *param) \
{ process_hook_unwind_scope unwind;title_scope scope;HWND result=NULL;DWORD error=ERROR_SUCCESS; \
  if(startup_window_created){ \
    result=create_##S(ex,cls,text,style,x,y,cx,cy,parent,menu,instance,param);error=GetLastError(); \
    if(result)startup_window_created(result,style); \
    SetLastError(error);return result; \
  } \
  if(!begin_titles(&scope,FALSE))return NULL; \
  PROCESS_HOOK_UNWIND_BEGIN(unwind,end_titles,&scope); { result=create_##S(ex,cls,text,style,x,y,cx,cy,parent,menu,instance,param);error=GetLastError(); \
  } PROCESS_HOOK_UNWIND_END(unwind); \
  if(scope.failure){if(result)DestroyWindow(result);result=NULL;error=scope.failure;} \
  if(result && scope.enabled){mark_menu_owner(result,__builtin_return_address(0));} SetLastError(error);return result; }
WINDOW_HOOK(A,LPCSTR)
WINDOW_HOOK(W,LPCWSTR)

/* Some modern desktop windows use this optional USER32 entry point rather
 * than CreateWindowExW. Preserve its band and use the same creation scope. */
typedef HWND (WINAPI *create_window_band_fn)(DWORD,LPCWSTR,LPCWSTR,DWORD,int,int,int,int,HWND,HMENU,HINSTANCE,void *,DWORD);
static create_window_band_fn create_band;
static PROCESS_HOOK_UNWIND_FUNCTION HWND WINAPI window_band(DWORD ex,LPCWSTR cls,LPCWSTR text,DWORD style,int x,int y,int cx,int cy,HWND parent,HMENU menu,HINSTANCE instance,void *param,DWORD band)
{
    process_hook_unwind_scope unwind;title_scope scope;HWND result=NULL;DWORD error=ERROR_SUCCESS;
    if(!begin_titles(&scope,FALSE))return NULL;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,end_titles,&scope);
    {result=create_band(ex,cls,text,style,x,y,cx,cy,parent,menu,instance,param,band);error=GetLastError();}
    PROCESS_HOOK_UNWIND_END(unwind);
    if(scope.failure){if(result)DestroyWindow(result);result=NULL;error=scope.failure;}
    if(result && scope.enabled)mark_menu_owner(result,__builtin_return_address(0));
    SetLastError(error);return result;
}

static BOOL startup_observes_dialogs;

static LRESULT CALLBACK startup_dialog_return(int code,WPARAM wp,LPARAM lp)
{
    DWORD incoming=GetLastError();
    const CWPRETSTRUCT *event=style_message_dialog_initialized(code,lp);
    if(event)ntw16s_observed_dialog_initialize(event->hwnd);
    SetLastError(incoming);
    return CallNextHookEx(NULL,code,wp,lp);
}

static BOOL begin_standard_dialog(style_dialog_observation *scope,DWORD incoming)
{
    (void)incoming;
    if(startup_observes_dialogs)
        return style_dialog_observation_begin(scope,NULL,startup_dialog_return,FALSE);
    return begin_titles(&scope->creation,TRUE);
}

static void prepare_standard_dialog(style_dialog_observation *scope,DWORD incoming)
{
    (void)scope;
    if(startup_observes_dialogs)SetLastError(incoming);
}

static void end_standard_dialog(void *data)
{
    style_dialog_observation *scope=data;
    if(startup_observes_dialogs)style_dialog_observation_end(scope);
    else end_titles(&scope->creation);
}

#define TITLE_DIALOG_BEGIN(state,incoming) ((void)(incoming),begin_titles((state),TRUE))
#define TITLE_DIALOG_PREPARE(state,incoming) ((void)(state),(void)(incoming))
#define STANDARD_DIALOG_FINISH(state,modal,result,error,fail) do { \
    if(startup_observes_dialogs){ \
        if(!(modal) && (result))ntw16s_observed_dialog_initialize((HWND)(INT_PTR)(result)); \
    }else if((state)->creation.failure){ \
        if(!(modal) && (result))DestroyWindow((HWND)(INT_PTR)(result)); \
        (result)=(fail);(error)=(state)->creation.failure; \
    } \
} while(0)
#define DIALOG_HOOK(NAME,TYPE,RET,MODAL,FAIL) \
    STYLE_DIALOG_HOOK(NAME,(HINSTANCE instance,TYPE name,HWND parent,DLGPROC proc,LPARAM param),(instance,name,parent,proc,param),RET,FAIL,MODAL,style_dialog_observation,begin_standard_dialog,prepare_standard_dialog,end_standard_dialog,STANDARD_DIALOG_FINISH)
DIALOG_HOOK(CreateDialogParamA,LPCSTR,HWND,FALSE,NULL)
DIALOG_HOOK(CreateDialogParamW,LPCWSTR,HWND,FALSE,NULL)
DIALOG_HOOK(CreateDialogIndirectParamA,LPCDLGTEMPLATEA,HWND,FALSE,NULL)
DIALOG_HOOK(CreateDialogIndirectParamW,LPCDLGTEMPLATEW,HWND,FALSE,NULL)
DIALOG_HOOK(DialogBoxParamA,LPCSTR,INT_PTR,TRUE,-1)
DIALOG_HOOK(DialogBoxParamW,LPCWSTR,INT_PTR,TRUE,-1)
DIALOG_HOOK(DialogBoxIndirectParamA,LPCDLGTEMPLATEA,INT_PTR,TRUE,-1)
DIALOG_HOOK(DialogBoxIndirectParamW,LPCDLGTEMPLATEW,INT_PTR,TRUE,-1)

/* Find/Replace are modeless common dialogs.  Their internal dialog creation
 * can bypass exported CreateDialog hooks, so keep the ordinary creation scope
 * active around the public entry point and return the original HWND. */
#define TITLE_PARAM_FINISH(state,rollback,result,error,fail) do { \
    if((state)->failure){rollback;(result)=(fail);(error)=(state)->failure;} \
} while(0)
#define SIMPLE_DIALOG_HOOK(NAME,PARAM,RET,FAIL,CLEANUP,ROUTE) \
    STYLE_DIALOG_HOOK_ROUTED(NAME,(PARAM parameters),(parameters),RET,FAIL,CLEANUP,title_scope,TITLE_DIALOG_BEGIN,TITLE_DIALOG_PREPARE,end_titles,TITLE_PARAM_FINISH,ROUTE)
#define FIND_DIALOG_HOOK(NAME,PARAM,CALL,COMPACT) \
    SIMPLE_DIALOG_HOOK(NAME,PARAM *,HWND,NULL,if(result)DestroyWindow(result), \
        const classic_dialog_policy policy=ntw16s_dialog_policy(); \
        if(policy.find<=CLASSIC_STOCK_NATIVE)return real_##NAME(parameters); \
        if(policy.find==CLASSIC_STOCK_OBSERVED_LAYOUT || policy.window_provider==CLASSIC_WINDOW_PROVIDER_NONCLIENT) \
            return CALL(parameters,real_##NAME,&ntw16s_observed_dialog_windows,COMPACT && policy.find==CLASSIC_STOCK_OBSERVED_LAYOUT))
FIND_DIALOG_HOOK(FindTextA,FINDREPLACEA,style_observed_find_a,TRUE)
FIND_DIALOG_HOOK(FindTextW,FINDREPLACEW,style_observed_find_w,TRUE)
FIND_DIALOG_HOOK(ReplaceTextA,FINDREPLACEA,style_observed_find_a,FALSE)
FIND_DIALOG_HOOK(ReplaceTextW,FINDREPLACEW,style_observed_find_w,FALSE)
#undef FIND_DIALOG_HOOK

/* Font selection uses owner-drawn system combos.  Observe only the current
 * thread and only the standard font dialog's three required combo IDs; keep
 * the application's hook and FIND/COMDLG argument addresses untouched. */
static LRESULT CALLBACK font_dialog_return(int code,WPARAM wparam,LPARAM lparam)
{
    const CWPRETSTRUCT *message=style_message_dialog_initialized(code,lparam);
    if(message && style_classic_font_template(message->hwnd)){
        ntw16s_observed_dialog_initialize(message->hwnd);
        ntw16s_font_dialog_chrome(message->hwnd);
    }
    return CallNextHookEx(NULL,code,wparam,lparam);
}
/* The system's colour picker subclasses several STATIC surfaces and owns
 * their colour-grid painting.  Do not replace these clients with our generic
 * STATIC frame painter; the surrounding standard controls remain styled. */
static void color_dialog_remove_surface(HWND window){(void)Ntw16sWindowRemove(window);}
static LRESULT CALLBACK color_dialog_return(int code,WPARAM wparam,LPARAM lparam)
{
    const CWPRETSTRUCT *message=style_message_dialog_initialized(code,lparam);
    if(message && style_classic_color_template(message->hwnd)){
        const int ids[]={720,721,710,702,709};
        ntw16s_observed_dialog_initialize(message->hwnd);
        style_dialog_observation_found(message->hwnd);
        style_classic_color_controls(message->hwnd,ids,ARRAYSIZE(ids),color_dialog_remove_surface);
    }
    return CallNextHookEx(NULL,code,wparam,lparam);
}
#define OBSERVED_DIALOG_HOOK(NAME,TYPE,FIELD,OBSERVER,TRACKED,CALL) \
static __typeof__(&NAME) real_##NAME; \
static PROCESS_HOOK_UNWIND_FUNCTION BOOL WINAPI hook_##NAME(TYPE *parameters) \
{ process_hook_unwind_scope unwind;style_dialog_observation scope;BOOL result=FALSE;DWORD error=ERROR_SUCCESS; \
  const classic_dialog_policy policy=ntw16s_dialog_policy(); \
  if(policy.FIELD<=CLASSIC_STOCK_NATIVE)return real_##NAME(parameters); \
  if(policy.FIELD==CLASSIC_STOCK_OBSERVED_LAYOUT)return CALL(parameters,real_##NAME,&ntw16s_observed_dialog_windows); \
  style_creation_context *creation=policy.window_provider==CLASSIC_WINDOW_PROVIDER_NONCLIENT?NULL:&title_context; \
  if(!style_dialog_observation_begin(&scope,creation,OBSERVER,TRACKED))return FALSE; \
  PROCESS_HOOK_UNWIND_BEGIN(unwind,style_dialog_observation_end,&scope); { result=real_##NAME(parameters);error=GetLastError(); \
  } PROCESS_HOOK_UNWIND_END(unwind); \
  if(scope.creation.failure){result=FALSE;error=scope.creation.failure;}SetLastError(error);return result; }
OBSERVED_DIALOG_HOOK(ChooseFontA,CHOOSEFONTA,font,font_dialog_return,FALSE,style_observed_font_a)
OBSERVED_DIALOG_HOOK(ChooseFontW,CHOOSEFONTW,font,font_dialog_return,FALSE,style_observed_font_w)
OBSERVED_DIALOG_HOOK(ChooseColorA,CHOOSECOLORA,color,color_dialog_return,TRUE,style_observed_color_a)
OBSERVED_DIALOG_HOOK(ChooseColorW,CHOOSECOLORW,color,color_dialog_return,TRUE,style_observed_color_w)
#undef OBSERVED_DIALOG_HOOK

/* An already-installed observation provider owns these six entries when
 * a later attachment also installs the general window hooks. */
static BOOL startup_observes_print;

/* A no-op print hook selects the system's traditional PrintDlg layout.
 * Preserve caller-owned customization and less common flag contracts unchanged. */
#define PRINT_HOOK(NAME,TYPE,SUFFIX) \
static __typeof__(&NAME) real_##NAME; \
static PROCESS_HOOK_UNWIND_FUNCTION BOOL WINAPI hook_##NAME(TYPE *parameters) \
{ process_hook_unwind_scope unwind;style_dialog_observation scope;BOOL result=FALSE; \
  DWORD incoming=GetLastError(),error=ERROR_SUCCESS; \
  enum classic_print_route route=ntw16s_dialog_policy().print; \
  if(route==CLASSIC_PRINT_NATIVE)return real_##NAME(parameters); \
  if(route==CLASSIC_PRINT_GENERAL_PAGE)return style_observed_print_##SUFFIX(parameters,real_##NAME,&ntw16s_observed_dialog_windows); \
  if(!begin_standard_dialog(&scope,incoming))return FALSE; \
  PROCESS_HOOK_UNWIND_BEGIN(unwind,end_standard_dialog,&scope); { \
    prepare_standard_dialog(&scope,incoming); \
    result=style_classic_print_call_##SUFFIX(parameters,real_##NAME);error=GetLastError(); \
  } PROCESS_HOOK_UNWIND_END(unwind); \
  STANDARD_DIALOG_FINISH(&scope,TRUE,result,error,FALSE);SetLastError(error);return result; }
PRINT_HOOK(PrintDlgA,PRINTDLGA,a)
PRINT_HOOK(PrintDlgW,PRINTDLGW,w)
#undef PRINT_HOOK

/* Synchronous common dialogs also create their own internal windows. */
#define PAGE_SETUP_HOOK(NAME,TYPE,SUFFIX) \
    STYLE_DIALOG_HOOK_ROUTED(NAME,(TYPE *parameters),(parameters),BOOL,FALSE,TRUE,style_dialog_observation, \
        begin_standard_dialog,prepare_standard_dialog,end_standard_dialog,STANDARD_DIALOG_FINISH, \
        enum classic_print_route route=ntw16s_dialog_policy().print; \
        if(route==CLASSIC_PRINT_NATIVE)return real_##NAME(parameters); \
        if(route==CLASSIC_PRINT_GENERAL_PAGE)return style_observed_page_setup_##SUFFIX(parameters,real_##NAME,&ntw16s_observed_dialog_windows))
PAGE_SETUP_HOOK(PageSetupDlgA,PAGESETUPDLGA,a)
PAGE_SETUP_HOOK(PageSetupDlgW,PAGESETUPDLGW,w)
#undef PAGE_SETUP_HOOK
#define PRINT_EXTENDED_HOOK(NAME,TYPE,GENERAL,BRIDGE,PRINT) \
static __typeof__(&NAME) real_##NAME; \
static PROCESS_HOOK_UNWIND_FUNCTION HRESULT WINAPI hook_##NAME(TYPE *parameters) \
{ DWORD incoming=GetLastError();enum classic_print_route route=ntw16s_dialog_policy().print;HRESULT result=E_FAIL; \
  SetLastError(incoming); \
  if(route==CLASSIC_PRINT_NATIVE)return real_##NAME(parameters); \
  if(startup_observes_print){ \
    style_dialog_observation observation;process_hook_unwind_scope observe_unwind;DWORD outgoing; \
    if(!style_dialog_observation_begin(&observation,NULL,startup_dialog_return,FALSE))return E_FAIL; \
    PROCESS_HOOK_UNWIND_BEGIN(observe_unwind,style_dialog_observation_end,&observation); \
    SetLastError(incoming); \
    if((route!=CLASSIC_PRINT_GENERAL_PAGE || !GENERAL(ntw16s_observed_dialog_windows.resources(),parameters,&result,classic_theme_dialog_resources)) && \
       !BRIDGE(parameters,PRINT,&result))result=real_##NAME(parameters); \
    outgoing=GetLastError();PROCESS_HOOK_UNWIND_END(observe_unwind);SetLastError(outgoing);return result; \
  } \
  if(route==CLASSIC_PRINT_GENERAL_PAGE){ \
    HMODULE resources=ntw16s_observed_dialog_windows.resources(); \
    if((resources && GENERAL(resources,parameters,&result,classic_theme_dialog_resources)) || BRIDGE(parameters,PRINT,&result))return result; \
    SetLastError(incoming);return real_##NAME(parameters); \
  } \
  process_hook_unwind_scope unwind;title_scope scope;DWORD error=ERROR_SUCCESS; \
  if(!begin_titles(&scope,TRUE))return HRESULT_FROM_WIN32(GetLastError()); \
  PROCESS_HOOK_UNWIND_BEGIN(unwind,end_titles,&scope); { \
    if(!BRIDGE(parameters,PRINT,&result)){result=real_##NAME(parameters);}error=GetLastError(); \
  } PROCESS_HOOK_UNWIND_END(unwind); \
  if(scope.failure){result=HRESULT_FROM_WIN32(scope.failure);error=scope.failure;}SetLastError(error);return result; }
PRINT_EXTENDED_HOOK(PrintDlgExA,PRINTDLGEXA,style_print_general_a,style_print_extended_a,real_PrintDlgA)
PRINT_EXTENDED_HOOK(PrintDlgExW,PRINTDLGEXW,style_print_general_w,style_print_extended_w,real_PrintDlgW)
#undef PRINT_EXTENDED_HOOK

/* Property sheets may create internal pages without public DialogBox calls. */
SIMPLE_DIALOG_HOOK(PropertySheetA,const PROPSHEETHEADERA *,INT_PTR,-1,(void)0,(void)0)
SIMPLE_DIALOG_HOOK(PropertySheetW,const PROPSHEETHEADERW *,INT_PTR,-1,(void)0,(void)0)
#undef SIMPLE_DIALOG_HOOK

/* TaskDialog lives in ComCtl32 v6; bind only when that module exports it. */
static __typeof__(&TaskDialog) real_TaskDialog;
static __typeof__(&TaskDialogIndirect) real_TaskDialogIndirect;
#define TASK_DIALOG_HOOK(NAME,PARAMETERS,ARGUMENTS) \
static PROCESS_HOOK_UNWIND_FUNCTION HRESULT WINAPI hook_##NAME PARAMETERS \
{ \
    process_hook_unwind_scope unwind;title_scope scope;HRESULT result=E_FAIL;DWORD error=ERROR_SUCCESS; \
    if(!begin_titles(&scope,TRUE))return HRESULT_FROM_WIN32(GetLastError()); \
    PROCESS_HOOK_UNWIND_BEGIN(unwind,end_titles,&scope); { \
        result=real_##NAME ARGUMENTS;error=GetLastError(); \
    } PROCESS_HOOK_UNWIND_END(unwind); \
    if(scope.failure){result=HRESULT_FROM_WIN32(scope.failure);error=scope.failure;}SetLastError(error);return result; \
}
TASK_DIALOG_HOOK(TaskDialog,(HWND owner,HINSTANCE instance,PCWSTR title,PCWSTR main_instruction,PCWSTR content,TASKDIALOG_COMMON_BUTTON_FLAGS buttons,PCWSTR icon,int *button),
    (owner,instance,title,main_instruction,content,buttons,icon,button))
TASK_DIALOG_HOOK(TaskDialogIndirect,(const TASKDIALOGCONFIG *config,int *button,int *radio,BOOL *verification),
    (config,button,radio,verification))
#undef TASK_DIALOG_HOOK

/* MessageBox creates its own dialog inside USER rather than necessarily
 * calling an exported DialogBox entry. Keep its API/return contract and use
 * the same bounded creation scope as the other modal entry points. */
#define MESSAGE_HOOK(NAME,PARAMETERS,ARGUMENTS) \
    STYLE_DIALOG_HOOK(NAME,PARAMETERS,ARGUMENTS,int,0,TRUE,style_dialog_observation,begin_standard_dialog,prepare_standard_dialog,end_standard_dialog,STANDARD_DIALOG_FINISH)
MESSAGE_HOOK(MessageBoxA,(HWND owner,LPCSTR text,LPCSTR caption,UINT type),(owner,text,caption,type))
MESSAGE_HOOK(MessageBoxW,(HWND owner,LPCWSTR text,LPCWSTR caption,UINT type),(owner,text,caption,type))
MESSAGE_HOOK(MessageBoxExA,(HWND owner,LPCSTR text,LPCSTR caption,UINT type,WORD language),(owner,text,caption,type,language))
MESSAGE_HOOK(MessageBoxExW,(HWND owner,LPCWSTR text,LPCWSTR caption,UINT type,WORD language),(owner,text,caption,type,language))
MESSAGE_HOOK(MessageBoxIndirectA,(const MSGBOXPARAMSA *parameters),(parameters))
MESSAGE_HOOK(MessageBoxIndirectW,(const MSGBOXPARAMSW *parameters),(parameters))
#undef MESSAGE_HOOK
#undef STANDARD_DIALOG_FINISH

/* The DLL uses the three original native leaves bound at load, so application
 * reentry is never accidentally bypassed by an ambient recursion guard. */
static __typeof__(&SetScrollPos) real_set_pos;
static __typeof__(&SetScrollRange) real_set_range;
static __typeof__(&ShowScrollBar) real_show_scroll;
static __typeof__(&EnableScrollBar) real_enable_scroll;
static HRESULT WINAPI hook_set_theme(HWND w,LPCWSTR application,LPCWSTR ids)
{
    if(startup_window_created)return style_theme_intercept(w,application,ids,ntwshk_observed_native_theme);
    if(!InterlockedCompareExchange(&style_ready,0,0))return real_set_theme(w,application,ids);
    return style_theme_intercept(w,application,ids,ntw16s_native_set_window_theme);
}
static BOOL eligible_scroll(HWND w)
{
    DWORD saved=GetLastError();
    BOOL eligible=InterlockedCompareExchange(&style_ready,0,0) &&
        (Ntw16sWindowAttached(w) || ntw16s_managed_scrollbars_active(w));
    SetLastError(saved);return eligible;
}
static void redraw_scroll(HWND w)
{
    if(ntw16s_managed_scrollbars_active(w))win16_style_scrollbars_paint(w,NULL,NULL);
    else Ntw16sScrollChanged(w);
}
static BOOL WINAPI hook_get_scroll(HWND w,int bar,LPSCROLLINFO info)
{
    if(!eligible_scroll(w))return real_get_scroll(w,bar,info);
    return win16_style_get_scroll_info(w,bar,info);
}
static int WINAPI hook_set_scroll(HWND w,int bar,LPCSCROLLINFO info,BOOL redraw)
{
    if(!eligible_scroll(w))return real_set_scroll(w,bar,info,redraw);
    return win16_style_set_scroll_info(w,bar,info,redraw);
}
static int WINAPI hook_set_pos(HWND w,int bar,int position,BOOL redraw)
{
    int result;DWORD error;
    if(!eligible_scroll(w))return real_set_pos(w,bar,position,redraw);
    result=real_set_pos(w,bar,position,FALSE);error=GetLastError();if(redraw)redraw_scroll(w);
    SetLastError(error);return result;
}
static BOOL WINAPI hook_set_range(HWND w,int bar,int first,int last,BOOL redraw)
{
    BOOL result;DWORD error;
    if(!eligible_scroll(w))return real_set_range(w,bar,first,last,redraw);
    result=real_set_range(w,bar,first,last,FALSE);error=GetLastError();if(result && redraw)redraw_scroll(w);
    SetLastError(error);return result;
}
static BOOL WINAPI hook_show_scroll(HWND w,int bar,BOOL show)
{
    BOOL result;DWORD error;
    if(!eligible_scroll(w))return real_show_scroll(w,bar,show);
    result=real_show_scroll(w,bar,show);error=GetLastError();if(result)redraw_scroll(w);
    SetLastError(error);return result;
}
static BOOL WINAPI hook_enable_scroll(HWND w,UINT bar,UINT arrows)
{
    BOOL result;DWORD error;
    if(!eligible_scroll(w))return real_enable_scroll(w,bar,arrows);
    result=real_enable_scroll(w,bar,arrows);error=GetLastError();redraw_scroll(w);
    SetLastError(error);return result;
}

/* Process-local native text leaves. Outside a verified VB6/report paint scope these
 * are transparent trampolines. The temporary font is restored before any
 * lease can be released; no application HFONT/property/layout is modified. */
static _Thread_local style_font_gate vb_text_gate;
/* The report adapter promises only whole, reversible CP1252 runs. Keep
 * measurements and unsupported language/RTL runs on the native font. */
static BOOL report_wide_text(HDC dc,const WCHAR *text,int count)
{return style_text_single_byte_wide(dc,text,count,style_runtime_text_report_active(),1252);}
static BOOL report_ansi_text(HDC dc,const char *text,int count)
{return style_text_single_byte_ansi(dc,text,count,style_runtime_text_report_active(),1252);}
#define VB_TEXT_HOOK(NAME,TYPE,PARAMS,ARGS,ALLOW) \
static __typeof__(&NAME) real_##NAME; \
static TYPE WINAPI hook_##NAME PARAMS \
{ style_font_scope scope;DWORD entry=GetLastError(),error;TYPE result; \
  if(style_font_gate_begin(&vb_text_gate,&scope,dc) && (ALLOW) && style_runtime_text_active() && style_runtime_text_caller(__builtin_return_address(0)))style_font_scope_select(&scope,style_runtime_vb_paint_font(dc)); \
  SetLastError(entry);result=real_##NAME ARGS;error=GetLastError(); \
  style_font_scope_end(&scope);style_font_gate_end(&vb_text_gate);SetLastError(error);return result; }
VB_TEXT_HOOK(DrawTextA,int,(HDC dc,LPCSTR text,int count,LPRECT rect,UINT format),(dc,text,count,rect,format),((!style_runtime_text_report_active() && style_runtime_text_direct_allowed()) || !(format&DT_CALCRECT)) && style_runtime_text_label_caller(__builtin_return_address(0)) && report_ansi_text(dc,text,count))
VB_TEXT_HOOK(DrawTextExA,int,(HDC dc,LPSTR text,int count,LPRECT rect,UINT format,LPDRAWTEXTPARAMS params),(dc,text,count,rect,format,params),((!style_runtime_text_report_active() && style_runtime_text_direct_allowed()) || !(format&DT_CALCRECT)) && style_runtime_text_label_caller(__builtin_return_address(0)) && report_ansi_text(dc,text,count))
VB_TEXT_HOOK(TextOutA,BOOL,(HDC dc,int x,int y,LPCSTR text,int count),(dc,x,y,text,count),(style_runtime_text_direct_allowed() || (GetMapMode(dc)==MM_TEXT && !(GetTextAlign(dc)&TA_UPDATECP))) && style_runtime_text_label_caller(__builtin_return_address(0)) && report_ansi_text(dc,text,count))
VB_TEXT_HOOK(ExtTextOutA,BOOL,(HDC dc,int x,int y,UINT options,const RECT *rect,LPCSTR text,UINT count,const INT *dx),(dc,x,y,options,rect,text,count,dx),style_runtime_text_direct_allowed() && (!style_runtime_text_report_active() || !(options&(ETO_GLYPH_INDEX|ETO_RTLREADING))) && report_ansi_text(dc,text,(int)count))
VB_TEXT_HOOK(TextOutW,BOOL,(HDC dc,int x,int y,LPCWSTR text,int count),(dc,x,y,text,count),style_runtime_text_direct_allowed() && report_wide_text(dc,text,count))

#undef VB_TEXT_HOOK

void ntw16s_rich_witness(WNDPROC,HWND,UINT,const void *,BOOL);
void ntw16s_box_witness(HWND,UINT,const void *);
static __typeof__(&DefWindowProcW) real_DefWindowProcW;
static LRESULT WINAPI hook_DefWindowProcW(HWND w,UINT msg,WPARAM wp,LPARAM lp)
{DWORD entry=GetLastError();ntw16s_box_witness(w,msg,__builtin_return_address(0));SetLastError(entry);return real_DefWindowProcW(w,msg,wp,lp);}
void ntw16s_dbedit_ansi_witness(WNDPROC,HWND,UINT,const void *);
void ntw16s_dbcombo_ansi_witness(WNDPROC,HWND,UINT,const void *);
static __typeof__(&CallWindowProcA) real_CallWindowProcA;
static LRESULT WINAPI hook_CallWindowProcA(WNDPROC proc,HWND w,UINT message,WPARAM wp,LPARAM lp)
{DWORD entry=GetLastError();ntw16s_dbcombo_ansi_witness(proc,w,message,__builtin_return_address(0));ntw16s_dbedit_ansi_witness(proc,w,message,__builtin_return_address(0));ntw16s_rich_witness(proc,w,message,__builtin_return_address(0),FALSE);SetLastError(entry);return real_CallWindowProcA(proc,w,message,wp,lp);}
static __typeof__(&CallWindowProcW) real_CallWindowProcW;
static PROCESS_HOOK_UNWIND_FUNCTION LRESULT WINAPI hook_CallWindowProcW(WNDPROC proc,HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    style_runtime_native_call call;process_hook_unwind_scope unwind;LRESULT result;DWORD entry=GetLastError(),error;
    ntw16s_rich_witness(proc,w,message,__builtin_return_address(0),TRUE);
    if(!ntw16s_static_call_begin(&call,proc,w,message,__builtin_return_address(0))){
        SetLastError(entry);return real_CallWindowProcW(proc,w,message,wp,lp);
    }
    PROCESS_HOOK_UNWIND_BEGIN(unwind,style_runtime_native_call_end,&call);
    SetLastError(entry);result=real_CallWindowProcW(proc,w,message,wp,lp);error=GetLastError();
    PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result;
}
HFONT ntw16s_status_simple_font(HDC,LPCWSTR,int,UINT,const void *,void **);
void ntw16s_status_simple_return(void *,HFONT);
static __typeof__(&DrawTextW) real_DrawTextW;
static PROCESS_HOOK_UNWIND_FUNCTION int WINAPI hook_DrawTextW(HDC dc,LPCWSTR text,int count,LPRECT rect,UINT flags)
{
    DWORD entry=GetLastError(),error;HFONT font=ntw16s_static_text_font(dc,text,count,flags,__builtin_return_address(0));
    if(!font){SetLastError(entry);return real_DrawTextW(dc,text,count,rect,flags);}
    style_font_scope borrow={.dc=dc,.font=font,.release=release_system_font};
    style_font_scope_select(&borrow,font);process_hook_unwind_scope unwind;int result;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,style_font_scope_end,&borrow);
    SetLastError(entry);result=real_DrawTextW(dc,text,count,rect,flags);error=GetLastError();
    PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result;
}


static __typeof__(&DrawTextExW) real_DrawTextExW;
static PROCESS_HOOK_UNWIND_FUNCTION int WINAPI hook_DrawTextExW(HDC dc,LPWSTR text,int count,LPRECT rect,UINT flags,LPDRAWTEXTPARAMS params)
{
    DWORD entry=GetLastError(),error;void *token=NULL;HFONT font=ntw16s_status_simple_font(dc,text,count,flags,__builtin_return_address(0),&token);
    if(!font){SetLastError(entry);return real_DrawTextExW(dc,text,count,rect,flags,params);}
    style_font_scope borrow={.dc=dc,.font=font,.context=token,.release=ntw16s_status_simple_return};
    style_font_scope_select(&borrow,font);process_hook_unwind_scope unwind;int result;
    PROCESS_HOOK_UNWIND_BEGIN(unwind,style_font_scope_end,&borrow);
    SetLastError(entry);result=real_DrawTextExW(dc,text,count,rect,flags,params);error=GetLastError();
    PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result;
}


/* USER's EDIT can submit shaped glyph IDs. Those IDs belong to its source
 * font, not the replacement FNT. Reverse only verified CP1252 glyphs; any
 * missing/complex glyph leaves the whole native run unchanged. Keep its dx,
 * ETO_PDY, clipping and visual order, and do not reshape the recovered run. */
static __typeof__(&ExtTextOutW) real_ExtTextOutW;
static BOOL WINAPI hook_ExtTextOutW(HDC dc,int x,int y,UINT options,const RECT *rect,LPCWSTR text,UINT count,const INT *dx)
{
    DWORD entry=GetLastError(),error;HFONT font=NULL;style_font_scope scope;style_glyph_text glyphs={0};BOOL result;LPCWSTR original_text=text;UINT original_options=options;
    if(style_font_gate_begin(&vb_text_gate,&scope,dc) && style_runtime_text_direct_allowed() && style_runtime_text_active() && style_runtime_text_caller(__builtin_return_address(0))){
        font=style_runtime_vb_paint_font(dc);
        if(font && (options&ETO_GLYPH_INDEX)){
            if(!style_text_glyph_run_allowed(dc,options,count) ||
                !style_glyph_text_prepare(&glyphs,dc,text,count,1252,style_runtime_vb_decode_glyphs))font=NULL;
            else{text=glyphs.decoded;options&=~ETO_GLYPH_INDEX;}
        }
        if(font && !(options&ETO_GLYPH_INDEX) &&
            (!report_wide_text(dc,text,(int)count) || (style_runtime_text_report_active() && (options&ETO_RTLREADING))))font=NULL;
        style_font_scope_select(&scope,font);
        if(!scope.previous){
            text=original_text;options=original_options;
            style_glyph_text_release_ansi(&glyphs);
        }
    }
    SetLastError(entry);
    if(scope.previous && glyphs.encoded)result=real_ExtTextOutA(dc,x,y,options,rect,glyphs.encoded,count,dx);
    else result=real_ExtTextOutW(dc,x,y,options,rect,text,count,dx);
    error=GetLastError();
    style_font_scope_end(&scope);
    style_glyph_text_end(&glyphs);
    style_font_gate_end(&vb_text_gate);SetLastError(error);return result;
}

static __typeof__(&DrawFrameControl) real_DrawFrameControl;
static __typeof__(&DrawEdge) real_DrawEdge;
static __typeof__(&DrawIconEx) real_DrawIconEx;
static BOOL WINAPI hook_DrawIconEx(HDC dc,int x,int y,HICON icon,int cx,int cy,UINT step,HBRUSH brush,UINT flags)
{
    DWORD saved=GetLastError();
    ntw16s_status_icon_frame(dc,x,y,cx,cy,icon,__builtin_return_address(0));
    SetLastError(saved);return real_DrawIconEx(dc,x,y,icon,cx,cy,step,brush,flags);
}
void ntw16s_month_header_text(HTHEME,HDC,int,int,const RECT *,DWORD,const void *);
void ntw16s_month_selection_background(HTHEME,HDC,int,int,const RECT *,const RECT *,const void *);
void ntw16s_month_selection_text(HTHEME,HDC,int,int,const RECT *,DWORD,const void *);
static __typeof__(&DrawThemeText) real_DrawThemeText;
static PROCESS_HOOK_UNWIND_FUNCTION HRESULT WINAPI hook_DrawThemeText(HTHEME theme,HDC dc,int part,int state,LPCWSTR text,int count,DWORD flags,DWORD flags2,const RECT *rect)
{
    DWORD saved=GetLastError(),error;HRESULT result;style_control_status_theme scope;process_hook_unwind_scope unwind;
    ntw16s_month_header_text(theme,dc,part,state,rect,flags,__builtin_return_address(0));
    ntw16s_month_selection_text(theme,dc,part,state,rect,flags,__builtin_return_address(0));
    ntw16s_status_text_frame(dc,rect,flags,__builtin_return_address(0));
    BOOL entered=ntw16s_status_theme_begin(&scope,dc,part,state,text,count,flags,rect,__builtin_return_address(0));
    if(!entered && text){
        int n=0;BOOL supported=style_control_month_text_length(text,count,&n);
        HFONT font=supported?ntw16s_month_text_borrow(theme,dc,part,state,flags,flags2,rect,__builtin_return_address(0)):NULL;
        if(font){style_font_scope borrow={.dc=dc,.font=font,.saved=SaveDC(dc),.release=release_system_font};BOOL done=FALSE;process_hook_unwind_scope cleanup;PROCESS_HOOK_UNWIND_BEGIN(cleanup,style_font_scope_end,&borrow);
          if(borrow.saved){month_text_room_context room={theme,dc,part,state,flags,flags2,rect,__builtin_return_address(0)};done=style_control_month_text(theme,dc,part,state,font,text,n,flags,rect,real_DrawTextW,month_text_room,&room);}
          PROCESS_HOOK_UNWIND_END(cleanup);if(done){SetLastError(saved);return S_OK;}}}
    if(!entered){SetLastError(saved);return real_DrawThemeText(theme,dc,part,state,text,count,flags,flags2,rect);}
    PROCESS_HOOK_UNWIND_BEGIN(unwind,ntw16s_status_theme_end,&scope);
    SetLastError(saved);result=real_DrawThemeText(theme,dc,part,state,text,count,flags,flags2,rect);error=GetLastError();
    PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result;
}
void ntw16s_group_frame(HDC,int,int,const RECT *,const RECT *,LPCWSTR,int,DWORD,const void *,BOOL);
static __typeof__(&DrawThemeTextEx) real_DrawThemeTextEx;
static HRESULT WINAPI hook_DrawThemeTextEx(HTHEME theme,HDC dc,int part,int state,LPCWSTR text,int count,DWORD flags,LPRECT rect,const DTTOPTS *options)
{
 HRESULT result=real_DrawThemeTextEx(theme,dc,part,state,text,count,flags,rect,options);DWORD error=GetLastError();
 if(SUCCEEDED(result))ntw16s_group_frame(dc,part,state,rect,NULL,text,count,flags,__builtin_return_address(0),TRUE);
 SetLastError(error);return result;
}
static __typeof__(&OpenThemeData) real_OpenThemeData;
static __typeof__(&OpenThemeDataEx) real_OpenThemeDataEx;
static __typeof__(&CloseThemeData) real_CloseThemeData;
typedef HTHEME (WINAPI *theme_dpi_fn)(HWND,LPCWSTR,UINT);
static theme_dpi_fn real_OpenThemeDataForDpi;
#define THEME_OPEN_HOOK(NAME,PARAMETERS,ARGUMENTS) \
static PROCESS_HOOK_UNWIND_FUNCTION HTHEME WINAPI hook_##NAME PARAMETERS \
{ \
    DWORD entry=GetLastError(),error;HTHEME h;process_hook_unwind_scope unwind; \
    style_theme_track_scope scope={TRUE,FALSE,NULL};style_theme_track_ticket ticket=style_theme_track_open_begin(); \
    PROCESS_HOOK_UNWIND_BEGIN(unwind,style_theme_track_scope_end,&scope); \
    SetLastError(entry);h=real_##NAME ARGUMENTS;error=GetLastError(); \
    style_theme_track_open_end(ticket,h,h?style_theme_classify(name):STYLE_THEME_OTHER);scope.active=FALSE; \
    PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return h; \
}
THEME_OPEN_HOOK(OpenThemeData,(HWND w,LPCWSTR name),(w,name))
THEME_OPEN_HOOK(OpenThemeDataEx,(HWND w,LPCWSTR name,DWORD flags),(w,name,flags))
THEME_OPEN_HOOK(OpenThemeDataForDpi,(HWND w,LPCWSTR name,UINT dpi),(w,name,dpi))
#undef THEME_OPEN_HOOK
static PROCESS_HOOK_UNWIND_FUNCTION HRESULT WINAPI hook_CloseThemeData(HTHEME h)
{
    DWORD entry=GetLastError(),error;HRESULT result;process_hook_unwind_scope unwind;
    style_theme_track_scope scope={TRUE,TRUE,h};style_theme_track_close_begin(h);
    PROCESS_HOOK_UNWIND_BEGIN(unwind,style_theme_track_scope_end,&scope);
    SetLastError(entry);result=real_CloseThemeData(h);error=GetLastError();
    style_theme_track_close_end(h,SUCCEEDED(result));scope.active=FALSE;
    PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result;
}
void ntw16s_modern_spin_dc_created(HDC,HDC);
void ntw16s_modern_spin_dc_deleted(HDC);
void ntw16s_modern_spin_selected(HDC);
void ntw16s_modern_spin_object_deleted(HGDIOBJ);
void ntw16s_modern_spin_theme_begin(HDC,const RECT *);
void ntw16s_modern_spin_theme(HDC,int,int,const RECT *,const RECT *,const void *);
void ntw16s_modern_spin_blit(HDC,int,int,int,int,HDC,int,int,DWORD);
static __typeof__(&DrawThemeBackground) real_DrawThemeBackground;
static __typeof__(&BitBlt) real_BitBlt;
static __typeof__(&StretchBlt) real_StretchBlt;
static BOOL WINAPI hook_StretchBlt(HDC dc,int x,int y,int width,int height,HDC source,int sx,int sy,int sw,int sh,DWORD rop)
{
    BOOL result=real_StretchBlt(dc,x,y,width,height,source,sx,sy,sw,sh,rop);DWORD error=GetLastError();
    if(result)win16_style_vb_data_arrow(dc,x,y,width,height,source,sx,sy,sw,sh,rop,__builtin_return_address(0));
    SetLastError(error);return result;
}
static BOOL WINAPI hook_BitBlt(HDC dc,int x,int y,int width,int height,HDC source,int sx,int sy,DWORD rop)
{
    BOOL result=real_BitBlt(dc,x,y,width,height,source,sx,sy,rop);DWORD error=GetLastError();
    if(result)ntw16s_modern_spin_blit(dc,x,y,width,height,source,sx,sy,rop);
    if(result)style_runtime_vb_checklist(dc,x,y,width,height,source,sx,sy,rop,__builtin_return_address(0),win16_style_checklist_glyph);
    SetLastError(error);return result;
}
void ntw16s_panel_polyline(HDC,const POINT *,int,const void *,BOOL (WINAPI *)(HDC,const POINT *,int));
static __typeof__(&Polyline) real_Polyline;
static BOOL WINAPI hook_Polyline(HDC dc,const POINT *points,int count)
{
 BOOL result=real_Polyline(dc,points,count);DWORD error=GetLastError();
 if(result)ntw16s_panel_polyline(dc,points,count,__builtin_return_address(0),real_Polyline);
 SetLastError(error);return result;
}
static __typeof__(&CreateCompatibleDC) real_CreateCompatibleDC;
static __typeof__(&DeleteDC) real_DeleteDC;
static __typeof__(&SelectObject) real_SelectObject;
static __typeof__(&DeleteObject) real_DeleteObject;
static HGDIOBJ WINAPI hook_SelectObject(HDC dc,HGDIOBJ object)
{
    HGDIOBJ old=real_SelectObject(dc,object);DWORD error=GetLastError();
    if(old && old!=HGDI_ERROR)ntw16s_modern_spin_selected(dc);
    SetLastError(error);return old;
}
static BOOL WINAPI hook_DeleteObject(HGDIOBJ object)
{
    BOOL result=real_DeleteObject(object);DWORD error=GetLastError();
    if(result)ntw16s_modern_spin_object_deleted(object);
    SetLastError(error);return result;
}
static BOOL WINAPI hook_DrawFrameControl(HDC dc,LPRECT rect,UINT type,UINT state)
{
    DWORD entry=GetLastError();BOOL drawn=style_control_updown_draw_frame(dc,rect,type,state,__builtin_return_address(0),style_runtime_text_native_target);
    if(drawn){SetLastError(entry);return TRUE;}
    if(startup_window_created && !InterlockedCompareExchange(&style_ready,0,0)){
        SetLastError(entry);return real_DrawFrameControl(dc,rect,type,state);
    }
    RECT spin_original={0};if(rect)spin_original=*rect;
    RECT original;BOOL graphical=win16_style_graphical_frame_prepare(dc,rect,type,state,__builtin_return_address(0),&original);
    SetLastError(entry);BOOL result=real_DrawFrameControl(dc,rect,type,state);DWORD error=GetLastError();
    if(result && graphical)win16_style_graphical_frame(dc,&original,type,state,__builtin_return_address(0));
    if(result)win16_style_checklist_frame(dc,rect,type,state,__builtin_return_address(0));
    if(result && rect)ntw16s_spin_frame(dc,&spin_original,type,state,__builtin_return_address(0));
    SetLastError(error);return result;
}
static BOOL WINAPI hook_DrawEdge(HDC dc,LPRECT rect,UINT edge,UINT flags)
{
    DWORD incoming=GetLastError();
    RECT original={0};BOOL eligible=rect && edge==BDR_SUNKENOUTER && flags==(BF_RECT|BF_ADJUST);
    BOOL controlbar=rect && flags==(BF_RECT|BF_ADJUST) && style_runtime_controlbar_edge_allowed(edge) && ntw16s_controlbar_edge_active();
    BOOL color=FALSE;HWND dialog=style_dialog_observation_current();HMODULE origin=NULL;
    if(rect&&dialog&&edge==EDGE_SUNKEN&&flags==(BF_RECT|BF_ADJUST)&&WindowFromDC(dc)==dialog&&
       GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)__builtin_return_address(0),&origin)&&origin==GetModuleHandleW(L"comdlg32.dll")){
        color=style_classic_color_swatch(dialog,dc,rect);
    }
    if(eligible || controlbar || color)original=*rect;
    SetLastError(incoming);BOOL result=real_DrawEdge(dc,rect,edge,flags);DWORD error=GetLastError();
    if(result&&color)win16_control_color_swatch_frame(dc,rect,&original,win16_style_dialog_edge(dialog));
    if(result && eligible)ntw16s_status_frame(dc,&original,NULL,__builtin_return_address(0));
    if(result && controlbar)ntw16s_controlbar_edge(dc,&original,edge,flags,__builtin_return_address(0));
    SetLastError(error);return result;
}
static HRESULT WINAPI hook_DrawThemeBackground(HTHEME theme,HDC dc,int part,int state,const RECT *rect,const RECT *clip)
{
    DWORD entry=GetLastError();
    if(startup_window_created && !InterlockedCompareExchange(&style_ready,0,0)){
        BOOL drawn=style_control_updown_draw_theme(theme,dc,part,state,rect,clip,
            __builtin_return_address(0),style_runtime_text_native_target);
        SetLastError(entry);return drawn?S_OK:real_DrawThemeBackground(theme,dc,part,state,rect,clip);
    }
    style_theme_track_ticket ticket=style_theme_track_begin(theme,STYLE_THEME_BUTTON);
    style_theme_track_ticket combo={0};if(ntw16s_inplace_paint(dc))combo=style_theme_track_begin(theme,STYLE_THEME_COMBOBOX);
    style_theme_track_ticket glow=style_theme_track_begin(theme,STYLE_THEME_TEXTGLOW);
    BOOL textglow=part==1 && state==0 && glow.eligible;
    if(!textglow)ntw16s_modern_spin_theme_begin(dc,rect);
    BOOL drawn=style_control_updown_draw_theme(theme,dc,part,state,rect,clip,__builtin_return_address(0),style_runtime_text_native_target);
    SetLastError(entry);if(drawn)return S_OK;
    HRESULT result=real_DrawThemeBackground(theme,dc,part,state,rect,clip);DWORD error=GetLastError();
    if(textglow && !style_theme_track_end(theme,glow,STYLE_THEME_TEXTGLOW))ntw16s_modern_spin_theme_begin(dc,NULL);
    if(SUCCEEDED(result) && style_theme_track_end(theme,ticket,STYLE_THEME_BUTTON)){
        ntw16s_nav_theme(dc,part,state,rect,clip,__builtin_return_address(0));
        ntw16s_inplace_button(dc,part,state,rect,clip,__builtin_return_address(0));
        ntw16s_modern_spin_theme(dc,part,state,rect,clip,__builtin_return_address(0));}
    if(SUCCEEDED(result) && combo.eligible && style_theme_track_end(theme,combo,STYLE_THEME_COMBOBOX))ntw16s_inplace_dropdown(dc,part,state,rect,clip,__builtin_return_address(0));
    if(SUCCEEDED(result) && part==1 && state==0)ntw16s_status_frame(dc,rect,clip,__builtin_return_address(0));
    if(SUCCEEDED(result))ntw16s_bit_theme_frame(dc,part,rect,clip,__builtin_return_address(0));
    if(SUCCEEDED(result))ntw16s_month_arrow(theme,dc,part,state,rect,clip,__builtin_return_address(0));
    if(SUCCEEDED(result))ntw16s_month_selection_background(theme,dc,part,state,rect,clip,__builtin_return_address(0));
    if(SUCCEEDED(result))ntw16s_group_frame(dc,part,state,rect,clip,NULL,0,0,__builtin_return_address(0),FALSE);
    SetLastError(error);return result;
}
static HDC WINAPI hook_CreateCompatibleDC(HDC source)
{
    HDC result=real_CreateCompatibleDC(source);DWORD error=GetLastError();
    if(!startup_window_created || InterlockedCompareExchange(&style_ready,0,0))ntw16s_modern_spin_dc_created(source,result);
    style_control_updown_dc_created(source,result,__builtin_return_address(0),style_runtime_text_native_target);SetLastError(error);return result;
}
static BOOL WINAPI hook_DeleteDC(HDC dc)
{
    BOOL result=real_DeleteDC(dc);DWORD error=GetLastError();
    if(result){
        if(!startup_window_created || InterlockedCompareExchange(&style_ready,0,0))ntw16s_modern_spin_dc_deleted(dc);
        style_control_updown_dc_deleted(dc);
    }
    SetLastError(error);return result;
}


static __typeof__(&TrackPopupMenu) real_TrackPopupMenu;
static PROCESS_HOOK_UNWIND_FUNCTION BOOL WINAPI hook_TrackPopupMenu(HMENU m,UINT flags,int x,int y,int reserved,HWND w,const RECT *r)
{
 process_hook_unwind_scope unwind;style_menu_owner_scope scope={w,FALSE,win16_style_frame_remove_owned};DWORD incoming=GetLastError(),error;BOOL result;
 if(style_menu_owner_eligible(w,menu_owner_property,(HANDLE)&menu_owner_marker))
     scope.installed=win16_style_menu_owner_install_created(w,&scope,NULL);
 PROCESS_HOOK_UNWIND_BEGIN(unwind,style_menu_owner_scope_end,&scope);
 SetLastError(incoming);result=real_TrackPopupMenu(m,flags,x,y,reserved,w,r);error=GetLastError();
 PROCESS_HOOK_UNWIND_END(unwind);SetLastError(error);return result;
}

static BOOL startup_stock_dialogs;
static LONG attach_stock_dialogs(DWORD (*attach)(void *,void *))
{
    LONG error=NO_ERROR;
#define ATTACH(NAME) do { if(!error){real_##NAME=NAME;error=(LONG)attach(&real_##NAME,(void *)hook_##NAME);} } while(0)
    ATTACH(FindTextA);ATTACH(FindTextW);ATTACH(ReplaceTextA);ATTACH(ReplaceTextW);
    ATTACH(ChooseColorA);ATTACH(ChooseColorW);ATTACH(ChooseFontA);ATTACH(ChooseFontW);
#undef ATTACH
    return error;
}

#ifdef NTWSHK_UNIFIED
LONG ntw16s_attach_startup_stock_using(DWORD (*attach)(void *,void *))
{
    LONG error=attach_stock_dialogs(attach);
    if(!error)startup_stock_dialogs=TRUE;
    return error;
}
#endif

static LONG attach_standard_dialogs(DWORD (*attach)(void *,void *))
{
    LONG error=NO_ERROR;
#define ATTACH(NAME) do { if(!error){real_##NAME=NAME;error=(LONG)attach(&real_##NAME,(void *)hook_##NAME);} } while(0)
    ATTACH(CreateDialogParamA);ATTACH(CreateDialogParamW);
    ATTACH(CreateDialogIndirectParamA);ATTACH(CreateDialogIndirectParamW);
    ATTACH(DialogBoxParamA);ATTACH(DialogBoxParamW);
    ATTACH(DialogBoxIndirectParamA);ATTACH(DialogBoxIndirectParamW);
    ATTACH(MessageBoxA);ATTACH(MessageBoxW);
    ATTACH(MessageBoxExA);ATTACH(MessageBoxExW);
    ATTACH(MessageBoxIndirectA);ATTACH(MessageBoxIndirectW);
#undef ATTACH
    return error;
}

#ifdef NTWSHK_UNIFIED
LONG ntw16s_attach_startup_dialogs_using(DWORD (*attach)(void *,void *))
{
    LONG error=attach_standard_dialogs(attach);
    if(!error)startup_observes_dialogs=TRUE;
    return error;
}
#endif

static LONG attach_print_dialogs(DWORD (*attach)(void *,void *))
{
    LONG error=NO_ERROR;
#define ATTACH(NAME) do { if(!error){real_##NAME=NAME;error=(LONG)attach(&real_##NAME,(void *)hook_##NAME);} } while(0)
    ATTACH(PrintDlgA);ATTACH(PrintDlgW);ATTACH(PageSetupDlgA);ATTACH(PageSetupDlgW);
    ATTACH(PrintDlgExA);ATTACH(PrintDlgExW);
#undef ATTACH
    return error;
}

#ifdef NTWSHK_UNIFIED
LONG ntw16s_attach_startup_print_using(DWORD (*attach)(void *,void *))
{
    LONG error=attach_print_dialogs(attach);
    if(!error)startup_observes_print=TRUE;
    return error;
}
#endif

static LONG attach_window_creation(DWORD (*attach)(void *,void *))
{
    create_A=CreateWindowExA;create_W=CreateWindowExW;
    LONG error=(LONG)attach(&create_A,(void *)window_A);
    return error?error:(LONG)attach(&create_W,(void *)window_W);
}

static LONG attach_window_theme(DWORD (*attach)(void *,void *))
{
    real_set_theme=SetWindowTheme;
    return (LONG)attach(&real_set_theme,(void *)hook_set_theme);
}

static LONG attach_updown_draw(DWORD (*attach)(void *,void *))
{
    real_DrawFrameControl=DrawFrameControl;real_DrawThemeBackground=DrawThemeBackground;
    real_CreateCompatibleDC=CreateCompatibleDC;real_DeleteDC=DeleteDC;
    LONG error=(LONG)attach(&real_DrawFrameControl,(void *)hook_DrawFrameControl);
    if(!error)error=(LONG)attach(&real_DrawThemeBackground,(void *)hook_DrawThemeBackground);
    if(!error)error=(LONG)attach(&real_CreateCompatibleDC,(void *)hook_CreateCompatibleDC);
    return error?error:(LONG)attach(&real_DeleteDC,(void *)hook_DeleteDC);
}

#ifdef NTWSHK_UNIFIED
LONG ntw16s_attach_startup_windows_using(DWORD (*attach)(void *,void *),void (*created)(HWND,DWORD))
{
    LONG error=attach_window_creation(attach);
    if(!error)error=attach_window_theme(attach);
    if(!error)error=attach_updown_draw(attach);
    if(!error)startup_window_created=created;
    return error;
}

#endif

LONG ntw16s_attach_titles_using(DWORD (*attach)(void *,void *))
{
    if(!attach)return ERROR_INVALID_PARAMETER;
    LONG error=NO_ERROR;
    if(!(ntw16s_process_context.flags&NTWSHK_POLICY_STYLE))return error;
    real_get_scroll=GetScrollInfo;real_set_scroll=SetScrollInfo;
    real_set_pos=SetScrollPos;real_set_range=SetScrollRange;
    real_show_scroll=ShowScrollBar;real_enable_scroll=EnableScrollBar;
    if(!startup_window_created)error=attach_window_creation(attach);
    FARPROC band_symbol=GetProcAddress(GetModuleHandleW(L"user32.dll"),"CreateWindowInBand");
    memcpy(&create_band,&band_symbol,sizeof(band_symbol));
    if(create_band && !error)error=(LONG)attach(&create_band,(void *)window_band);
    if(!error)error=(LONG)attach(&real_get_scroll,(void *)hook_get_scroll);
    if(!error)error=(LONG)attach(&real_set_scroll,(void *)hook_set_scroll);
    if(!error)error=(LONG)attach(&real_set_pos,(void *)hook_set_pos);
    if(!error)error=(LONG)attach(&real_set_range,(void *)hook_set_range);
    if(!error)error=(LONG)attach(&real_show_scroll,(void *)hook_show_scroll);
    if(!error)error=(LONG)attach(&real_enable_scroll,(void *)hook_enable_scroll);
    if(!error && !startup_window_created)error=attach_window_theme(attach);
    if(!error && !startup_window_created)error=attach_updown_draw(attach);
#define ATTACH(NAME) do { real_##NAME=NAME; if(!error)error=(LONG)attach(&real_##NAME,(void *)hook_##NAME); } while(0)
    ATTACH(OpenThemeData);ATTACH(OpenThemeDataEx);ATTACH(CloseThemeData);
    FARPROC dpi_symbol=GetProcAddress(GetModuleHandleW(L"uxtheme.dll"),"OpenThemeDataForDpi");
    memcpy(&real_OpenThemeDataForDpi,&dpi_symbol,sizeof(dpi_symbol));
    if(real_OpenThemeDataForDpi && !error)error=(LONG)attach(&real_OpenThemeDataForDpi,(void *)hook_OpenThemeDataForDpi);
    ATTACH(TrackPopupMenu);
    if(!error && !startup_stock_dialogs)error=attach_stock_dialogs(attach);
    if(!error && !startup_observes_print)error=attach_print_dialogs(attach);
    ATTACH(PropertySheetA);ATTACH(PropertySheetW);
    FARPROC task_symbol=GetProcAddress(GetModuleHandleW(L"comctl32.dll"),"TaskDialog");
    memcpy(&real_TaskDialog,&task_symbol,sizeof(task_symbol));
    if(real_TaskDialog && !error)error=(LONG)attach(&real_TaskDialog,(void *)hook_TaskDialog);
    task_symbol=GetProcAddress(GetModuleHandleW(L"comctl32.dll"),"TaskDialogIndirect");
    memcpy(&real_TaskDialogIndirect,&task_symbol,sizeof(task_symbol));
    if(real_TaskDialogIndirect && !error)error=(LONG)attach(&real_TaskDialogIndirect,(void *)hook_TaskDialogIndirect);
    if(!error && !startup_observes_dialogs)error=attach_standard_dialogs(attach);
    ATTACH(DrawTextA);ATTACH(DrawTextExA);ATTACH(TextOutA);ATTACH(ExtTextOutA);
    ATTACH(TextOutW);ATTACH(ExtTextOutW);
    ATTACH(DefWindowProcW);ATTACH(CallWindowProcA);ATTACH(CallWindowProcW);ATTACH(DrawTextW);ATTACH(DrawTextExW);
    ATTACH(Polyline);ATTACH(BitBlt);ATTACH(StretchBlt);ATTACH(DrawEdge);ATTACH(DrawThemeText);ATTACH(DrawThemeTextEx);ATTACH(DrawIconEx);ATTACH(SelectObject);ATTACH(DeleteObject);
#undef ATTACH
    return error;
}
