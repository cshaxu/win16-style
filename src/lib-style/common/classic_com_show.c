/* Common-dialog display and result conversion. The caller owns state and scopes. */
#define COBJMACROS
#include "lib-style/common/classic_com_show.h"
#include "lib-style/common/classic_file_contract.h"
#include "lib-style/common/classic_shell_folder.h"
#include "lib-style/common/classic_shell_item.h"
#include <wchar.h>

typedef struct classic_call {OPENFILENAMEW *ofn;BOOL save;BOOL selected;DWORD error;} classic_call;
static HRESULT classic_show(void *data)
{
    classic_call *call=data;call->selected=call->save?GetSaveFileNameW(call->ofn):GetOpenFileNameW(call->ofn);
    call->error=call->selected?0:CommDlgExtendedError();return S_OK;
}
HRESULT style_classic_com_show(style_classic_com_show_state *p,HWND owner)
{
    FILEOPENDIALOGOPTIONS flags=0;
    if(*p->result){IShellItem_Release(*p->result);*p->result=NULL;}
    if(*p->results){IShellItemArray_Release(*p->results);*p->results=NULL;}*p->completed=FALSE;
    if(!p->session_active())return p->show_native(p->owner,owner);
    HRESULT hr=IFileDialog_GetOptions(p->native,&flags);
    if(!style_classic_com_can_bridge(hr,flags,*p->unsupported,p->has_events(p->owner),p->save,
        p->experimental_events,p->experimental_hidden))return p->show_native(p->owner,owner);
    if(p->experimental_hidden)*p->classic_hidden=(flags&FOS_FORCESHOWHIDDEN)!=0;
    WCHAR file[32768]=L"";PWSTR value=NULL;IShellItem *folder=NULL;PWSTR folder_path=NULL;
    if(SUCCEEDED(IFileDialog_GetFileName(p->native,&value)) && value){
        if(wcslen(value)>=ARRAYSIZE(file)){CoTaskMemFree(value);return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);}
        wcscpy(file,value);CoTaskMemFree(value);
    }
    if(SUCCEEDED(IFileDialog_GetFolder(p->native,&folder)) && folder){
        (void)IShellItem_GetDisplayName(folder,SIGDN_FILESYSPATH,&folder_path);IShellItem_Release(folder);
    }
    if(flags&FOS_PICKFOLDERS){
        if(p->save || !(flags&FOS_FORCEFILESYSTEM) || *p->ok_label || *p->cancel_label || *p->file_label || p->has_events(p->owner)){
            if(folder_path)CoTaskMemFree(folder_path);
            return p->show_native(p->owner,owner);
        }
        WCHAR selected[32768];style_classic_shell_folder_call call={.owner=owner,.title=*p->title,.initial=folder_path,
            .result=selected,.capacity=ARRAYSIZE(selected)};
        hr=p->scope(style_classic_shell_folder_show,&call);if(folder_path)CoTaskMemFree(folder_path);
        if(p->experimental_events && FAILED(hr) && *p->classic_close_pending)return *p->classic_close_result;
        if(FAILED(hr))return hr;
        if(p->experimental_events && p->has_events(p->owner) && *p->completed && *p->result)return S_OK;
        hr=SHCreateItemFromParsingName(selected,NULL,&IID_IShellItem,(void **)p->result);
        if(SUCCEEDED(hr))*p->completed=TRUE;
        return hr;
    }
    OPENFILENAMEW ofn={0};ofn.lStructSize=sizeof(ofn);ofn.hwndOwner=owner;
    ofn.lpstrFile=file;ofn.nMaxFile=ARRAYSIZE(file);ofn.lpstrInitialDir=folder_path;ofn.lpstrTitle=*p->title;
    ofn.lpstrFilter=*p->filter?*p->filter:L"All files\0*.*\0\0";ofn.nFilterIndex=*p->filter_index?*p->filter_index:1;ofn.lpstrDefExt=*p->extension;
    ofn.Flags=style_classic_com_file_flags(flags);ofn.lpfnHook=p->classic_hook;ofn.lCustData=(LPARAM)p->owner;
    classic_call call={&ofn,p->save,FALSE,0};
    hr=p->scope(classic_show,&call);if(folder_path)CoTaskMemFree(folder_path);
    if(FAILED(hr))return hr;
    if(!call.selected){
        if(p->experimental_events && *p->classic_close_pending)return *p->classic_close_result;
        return call.error?HRESULT_FROM_WIN32(ERROR_INVALID_DATA):HRESULT_FROM_WIN32(ERROR_CANCELLED);}
    if(*p->filter && ofn.nFilterIndex){*p->filter_index=ofn.nFilterIndex;(void)IFileDialog_SetFileTypeIndex(p->native,*p->filter_index);}
    if(flags&FOS_ALLOWMULTISELECT){
        if(p->experimental_events){
            if(*p->result){IShellItem_Release(*p->result);*p->result=NULL;}
            if(*p->results){IShellItemArray_Release(*p->results);*p->results=NULL;}
            *p->completed=FALSE;
        }
        return style_classic_multi_result(file,ofn.nFileOffset,(flags&FOS_NODEREFERENCELINKS)!=0,p->result,p->results,p->completed);
    }
    if(p->experimental_events && *p->result){IShellItem_Release(*p->result);*p->result=NULL;*p->completed=FALSE;}
    hr=p->save?SHCreateItemFromParsingName(file,NULL,&IID_IShellItem,(void **)p->result):style_classic_open_item(file,(flags&FOS_NODEREFERENCELINKS)!=0,p->result);
    if(p->save && hr==HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))hr=style_classic_save_item(file,p->result);
    if(SUCCEEDED(hr))*p->completed=TRUE;
    return hr;
}
