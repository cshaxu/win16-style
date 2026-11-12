#include <windows.h>
#include <stdio.h>
#include "lib-style/common/theme_state.h"
static WCHAR app_seen[64],ids_seen[64];
static unsigned calls;
static HRESULT observe(HWND w,LPCWSTR app,LPCWSTR ids)
{
    (void)w;++calls;lstrcpynW(app_seen,app?app:L"<null>",64);
    lstrcpynW(ids_seen,ids?ids:L"<null>",64);return S_OK;
}
int main(void)
{
    HWND w=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,40,40,NULL,NULL,GetModuleHandleW(NULL),NULL);
    if(!w)return 1;
    int owner=1,other=2;BOOL ok=style_theme_stage(w) &&
        SUCCEEDED(style_theme_set(w,L"First",L"Tree",observe)) &&
        style_theme_commit(w) && style_theme_suppress(w,&owner,observe) &&
        !lstrcmpW(app_seen,L"") && !lstrcmpW(ids_seen,L"") && style_theme_retained(w) &&
        SUCCEEDED(style_theme_set(w,L"Second",NULL,observe)) && !lstrcmpW(app_seen,L"");
    style_theme_remove(w,&other,observe);
    ok=ok && style_theme_retained(w);
    style_theme_remove(w,&owner,observe);
    ok=ok && !style_theme_retained(w) && !lstrcmpW(app_seen,L"Second") &&
        !lstrcmpW(ids_seen,L"<null>") && calls==4;
    ok=ok && style_theme_stage(w);style_theme_cancel_stage(w);
    ok=ok && !style_theme_retained(w) && style_theme_capture(w) && style_theme_terminal(w,NULL);
    DestroyWindow(w);
    if(!ok)return 2;
    puts("STYLE THEME PASS stage=1 suppress=1 latest_restore=1 ownership=1 cancel=1");return 0;
}
