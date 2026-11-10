#define COBJMACROS
#include "proxy_dispatch.h"
#include "file_live.h"
#include <shlobj.h>

HRESULT win16_dialog_proxy_dispatch(win16_dialog_proxy *p,win16_dialog_v2_operation op,ULONG_PTR a,ULONG_PTR b,const win16_dialog_proxy_dispatch_services *services,BOOL experimental_events)
{
    switch(op){
    case V2_SHOW:return services->show(p,(HWND)a);
    case V2_TYPES:return win16_dialog_proxy_types(p,(UINT)a,(const COMDLG_FILTERSPEC *)b);
    case V2_SET_INDEX:return win16_dialog_live_set_filter(p->native,&p->filter_index,(UINT)a);
    case V2_GET_INDEX:return win16_dialog_live_get_filter(p->native,&p->active_window,&p->filter,(UINT *)a);
    case V2_ADVISE:return services->advise(p,(IFileDialogEvents *)a,(DWORD *)b);
    case V2_UNADVISE:return services->unadvise(p,(DWORD)a);
    case V2_SET_FOLDER:return services->set_folder(p,(IShellItem *)a);
    case V2_GET_FOLDER:return win16_dialog_proxy_get_folder(p,(IShellItem **)a);
    case V2_GET_SELECTION:
        if(experimental_events)return win16_dialog_proxy_get_selection(p,(IShellItem **)a);
        return IFileDialog2_GetCurrentSelection(p->v2_native,(IShellItem **)a);
    /* Keep the native configuration and the active classic edit synchronized. */
    case V2_SET_NAME:return win16_dialog_live_set_name(p->native,&p->active_window,(LPCWSTR)a);
    case V2_GET_NAME:return win16_dialog_live_get_name(p->native,&p->active_window,(LPWSTR *)a);
    case V2_TITLE:return win16_dialog_proxy_title(p,(LPCWSTR)a);
    case V2_LABEL:return win16_dialog_proxy_label(p,(LPCWSTR)a,(BOOL)b);
    case V2_RESULT:return win16_dialog_proxy_result(p,(IShellItem **)a);
    case V2_EXTENSION:return win16_dialog_proxy_extension(p,(LPCWSTR)a);
    case V2_PRIMARY_GET_SELECTION:
        if(experimental_events)return win16_dialog_proxy_get_selection(p,(IShellItem **)a);
        return p->save?IFileSaveDialog_GetCurrentSelection((IFileSaveDialog *)p->variant,(IShellItem **)a):
            IFileOpenDialog_GetCurrentSelection((IFileOpenDialog *)p->variant,(IShellItem **)a);
    case V2_PRIMARY_CLOSE:
        if(experimental_events)return win16_dialog_proxy_close(p,(HRESULT)a);
        p->unsupported=TRUE;return p->save?IFileSaveDialog_Close((IFileSaveDialog *)p->variant,(HRESULT)a):
            IFileOpenDialog_Close((IFileOpenDialog *)p->variant,(HRESULT)a);
    case V2_PRIMARY_GET_RESULTS: {
        IShellItemArray **out=(IShellItemArray **)a;
        if(p->completed && p->results){if(!out)return E_POINTER;*out=p->results;IShellItemArray_AddRef(*out);return S_OK;}
        if(p->completed && p->result)return SHCreateShellItemArrayFromShellItem(p->result,&IID_IShellItemArray,(void **)out);
        return IFileOpenDialog_GetResults((IFileOpenDialog *)p->variant,out);
    }
    case V2_CLOSE:
        if(experimental_events)return win16_dialog_proxy_close(p,(HRESULT)a);
        p->unsupported=TRUE;return IFileDialog2_Close(p->v2_native,(HRESULT)a);
    default:return E_INVALIDARG;
    }
}
