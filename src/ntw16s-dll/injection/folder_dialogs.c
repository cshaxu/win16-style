/* Select the older Shell tree for a limited filesystem browse request.
 * Shell owns the dialog and returned PIDL. Other contracts remain native. */
#include "intercept.h"
#include "../../lib-win32/process-hook/startup_hooks.h"
#include "../../lib-style/win16-dialog/chrome.h"
#include "../../lib-style/win16-dialog/browse.h"
#include "ntw16s-dll/controls.h"
#include <shlobj.h>
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

static PIDLIST_ABSOLUTE WINAPI browse_a(LPBROWSEINFOA info)
{return win16_dialog_browse_a(info,native_browse_a,ntw16s_process_session_active());}
static PIDLIST_ABSOLUTE WINAPI browse_w(LPBROWSEINFOW info)
{return win16_dialog_browse_w(info,native_browse_w,ntw16s_process_session_active());}
LONG ntw16s_attach_folder_dialogs_using(DWORD (*attach)(void *,void *))
{
    if(!attach)return ERROR_INVALID_PARAMETER;
    if(!(ntw16s_process_context.flags & NTWSHK_POLICY_STYLE))return NO_ERROR;
    native_browse_a=SHBrowseForFolderA;native_browse_w=SHBrowseForFolderW;
    LONG error=(LONG)attach(&native_browse_a,(void *)browse_a);
    if(!error)error=(LONG)attach(&native_browse_w,(void *)browse_w);
    return error;
}

LONG ntw16s_attach_folder_dialogs(void){return ntw16s_attach_folder_dialogs_using(process_hook_startup_attach);}
