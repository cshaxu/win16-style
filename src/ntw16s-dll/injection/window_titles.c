#include "intercept.h"
#include "startup_hooks.h"
#include "unwind.h"
#include "theme_track.h"
#include "print_extended.h"
#include "ntw16s-dll/ntw16s.h"
#include "ntw16s-dll/attachment.h"
#include "ntw16s-dll/native_api.h"
#include "ntw16s-dll/vb_text.h"
#include "ntw16s-dll/updown_paint.h"
#include "ntw16s-dll/controls.h"
#include "ntw16s-dll/static_text.h"
#include "ntw16s-dll/status_theme.h"
#include "ntw16s-dll/win16_style.h"
#include <wchar.h>
#include <uxtheme.h>
#include <commdlg.h>
#include <commctrl.h>
#include <vssym32.h>
#include "ntw16s-dll/font.h"
#include "ntw16s-dll/month_text.h"
HFONT ntw16s_month_text_borrow(HTHEME,HDC,int,int,DWORD,DWORD,const RECT *,const void *);
BOOL ntw16s_month_text_room(HTHEME,HDC,int,int,DWORD,DWORD,const RECT *,int,const void *);
typedef struct month_font_borrow {HDC dc;int saved;HFONT font;} month_font_borrow;
static void month_font_return(void *p){month_font_borrow *b=p;if(b->saved)RestoreDC(b->dc,b->saved);win16_style_system_release(b->font);}

/* Private, synchronous coordinator scope; not part of the public ABI. */
void ntw16s_status_frame(HDC,const RECT *,const RECT *,const void *);
BOOL ntw16s_controlbar_edge_active(void);
BOOL ntw16s_controlbar_edge_allowed(UINT);
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
static INIT_ONCE title_once=INIT_ONCE_STATIC_INIT;
static LONG style_ready;
static __typeof__(&GetScrollInfo) real_get_scroll;
static __typeof__(&SetScrollInfo) real_set_scroll;
static __typeof__(&SetWindowTheme) real_set_theme;
static BOOL CALLBACK bind_titles(PINIT_ONCE once,void *parameter,void **context)
{
    (void)once;(void)parameter;(void)context;
    ntw16s_native_api api={sizeof(api),real_get_scroll,real_set_scroll,real_set_theme};
    if(!ntw16s_bind_native_api(&api))return FALSE;
    ntw16s_updown_api_enable();
    ntw16s_status_hooks_enable();
    InterlockedExchange(&style_ready,1);return TRUE;
}
static const WCHAR menu_owner_property[]=L"Win16Style.T18.MenuOwner";
static int menu_owner_marker;
static void mark_menu_owner(HWND w,const void *caller)
{ WCHAR cls[64],real[64];DWORD pid=0;HMODULE origin=NULL;
 if(w && GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() &&
    !(GetWindowLongPtrW(w,GWL_STYLE)&(WS_VISIBLE|WS_CHILD)) &&
    (GetWindowLongPtrW(w,GWL_STYLE)&WS_POPUP) && (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_TOOLWINDOW) &&
    GetClassNameW(w,cls,64) && !lstrcmpW(cls,L"TPUtilWindow") &&
    RealGetWindowClassW(w,real,64) && !lstrcmpW(real,cls) &&
    GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)caller,&origin) && origin==(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE))SetPropW(w,menu_owner_property,(HANDLE)&menu_owner_marker);
}
typedef struct title_scope { ntw16s_creation_scope *creation;DWORD failure; } title_scope;
static BOOL begin_titles(title_scope *scope,BOOL modal)
{
    ZeroMemory(scope,sizeof(*scope));
    if(!InitOnceExecuteOnce(&title_once,bind_titles,NULL,NULL))return FALSE;
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
    scope->creation=modal?ntw16s_creation_begin_dialog(TRUE):Ntw16sCreationBegin(TRUE);
#else
    (void)modal;scope->creation=Ntw16sCreationBegin(TRUE);
#endif
    return scope->creation!=NULL;
}
static void end_titles(void *data)
{
    title_scope *scope=data;
    ntw16s_creation_result result={0};
    if(!Ntw16sCreationEnd(scope->creation,&result))scope->failure=GetLastError();
    else scope->failure=result.failure;
}

/* Shell COM dialogs can create their window without passing the exported
 * CreateWindow entry points. Keep attachment active during their modal Show. */
HRESULT ntw16s_dialog_style_scope(HRESULT (*callback)(void *),void *context)
{
    ntw16s_unwind_scope unwind;title_scope scope;HRESULT result=E_FAIL;
    if(!callback)return E_INVALIDARG;
    if(!begin_titles(&scope,TRUE))return HRESULT_FROM_WIN32(GetLastError());
    NTW16S_UNWIND_BEGIN(unwind,end_titles,&scope); {result=callback(context);}
    NTW16S_UNWIND_END(unwind);
    return scope.failure?HRESULT_FROM_WIN32(scope.failure):result;
}

#define WINDOW_HOOK(S,T) \
static __typeof__(&CreateWindowEx##S) create_##S; \
static NTW16S_UNWIND_FUNCTION HWND WINAPI window_##S(DWORD ex,T cls,T text,DWORD style,int x,int y,int cx,int cy,HWND parent,HMENU menu,HINSTANCE instance,void *param) \
{ ntw16s_unwind_scope unwind;title_scope scope;HWND result=NULL;DWORD error=ERROR_SUCCESS; \
  if(!begin_titles(&scope,FALSE))return NULL; \
  NTW16S_UNWIND_BEGIN(unwind,end_titles,&scope); { result=create_##S(ex,cls,text,style,x,y,cx,cy,parent,menu,instance,param);error=GetLastError(); \
  } NTW16S_UNWIND_END(unwind); \
  if(scope.failure){if(result)DestroyWindow(result);result=NULL;error=scope.failure;} \
  if(result){mark_menu_owner(result,__builtin_return_address(0));} SetLastError(error);return result; }
WINDOW_HOOK(A,LPCSTR)
WINDOW_HOOK(W,LPCWSTR)

#define DIALOG_HOOK(NAME,TYPE,RET,MODAL,FAIL) \
static __typeof__(&NAME) real_##NAME; \
static NTW16S_UNWIND_FUNCTION RET WINAPI hook_##NAME(HINSTANCE instance,TYPE name,HWND parent,DLGPROC proc,LPARAM param) \
{ ntw16s_unwind_scope unwind;title_scope scope;RET result=FAIL;DWORD error=ERROR_SUCCESS; \
  if(!begin_titles(&scope,TRUE))return FAIL; \
  NTW16S_UNWIND_BEGIN(unwind,end_titles,&scope); { result=real_##NAME(instance,name,parent,proc,param);error=GetLastError(); \
  } NTW16S_UNWIND_END(unwind); \
  if(scope.failure){if(!(MODAL)){if(result)DestroyWindow((HWND)result);}result=FAIL;error=scope.failure;} \
  SetLastError(error);return result; }
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
#define FIND_HOOK(NAME, TYPE) \
static __typeof__(&NAME) real_##NAME; \
static NTW16S_UNWIND_FUNCTION HWND WINAPI hook_##NAME(TYPE *parameters) \
{ ntw16s_unwind_scope unwind;title_scope scope;HWND result=NULL;DWORD error=ERROR_SUCCESS; \
  if(!begin_titles(&scope,TRUE))return NULL; \
  NTW16S_UNWIND_BEGIN(unwind,end_titles,&scope); { result=real_##NAME(parameters);error=GetLastError(); \
  } NTW16S_UNWIND_END(unwind); \
  if(scope.failure){if(result)DestroyWindow(result);result=NULL;error=scope.failure;} \
  SetLastError(error);return result; }
FIND_HOOK(FindTextA,FINDREPLACEA)
FIND_HOOK(FindTextW,FINDREPLACEW)
FIND_HOOK(ReplaceTextA,FINDREPLACEA)
FIND_HOOK(ReplaceTextW,FINDREPLACEW)
#undef FIND_HOOK

/* Font selection uses owner-drawn system combos.  Observe only the current
 * thread and only the standard font dialog's three required combo IDs; keep
 * the application's hook and FIND/COMDLG argument addresses untouched. */
static LRESULT CALLBACK font_dialog_return(int code,WPARAM wparam,LPARAM lparam)
{
    if(code>=0 && lparam){
        const CWPRETSTRUCT *m=(const CWPRETSTRUCT *)lparam;
        if(m->message==WM_INITDIALOG){
            WCHAR cls[32];HWND first=GetDlgItem(m->hwnd,1136),second=GetDlgItem(m->hwnd,1137),third=GetDlgItem(m->hwnd,1138);
            if(first && second && third && GetClassNameW(m->hwnd,cls,ARRAYSIZE(cls)) && !lstrcmpW(cls,L"#32770"))
                ntw16s_font_dialog_chrome(m->hwnd);
        }
    }
    return CallNextHookEx(NULL,code,wparam,lparam);
}
typedef struct font_dialog_scope {title_scope title;HHOOK hook;} font_dialog_scope;
static void font_dialog_end(void *value)
{
    font_dialog_scope *scope=value;
    if(scope->hook)UnhookWindowsHookEx(scope->hook);
    end_titles(&scope->title);
}
#define FONT_DIALOG_HOOK(NAME, TYPE) \
static __typeof__(&NAME) real_##NAME; \
static NTW16S_UNWIND_FUNCTION BOOL WINAPI hook_##NAME(TYPE *parameters) \
{ ntw16s_unwind_scope unwind;font_dialog_scope scope={0};BOOL result=FALSE;DWORD error=ERROR_SUCCESS; \
  if(!begin_titles(&scope.title,TRUE))return FALSE; \
  scope.hook=SetWindowsHookExW(WH_CALLWNDPROCRET,font_dialog_return,NULL,GetCurrentThreadId()); \
  NTW16S_UNWIND_BEGIN(unwind,font_dialog_end,&scope); { result=real_##NAME(parameters);error=GetLastError(); \
  } NTW16S_UNWIND_END(unwind); \
  if(scope.title.failure){result=FALSE;error=scope.title.failure;} SetLastError(error);return result; }
FONT_DIALOG_HOOK(ChooseFontA,CHOOSEFONTA)
FONT_DIALOG_HOOK(ChooseFontW,CHOOSEFONTW)
#undef FONT_DIALOG_HOOK

/* The system's colour picker subclasses several STATIC surfaces and owns
 * their colour-grid painting.  Do not replace these clients with our generic
 * STATIC frame painter; the surrounding standard controls remain styled. */
static LRESULT CALLBACK color_dialog_return(int code,WPARAM wparam,LPARAM lparam)
{
    if(code>=0 && lparam){
        const CWPRETSTRUCT *m=(const CWPRETSTRUCT *)lparam;
        if(m->message==WM_INITDIALOG){
            WCHAR cls[32];HWND basic=GetDlgItem(m->hwnd,720),custom=GetDlgItem(m->hwnd,721);
            if(basic && custom && GetClassNameW(m->hwnd,cls,ARRAYSIZE(cls)) && !lstrcmpW(cls,L"#32770")){
                const int ids[]={720,721,710,702,709};
                for(unsigned i=0;i<sizeof(ids)/sizeof(ids[0]);++i){
                    HWND surface=GetDlgItem(m->hwnd,ids[i]);
                    if(surface && GetParent(surface)==m->hwnd && GetClassNameW(surface,cls,ARRAYSIZE(cls)) && !lstrcmpiW(cls,L"Static"))
                        (void)Ntw16sWindowRemove(surface);
                }
            }
        }
    }
    return CallNextHookEx(NULL,code,wparam,lparam);
}
#define COLOR_DIALOG_HOOK(NAME, TYPE) \
static __typeof__(&NAME) real_##NAME; \
static NTW16S_UNWIND_FUNCTION BOOL WINAPI hook_##NAME(TYPE *parameters) \
{ ntw16s_unwind_scope unwind;font_dialog_scope scope={0};BOOL result=FALSE;DWORD error=ERROR_SUCCESS; \
  if(!begin_titles(&scope.title,TRUE))return FALSE; \
  scope.hook=SetWindowsHookExW(WH_CALLWNDPROCRET,color_dialog_return,NULL,GetCurrentThreadId()); \
  NTW16S_UNWIND_BEGIN(unwind,font_dialog_end,&scope); { result=real_##NAME(parameters);error=GetLastError(); \
  } NTW16S_UNWIND_END(unwind); \
  if(scope.title.failure){result=FALSE;error=scope.title.failure;} SetLastError(error);return result; }
COLOR_DIALOG_HOOK(ChooseColorA,CHOOSECOLORA)
COLOR_DIALOG_HOOK(ChooseColorW,CHOOSECOLORW)
#undef COLOR_DIALOG_HOOK

/* A no-op print hook selects the system's traditional PrintDlg layout.
 * Preserve caller-owned customization and less common flag contracts unchanged. */
static UINT_PTR CALLBACK classic_print_hook(HWND window,UINT message,WPARAM wparam,LPARAM lparam)
{ (void)window;(void)message;(void)wparam;(void)lparam;return 0; }
#define CLASSIC_PRINT_FLAGS (PD_ALLPAGES|PD_SELECTION|PD_PAGENUMS|PD_NOSELECTION|PD_NOPAGENUMS|PD_COLLATE|PD_PRINTTOFILE|PD_RETURNDC|PD_RETURNIC|PD_NOWARNING|PD_USEDEVMODECOPIESANDCOLLATE|PD_DISABLEPRINTTOFILE|PD_HIDEPRINTTOFILE|PD_NONETWORKBUTTON)
#define PRINT_HOOK(NAME,TYPE) \
static __typeof__(&NAME) real_##NAME; \
static NTW16S_UNWIND_FUNCTION BOOL WINAPI hook_##NAME(TYPE *parameters) \
{ ntw16s_unwind_scope unwind;title_scope scope;BOOL result=FALSE;DWORD error=ERROR_SUCCESS; \
  if(!begin_titles(&scope,TRUE))return FALSE; \
  NTW16S_UNWIND_BEGIN(unwind,end_titles,&scope); { \
    if(parameters && parameters->lStructSize==sizeof(TYPE) && !(parameters->Flags & ~CLASSIC_PRINT_FLAGS)) { \
      TYPE copy=*parameters;copy.Flags|=PD_ENABLEPRINTHOOK;copy.lpfnPrintHook=classic_print_hook; \
      result=real_##NAME(&copy);error=GetLastError(); \
      copy.Flags=(copy.Flags & ~PD_ENABLEPRINTHOOK)|(parameters->Flags & PD_ENABLEPRINTHOOK); \
      copy.lpfnPrintHook=parameters->lpfnPrintHook;*parameters=copy; \
    } else { result=real_##NAME(parameters);error=GetLastError(); } \
  } NTW16S_UNWIND_END(unwind); \
  if(scope.failure){result=FALSE;error=scope.failure;}SetLastError(error);return result; }
PRINT_HOOK(PrintDlgA,PRINTDLGA)
PRINT_HOOK(PrintDlgW,PRINTDLGW)
#undef PRINT_HOOK
#undef CLASSIC_PRINT_FLAGS

/* Synchronous common dialogs also create their own internal windows. */
#define COMMON_HOOK(NAME, TYPE) \
static __typeof__(&NAME) real_##NAME; \
static NTW16S_UNWIND_FUNCTION BOOL WINAPI hook_##NAME(TYPE *parameters) \
{ ntw16s_unwind_scope unwind;title_scope scope;BOOL result=FALSE;DWORD error=ERROR_SUCCESS; \
  if(!begin_titles(&scope,TRUE))return FALSE; \
  NTW16S_UNWIND_BEGIN(unwind,end_titles,&scope); { result=real_##NAME(parameters);error=GetLastError(); \
  } NTW16S_UNWIND_END(unwind); \
  if(scope.failure){result=FALSE;error=scope.failure;} SetLastError(error);return result; }
COMMON_HOOK(PageSetupDlgA,PAGESETUPDLGA)
COMMON_HOOK(PageSetupDlgW,PAGESETUPDLGW)
#undef COMMON_HOOK
#define COMMON_EX_HOOK(NAME, TYPE) \
static __typeof__(&NAME) real_##NAME; \
static NTW16S_UNWIND_FUNCTION HRESULT WINAPI hook_##NAME(TYPE *parameters) \
{ ntw16s_unwind_scope unwind;title_scope scope;HRESULT result=E_FAIL;DWORD error=ERROR_SUCCESS; \
  if(!begin_titles(&scope,TRUE))return HRESULT_FROM_WIN32(GetLastError()); \
  NTW16S_UNWIND_BEGIN(unwind,end_titles,&scope); { result=real_##NAME(parameters);error=GetLastError(); \
  } NTW16S_UNWIND_END(unwind); \
  if(scope.failure){result=HRESULT_FROM_WIN32(scope.failure);error=scope.failure;} SetLastError(error);return result; }
COMMON_EX_HOOK(PrintDlgExA,PRINTDLGEXA)
#undef COMMON_EX_HOOK
static __typeof__(&PrintDlgExW) real_PrintDlgExW;
static NTW16S_UNWIND_FUNCTION HRESULT WINAPI hook_PrintDlgExW(PRINTDLGEXW *parameters)
{
    ntw16s_unwind_scope unwind;title_scope scope;HRESULT result=E_FAIL;DWORD error=ERROR_SUCCESS;
    if(!begin_titles(&scope,TRUE))return HRESULT_FROM_WIN32(GetLastError());
    NTW16S_UNWIND_BEGIN(unwind,end_titles,&scope); {
        if(!ntw16s_print_extended_w(parameters,real_PrintDlgW,&result))result=real_PrintDlgExW(parameters);
        error=GetLastError();
    } NTW16S_UNWIND_END(unwind);
    if(scope.failure){result=HRESULT_FROM_WIN32(scope.failure);error=scope.failure;}
    SetLastError(error);return result;
}

/* Property sheets may create internal pages without public DialogBox calls. */
#define SHEET_HOOK(NAME, TYPE) \
static __typeof__(&NAME) real_##NAME; \
static NTW16S_UNWIND_FUNCTION INT_PTR WINAPI hook_##NAME(const TYPE *parameters) \
{ ntw16s_unwind_scope unwind;title_scope scope;INT_PTR result=-1;DWORD error=ERROR_SUCCESS; \
  if(!begin_titles(&scope,TRUE))return -1; \
  NTW16S_UNWIND_BEGIN(unwind,end_titles,&scope); { result=real_##NAME(parameters);error=GetLastError(); \
  } NTW16S_UNWIND_END(unwind); \
  if(scope.failure){result=-1;error=scope.failure;} SetLastError(error);return result; }
SHEET_HOOK(PropertySheetA,PROPSHEETHEADERA)
SHEET_HOOK(PropertySheetW,PROPSHEETHEADERW)
#undef SHEET_HOOK

/* TaskDialog lives in ComCtl32 v6; bind only when that module exports it. */
static __typeof__(&TaskDialog) real_TaskDialog;
static __typeof__(&TaskDialogIndirect) real_TaskDialogIndirect;
static NTW16S_UNWIND_FUNCTION HRESULT WINAPI hook_TaskDialog(HWND owner,HINSTANCE instance,
    PCWSTR title,PCWSTR main_instruction,PCWSTR content,TASKDIALOG_COMMON_BUTTON_FLAGS buttons,
    PCWSTR icon,int *button)
{
    ntw16s_unwind_scope unwind;title_scope scope;HRESULT result=E_FAIL;DWORD error=ERROR_SUCCESS;
    if(!begin_titles(&scope,TRUE))return HRESULT_FROM_WIN32(GetLastError());
    NTW16S_UNWIND_BEGIN(unwind,end_titles,&scope); {
        result=real_TaskDialog(owner,instance,title,main_instruction,content,buttons,icon,button);error=GetLastError();
    } NTW16S_UNWIND_END(unwind);
    if(scope.failure){result=HRESULT_FROM_WIN32(scope.failure);error=scope.failure;}SetLastError(error);return result;
}
static NTW16S_UNWIND_FUNCTION HRESULT WINAPI hook_TaskDialogIndirect(const TASKDIALOGCONFIG *config,
    int *button,int *radio,BOOL *verification)
{
    ntw16s_unwind_scope unwind;title_scope scope;HRESULT result=E_FAIL;DWORD error=ERROR_SUCCESS;
    if(!begin_titles(&scope,TRUE))return HRESULT_FROM_WIN32(GetLastError());
    NTW16S_UNWIND_BEGIN(unwind,end_titles,&scope); {
        result=real_TaskDialogIndirect(config,button,radio,verification);error=GetLastError();
    } NTW16S_UNWIND_END(unwind);
    if(scope.failure){result=HRESULT_FROM_WIN32(scope.failure);error=scope.failure;}SetLastError(error);return result;
}

/* MessageBox creates its own dialog inside USER rather than necessarily
 * calling an exported DialogBox entry. Keep its API/return contract and use
 * the same bounded creation scope as the other modal entry points. */
#define MESSAGE_HOOK(NAME,PARAMETERS,ARGUMENTS) \
static __typeof__(&NAME) real_##NAME; \
static NTW16S_UNWIND_FUNCTION int WINAPI hook_##NAME PARAMETERS \
{ ntw16s_unwind_scope unwind;title_scope scope;int result=0;DWORD error=ERROR_SUCCESS; \
  if(!begin_titles(&scope,TRUE))return 0; \
  NTW16S_UNWIND_BEGIN(unwind,end_titles,&scope); { result=real_##NAME ARGUMENTS;error=GetLastError(); \
  } NTW16S_UNWIND_END(unwind); \
  if(scope.failure){result=0;error=scope.failure;} SetLastError(error);return result; }
MESSAGE_HOOK(MessageBoxA,(HWND owner,LPCSTR text,LPCSTR caption,UINT type),(owner,text,caption,type))
MESSAGE_HOOK(MessageBoxW,(HWND owner,LPCWSTR text,LPCWSTR caption,UINT type),(owner,text,caption,type))
MESSAGE_HOOK(MessageBoxExA,(HWND owner,LPCSTR text,LPCSTR caption,UINT type,WORD language),(owner,text,caption,type,language))
MESSAGE_HOOK(MessageBoxExW,(HWND owner,LPCWSTR text,LPCWSTR caption,UINT type,WORD language),(owner,text,caption,type,language))
MESSAGE_HOOK(MessageBoxIndirectA,(const MSGBOXPARAMSA *parameters),(parameters))
MESSAGE_HOOK(MessageBoxIndirectW,(const MSGBOXPARAMSW *parameters),(parameters))
#undef MESSAGE_HOOK

/* The DLL uses the three original native leaves bound at load, so application
 * reentry is never accidentally bypassed by an ambient recursion guard. */
static __typeof__(&SetScrollPos) real_set_pos;
static __typeof__(&SetScrollRange) real_set_range;
static __typeof__(&ShowScrollBar) real_show_scroll;
static __typeof__(&EnableScrollBar) real_enable_scroll;
static HRESULT WINAPI hook_set_theme(HWND w,LPCWSTR application,LPCWSTR ids)
{
    if(!InterlockedCompareExchange(&style_ready,0,0))return real_set_theme(w,application,ids);
    return Ntw16sSetWindowTheme(w,application,ids);
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
static _Thread_local unsigned vb_text_busy;
static BOOL vb_text_caller(const void *address)
{
    HMODULE caller=NULL;
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (const WCHAR *)address,&caller))return FALSE;
    return ntw16s_report_paint_caller(caller) || (caller==GetModuleHandleW(L"MSVBVM60.DLL") || caller==GetModuleHandleW(L"MSVBVM50.DLL")) || caller==GetModuleHandleW(L"USER32.DLL") ||
        caller==GetModuleHandleW(L"LPK.DLL") || caller==GetModuleHandleW(L"USP10.DLL") ||
        caller==GetModuleHandleW(L"GDI32FULL.DLL");
}
static BOOL vb_label_caller(const void *address)
{
    if(ntw16s_vb_direct_text_allowed())return TRUE;
    HMODULE caller=NULL;
    return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        (const WCHAR *)address,&caller) && (caller==GetModuleHandleW(L"MSVBVM50.DLL") || caller==GetModuleHandleW(L"MSVBVM60.DLL"));
}
/* The report adapter promises only whole, reversible CP1252 runs. Keep
 * measurements and unsupported language/RTL runs on the native font. */
static BOOL report_wide_text(HDC dc,const WCHAR *text,int count)
{
    if(!ntw16s_report_paint_active())return TRUE;
    if(count<0 || count>4096 || (GetTextAlign(dc)&TA_RTLREADING))return FALSE;
    if(!count)return TRUE;
    BOOL used=FALSE;char buffer[4096];
    return text && WideCharToMultiByte(1252,WC_NO_BEST_FIT_CHARS,text,count,buffer,sizeof(buffer),NULL,&used)==count && !used;
}
static BOOL report_ansi_text(HDC dc,const char *text,int count)
{
    if(!ntw16s_report_paint_active())return TRUE;
    if(!text || (GetTextAlign(dc)&TA_RTLREADING))return FALSE;
    if(count==-1){count=0;while(count<=4096 && text[count])++count;}
    if(count<0 || count>4096)return FALSE;
    if(GetACP()!=1252)for(int i=0;i<count;++i)if((unsigned char)text[i]>=128)return FALSE;
    return TRUE;
}
#define VB_TEXT_HOOK(NAME,TYPE,PARAMS,ARGS,ALLOW) \
static __typeof__(&NAME) real_##NAME; \
static TYPE WINAPI hook_##NAME PARAMS \
{ HFONT font=NULL,old=NULL;DWORD entry=GetLastError(),error;TYPE result; \
  if(!vb_text_busy++ && (ALLOW) && ntw16s_vb_paint_active() && vb_text_caller(__builtin_return_address(0))){font=ntw16s_vb_paint_font(dc);if(font){old=SelectObject(dc,font);if(old==HGDI_ERROR)old=NULL;}} \
  SetLastError(entry);result=real_##NAME ARGS;error=GetLastError(); \
  if(old){SelectObject(dc,old);}--vb_text_busy;SetLastError(error);return result; }
VB_TEXT_HOOK(DrawTextA,int,(HDC dc,LPCSTR text,int count,LPRECT rect,UINT format),(dc,text,count,rect,format),((!ntw16s_report_paint_active() && ntw16s_vb_direct_text_allowed()) || !(format&DT_CALCRECT)) && vb_label_caller(__builtin_return_address(0)) && report_ansi_text(dc,text,count))
VB_TEXT_HOOK(DrawTextExA,int,(HDC dc,LPSTR text,int count,LPRECT rect,UINT format,LPDRAWTEXTPARAMS params),(dc,text,count,rect,format,params),((!ntw16s_report_paint_active() && ntw16s_vb_direct_text_allowed()) || !(format&DT_CALCRECT)) && vb_label_caller(__builtin_return_address(0)) && report_ansi_text(dc,text,count))
VB_TEXT_HOOK(TextOutA,BOOL,(HDC dc,int x,int y,LPCSTR text,int count),(dc,x,y,text,count),(ntw16s_vb_direct_text_allowed() || (GetMapMode(dc)==MM_TEXT && !(GetTextAlign(dc)&TA_UPDATECP))) && vb_label_caller(__builtin_return_address(0)) && report_ansi_text(dc,text,count))
VB_TEXT_HOOK(ExtTextOutA,BOOL,(HDC dc,int x,int y,UINT options,const RECT *rect,LPCSTR text,UINT count,const INT *dx),(dc,x,y,options,rect,text,count,dx),ntw16s_vb_direct_text_allowed() && (!ntw16s_report_paint_active() || !(options&(ETO_GLYPH_INDEX|ETO_RTLREADING))) && report_ansi_text(dc,text,(int)count))
VB_TEXT_HOOK(TextOutW,BOOL,(HDC dc,int x,int y,LPCWSTR text,int count),(dc,x,y,text,count),ntw16s_vb_direct_text_allowed() && report_wide_text(dc,text,count))

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
static NTW16S_UNWIND_FUNCTION LRESULT WINAPI hook_CallWindowProcW(WNDPROC proc,HWND w,UINT message,WPARAM wp,LPARAM lp)
{
    ntw16s_static_call call;ntw16s_unwind_scope unwind;LRESULT result;DWORD entry=GetLastError(),error;
    ntw16s_rich_witness(proc,w,message,__builtin_return_address(0),TRUE);
    if(!ntw16s_static_call_begin(&call,proc,w,message,__builtin_return_address(0)) && !ntw16s_static_call_suspend(&call)){
        SetLastError(entry);return real_CallWindowProcW(proc,w,message,wp,lp);
    }
    NTW16S_UNWIND_BEGIN(unwind,ntw16s_static_call_end,&call);
    SetLastError(entry);result=real_CallWindowProcW(proc,w,message,wp,lp);error=GetLastError();
    NTW16S_UNWIND_END(unwind);SetLastError(error);return result;
}
HFONT ntw16s_status_simple_font(HDC,LPCWSTR,int,UINT,const void *,void **);
void ntw16s_status_simple_return(void *,HFONT);
static __typeof__(&DrawTextW) real_DrawTextW;
typedef struct static_font_borrow {HDC dc;HFONT font;HGDIOBJ previous;} static_font_borrow;
static void static_font_return(void *data)
{
    static_font_borrow *borrow=data;
    if(borrow->previous)SelectObject(borrow->dc,borrow->previous);
    if(borrow->font)win16_style_system_release(borrow->font);
}
static NTW16S_UNWIND_FUNCTION int WINAPI hook_DrawTextW(HDC dc,LPCWSTR text,int count,LPRECT rect,UINT flags)
{
    DWORD entry=GetLastError(),error;HFONT font=ntw16s_static_text_font(dc,text,count,flags,__builtin_return_address(0));
    HGDIOBJ old=font?SelectObject(dc,font):NULL;
    if(old==HGDI_ERROR)old=NULL;
    if(!font){SetLastError(entry);return real_DrawTextW(dc,text,count,rect,flags);}
    static_font_borrow borrow={dc,font,old};ntw16s_unwind_scope unwind;int result;
    NTW16S_UNWIND_BEGIN(unwind,static_font_return,&borrow);
    SetLastError(entry);result=real_DrawTextW(dc,text,count,rect,flags);error=GetLastError();
    NTW16S_UNWIND_END(unwind);SetLastError(error);return result;
}


typedef struct status_font_borrow {HDC dc;HFONT font;HGDIOBJ previous;void *token;} status_font_borrow;
static void status_font_return(void *data){status_font_borrow *b=data;if(b->previous)SelectObject(b->dc,b->previous);ntw16s_status_simple_return(b->token,b->font);}
static __typeof__(&DrawTextExW) real_DrawTextExW;
static NTW16S_UNWIND_FUNCTION int WINAPI hook_DrawTextExW(HDC dc,LPWSTR text,int count,LPRECT rect,UINT flags,LPDRAWTEXTPARAMS params)
{
    DWORD entry=GetLastError(),error;void *token=NULL;HFONT font=ntw16s_status_simple_font(dc,text,count,flags,__builtin_return_address(0),&token);
    HGDIOBJ old=font?SelectObject(dc,font):NULL;if(old==HGDI_ERROR)old=NULL;
    if(!font){SetLastError(entry);return real_DrawTextExW(dc,text,count,rect,flags,params);}
    status_font_borrow borrow={dc,font,old,token};ntw16s_unwind_scope unwind;int result;
    NTW16S_UNWIND_BEGIN(unwind,status_font_return,&borrow);
    SetLastError(entry);result=real_DrawTextExW(dc,text,count,rect,flags,params);error=GetLastError();
    NTW16S_UNWIND_END(unwind);SetLastError(error);return result;
}


/* USER's EDIT can submit shaped glyph IDs. Those IDs belong to its source
 * font, not the replacement FNT. Reverse only verified CP1252 glyphs; any
 * missing/complex glyph leaves the whole native run unchanged. Keep its dx,
 * ETO_PDY, clipping and visual order, and do not reshape the recovered run. */
static __typeof__(&ExtTextOutW) real_ExtTextOutW;
static BOOL WINAPI hook_ExtTextOutW(HDC dc,int x,int y,UINT options,const RECT *rect,LPCWSTR text,UINT count,const INT *dx)
{
    DWORD entry=GetLastError(),error;HFONT font=NULL,old=NULL;WCHAR *decoded=NULL;char *encoded=NULL;BOOL result;LPCWSTR original_text=text;UINT original_options=options;
    if(!vb_text_busy++ && ntw16s_vb_direct_text_allowed() && ntw16s_vb_paint_active() && vb_text_caller(__builtin_return_address(0))){
        font=ntw16s_vb_paint_font(dc);
        if(font && (options&ETO_GLYPH_INDEX)){
            BOOL valid=count<=4096 && !(options&ETO_RTLREADING) && !(GetTextAlign(dc)&TA_RTLREADING);
            if(valid && count)decoded=HeapAlloc(GetProcessHeap(),0,count*sizeof(WCHAR));
            if(count && !decoded)valid=FALSE;
            if(valid)valid=ntw16s_vb_decode_glyphs(dc,text,count,decoded);
            if(valid && count){
                BOOL used_default=FALSE;encoded=HeapAlloc(GetProcessHeap(),0,count);
                if(!encoded || WideCharToMultiByte(1252,WC_NO_BEST_FIT_CHARS,decoded,(int)count,encoded,(int)count,NULL,&used_default)!=(int)count || used_default)valid=FALSE;
            }
            if(valid){text=decoded;options&=~ETO_GLYPH_INDEX;}
            else font=NULL;
        }
        if(font && !(options&ETO_GLYPH_INDEX) &&
            (!report_wide_text(dc,text,(int)count) || (ntw16s_report_paint_active() && (options&ETO_RTLREADING))))font=NULL;
        if(font)old=SelectObject(dc,font);
        if(!old || old==HGDI_ERROR){
            old=NULL;text=original_text;options=original_options;
            if(encoded){HeapFree(GetProcessHeap(),0,encoded);encoded=NULL;}
        }
    }
    SetLastError(entry);
    if(old && encoded)result=real_ExtTextOutA(dc,x,y,options,rect,encoded,count,dx);
    else result=real_ExtTextOutW(dc,x,y,options,rect,text,count,dx);
    error=GetLastError();
    if(old)SelectObject(dc,old);
    if(decoded)HeapFree(GetProcessHeap(),0,decoded);
    if(encoded)HeapFree(GetProcessHeap(),0,encoded);
    --vb_text_busy;SetLastError(error);return result;
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
static NTW16S_UNWIND_FUNCTION HRESULT WINAPI hook_DrawThemeText(HTHEME theme,HDC dc,int part,int state,LPCWSTR text,int count,DWORD flags,DWORD flags2,const RECT *rect)
{
    DWORD saved=GetLastError(),error;HRESULT result;ntw16s_status_theme_scope scope;ntw16s_unwind_scope unwind;
    ntw16s_month_header_text(theme,dc,part,state,rect,flags,__builtin_return_address(0));
    ntw16s_month_selection_text(theme,dc,part,state,rect,flags,__builtin_return_address(0));
    ntw16s_status_text_frame(dc,rect,flags,__builtin_return_address(0));
    BOOL entered=ntw16s_status_theme_begin(&scope,dc,part,state,text,count,flags,rect,__builtin_return_address(0));
    if(!entered && text){
        int n=0;BOOL supported=ntw16s_month_text_length(text,count,&n);
        HFONT font=supported?ntw16s_month_text_borrow(theme,dc,part,state,flags,flags2,rect,__builtin_return_address(0)):NULL;
        if(font){month_font_borrow borrow={dc,SaveDC(dc),font};BOOL done=FALSE;ntw16s_unwind_scope cleanup;NTW16S_UNWIND_BEGIN(cleanup,month_font_return,&borrow);
          if(borrow.saved){RECT bounds=*rect,measure=*rect;COLORREF color;HGDIOBJ previous=SelectObject(dc,font);if(SUCCEEDED(GetThemeColor(theme,part,state,TMT_TEXTCOLOR,&color)) && previous && previous!=HGDI_ERROR){SetTextColor(dc,color);SetBkMode(dc,TRANSPARENT);int h=real_DrawTextW(dc,text,n,&measure,flags|DT_CALCRECT);if(h>0 && measure.right-measure.left<=rect->right-rect->left && measure.bottom-measure.top<=rect->bottom-rect->top && ntw16s_month_text_room(theme,dc,part,state,flags,flags2,rect,measure.right-measure.left,__builtin_return_address(0)) && IntersectClipRect(dc,rect->left,rect->top,rect->right,rect->bottom)!=ERROR)done=real_DrawTextW(dc,text,n,&bounds,flags)!=0;}}
          NTW16S_UNWIND_END(cleanup);if(done){SetLastError(saved);return S_OK;}}}
    if(!entered){SetLastError(saved);return real_DrawThemeText(theme,dc,part,state,text,count,flags,flags2,rect);}
    NTW16S_UNWIND_BEGIN(unwind,ntw16s_status_theme_end,&scope);
    SetLastError(saved);result=real_DrawThemeText(theme,dc,part,state,text,count,flags,flags2,rect);error=GetLastError();
    NTW16S_UNWIND_END(unwind);SetLastError(error);return result;
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
typedef struct theme_unwind { BOOL active, closing; HTHEME handle; } theme_unwind;
static void theme_end(void *data)
{
    theme_unwind *scope=data;
    if (!scope->active) return;
    if (scope->closing) ntw16s_theme_close_end(scope->handle,FALSE);
    else ntw16s_theme_open_cancel();
    scope->active=FALSE;
}
static int theme_kind(LPCWSTR name)
{
    WCHAR safe[9];SIZE_T got=0;
    if(!name || !ReadProcessMemory(GetCurrentProcess(),name,safe,7*sizeof(WCHAR),&got) || got!=7*sizeof(WCHAR))
        return NTW16S_THEME_OTHER;
    if(safe[6]==0 && !lstrcmpiW(safe,L"BUTTON"))return NTW16S_THEME_BUTTON;
    if(!ReadProcessMemory(GetCurrentProcess(),name,safe,sizeof(safe),&got) || got!=sizeof(safe))return NTW16S_THEME_OTHER;
    if(safe[8]==0 && !lstrcmpiW(safe,L"COMBOBOX"))return NTW16S_THEME_COMBOBOX;
    return safe[8]==0 && !lstrcmpiW(safe,L"TEXTGLOW")?NTW16S_THEME_TEXTGLOW:NTW16S_THEME_OTHER;
}
static NTW16S_UNWIND_FUNCTION HTHEME WINAPI hook_OpenThemeData(HWND w,LPCWSTR name)
{
    DWORD entry=GetLastError(),error;HTHEME h;ntw16s_unwind_scope unwind;
    theme_unwind scope={TRUE,FALSE,NULL};ntw16s_theme_ticket ticket=ntw16s_theme_open_begin();
    NTW16S_UNWIND_BEGIN(unwind,theme_end,&scope);
    SetLastError(entry);h=real_OpenThemeData(w,name);error=GetLastError();
    ntw16s_theme_open_end(ticket,h,h?theme_kind(name):NTW16S_THEME_OTHER);scope.active=FALSE;
    NTW16S_UNWIND_END(unwind);SetLastError(error);return h;
}
static NTW16S_UNWIND_FUNCTION HTHEME WINAPI hook_OpenThemeDataEx(HWND w,LPCWSTR name,DWORD flags)
{
    DWORD entry=GetLastError(),error;HTHEME h;ntw16s_unwind_scope unwind;
    theme_unwind scope={TRUE,FALSE,NULL};ntw16s_theme_ticket ticket=ntw16s_theme_open_begin();
    NTW16S_UNWIND_BEGIN(unwind,theme_end,&scope);
    SetLastError(entry);h=real_OpenThemeDataEx(w,name,flags);error=GetLastError();
    ntw16s_theme_open_end(ticket,h,h?theme_kind(name):NTW16S_THEME_OTHER);scope.active=FALSE;
    NTW16S_UNWIND_END(unwind);SetLastError(error);return h;
}
static NTW16S_UNWIND_FUNCTION HTHEME WINAPI hook_OpenThemeDataForDpi(HWND w,LPCWSTR name,UINT dpi)
{
    DWORD entry=GetLastError(),error;HTHEME h;ntw16s_unwind_scope unwind;
    theme_unwind scope={TRUE,FALSE,NULL};ntw16s_theme_ticket ticket=ntw16s_theme_open_begin();
    NTW16S_UNWIND_BEGIN(unwind,theme_end,&scope);
    SetLastError(entry);h=real_OpenThemeDataForDpi(w,name,dpi);error=GetLastError();
    ntw16s_theme_open_end(ticket,h,h?theme_kind(name):NTW16S_THEME_OTHER);scope.active=FALSE;
    NTW16S_UNWIND_END(unwind);SetLastError(error);return h;
}
static NTW16S_UNWIND_FUNCTION HRESULT WINAPI hook_CloseThemeData(HTHEME h)
{
    DWORD entry=GetLastError(),error;HRESULT result;ntw16s_unwind_scope unwind;
    theme_unwind scope={TRUE,TRUE,h};ntw16s_theme_close_begin(h);
    NTW16S_UNWIND_BEGIN(unwind,theme_end,&scope);
    SetLastError(entry);result=real_CloseThemeData(h);error=GetLastError();
    ntw16s_theme_close_end(h,SUCCEEDED(result));scope.active=FALSE;
    NTW16S_UNWIND_END(unwind);SetLastError(error);return result;
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
    if(result)win16_style_vb_checklist(dc,x,y,width,height,source,sx,sy,rop,__builtin_return_address(0));
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
    DWORD entry=GetLastError();BOOL drawn=ntw16s_updown_draw_frame(dc,rect,type,state,__builtin_return_address(0));
    if(drawn){SetLastError(entry);return TRUE;}
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
    RECT original={0};BOOL eligible=rect && edge==BDR_SUNKENOUTER && flags==(BF_RECT|BF_ADJUST);
    BOOL controlbar=rect && flags==(BF_RECT|BF_ADJUST) && ntw16s_controlbar_edge_allowed(edge) && ntw16s_controlbar_edge_active();
    if(eligible || controlbar)original=*rect;
    BOOL result=real_DrawEdge(dc,rect,edge,flags);DWORD error=GetLastError();
    if(result && eligible)ntw16s_status_frame(dc,&original,NULL,__builtin_return_address(0));
    if(result && controlbar)ntw16s_controlbar_edge(dc,&original,edge,flags,__builtin_return_address(0));
    SetLastError(error);return result;
}
static HRESULT WINAPI hook_DrawThemeBackground(HTHEME theme,HDC dc,int part,int state,const RECT *rect,const RECT *clip)
{
    DWORD entry=GetLastError();ntw16s_theme_ticket ticket=ntw16s_theme_button_begin(theme);
    ntw16s_theme_ticket combo={0};if(ntw16s_inplace_paint(dc))combo=ntw16s_theme_combo_begin(theme);
    ntw16s_theme_ticket glow=ntw16s_theme_glow_begin(theme);
    BOOL textglow=part==1 && state==0 && glow.eligible;
    if(!textglow)ntw16s_modern_spin_theme_begin(dc,rect);
    BOOL drawn=ntw16s_updown_draw_theme(theme,dc,part,state,rect,clip,__builtin_return_address(0));
    SetLastError(entry);if(drawn)return S_OK;
    HRESULT result=real_DrawThemeBackground(theme,dc,part,state,rect,clip);DWORD error=GetLastError();
    if(textglow && !ntw16s_theme_glow_end(theme,glow))ntw16s_modern_spin_theme_begin(dc,NULL);
    if(SUCCEEDED(result) && ntw16s_theme_button_end(theme,ticket)){
        ntw16s_nav_theme(dc,part,state,rect,clip,__builtin_return_address(0));
        ntw16s_inplace_button(dc,part,state,rect,clip,__builtin_return_address(0));
        ntw16s_modern_spin_theme(dc,part,state,rect,clip,__builtin_return_address(0));}
    if(SUCCEEDED(result) && combo.eligible && ntw16s_theme_combo_end(theme,combo))ntw16s_inplace_dropdown(dc,part,state,rect,clip,__builtin_return_address(0));
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
    ntw16s_modern_spin_dc_created(source,result);
    ntw16s_updown_dc_created(source,result,__builtin_return_address(0));SetLastError(error);return result;
}
static BOOL WINAPI hook_DeleteDC(HDC dc)
{
    BOOL result=real_DeleteDC(dc);DWORD error=GetLastError();if(result){ntw16s_modern_spin_dc_deleted(dc);ntw16s_updown_dc_deleted(dc);}
    SetLastError(error);return result;
}


static __typeof__(&TrackPopupMenu) real_TrackPopupMenu;
typedef struct popup_owner_scope { HWND window;BOOL installed; } popup_owner_scope;
static void popup_owner_end(void *data)
{
 popup_owner_scope *scope=data;BOOL installed=scope->installed;HWND w=scope->window;
 scope->installed=FALSE;if(installed && IsWindow(w))win16_style_frame_remove_owned(w,scope);
}
static NTW16S_UNWIND_FUNCTION BOOL WINAPI hook_TrackPopupMenu(HMENU m,UINT flags,int x,int y,int reserved,HWND w,const RECT *r)
{
 ntw16s_unwind_scope unwind;popup_owner_scope scope={w,FALSE};DWORD incoming=GetLastError(),pid=0,error;BOOL result;
 if(GetWindowThreadProcessId(w,&pid)==GetCurrentThreadId() && pid==GetCurrentProcessId() &&
    GetPropW(w,menu_owner_property)==(HANDLE)&menu_owner_marker &&
    !(GetWindowLongPtrW(w,GWL_STYLE)&(WS_VISIBLE|WS_CHILD)) &&
    (GetWindowLongPtrW(w,GWL_EXSTYLE)&WS_EX_TOOLWINDOW))
     scope.installed=win16_style_menu_owner_install_created(w,&scope,NULL);
 NTW16S_UNWIND_BEGIN(unwind,popup_owner_end,&scope);
 SetLastError(incoming);result=real_TrackPopupMenu(m,flags,x,y,reserved,w,r);error=GetLastError();
 NTW16S_UNWIND_END(unwind);SetLastError(error);return result;
}

LONG ntw16s_attach_titles(void)
{
    LONG error=NO_ERROR;
    if(!(ntw16s_process_context.flags&NTW16S_POLICY_STYLE))return error;
    create_A=CreateWindowExA;create_W=CreateWindowExW;
    real_get_scroll=GetScrollInfo;real_set_scroll=SetScrollInfo;real_set_theme=SetWindowTheme;
    real_set_pos=SetScrollPos;real_set_range=SetScrollRange;
    real_show_scroll=ShowScrollBar;real_enable_scroll=EnableScrollBar;
    error=(LONG)ntw16s_hooks_attach(&create_A,(void *)window_A);
    if(!error)error=(LONG)ntw16s_hooks_attach(&create_W,(void *)window_W);
    if(!error)error=(LONG)ntw16s_hooks_attach(&real_get_scroll,(void *)hook_get_scroll);
    if(!error)error=(LONG)ntw16s_hooks_attach(&real_set_scroll,(void *)hook_set_scroll);
    if(!error)error=(LONG)ntw16s_hooks_attach(&real_set_pos,(void *)hook_set_pos);
    if(!error)error=(LONG)ntw16s_hooks_attach(&real_set_range,(void *)hook_set_range);
    if(!error)error=(LONG)ntw16s_hooks_attach(&real_show_scroll,(void *)hook_show_scroll);
    if(!error)error=(LONG)ntw16s_hooks_attach(&real_enable_scroll,(void *)hook_enable_scroll);
    if(!error)error=(LONG)ntw16s_hooks_attach(&real_set_theme,(void *)hook_set_theme);
#define ATTACH(NAME) do { real_##NAME=NAME; if(!error)error=(LONG)ntw16s_hooks_attach(&real_##NAME,(void *)hook_##NAME); } while(0)
    ATTACH(OpenThemeData);ATTACH(OpenThemeDataEx);ATTACH(CloseThemeData);
    FARPROC dpi_symbol=GetProcAddress(GetModuleHandleW(L"uxtheme.dll"),"OpenThemeDataForDpi");
    memcpy(&real_OpenThemeDataForDpi,&dpi_symbol,sizeof(dpi_symbol));
    if(real_OpenThemeDataForDpi && !error)error=(LONG)ntw16s_hooks_attach(&real_OpenThemeDataForDpi,(void *)hook_OpenThemeDataForDpi);
    ATTACH(TrackPopupMenu);
    ATTACH(FindTextA);ATTACH(FindTextW);ATTACH(ReplaceTextA);ATTACH(ReplaceTextW);
    ATTACH(ChooseColorA);ATTACH(ChooseColorW);ATTACH(ChooseFontA);ATTACH(ChooseFontW);
    ATTACH(PrintDlgA);ATTACH(PrintDlgW);ATTACH(PageSetupDlgA);ATTACH(PageSetupDlgW);
    ATTACH(PrintDlgExA);ATTACH(PrintDlgExW);
    ATTACH(PropertySheetA);ATTACH(PropertySheetW);
    FARPROC task_symbol=GetProcAddress(GetModuleHandleW(L"comctl32.dll"),"TaskDialog");
    memcpy(&real_TaskDialog,&task_symbol,sizeof(task_symbol));
    if(real_TaskDialog && !error)error=(LONG)ntw16s_hooks_attach(&real_TaskDialog,(void *)hook_TaskDialog);
    task_symbol=GetProcAddress(GetModuleHandleW(L"comctl32.dll"),"TaskDialogIndirect");
    memcpy(&real_TaskDialogIndirect,&task_symbol,sizeof(task_symbol));
    if(real_TaskDialogIndirect && !error)error=(LONG)ntw16s_hooks_attach(&real_TaskDialogIndirect,(void *)hook_TaskDialogIndirect);
    ATTACH(CreateDialogParamA);ATTACH(CreateDialogParamW);
    ATTACH(CreateDialogIndirectParamA);ATTACH(CreateDialogIndirectParamW);
    ATTACH(DialogBoxParamA);ATTACH(DialogBoxParamW);
    ATTACH(DialogBoxIndirectParamA);ATTACH(DialogBoxIndirectParamW);
    ATTACH(MessageBoxA);ATTACH(MessageBoxW);
    ATTACH(MessageBoxExA);ATTACH(MessageBoxExW);
    ATTACH(MessageBoxIndirectA);ATTACH(MessageBoxIndirectW);
    ATTACH(DrawTextA);ATTACH(DrawTextExA);ATTACH(TextOutA);ATTACH(ExtTextOutA);
    ATTACH(TextOutW);ATTACH(ExtTextOutW);
    ATTACH(DefWindowProcW);ATTACH(CallWindowProcA);ATTACH(CallWindowProcW);ATTACH(DrawTextW);ATTACH(DrawTextExW);
    ATTACH(Polyline);ATTACH(BitBlt);ATTACH(StretchBlt);ATTACH(DrawFrameControl);ATTACH(DrawEdge);ATTACH(DrawThemeBackground);ATTACH(DrawThemeText);ATTACH(DrawThemeTextEx);ATTACH(DrawIconEx);ATTACH(CreateCompatibleDC);ATTACH(DeleteDC);ATTACH(SelectObject);ATTACH(DeleteObject);
#undef ATTACH
    return error;
}
