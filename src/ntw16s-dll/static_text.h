#ifndef NTW16S_STATIC_TEXT_H
#define NTW16S_STATIC_TEXT_H
#include <windows.h>

/* Private, same-thread borrow around one verified native STATIC call. */
typedef struct ntw16s_static_call {
    struct ntw16s_static_call *previous;
    HWND window;
    HMODULE native_module;
    const BOOL *cancelled;
    BOOL active;
} ntw16s_static_call;
BOOL ntw16s_static_call_begin(ntw16s_static_call *,WNDPROC,HWND,UINT,const void *);
BOOL ntw16s_static_call_suspend(ntw16s_static_call *);
void ntw16s_static_call_end(void *);
HFONT ntw16s_static_text_font(HDC,LPCWSTR,int,UINT,const void *);
#endif
