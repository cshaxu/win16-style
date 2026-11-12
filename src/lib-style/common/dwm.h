#ifndef STYLE_COMMON_DWM_H
#define STYLE_COMMON_DWM_H
#include <windows.h>
#include <dwmapi.h>

typedef HRESULT (WINAPI *style_dwm_get)(HWND,DWORD,PVOID,DWORD);
typedef HRESULT (WINAPI *style_dwm_set)(HWND,DWORD,LPCVOID,DWORD);

/* DWM exposes the effective state, not the original policy value. */
static inline BOOL style_dwm_capture(HWND window,style_dwm_get get,DWORD *policy)
{
    BOOL enabled=FALSE;
    if(FAILED(get(window,DWMWA_NCRENDERING_ENABLED,&enabled,sizeof(enabled))))return FALSE;
    *policy=enabled?DWMNCRP_ENABLED:DWMNCRP_DISABLED;
    return TRUE;
}
static inline HRESULT style_dwm_policy(HWND window,style_dwm_set set,DWORD policy)
{return set(window,DWMWA_NCRENDERING_POLICY,&policy,sizeof(policy));}
#endif
