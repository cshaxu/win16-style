#ifndef STYLE_CLASSIC_COM_HOOK_H
#define STYLE_CLASSIC_COM_HOOK_H
#include <windows.h>
#include <commdlg.h>
typedef struct style_classic_com_hook_services {
    PCWSTR property,navigation_message;
    void (*attach)(void *,HWND,OPENFILENAMEW *);
    PCWSTR (*label)(void *,int);
    void (*detach)(void *,HWND);
    BOOL (*command)(void *,HWND,WPARAM,UINT_PTR *);
    BOOL (*before_registered)(void *,HWND,UINT,WPARAM,UINT_PTR *);
    void (*selection)(void *,HWND,WPARAM);
    void (*navigate)(void *,HWND,PCWSTR);
    BOOL (*after_registered)(void *,HWND,UINT,LPARAM,UINT_PTR *);
} style_classic_com_hook_services;
UINT_PTR style_classic_com_hook(HWND,UINT,WPARAM,LPARAM,const style_classic_com_hook_services *);
#endif
