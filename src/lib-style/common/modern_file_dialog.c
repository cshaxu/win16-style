/* Modern dialog behavior, called by an owning module that supplies its scope. */
#define COBJMACROS
#define INITGUID
#include "lib-style/common/modern_file_dialog.h"
#include <shlobj.h>
#include <shobjidl.h>
#include <wchar.h>
#include <string.h>

typedef struct modern_show {IFileDialog *dialog;HWND owner;} modern_show;
static _Thread_local unsigned modern_depth;
BOOL style_dialog_modern_active(void){return modern_depth!=0;}
static HRESULT show_scoped(void *value)
{ modern_show *show=value;return IFileDialog_Show(show->dialog,show->owner); }

HRESULT style_dialog_show_modern(HWND owner, UINT kind, PWSTR path, DWORD capacity,
    style_dialog_scope scope)
{
    if (!path || !capacity || kind < 1 || kind > 3) return E_INVALIDARG;
    path[0] = L'\0';
    HRESULT init = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(init)) return init;
    IFileDialog *dialog = NULL;
    ++modern_depth;
    HRESULT hr = CoCreateInstance(kind == 2 ? &CLSID_FileSaveDialog : &CLSID_FileOpenDialog,
        NULL, CLSCTX_INPROC_SERVER, &IID_IFileDialog, (void **)&dialog);
    if (SUCCEEDED(hr) && kind == 3) {
        FILEOPENDIALOGOPTIONS options = 0;
        hr = IFileDialog_GetOptions(dialog, &options);
        if (SUCCEEDED(hr)) hr = IFileDialog_SetOptions(dialog, options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
    }
    if (SUCCEEDED(hr)) {modern_show show={dialog,owner};hr=scope(show_scoped,&show);}
    if (SUCCEEDED(hr)) {
        IShellItem *item = NULL;
        PWSTR selected = NULL;
        hr = IFileDialog_GetResult(dialog, &item);
        if (SUCCEEDED(hr)) hr = IShellItem_GetDisplayName(item, SIGDN_FILESYSPATH, &selected);
        if (SUCCEEDED(hr)) {
            size_t length = wcslen(selected);
            if (length >= capacity) hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            else memcpy(path, selected, (length + 1) * sizeof(*path));
        }
        if (selected) CoTaskMemFree(selected);
        if (item) IShellItem_Release(item);
    }
    if (dialog) IFileDialog_Release(dialog);
    --modern_depth;
    CoUninitialize();
    return hr;
}
