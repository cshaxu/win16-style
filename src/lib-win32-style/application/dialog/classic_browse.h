#ifndef STYLE_CLASSIC_BROWSE_H
#define STYLE_CLASSIC_BROWSE_H
#include <windows.h>
#include <shlobj.h>
/* Compatibility plan for filesystem Shell browsing; callback-bearing requests
 * retain their native contract. */
typedef enum style_classic_browse_mode {
    STYLE_CLASSIC_BROWSE_NATIVE,STYLE_CLASSIC_BROWSE_REWRITE,STYLE_CLASSIC_BROWSE_VALIDATE
} style_classic_browse_mode;
typedef struct style_classic_browse_plan {style_classic_browse_mode mode;UINT flags;} style_classic_browse_plan;
typedef struct style_classic_folder_selection {PIDLIST_ABSOLUTE selected;BOOL invalid;} style_classic_folder_selection;
style_classic_browse_plan style_classic_plan_browse(UINT flags,BOOL has_callback);
int CALLBACK style_classic_folder_selection_callback(HWND,UINT,LPARAM,LPARAM);
typedef PIDLIST_ABSOLUTE (WINAPI *style_classic_browse_native_a)(LPBROWSEINFOA);
typedef PIDLIST_ABSOLUTE (WINAPI *style_classic_browse_native_w)(LPBROWSEINFOW);
PIDLIST_ABSOLUTE style_classic_browse_a(LPBROWSEINFOA,style_classic_browse_native_a,BOOL);
PIDLIST_ABSOLUTE style_classic_browse_w(LPBROWSEINFOW,style_classic_browse_native_w,BOOL);
#endif
