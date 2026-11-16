#ifndef STYLE_CLASSIC_SHELL_ITEM_H
#define STYLE_CLASSIC_SHELL_ITEM_H
#include <windows.h>
#include <shobjidl.h>
#include <shlobj.h>
HRESULT style_classic_link_target_id(PCWSTR path,PIDLIST_ABSOLUTE *target);
HRESULT style_classic_save_item(PCWSTR path,IShellItem **item);
HRESULT style_classic_open_item(PCWSTR path,BOOL keep_link,IShellItem **item);
HRESULT style_classic_multi_result(WCHAR *file,WORD offset,BOOL keep_link,
    IShellItem **result,IShellItemArray **results,BOOL *completed);
#endif
