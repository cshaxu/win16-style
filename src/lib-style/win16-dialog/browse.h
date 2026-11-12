#ifndef WIN16_DIALOG_BROWSE_H
#define WIN16_DIALOG_BROWSE_H
#include <windows.h>
#include <shlobj.h>
typedef enum win16_dialog_browse_mode {
    WIN16_BROWSE_NATIVE,WIN16_BROWSE_REWRITE,WIN16_BROWSE_VALIDATE
} win16_dialog_browse_mode;
typedef struct win16_dialog_browse_plan {win16_dialog_browse_mode mode;UINT flags;} win16_dialog_browse_plan;
typedef struct win16_dialog_folder_selection {PIDLIST_ABSOLUTE selected;BOOL invalid;} win16_dialog_folder_selection;
win16_dialog_browse_plan win16_dialog_plan_browse(UINT flags,BOOL has_callback);
int CALLBACK win16_dialog_folder_selection_callback(HWND,UINT,LPARAM,LPARAM);
typedef PIDLIST_ABSOLUTE (WINAPI *win16_dialog_browse_native_a)(LPBROWSEINFOA);
typedef PIDLIST_ABSOLUTE (WINAPI *win16_dialog_browse_native_w)(LPBROWSEINFOW);
PIDLIST_ABSOLUTE win16_dialog_browse_a(LPBROWSEINFOA,win16_dialog_browse_native_a,BOOL);
PIDLIST_ABSOLUTE win16_dialog_browse_w(LPBROWSEINFOW,win16_dialog_browse_native_w,BOOL);
#endif
