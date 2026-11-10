/* Adapt compatible single-file common dialogs to the system's old layout.
 * All custom hooks/templates, multi-selection and extended options pass through.
 * No external dialog resources are loaded or distributed. */
#define COBJMACROS
#include "intercept.h"
#include "../../lib-win32/process-hook/startup_hooks.h"
#include "../../lib-win32/process-hook/unwind.h"
#include "../../lib-win32/win16-dialog/file_helpers.h"
#include "../../lib-win32/win16-dialog/file_contract.h"
#include "../../lib-win32/win16-dialog/file_hook.h"
#include <dlgs.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <string.h>
#include <wchar.h>

static BOOL (WINAPI *native_open_a)(LPOPENFILENAMEA);
static BOOL (WINAPI *native_open_w)(LPOPENFILENAMEW);
static BOOL (WINAPI *native_save_a)(LPOPENFILENAMEA);
static BOOL (WINAPI *native_save_w)(LPOPENFILENAMEW);

/* Old-style dialogs do not offer every Explorer-style contract.  In particular,
 * a caller's callback speaks a different protocol and must never be translated. */
/* The OS may also create an old-style template for callers with their own
 * hooks. Observe its initialized controls without replacing the caller hook. */
static void old_dialog_initialized(HWND window)
{win16_dialog_file_initialized(window,ntw16s_old_dialog_chrome);}
#ifndef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
static LRESULT CALLBACK old_dialog_return(int code,WPARAM wparam,LPARAM lparam)
{
    if(code>=0 && lparam){const CWPRETSTRUCT *m=(const CWPRETSTRUCT *)lparam;
        if(m->message==WM_INITDIALOG)old_dialog_initialized(m->hwnd);}
    return CallNextHookEx(NULL,code,wparam,lparam);
}
#endif
typedef struct old_dialog_scope {HHOOK hook;BOOL resolve_links;struct old_dialog_scope *previous_active;
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
    void (*previous)(HWND);
#endif
} old_dialog_scope;
static _Thread_local old_dialog_scope *active_file_dialog;
static void old_dialog_end(void *value)
{old_dialog_scope *scope=value;active_file_dialog=scope->previous_active;if(scope->hook)UnhookWindowsHookEx(scope->hook);
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
    ntw16s_creation_dialog_observer(scope->previous);
#endif
}
static UINT_PTR CALLBACK old_dialog_hook(HWND window,UINT message,WPARAM wparam,LPARAM lparam)
{return win16_dialog_file_hook(window,message,wparam,lparam,
    active_file_dialog && active_file_dialog->resolve_links,ntw16s_old_dialog_chrome);}

#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
#define NTW16S_OLD_DIALOG_OBSERVER(scope) ((scope).previous=ntw16s_creation_dialog_observer(old_dialog_initialized))
#else
#define NTW16S_OLD_DIALOG_OBSERVER(scope) ((scope).hook=SetWindowsHookExW(WH_CALLWNDPROCRET,old_dialog_return,NULL,GetCurrentThreadId()))
#endif
#define BRIDGE(name, structure, suffix, original, extra_flags) \
    typedef struct name##_call {structure *value;BOOL result;DWORD incoming,error;} name##_call; \
    static HRESULT name##_invoke(void *value) {name##_call *call=value;SetLastError(call->incoming); \
        call->result=original(call->value);call->error=GetLastError();return S_OK;} \
static PROCESS_HOOK_UNWIND_FUNCTION BOOL WINAPI name(structure *value) \
{ \
    if(!ntw16s_process_session_active())return original(value); \
    structure local;structure *passed=value; \
    BOOL eligible=win16_dialog_file_prepare_##suffix(value,&local,extra_flags,old_dialog_hook); \
    if(eligible)passed=&local; \
    DWORD incoming=GetLastError(); \
    old_dialog_scope observer={0};observer.resolve_links=win16_dialog_file_resolve_links(eligible,(extra_flags),eligible && value->lpstrFilter,eligible?value->Flags:0); \
    observer.previous_active=active_file_dialog;active_file_dialog=&observer;NTW16S_OLD_DIALOG_OBSERVER(observer); \
    process_hook_unwind_scope unwind;name##_call call={passed,FALSE,incoming,ERROR_SUCCESS}; \
    HRESULT status; \
    PROCESS_HOOK_UNWIND_BEGIN(unwind,old_dialog_end,&observer); {status=ntw16s_dialog_style_scope(name##_invoke,&call); \
    } PROCESS_HOOK_UNWIND_END(unwind); \
    BOOL result=call.result;DWORD error=call.error; \
    if(FAILED(status)){result=FALSE;error=HRESULT_CODE(status);} \
    if(eligible)win16_dialog_file_result_##suffix(value,&local); \
    SetLastError(error);return result; \
}
BRIDGE(bridge_open_a, OPENFILENAMEA, a, native_open_a, OFN_NODEREFERENCELINKS)
BRIDGE(bridge_open_w, OPENFILENAMEW, w, native_open_w, OFN_NODEREFERENCELINKS)
BRIDGE(bridge_save_a, OPENFILENAMEA, a, native_save_a, 0)
BRIDGE(bridge_save_w, OPENFILENAMEW, w, native_save_w, 0)

LONG ntw16s_attach_file_dialogs_using(DWORD (*attach)(void *,void *))
{
    if(!attach)return ERROR_INVALID_PARAMETER;
    if (!(ntw16s_process_context.flags & NTWSHK_POLICY_STYLE)) return NO_ERROR;
    native_open_a=GetOpenFileNameA; native_open_w=GetOpenFileNameW;
    native_save_a=GetSaveFileNameA; native_save_w=GetSaveFileNameW;
    LONG error=(LONG)attach(&native_open_a,(void *)bridge_open_a);
    if (!error) error=(LONG)attach(&native_open_w,(void *)bridge_open_w);
    if (!error) error=(LONG)attach(&native_save_a,(void *)bridge_save_a);
    if (!error) error=(LONG)attach(&native_save_w,(void *)bridge_save_w);
    return error;
}

LONG ntw16s_attach_file_dialogs(void){return ntw16s_attach_file_dialogs_using(process_hook_startup_attach);}
