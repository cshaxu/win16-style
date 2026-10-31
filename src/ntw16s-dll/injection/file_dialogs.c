/* Adapt compatible single-file common dialogs to the system's old layout.
 * All custom hooks/templates, multi-selection and extended options pass through.
 * No external dialog resources are loaded or distributed. */
#define COBJMACROS
#include "intercept.h"
#include "startup_hooks.h"
#include "unwind.h"
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
static BOOL old_dialog_folder(HWND window,WCHAR *out,size_t capacity)
{
    HWND list=GetDlgItem(window,lst2);WCHAR cls[32]=L"";
    if(!list||GetParent(list)!=window||!GetClassNameW(list,cls,ARRAYSIZE(cls))||
       lstrcmpiW(cls,L"ListBox")||!(GetWindowLongPtrW(list,GWL_STYLE)&LBS_HASSTRINGS))return FALSE;
    LRESULT selected=SendMessageW(list,LB_GETCURSEL,0,0),count=SendMessageW(list,LB_GETCOUNT,0,0);
    if(selected<0||selected>=count||selected>255)return FALSE;
    size_t used=0;out[0]=0;
    for(LRESULT i=0;i<=selected;++i){WCHAR part[MAX_PATH];LRESULT n=SendMessageW(list,LB_GETTEXTLEN,(WPARAM)i,0);
        if(n<=0||n>=MAX_PATH||used+(size_t)n+2>capacity||SendMessageW(list,LB_GETTEXT,(WPARAM)i,(LPARAM)part)!=n)return FALSE;
        if(i==0){if(!(n==3&&part[1]==L':'&&part[2]==L'\\'))return FALSE;}
        else if(wcspbrk(part,L"\\/:*?\"<>|")||!lstrcmpW(part,L".")||!lstrcmpW(part,L".."))return FALSE;
        if(used&&out[used-1]!=L'\\')out[used++]=L'\\';
        memcpy(out+used,part,((size_t)n+1)*sizeof(WCHAR));used+=(size_t)n;
    }
    return TRUE;
}
/* Let the system finish validation and produce offsets/buffer errors after a
 * regular file shortcut is resolved. Other link forms retain native handling. */
static void old_dialog_resolve_file(HWND window)
{
    WCHAR path[MAX_PATH]=L"",target[MAX_PATH]=L"";
    DWORD saved=GetLastError();
    UINT length=GetDlgItemTextW(window,edt1,path,ARRAYSIZE(path));
    if(!length||length>=ARRAYSIZE(path)-1)goto done;
    if(!(path[0]&&path[1]==L':'&&path[2]==L'\\')){
        WCHAR folder[MAX_PATH],combined[MAX_PATH];
        if(wcspbrk(path,L"\\/:*?\"<>|")||!old_dialog_folder(window,folder,ARRAYSIZE(folder)))goto done;
        int count=swprintf(combined,ARRAYSIZE(combined),L"%ls\\%ls",folder,path);
        if(count<0||count>=MAX_PATH)goto done;
        memcpy(path,combined,((size_t)count+1)*sizeof(WCHAR));
    }
    const WCHAR *extension=wcsrchr(path,L'.');if(!extension||lstrcmpiW(extension,L".lnk"))goto done;
    HRESULT initialized=CoInitializeEx(NULL,COINIT_APARTMENTTHREADED);
    if(FAILED(initialized)&&initialized!=RPC_E_CHANGED_MODE)goto done;
    IShellLinkW *link=NULL;IPersistFile *persist=NULL;
    HRESULT hr=CoCreateInstance(&CLSID_ShellLink,NULL,CLSCTX_INPROC_SERVER,&IID_IShellLinkW,(void **)&link);
    if(SUCCEEDED(hr))hr=IShellLinkW_QueryInterface(link,&IID_IPersistFile,(void **)&persist);
    if(SUCCEEDED(hr))hr=IPersistFile_Load(persist,path,STGM_READ);
    if(SUCCEEDED(hr))hr=IShellLinkW_GetPath(link,target,ARRAYSIZE(target),NULL,0);
    if(hr==S_OK&&target[0]&&target[1]==L':'&&target[2]==L'\\'){DWORD attributes=GetFileAttributesW(target);
        if(attributes!=INVALID_FILE_ATTRIBUTES&&!(attributes&FILE_ATTRIBUTE_DIRECTORY))(void)SetDlgItemTextW(window,edt1,target);}
    if(persist)IPersistFile_Release(persist);
    if(link)IShellLinkW_Release(link);
    if(SUCCEEDED(initialized))CoUninitialize();
done:
    SetLastError(saved);
}
static UINT_PTR CALLBACK old_dialog_hook(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    (void)lparam;
    if(message==WM_INITDIALOG)ntw16s_old_dialog_chrome(window);
    if(message==WM_COMMAND&&LOWORD(wparam)==IDOK&&active_file_dialog&&active_file_dialog->resolve_links)old_dialog_resolve_file(window);
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
#define BRIDGE(name, structure, legacy_size, original, extra_flags) \
    typedef struct name##_call {structure *value;BOOL result;DWORD incoming,error;} name##_call; \
    static HRESULT name##_invoke(void *value) {name##_call *call=value;SetLastError(call->incoming); \
        call->result=original(call->value);call->error=GetLastError();return S_OK;} \
static NTW16S_UNWIND_FUNCTION BOOL WINAPI name(structure *value) \
{ \
    BOOL full=value && value->lStructSize==sizeof(*value); \
    BOOL legacy=value && value->lStructSize==(legacy_size); \
    BOOL eligible=(full || legacy) && !(value->Flags & ~(COMPATIBLE_FLAGS | (extra_flags))) && \
        (!full || !value->FlagsEx); \
    structure local;structure *passed=value; \
    if(eligible){ZeroMemory(&local,sizeof(local));memcpy(&local,value,value->lStructSize); \
        local.lStructSize=sizeof(local);local.Flags=(local.Flags & ~OFN_EXPLORER) | OFN_LONGNAMES | OFN_ENABLEHOOK; \
        local.lpfnHook=old_dialog_hook;passed=&local;} \
    DWORD incoming=GetLastError(); \
    old_dialog_scope observer={0};observer.resolve_links=eligible && (extra_flags) && value->lpstrFilter && \
        !(value->Flags & (OFN_NODEREFERENCELINKS | OFN_NOVALIDATE)); \
    observer.previous_active=active_file_dialog;active_file_dialog=&observer;NTW16S_OLD_DIALOG_OBSERVER(observer); \
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
BRIDGE(bridge_open_a, OPENFILENAMEA, OPENFILENAME_SIZE_VERSION_400A, native_open_a, OFN_NODEREFERENCELINKS)
BRIDGE(bridge_open_w, OPENFILENAMEW, OPENFILENAME_SIZE_VERSION_400W, native_open_w, OFN_NODEREFERENCELINKS)
BRIDGE(bridge_save_a, OPENFILENAMEA, OPENFILENAME_SIZE_VERSION_400A, native_save_a, 0)
BRIDGE(bridge_save_w, OPENFILENAMEW, OPENFILENAME_SIZE_VERSION_400W, native_save_w, 0)

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
