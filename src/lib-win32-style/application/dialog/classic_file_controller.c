/* Adapt compatible single-file common dialogs to the system's old layout.
 * All custom hooks/templates, multi-selection and extended options pass through.
 * No external dialog resources are loaded or distributed. */
#define COBJMACROS
#include "classic_file_controller.h"
#include "../lifecycle/creation.h"
#include "lib-win32/process-hook/unwind.h"
#include "lib-win32-style/application/dialog/classic_file_helpers.h"
#include "lib-win32-style/application/dialog/classic_file_contract.h"
#include "lib-win32-style/application/dialog/classic_file_hook.h"
#include "lib-win32-style/application/dialog/message_observer.h"
#include <dlgs.h>
#include <commdlg.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <string.h>
#include <wchar.h>

static style_classic_file_options configuration;
static BOOL (WINAPI *native_open_a)(LPOPENFILENAMEA);
static BOOL (WINAPI *native_open_w)(LPOPENFILENAMEW);
static BOOL (WINAPI *native_save_a)(LPOPENFILENAMEA);
static BOOL (WINAPI *native_save_w)(LPOPENFILENAMEW);

/* Old-style dialogs do not offer every Explorer-style contract.  In particular,
 * a caller's callback speaks a different protocol and must never be translated. */
/* The OS may also create an old-style template for callers with their own
 * hooks. Observe its initialized controls without replacing the caller hook. */
static void old_dialog_initialized(HWND window)
{style_classic_file_initialized(window,configuration.chrome);}
static LRESULT CALLBACK old_dialog_return(int code,WPARAM wparam,LPARAM lparam)
{
    const CWPRETSTRUCT *message=style_message_dialog_initialized(code,lparam);
    if(message)old_dialog_initialized(message->hwnd);
    return CallNextHookEx(NULL,code,wparam,lparam);
}
typedef struct old_dialog_scope {HHOOK hook;BOOL resolve_links;struct old_dialog_scope *previous_active;
    void (*previous)(HWND);
} old_dialog_scope;
static _Thread_local old_dialog_scope *active_file_dialog;
static void old_dialog_end(void *value)
{
    old_dialog_scope *scope=value;active_file_dialog=scope->previous_active;style_message_observer_end(scope->hook);
    if(configuration.subclass_observer)style_creation_dialog_observer(scope->previous);
}
static UINT_PTR CALLBACK old_dialog_hook(HWND window,UINT message,WPARAM wparam,LPARAM lparam)
{return style_classic_file_hook(window,message,wparam,lparam,
    active_file_dialog && active_file_dialog->resolve_links,configuration.chrome);}

static void old_dialog_begin(old_dialog_scope *scope)
{
    if(configuration.subclass_observer)scope->previous=style_creation_dialog_observer(old_dialog_initialized);
    else scope->hook=style_message_observer_begin(old_dialog_return);
}
#define BRIDGE(name, structure, suffix, original, extra_flags) \
    typedef struct name##_call {structure *value;BOOL result;DWORD incoming,error;} name##_call; \
    static HRESULT name##_invoke(void *value) {name##_call *call=value;SetLastError(call->incoming); \
        call->result=original(call->value);call->error=GetLastError();return S_OK;} \
static PROCESS_HOOK_UNWIND_FUNCTION BOOL WINAPI name(structure *value) \
{ \
    if(!configuration.active())return original(value); \
    structure local;structure *passed=value; \
    BOOL eligible=style_classic_file_prepare_##suffix(value,&local,extra_flags,old_dialog_hook); \
    if(eligible)passed=&local; \
    DWORD incoming=GetLastError(); \
    old_dialog_scope observer={0};observer.resolve_links=style_classic_file_resolve_links(eligible,(extra_flags),eligible && value->lpstrFilter,eligible?value->Flags:0); \
    observer.previous_active=active_file_dialog;active_file_dialog=&observer;old_dialog_begin(&observer); \
    process_hook_unwind_scope unwind;name##_call call={passed,FALSE,incoming,ERROR_SUCCESS}; \
    HRESULT status; \
    PROCESS_HOOK_UNWIND_BEGIN(unwind,old_dialog_end,&observer); {status=configuration.scope(name##_invoke,&call); \
    } PROCESS_HOOK_UNWIND_END(unwind); \
    BOOL result=call.result;DWORD error=call.error; \
    if(FAILED(status)){result=FALSE;error=HRESULT_CODE(status);} \
    if(eligible)style_classic_file_result_##suffix(value,&local); \
    SetLastError(error);return result; \
}
BRIDGE(bridge_open_a, OPENFILENAMEA, a, native_open_a, OFN_NODEREFERENCELINKS)
BRIDGE(bridge_open_w, OPENFILENAMEW, w, native_open_w, OFN_NODEREFERENCELINKS)
BRIDGE(bridge_save_a, OPENFILENAMEA, a, native_save_a, 0)
BRIDGE(bridge_save_w, OPENFILENAMEW, w, native_save_w, 0)

DWORD style_classic_file_attach(DWORD (*attach)(void *,void *),const style_classic_file_options *options)
{
    if(!attach || !options || !options->active || !options->scope || !options->chrome)return ERROR_INVALID_PARAMETER;
    configuration=*options;
    native_open_a=GetOpenFileNameA;native_open_w=GetOpenFileNameW;
    native_save_a=GetSaveFileNameA;native_save_w=GetSaveFileNameW;
    DWORD error=attach(&native_open_a,(void *)bridge_open_a);
    if(!error)error=attach(&native_open_w,(void *)bridge_open_w);
    if(!error)error=attach(&native_save_a,(void *)bridge_save_a);
    if(!error)error=attach(&native_save_w,(void *)bridge_save_w);
    return error;
}
