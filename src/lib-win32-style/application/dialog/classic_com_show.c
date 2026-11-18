/* Display and result conversion operate on the one public proxy owner. */
#define COBJMACROS
#include "lib-win32-style/application/dialog/classic_com_show.h"
#include "lib-win32-style/application/dialog/classic_file_contract.h"
#include "lib-win32-style/application/dialog/classic_shell_folder.h"
#include "lib-win32-style/application/dialog/classic_shell_item.h"
#include "lib-win32/process-hook/unwind.h"
#include <wchar.h>

typedef struct classic_call {OPENFILENAMEW *ofn;BOOL save;BOOL selected;DWORD error;} classic_call;
static HRESULT classic_show(void *data)
{
    classic_call *call=data;call->selected=call->save?GetSaveFileNameW(call->ofn):GetOpenFileNameW(call->ofn);
    call->error=call->selected?0:CommDlgExtendedError();return S_OK;
}
typedef struct native_call {IFileDialog *dialog;HWND owner;} native_call;
static HRESULT native_show(void *data)
{native_call *call=data;return IFileDialog_Show(call->dialog,call->owner);}
static HRESULT show_native(style_classic_dialog_proxy *p,HWND owner,
    const style_classic_com_show_services *services)
{native_call call={p->native,owner};return services->scope(native_show,&call);}
static BOOL explorer_file_directory(WCHAR *file,PWSTR *folder)
{
    WCHAR *leaf=wcsrchr(file,L'\\');
    BOOL absolute=(file[0]==L'\\' && file[1]==L'\\') ||
        (file[0] && file[1]==L':' && file[2]==L'\\');
    if(!absolute || !leaf || !leaf[1])return TRUE;
    size_t length=(size_t)(leaf-file)+1;
    WCHAR *directory=CoTaskMemAlloc((length+1)*sizeof(*directory));
    if(!directory)return FALSE;
    wmemcpy(directory,file,length);
    directory[length]=L'\0';
    wmemmove(file,leaf+1,wcslen(leaf+1)+1);
    CoTaskMemFree(*folder);
    *folder=directory;
    return TRUE;
}
static HRESULT show_impl(style_classic_dialog_proxy *p,HWND owner,
    const style_classic_com_show_services *services)
{
    if(services->experimental_events)p->classic_close_pending=FALSE;
    FILEOPENDIALOGOPTIONS flags=0;
    if(p->result){IShellItem_Release(p->result);p->result=NULL;}
    if(p->results){IShellItemArray_Release(p->results);p->results=NULL;}p->completed=FALSE;
    if(!services->session_active())return show_native(p,owner,services);
    HRESULT hr=IFileDialog_GetOptions(p->native,&flags);
    if(!style_classic_com_can_bridge(hr,flags,p->unsupported,(p->events!=NULL),p->save,
        services->experimental_events,services->experimental_hidden))return show_native(p,owner,services);
    if(services->explorer_files && !(flags&FOS_PICKFOLDERS) &&
        (!(flags&FOS_FORCEFILESYSTEM) || p->ok_label || p->cancel_label || p->file_label || p->events))
        return show_native(p,owner,services);
    if(services->experimental_hidden)p->classic_hidden=(flags&FOS_FORCESHOWHIDDEN)!=0;
    WCHAR file[32768]=L"";PWSTR value=NULL;IShellItem *folder=NULL;PWSTR folder_path=NULL;
    if(SUCCEEDED(IFileDialog_GetFileName(p->native,&value)) && value){
        if(wcslen(value)>=ARRAYSIZE(file)){CoTaskMemFree(value);return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);}
        wcscpy(file,value);CoTaskMemFree(value);
    }
    if(SUCCEEDED(IFileDialog_GetFolder(p->native,&folder)) && folder){
        (void)IShellItem_GetDisplayName(folder,SIGDN_FILESYSPATH,&folder_path);IShellItem_Release(folder);
    }
    if(flags&FOS_PICKFOLDERS){
        if(p->save || !(flags&FOS_FORCEFILESYSTEM) || p->ok_label || p->cancel_label || p->file_label || (p->events!=NULL)){
            if(folder_path)CoTaskMemFree(folder_path);
            return show_native(p,owner,services);
        }
        WCHAR selected[32768];style_classic_shell_folder_call call={.owner=owner,.title=p->title,.initial=folder_path,
            .result=selected,.capacity=ARRAYSIZE(selected)};
        hr=services->scope(style_classic_shell_folder_show,&call);if(folder_path)CoTaskMemFree(folder_path);
        if(services->experimental_events && FAILED(hr) && p->classic_close_pending)return p->classic_close_result;
        if(FAILED(hr))return hr;
        if(services->experimental_events && (p->events!=NULL) && p->completed && p->result)return S_OK;
        hr=SHCreateItemFromParsingName(selected,NULL,&IID_IShellItem,(void **)&p->result);
        if(SUCCEEDED(hr))p->completed=TRUE;
        return hr;
    }
    if(services->explorer_files && !explorer_file_directory(file,&folder_path)){
        CoTaskMemFree(folder_path);
        return E_OUTOFMEMORY;
    }
    OPENFILENAMEW ofn={0};ofn.lStructSize=sizeof(ofn);ofn.hwndOwner=owner;
    ofn.lpstrFile=file;ofn.nMaxFile=ARRAYSIZE(file);ofn.lpstrInitialDir=folder_path;ofn.lpstrTitle=p->title;
    ofn.lpstrFilter=p->filter?p->filter:L"All files\0*.*\0\0";ofn.nFilterIndex=p->filter_index?p->filter_index:1;ofn.lpstrDefExt=p->extension;
    ofn.Flags=style_classic_com_file_flags(flags);
    if(services->explorer_files){
        ofn.Flags=(ofn.Flags&~OFN_ENABLEHOOK)|OFN_EXPLORER;
    }else{
        ofn.lpfnHook=services->classic_hook;
        ofn.lCustData=(LPARAM)p;
    }
    classic_call call={&ofn,p->save,FALSE,0};
    hr=services->scope(classic_show,&call);if(folder_path)CoTaskMemFree(folder_path);
    if(FAILED(hr))return hr;
    if(!call.selected){
        if(services->experimental_events && p->classic_close_pending)return p->classic_close_result;
        return call.error?HRESULT_FROM_WIN32(ERROR_INVALID_DATA):HRESULT_FROM_WIN32(ERROR_CANCELLED);}
    if(p->filter && ofn.nFilterIndex){p->filter_index=ofn.nFilterIndex;(void)IFileDialog_SetFileTypeIndex(p->native,p->filter_index);}
    if(flags&FOS_ALLOWMULTISELECT){
        if(services->experimental_events){
            if(p->result){IShellItem_Release(p->result);p->result=NULL;}
            if(p->results){IShellItemArray_Release(p->results);p->results=NULL;}
            p->completed=FALSE;
        }
        return style_classic_multi_result(file,ofn.nFileOffset,(flags&FOS_NODEREFERENCELINKS)!=0,&p->result,&p->results,&p->completed);
    }
    if(services->experimental_events && p->result){IShellItem_Release(p->result);p->result=NULL;p->completed=FALSE;}
    hr=p->save?SHCreateItemFromParsingName(file,NULL,&IID_IShellItem,(void **)&p->result):style_classic_open_item(file,(flags&FOS_NODEREFERENCELINKS)!=0,&p->result);
    if(p->save && hr==HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))hr=style_classic_save_item(file,&p->result);
    if(SUCCEEDED(hr))p->completed=TRUE;
    return hr;
}

typedef struct show_scope {style_classic_dialog_proxy *proxy;BOOL completed;} show_scope;
static void show_cleanup(void *data)
{
    show_scope *scope=data;
    if(!scope->completed)scope->proxy->classic_show_aborted=TRUE;
    style_classic_dialog_proxy_release(scope->proxy);
}
PROCESS_HOOK_UNWIND_FUNCTION HRESULT style_classic_com_show(style_classic_dialog_proxy *p,HWND owner,
    const style_classic_com_show_services *services)
{
    if(!services->experimental_events)return show_impl(p,owner,services);
    if(p->classic_show_aborted)return E_UNEXPECTED;
    style_classic_dialog_proxy_add(p);
    process_hook_unwind_scope unwind;HRESULT result;show_scope scope={p,FALSE};
    PROCESS_HOOK_UNWIND_BEGIN(unwind,show_cleanup,&scope);
    result=show_impl(p,owner,services);scope.completed=TRUE;
    PROCESS_HOOK_UNWIND_END(unwind);return result;
}
