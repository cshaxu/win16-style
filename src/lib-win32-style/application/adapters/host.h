#ifndef STYLE_UWP_HOST_H
#define STYLE_UWP_HOST_H
#include <windows.h>
#include <commctrl.h>
#include "../lifecycle/instance.h"

/* Initialize instance with one retained call and a dispose operation; every synchronous entry that
 * can reenter or detach must retain until it returns. Detach requires a held
 * reference and a successfully installed owner subclass. It leaves the host
 * attached on owner-subclass removal failure; repeated detach is harmless. */
typedef struct style_uwp_host {
    style_instance instance;
    HWND owner,title,surface;
    BOOL (*surface_message)(void *,HWND,UINT,WPARAM,LPARAM,LRESULT *);
    BOOL (*owner_message)(void *,HWND,UINT,WPARAM,LPARAM,LRESULT *);
    void (*owner_activation)(void *,UINT,WPARAM);
    void (*before_title_remove)(void *);
    void (*after_title_remove)(void *);
    BOOL (*refresh)(void *);
} style_uwp_host;

BOOL style_uwp_host_detach(style_uwp_host *host);
LRESULT CALLBACK style_uwp_host_title_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
LRESULT CALLBACK style_uwp_host_owner_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
LRESULT CALLBACK style_uwp_host_surface_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
/* Caller holds one reference. Installs owner, title and surface; failure to
 * install the owner marks detached without calling detach callbacks. Later
 * failures detach installed windows. refresh may synchronously detach. */
BOOL style_uwp_host_start(style_uwp_host *host,BOOL (*refresh)(void *));
#endif
