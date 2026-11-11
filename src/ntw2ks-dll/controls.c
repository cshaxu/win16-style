#include <windows.h>
#include <commdlg.h>
#include <uxtheme.h>
#include <wchar.h>
#include "lib-win32/process-hook/startup_hooks.h"
#include "lib-win32/process-hook/unwind.h"
#include "lib-win32/win2k-window/frame.h"

static __typeof__(&CreateWindowExW) create_W;
static __typeof__(&CreateWindowExA) create_A;

static void classic_control(HWND window,DWORD style)
{
    if(!window || !(style&WS_CHILD))return;
    WCHAR name[32];
    if(!GetClassNameW(window,name,ARRAYSIZE(name)))return;
    static const WCHAR *const classes[]={L"Button",L"Edit",L"ComboBox",L"ListBox",L"ScrollBar"};
    for(unsigned i=0;i<ARRAYSIZE(classes);++i){
        if(!_wcsicmp(name,classes[i])){SetWindowTheme(window,L"",L"");break;}
    }
}

#define WINDOW_HOOK(S,T) \
static HWND WINAPI hook_##S(DWORD ex,T cls,T text,DWORD style,int x,int y,int width,int height,HWND parent,HMENU menu,HINSTANCE instance,void *data) \
{ HWND result=create_##S(ex,cls,text,style,x,y,width,height,parent,menu,instance,data);DWORD error=GetLastError(); \
  classic_control(result,style);if(result && !(style&WS_CHILD))win2k_frame_attach(result);SetLastError(error);return result; }
WINDOW_HOOK(W,LPCWSTR)
WINDOW_HOOK(A,LPCSTR)

/* Dialog manager windows can bypass the exported CreateWindowEx entry points. */
static LRESULT CALLBACK dialog_created(int code,WPARAM wp,LPARAM lp)
{
    DWORD incoming=GetLastError();
    if(code>=0 && lp){
        const CWPRETSTRUCT *event=(const CWPRETSTRUCT *)lp;
        if(event->message==WM_INITDIALOG)win2k_frame_attach(event->hwnd);
    }
    SetLastError(incoming);return CallNextHookEx(NULL,code,wp,lp);
}
static void dialog_scope_end(void *context)
{
    HHOOK hook=*(HHOOK *)context;
    if(hook)UnhookWindowsHookEx(hook);
}
#define DIALOG_HOOK(NAME,TYPE,RET,MODELESS) \
static __typeof__(&NAME) real_##NAME; \
static PROCESS_HOOK_UNWIND_FUNCTION RET WINAPI hook_##NAME(HINSTANCE instance,TYPE name,HWND parent,DLGPROC proc,LPARAM param) \
{ DWORD incoming=GetLastError();HHOOK hook=SetWindowsHookExW(WH_CALLWNDPROCRET,dialog_created,NULL,GetCurrentThreadId()); \
  process_hook_unwind_scope scope;RET result;DWORD error; \
  PROCESS_HOOK_UNWIND_BEGIN(scope,dialog_scope_end,&hook);SetLastError(incoming); \
  { result=real_##NAME(instance,name,parent,proc,param);error=GetLastError(); } \
  PROCESS_HOOK_UNWIND_END(scope);if((MODELESS) && result)win2k_frame_attach((HWND)result);SetLastError(error);return result; }
DIALOG_HOOK(CreateDialogParamA,LPCSTR,HWND,TRUE)
DIALOG_HOOK(CreateDialogParamW,LPCWSTR,HWND,TRUE)
DIALOG_HOOK(CreateDialogIndirectParamA,LPCDLGTEMPLATEA,HWND,TRUE)
DIALOG_HOOK(CreateDialogIndirectParamW,LPCDLGTEMPLATEW,HWND,TRUE)
DIALOG_HOOK(DialogBoxParamA,LPCSTR,INT_PTR,FALSE)
DIALOG_HOOK(DialogBoxParamW,LPCWSTR,INT_PTR,FALSE)
DIALOG_HOOK(DialogBoxIndirectParamA,LPCDLGTEMPLATEA,INT_PTR,FALSE)
DIALOG_HOOK(DialogBoxIndirectParamW,LPCDLGTEMPLATEW,INT_PTR,FALSE)

/* Message boxes may create dialogs internally without calling exported dialog APIs. */
#define MESSAGE_HOOK(NAME,ARGS,CALL) \
static __typeof__(&NAME) real_##NAME; \
static PROCESS_HOOK_UNWIND_FUNCTION int WINAPI hook_##NAME ARGS \
{ DWORD incoming=GetLastError();HHOOK hook=SetWindowsHookExW(WH_CALLWNDPROCRET,dialog_created,NULL,GetCurrentThreadId()); \
  process_hook_unwind_scope scope;int result;DWORD error; \
  PROCESS_HOOK_UNWIND_BEGIN(scope,dialog_scope_end,&hook);SetLastError(incoming); \
  { result=real_##NAME CALL;error=GetLastError(); } \
  PROCESS_HOOK_UNWIND_END(scope);SetLastError(error);return result; }
MESSAGE_HOOK(MessageBoxA,(HWND owner,LPCSTR text,LPCSTR title,UINT type),(owner,text,title,type))
MESSAGE_HOOK(MessageBoxW,(HWND owner,LPCWSTR text,LPCWSTR title,UINT type),(owner,text,title,type))
MESSAGE_HOOK(MessageBoxExA,(HWND owner,LPCSTR text,LPCSTR title,UINT type,WORD language),(owner,text,title,type,language))
MESSAGE_HOOK(MessageBoxExW,(HWND owner,LPCWSTR text,LPCWSTR title,UINT type,WORD language),(owner,text,title,type,language))
MESSAGE_HOOK(MessageBoxIndirectA,(const MSGBOXPARAMSA *parameters),(parameters))
MESSAGE_HOOK(MessageBoxIndirectW,(const MSGBOXPARAMSW *parameters),(parameters))

/* Find and replace create modeless dialogs inside COMDLG32. */
#define FIND_HOOK(NAME,TYPE) \
static __typeof__(&NAME) real_##NAME; \
static PROCESS_HOOK_UNWIND_FUNCTION HWND WINAPI hook_##NAME(TYPE *parameters) \
{ DWORD incoming=GetLastError();HHOOK hook=SetWindowsHookExW(WH_CALLWNDPROCRET,dialog_created,NULL,GetCurrentThreadId()); \
  process_hook_unwind_scope scope;HWND result;DWORD error; \
  PROCESS_HOOK_UNWIND_BEGIN(scope,dialog_scope_end,&hook);SetLastError(incoming); \
  { result=real_##NAME(parameters);error=GetLastError(); } \
  PROCESS_HOOK_UNWIND_END(scope);if(result)win2k_frame_attach(result);SetLastError(error);return result; }
FIND_HOOK(FindTextA,FINDREPLACEA)
FIND_HOOK(FindTextW,FINDREPLACEW)
FIND_HOOK(ReplaceTextA,FINDREPLACEA)
FIND_HOOK(ReplaceTextW,FINDREPLACEW)

/* Older file dialogs can create their window without exported dialog calls. */
#define FILE_HOOK(NAME,TYPE) \
static __typeof__(&NAME) real_##NAME; \
static PROCESS_HOOK_UNWIND_FUNCTION BOOL WINAPI hook_##NAME(TYPE *parameters) \
{ DWORD incoming=GetLastError();HHOOK hook=SetWindowsHookExW(WH_CALLWNDPROCRET,dialog_created,NULL,GetCurrentThreadId()); \
  process_hook_unwind_scope scope;BOOL result;DWORD error; \
  PROCESS_HOOK_UNWIND_BEGIN(scope,dialog_scope_end,&hook);SetLastError(incoming); \
  { result=real_##NAME(parameters);error=GetLastError(); } \
  PROCESS_HOOK_UNWIND_END(scope);SetLastError(error);return result; }
FILE_HOOK(GetOpenFileNameA,OPENFILENAMEA)
FILE_HOOK(GetOpenFileNameW,OPENFILENAMEW)
FILE_HOOK(GetSaveFileNameA,OPENFILENAMEA)
FILE_HOOK(GetSaveFileNameW,OPENFILENAMEW)

DWORD ntw2ks_controls_attach(void)
{
    create_W=CreateWindowExW;create_A=CreateWindowExA;
    DWORD error=process_hook_startup_attach(&create_W,(void *)hook_W);
    if(!error)error=process_hook_startup_attach(&create_A,(void *)hook_A);
#define ATTACH_DIALOG(NAME) do { if(!error){real_##NAME=NAME;error=process_hook_startup_attach(&real_##NAME,(void *)hook_##NAME);} } while(0)
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
#undef ATTACH_DIALOG
    return error;
}
