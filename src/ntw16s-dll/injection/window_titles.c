#include "intercept.h"
#include "startup_hooks.h"
#include "unwind.h"
#include "ntw16s-dll/ntw16s.h"
#include "ntw16s-dll/native_api.h"
#include <wchar.h>
#include <uxtheme.h>

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
    InterlockedExchange(&style_ready,1);return TRUE;
}
typedef struct title_scope { ntw16s_creation_scope *creation;DWORD failure; } title_scope;
static BOOL begin_titles(title_scope *scope,BOOL modal)
{
    (void)modal;ZeroMemory(scope,sizeof(*scope));
    if(!InitOnceExecuteOnce(&title_once,bind_titles,NULL,NULL))return FALSE;
    scope->creation=Ntw16sCreationBegin(TRUE);return scope->creation!=NULL;
}
static void end_titles(void *data)
{
    title_scope *scope=data;
    ntw16s_creation_result result={0};
    if(!Ntw16sCreationEnd(scope->creation,&result))scope->failure=GetLastError();
    else scope->failure=result.failure;
}

#define WINDOW_HOOK(S,T) \
static __typeof__(&CreateWindowEx##S) create_##S; \
static NTW16S_UNWIND_FUNCTION HWND WINAPI window_##S(DWORD ex,T cls,T text,DWORD style,int x,int y,int cx,int cy,HWND parent,HMENU menu,HINSTANCE instance,void *param) \
{ ntw16s_unwind_scope unwind;title_scope scope;HWND result=NULL;DWORD error=ERROR_SUCCESS; \
  if(!begin_titles(&scope,FALSE))return NULL; \
  NTW16S_UNWIND_BEGIN(unwind,end_titles,&scope); { result=create_##S(ex,cls,text,style,x,y,cx,cy,parent,menu,instance,param);error=GetLastError(); \
  } NTW16S_UNWIND_END(unwind); \
  if(scope.failure){if(result)DestroyWindow(result);result=NULL;error=scope.failure;} \
  SetLastError(error);return result; }
WINDOW_HOOK(A,LPCSTR)
WINDOW_HOOK(W,LPCWSTR)

#define DIALOG_HOOK(NAME,TYPE,RET,MODAL,FAIL) \
static __typeof__(&NAME) real_##NAME; \
static NTW16S_UNWIND_FUNCTION RET WINAPI hook_##NAME(HINSTANCE instance,TYPE name,HWND parent,DLGPROC proc,LPARAM param) \
{ ntw16s_unwind_scope unwind;title_scope scope;RET result=FAIL;DWORD error=ERROR_SUCCESS; \
  if(!begin_titles(&scope,MODAL))return FAIL; \
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
    BOOL eligible=InterlockedCompareExchange(&style_ready,0,0) && Ntw16sWindowAttached(w);
    SetLastError(saved);return eligible;
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
    result=real_set_pos(w,bar,position,FALSE);error=GetLastError();if(redraw)Ntw16sScrollChanged(w);
    SetLastError(error);return result;
}
static BOOL WINAPI hook_set_range(HWND w,int bar,int first,int last,BOOL redraw)
{
    BOOL result;DWORD error;
    if(!eligible_scroll(w))return real_set_range(w,bar,first,last,redraw);
    result=real_set_range(w,bar,first,last,FALSE);error=GetLastError();if(result && redraw)Ntw16sScrollChanged(w);
    SetLastError(error);return result;
}
static BOOL WINAPI hook_show_scroll(HWND w,int bar,BOOL show)
{
    BOOL result;DWORD error;
    if(!eligible_scroll(w))return real_show_scroll(w,bar,show);
    result=real_show_scroll(w,bar,show);error=GetLastError();if(result)Ntw16sScrollChanged(w);
    SetLastError(error);return result;
}
static BOOL WINAPI hook_enable_scroll(HWND w,UINT bar,UINT arrows)
{
    BOOL result;DWORD error;
    if(!eligible_scroll(w))return real_enable_scroll(w,bar,arrows);
    result=real_enable_scroll(w,bar,arrows);error=GetLastError();Ntw16sScrollChanged(w);
    SetLastError(error);return result;
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
