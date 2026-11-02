/* Adapt filesystem folder requests to the system Shell tree. */
#include "shell_folder.h"
#include <shlobj.h>
#include <wchar.h>

static int CALLBACK shell_folder_initial(HWND window,UINT message,LPARAM parameter,LPARAM data)
{win16_dialog_shell_folder_call *call=(win16_dialog_shell_folder_call *)data;(void)parameter;
 if(message==BFFM_INITIALIZED && call){
    if(call->title)SetWindowTextW(window,call->title);
    if(call->initial && *call->initial)SendMessageW(window,BFFM_SETSELECTIONW,TRUE,(LPARAM)call->initial);
 }
 return 0;}
HRESULT win16_dialog_shell_folder_show(void *data)
{
    win16_dialog_shell_folder_call *call=data;BROWSEINFOW info={0};WCHAR path[MAX_PATH];
    info.hwndOwner=call->owner;info.lpszTitle=call->title;info.ulFlags=BIF_RETURNONLYFSDIRS;
    info.lpfn=shell_folder_initial;info.lParam=(LPARAM)call;
    PIDLIST_ABSOLUTE selected=SHBrowseForFolderW(&info);
    if(!selected)return HRESULT_FROM_WIN32(ERROR_CANCELLED);
    BOOL ok=SHGetPathFromIDListW(selected,path);CoTaskMemFree(selected);
    if(!ok)return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    if(wcslen(path)>=call->capacity)return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
    wcscpy(call->result,path);return S_OK;
}
