#ifndef WIN16_DIALOG_PROXY_H
#define WIN16_DIALOG_PROXY_H
#include <windows.h>
#include <shobjidl.h>
#include <commdlg.h>
#include "primary.h"
#include "ole_window.h"
#include "customize.h"
#include "lib-style/common/file_events.h"
#include "v2.h"
typedef struct win16_dialog_proxy {
    win16_dialog_primary primary;
    LONG refs;
    BOOL save, unsupported, completed;
    BOOL classic_hidden;
    IFileDialog *native;
    IUnknown *variant;
    win16_dialog_ole_window ole;
    IOleWindow *ole_native;
    HWND active_window;
    win16_dialog_v2 v2;
    IFileDialog2 *v2_native;
    win16_dialog_customize custom;
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
    win16_dialog_v2_call dispatch;
} win16_dialog_proxy;
win16_dialog_proxy *win16_dialog_proxy_new(IUnknown *,BOOL,win16_dialog_v2_call,BOOL);
ULONG win16_dialog_proxy_add(win16_dialog_proxy *);
ULONG win16_dialog_proxy_release(win16_dialog_proxy *);
HRESULT win16_dialog_proxy_query(win16_dialog_proxy *,REFIID,void **);
HRESULT win16_dialog_proxy_result(win16_dialog_proxy *,IShellItem **);
HRESULT win16_dialog_proxy_get_folder(win16_dialog_proxy *,IShellItem **);
HRESULT win16_dialog_proxy_get_selection(win16_dialog_proxy *,IShellItem **);
HRESULT win16_dialog_proxy_close(win16_dialog_proxy *,HRESULT);
HRESULT win16_dialog_proxy_title(win16_dialog_proxy *,LPCWSTR);
HRESULT win16_dialog_proxy_label(win16_dialog_proxy *,LPCWSTR,BOOL);
HRESULT win16_dialog_proxy_types(win16_dialog_proxy *,UINT,const COMDLG_FILTERSPEC *);
HRESULT win16_dialog_proxy_extension(win16_dialog_proxy *,LPCWSTR);
void win16_dialog_proxy_capture_folder(win16_dialog_proxy *,BOOL);
void win16_dialog_proxy_capture_selection(HWND,win16_dialog_proxy *);
HRESULT win16_dialog_proxy_set_folder(win16_dialog_proxy *,IShellItem *,PCWSTR);
void win16_dialog_proxy_navigate_contents(HWND,win16_dialog_proxy *,PCWSTR,PCWSTR,WCHAR **,BOOL);
#endif
