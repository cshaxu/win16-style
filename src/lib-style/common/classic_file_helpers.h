#ifndef STYLE_CLASSIC_FILE_HELPERS_H
#define STYLE_CLASSIC_FILE_HELPERS_H
#include <windows.h>
#include <commdlg.h>
#include <shobjidl.h>
/* Helpers operate on the standard classic-dialog control IDs; returned Shell
 * items are owned by the caller. Folder buffers are supplied by the caller. */
IShellItem *style_classic_file_list_target(HWND window,IShellItem *folder);
BOOL style_classic_file_typed_target(HWND window,IShellItem *folder,IShellItem **out);
IShellItem *style_classic_file_drive_target(HWND window,LRESULT *selected,HWND *combo_out);
IShellItem *style_classic_file_overwrite_target(HWND window,IShellItem *folder,PCWSTR extension);
void style_classic_file_resolve_link(HWND window);
BOOL style_classic_file_folder(HWND window,PWSTR path,size_t capacity);
BOOL style_classic_file_folder_adjust(const OPENFILENAMEW *,WCHAR *,size_t,OPENFILENAMEW *);
#endif
