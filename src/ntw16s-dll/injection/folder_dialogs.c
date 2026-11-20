/* Select the older Shell tree for a limited filesystem browse request.
 * Shell owns the dialog and returned PIDL. Other contracts remain native. */
#include "intercept.h"
#include "lib-win32-style/application/dialog/classic_browse.h"
#include "lib-win32-style/application/dialog/classic_dialog_controls.h"
#include "ntw16s-dll/controls.h"
#include <shlobj.h>
static PIDLIST_ABSOLUTE (WINAPI *native_browse_a)(LPBROWSEINFOA);
static PIDLIST_ABSOLUTE (WINAPI *native_browse_w)(LPBROWSEINFOW);

static BOOL dialog_grid_install(HWND window,const void *owner)
{return win16_style_grid_install_created(window,NULL,owner,NULL);}
static const style_classic_dialog_controls dialog_chrome_services={
    dialog_grid_install,win16_style_list_scrollbar_enable,
    win16_style_control_remove_owned,win16_style_common_dialog_combo_install
};
void ntw16s_old_dialog_chrome(HWND window)
{static int owner;style_classic_file_controls(window,&owner,&dialog_chrome_services);}
void ntw16s_font_dialog_chrome(HWND window)
{static int owner;style_classic_font_controls(window,&owner,&dialog_chrome_services);}

static PIDLIST_ABSOLUTE WINAPI browse_a(LPBROWSEINFOA info)
{return style_classic_browse_a(info,native_browse_a,ntw16s_dialog_policy().folder==CLASSIC_FOLDER_LEGACY_TREE);}
static PIDLIST_ABSOLUTE WINAPI browse_w(LPBROWSEINFOW info)
{return style_classic_browse_w(info,native_browse_w,ntw16s_dialog_policy().folder==CLASSIC_FOLDER_LEGACY_TREE);}
static LONG attach_dialogs(DWORD (*attach)(void *,void *))
{
    if(!attach)return ERROR_INVALID_PARAMETER;
    native_browse_a=SHBrowseForFolderA;native_browse_w=SHBrowseForFolderW;
    LONG error=(LONG)attach(&native_browse_a,(void *)browse_a);
    if(!error)error=(LONG)attach(&native_browse_w,(void *)browse_w);
    return error;
}

#ifdef NTWSHK_UNIFIED
static BOOL win2k_startup_hooks;
LONG ntw16s_attach_win2k_folder_dialogs_using(DWORD (*attach)(void *,void *))
{
    LONG error=attach_dialogs(attach);
    if(!error)win2k_startup_hooks=TRUE;
    return error;
}
#endif
LONG ntw16s_attach_folder_dialogs_using(DWORD (*attach)(void *,void *))
{
    if(!attach)return ERROR_INVALID_PARAMETER;
#ifdef NTWSHK_UNIFIED
    if(win2k_startup_hooks)return NO_ERROR;
#endif
    return (ntw16s_process_context.flags&NTWSHK_POLICY_STYLE)?attach_dialogs(attach):NO_ERROR;
}
