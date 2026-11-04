/* Select the older Shell tree for a limited filesystem browse request.
 * Shell owns the dialog and returned PIDL. Other contracts remain native. */
#include "intercept.h"
#include "startup_hooks.h"
#include "unwind.h"
#include "../../lib-win32/win16-dialog/folder_create.h"
#include "../../lib-win32/win16-dialog/chrome.h"
#include "../../lib-win32/win16-dialog/browse.h"
#include "../../lib-win32/win16-dialog/classic_folder.h"
#include "ntw16s-dll/controls.h"
#include <dlgs.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shlobj.h>
#include <wchar.h>
#include <stdio.h>

#define BRIDGE_INIT_TIMER 0x5715u
#define BRIDGE_TIMER 0x5716u

typedef win16_dialog_classic_folder_context folder_context;
static _Thread_local folder_context *active;
static PIDLIST_ABSOLUTE (WINAPI *native_browse_a)(LPBROWSEINFOA);
static PIDLIST_ABSOLUTE (WINAPI *native_browse_w)(LPBROWSEINFOW);

static BOOL dialog_grid_install(HWND window,const void *owner)
{return win16_style_grid_install_created(window,NULL,owner,NULL);}
static const win16_dialog_chrome_services dialog_chrome_services={
    dialog_grid_install,win16_style_list_scrollbar_enable,
    win16_style_control_remove_owned,win16_style_common_dialog_combo_install
};
void ntw16s_old_dialog_chrome(HWND window)
{win16_dialog_old_chrome(window,&dialog_chrome_services);}
void ntw16s_font_dialog_chrome(HWND window)
{win16_dialog_font_chrome(window,&dialog_chrome_services);}

static UINT_PTR CALLBACK old_folder_hook(HWND window,UINT message,WPARAM wparam,LPARAM lparam)
{return win16_dialog_classic_folder_hook(window,message,wparam,lparam,active,ntw16s_old_dialog_chrome);}
/* COM folder requests need a Shell item, not a fabricated PIDL. Reuse the
 * classic system template and return its selected directory to the caller. */
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
static void classic_folder_active_cleanup(void *value)
{
    folder_context *ctx=value;HWND window=ctx->classic_window;
    if(window && IsWindow(window))DestroyWindow(window);
    ctx->classic_window=NULL;active=NULL;
}
NTW16S_UNWIND_FUNCTION HRESULT ntw16s_classic_folder(HWND owner,PCWSTR title,PCWSTR initial,PCWSTR ok_label,PCWSTR cancel_label,PCWSTR file_label,PWSTR result,DWORD capacity,ntw16s_folder_observer observer,void *context)
#else
HRESULT ntw16s_classic_folder(HWND owner,PCWSTR title,PCWSTR initial,PCWSTR ok_label,PCWSTR cancel_label,PCWSTR file_label,PWSTR result,DWORD capacity)
#endif
{
    if(!result || !capacity)return E_INVALIDARG;
    result[0]=0;if(active)return HRESULT_FROM_WIN32(ERROR_BUSY);
    folder_context ctx={0};
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    ctx.experimental=TRUE;ctx.observer=observer;ctx.observer_context=context;
#endif
    WCHAR file[MAX_PATH];OPENFILENAMEW ofn;
    HRESULT prepared=win16_dialog_classic_folder_prepare(&ctx,&ofn,file,owner,title,initial,ok_label,cancel_label,file_label,old_folder_hook);
    if(FAILED(prepared))return prepared;
    active=&ctx;
#ifdef NTW16S_EXPERIMENTAL_CLASSIC_EVENTS
    ntw16s_unwind_scope unwind;BOOL chosen;DWORD error;
    NTW16S_UNWIND_BEGIN(unwind,classic_folder_active_cleanup,&ctx);
    chosen=GetOpenFileNameW(&ofn);error=chosen?0:CommDlgExtendedError();
    NTW16S_UNWIND_END(unwind);
#else
    BOOL chosen=GetOpenFileNameW(&ofn);DWORD error=chosen?0:CommDlgExtendedError();active=NULL;
#endif
    return win16_dialog_classic_folder_finish(chosen,error,file,result,capacity);
}
static PIDLIST_ABSOLUTE WINAPI browse_a(LPBROWSEINFOA info)
{return win16_dialog_browse_a(info,native_browse_a,ntw16s_process_session_active() && !active);}
static PIDLIST_ABSOLUTE WINAPI browse_w(LPBROWSEINFOW info)
{return win16_dialog_browse_w(info,native_browse_w,ntw16s_process_session_active() && !active);}
LONG ntw16s_attach_folder_dialogs_using(DWORD (*attach)(void *,void *))
{
    if(!attach)return ERROR_INVALID_PARAMETER;
    if(!(ntw16s_process_context.flags & NTW16S_POLICY_STYLE))return NO_ERROR;
    native_browse_a=SHBrowseForFolderA;native_browse_w=SHBrowseForFolderW;
    LONG error=(LONG)attach(&native_browse_a,(void *)browse_a);
    if(!error)error=(LONG)attach(&native_browse_w,(void *)browse_w);
    return error;
}

LONG ntw16s_attach_folder_dialogs(void){return ntw16s_attach_folder_dialogs_using(ntw16s_hooks_attach);}
