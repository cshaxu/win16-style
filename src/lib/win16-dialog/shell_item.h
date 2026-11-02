#ifndef WIN16_DIALOG_SHELL_ITEM_H
#define WIN16_DIALOG_SHELL_ITEM_H
#include <windows.h>
#include <shobjidl.h>
#include <shlobj.h>
HRESULT win16_dialog_link_target_id(PCWSTR path,PIDLIST_ABSOLUTE *target);
HRESULT win16_dialog_save_item(PCWSTR path,IShellItem **item);
HRESULT win16_dialog_open_item(PCWSTR path,BOOL keep_link,IShellItem **item);
HRESULT win16_dialog_multi_result(WCHAR *file,WORD offset,BOOL keep_link,
    IShellItem **result,IShellItemArray **results,BOOL *completed);
#endif
