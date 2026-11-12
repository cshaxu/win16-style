#ifndef WIN16_CONTROL_VCL_NATIVE_PAINT_H
#define WIN16_CONTROL_VCL_NATIVE_PAINT_H
#include <windows.h>
typedef struct win16_control_vcl_paint_view {
    HWND window;
    const BOOL *candidate,*detached;
    BOOL *verified;
    const HMODULE *owner;
    const HWND *parent;
    BOOL (*identity)(HWND,HMODULE,HWND),(*lookup)(HWND,HMODULE,HWND);
    BOOL (*class_is)(HWND,const WCHAR *);
} win16_control_vcl_paint_view;
void win16_control_vcl_inplace_draw(const win16_control_vcl_paint_view *,HDC,int,int,const RECT *,const RECT *,const void *,BOOL);
void win16_control_vcl_nav_draw(const win16_control_vcl_paint_view *,HDC,int,int,const RECT *,const RECT *,const void *);
#endif
