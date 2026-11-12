#ifndef STYLE_CLASSIC_DIALOG_PROXY_H
#define STYLE_CLASSIC_DIALOG_PROXY_H
#include <windows.h>
#include <shobjidl.h>
#include <commdlg.h>
#include "lib-style/common/dialog_primary.h"
#include "lib-style/common/dialog_ole_window.h"
#include "lib-style/common/dialog_customize.h"
#include "lib-style/common/file_events.h"
#include "lib-style/common/dialog_v2.h"
typedef struct style_classic_dialog_proxy {
    style_dialog_primary primary;
    LONG refs;
    BOOL save, unsupported, completed;
    BOOL classic_hidden;
    IFileDialog *native;
    IUnknown *variant;
    style_dialog_ole_window ole;
    IOleWindow *ole_native;
    HWND active_window;
    style_dialog_v2 v2;
    IFileDialog2 *v2_native;
    style_dialog_customize custom;
    IFileDialogCustomize *custom_native;
    IShellItem *result;
    IShellItem *live_folder;
    IShellItem *live_selection;
    IShellItem *initial_default;
    BOOL classic_navigating;
    BOOL classic_initializing;
    LRESULT classic_drive_index;
    BOOL classic_close_pending;
    BOOL classic_show_aborted;
    HRESULT classic_close_result;
    OPENFILENAMEW *classic_ofn;
    BOOL classic_overwrite_suppressed;
    HRESULT live_folder_error;
    IShellItemArray *results;
    WCHAR *title,*filter,*extension,*ok_label,*cancel_label,*file_label;
    WCHAR *pending_folder;
    UINT filter_index;
    struct style_dialog_event_adapter *events;
    style_dialog_v2_call dispatch;
} style_classic_dialog_proxy;
style_classic_dialog_proxy *style_classic_dialog_proxy_new(IUnknown *,BOOL,style_dialog_v2_call,BOOL);
ULONG style_classic_dialog_proxy_add(style_classic_dialog_proxy *);
ULONG style_classic_dialog_proxy_release(style_classic_dialog_proxy *);
HRESULT style_classic_dialog_proxy_query(style_classic_dialog_proxy *,REFIID,void **);
HRESULT style_classic_dialog_proxy_result(style_classic_dialog_proxy *,IShellItem **);
HRESULT style_classic_dialog_proxy_get_folder(style_classic_dialog_proxy *,IShellItem **);
HRESULT style_classic_dialog_proxy_get_selection(style_classic_dialog_proxy *,IShellItem **);
HRESULT style_classic_dialog_proxy_close(style_classic_dialog_proxy *,HRESULT);
HRESULT style_classic_dialog_proxy_title(style_classic_dialog_proxy *,LPCWSTR);
HRESULT style_classic_dialog_proxy_label(style_classic_dialog_proxy *,LPCWSTR,BOOL);
HRESULT style_classic_dialog_proxy_types(style_classic_dialog_proxy *,UINT,const COMDLG_FILTERSPEC *);
HRESULT style_classic_dialog_proxy_extension(style_classic_dialog_proxy *,LPCWSTR);
void style_classic_dialog_proxy_capture_folder(style_classic_dialog_proxy *,BOOL);
void style_classic_dialog_proxy_capture_selection(HWND,style_classic_dialog_proxy *);
HRESULT style_classic_dialog_proxy_set_folder(style_classic_dialog_proxy *,IShellItem *,PCWSTR);
void style_classic_dialog_proxy_navigate_contents(HWND,style_classic_dialog_proxy *,PCWSTR,PCWSTR,WCHAR **,BOOL);
#endif
