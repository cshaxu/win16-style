/* Adapt compatible single-file common dialogs to the system's old layout.
 * All custom hooks/templates, multi-selection and extended options pass through.
 * No external dialog resources are loaded or distributed. */
#include "intercept.h"
#include "startup_hooks.h"
#include "unwind.h"
#include <dlgs.h>
#include <commdlg.h>
#include <string.h>

static BOOL (WINAPI *native_open_a)(LPOPENFILENAMEA);
static BOOL (WINAPI *native_open_w)(LPOPENFILENAMEW);
static BOOL (WINAPI *native_save_a)(LPOPENFILENAMEA);
static BOOL (WINAPI *native_save_w)(LPOPENFILENAMEW);

/* Old-style dialogs do not offer every Explorer-style contract.  In particular,
 * a caller's callback speaks a different protocol and must never be translated. */
/* The OS may also create an old-style template for callers with their own
 * hooks. Observe its initialized controls without replacing the caller hook. */
static void old_dialog_initialized(HWND window)
{
    WCHAR dialog_class[32],list_class[32];HWND list=GetDlgItem(window,lst2);
    if(list && GetParent(list)==window && GetClassNameW(window,dialog_class,ARRAYSIZE(dialog_class)) &&
        !lstrcmpW(dialog_class,L"#32770") && GetClassNameW(list,list_class,ARRAYSIZE(list_class)) &&
        !lstrcmpiW(list_class,L"ListBox"))ntw16s_old_dialog_chrome(window);
}
#ifndef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
static LRESULT CALLBACK old_dialog_return(int code,WPARAM wparam,LPARAM lparam)
{
    if(code>=0 && lparam){const CWPRETSTRUCT *m=(const CWPRETSTRUCT *)lparam;
        if(m->message==WM_INITDIALOG)old_dialog_initialized(m->hwnd);}
    return CallNextHookEx(NULL,code,wparam,lparam);
}
#endif
typedef struct old_dialog_scope {HHOOK hook;
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
    void (*previous)(HWND);
#endif
} old_dialog_scope;
static void old_dialog_end(void *value)
{old_dialog_scope *scope=value;if(scope->hook)UnhookWindowsHookEx(scope->hook);
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
    ntw16s_creation_dialog_observer(scope->previous);
#endif
}
static UINT_PTR CALLBACK old_dialog_hook(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    (void)wparam; (void)lparam;
    if(message==WM_INITDIALOG)ntw16s_old_dialog_chrome(window);
    return 0;
}

#define COMPATIBLE_FLAGS (OFN_READONLY | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY | \
    OFN_NOCHANGEDIR | OFN_SHOWHELP | OFN_NOVALIDATE | OFN_PATHMUSTEXIST | \
    OFN_FILEMUSTEXIST | OFN_CREATEPROMPT | OFN_NOREADONLYRETURN | OFN_NOTESTFILECREATE | \
    OFN_NONETWORKBUTTON | OFN_EXPLORER | OFN_LONGNAMES | OFN_EXTENSIONDIFFERENT)
#ifdef NTW16S_EXPERIMENTAL_CREATION_SUBCLASS
#define NTW16S_OLD_DIALOG_OBSERVER(scope) ((scope).previous=ntw16s_creation_dialog_observer(old_dialog_initialized))
#else
#define NTW16S_OLD_DIALOG_OBSERVER(scope) ((scope).hook=SetWindowsHookExW(WH_CALLWNDPROCRET,old_dialog_return,NULL,GetCurrentThreadId()))
#endif
#define BRIDGE(name, structure, legacy_size, original) \
    typedef struct name##_call {structure *value;BOOL result;DWORD incoming,error;} name##_call; \
    static HRESULT name##_invoke(void *value) {name##_call *call=value;SetLastError(call->incoming); \
        call->result=original(call->value);call->error=GetLastError();return S_OK;} \
static NTW16S_UNWIND_FUNCTION BOOL WINAPI name(structure *value) \
{ \
    BOOL full=value && value->lStructSize==sizeof(*value); \
    BOOL legacy=value && value->lStructSize==(legacy_size); \
    BOOL eligible=(full || legacy) && !(value->Flags & ~COMPATIBLE_FLAGS) && \
        (!full || !value->FlagsEx) && !value->lpstrCustomFilter; \
    structure local;structure *passed=value; \
    if(eligible){ZeroMemory(&local,sizeof(local));memcpy(&local,value,value->lStructSize); \
        local.lStructSize=sizeof(local);local.Flags=(local.Flags & ~OFN_EXPLORER) | OFN_LONGNAMES | OFN_ENABLEHOOK; \
        local.lpfnHook=old_dialog_hook;passed=&local;} \
    DWORD incoming=GetLastError(); \
    old_dialog_scope observer={0};NTW16S_OLD_DIALOG_OBSERVER(observer); \
    ntw16s_unwind_scope unwind;name##_call call={passed,FALSE,incoming,ERROR_SUCCESS}; \
    HRESULT status; \
    NTW16S_UNWIND_BEGIN(unwind,old_dialog_end,&observer); {status=ntw16s_dialog_style_scope(name##_invoke,&call); \
    } NTW16S_UNWIND_END(unwind); \
    BOOL result=call.result;DWORD error=call.error; \
    if(FAILED(status)){result=FALSE;error=HRESULT_CODE(status);} \
    if(eligible){value->Flags=(value->Flags & ~(OFN_READONLY | OFN_EXTENSIONDIFFERENT)) | \
        (local.Flags & (OFN_READONLY | OFN_EXTENSIONDIFFERENT)); \
        value->nFilterIndex=local.nFilterIndex;value->nFileOffset=local.nFileOffset; \
        value->nFileExtension=local.nFileExtension;} \
    SetLastError(error);return result; \
}
BRIDGE(bridge_open_a, OPENFILENAMEA, OPENFILENAME_SIZE_VERSION_400A, native_open_a)
BRIDGE(bridge_open_w, OPENFILENAMEW, OPENFILENAME_SIZE_VERSION_400W, native_open_w)
BRIDGE(bridge_save_a, OPENFILENAMEA, OPENFILENAME_SIZE_VERSION_400A, native_save_a)
BRIDGE(bridge_save_w, OPENFILENAMEW, OPENFILENAME_SIZE_VERSION_400W, native_save_w)

LONG ntw16s_attach_file_dialogs(void)
{
    if (!(ntw16s_process_context.flags & NTW16S_POLICY_STYLE)) return NO_ERROR;
    native_open_a=GetOpenFileNameA; native_open_w=GetOpenFileNameW;
    native_save_a=GetSaveFileNameA; native_save_w=GetSaveFileNameW;
    LONG error=(LONG)ntw16s_hooks_attach(&native_open_a,(void *)bridge_open_a);
    if (!error) error=(LONG)ntw16s_hooks_attach(&native_open_w,(void *)bridge_open_w);
    if (!error) error=(LONG)ntw16s_hooks_attach(&native_save_a,(void *)bridge_save_a);
    if (!error) error=(LONG)ntw16s_hooks_attach(&native_save_w,(void *)bridge_save_w);
    return error;
}
