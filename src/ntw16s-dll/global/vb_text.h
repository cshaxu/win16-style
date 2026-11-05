#ifndef NTW16S_GLOBAL_VB_TEXT_H
#define NTW16S_GLOBAL_VB_TEXT_H
#include <windows.h>

/* Called only on the owning GUI thread after the style DLL is pinned. */
DWORD ntw16s_global_vb_text(HWND window,BOOL *changed);
#endif
