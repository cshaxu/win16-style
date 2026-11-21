#include "host.h"

BOOL style_uwp_host_detach(style_uwp_host *host)
{
    if(host->instance.detached)return TRUE;
    if(!style_instance_detach(&host->instance))return FALSE;
    if(host->before_title_remove)host->before_title_remove(host->instance.context);
    if(host->title)RemoveWindowSubclass(host->title,style_uwp_host_title_proc,(UINT_PTR)style_uwp_host_title_proc);
    if(host->after_title_remove)host->after_title_remove(host->instance.context);
    HWND surface=host->surface;host->surface=NULL;
    if(surface){
        if(GetCapture()==surface)ReleaseCapture();
        RemoveWindowSubclass(surface,style_uwp_host_surface_proc,(UINT_PTR)style_uwp_host_surface_proc);
        DestroyWindow(surface);
    }
    return TRUE;
}

LRESULT CALLBACK style_uwp_host_surface_proc(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{
    style_uwp_host *host=(style_uwp_host *)data;(void)id;
    style_instance_retain(&host->instance);
    LRESULT result=0;
    BOOL handled=!host->instance.detached && host->surface_message &&
        host->surface_message(host->instance.context,window,message,wp,lp,&result);
    if(!handled){
        if(!host->instance.detached && message==WM_NCDESTROY){host->surface=NULL;style_uwp_host_detach(host);}
        result=DefSubclassProc(window,message,wp,lp);
    }
    style_instance_release(&host->instance);return result;
}

LRESULT CALLBACK style_uwp_host_owner_proc(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{
    style_uwp_host *host=(style_uwp_host *)data;(void)id;
    style_instance_retain(&host->instance);
    if(message==WM_NCDESTROY)style_uwp_host_detach(host);
    LRESULT result=0;
    BOOL handled=!host->instance.detached && host->owner_message && host->owner_message(host->instance.context,window,message,wp,lp,&result);
    if(!handled)result=DefSubclassProc(window,message,wp,lp);
    if(!host->instance.detached){
        switch(message){
        case WM_NCACTIVATE:case WM_ACTIVATE:
            if(host->owner_activation)host->owner_activation(host->instance.context,message,wp);
            if(!host->instance.detached)host->refresh(host->instance.context);
            break;
        case WM_WINDOWPOSCHANGED:case WM_SIZE:case WM_SHOWWINDOW:
        case WM_DPICHANGED:case WM_SETTEXT:case WM_SETICON:case WM_THEMECHANGED:
        case WM_SETTINGCHANGE:case WM_SYSCOLORCHANGE:
            host->refresh(host->instance.context);break;
        default:break;
        }
    }
    style_instance_release(&host->instance);return result;
}

LRESULT CALLBACK style_uwp_host_title_proc(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{
    style_uwp_host *host=(style_uwp_host *)data;(void)id;
    style_instance_retain(&host->instance);
    if(message==WM_NCDESTROY)style_uwp_host_detach(host);
    LRESULT result=DefSubclassProc(window,message,wp,lp);
    if(!host->instance.detached && (message==WM_WINDOWPOSCHANGED || message==WM_SHOWWINDOW))host->refresh(host->instance.context);
    style_instance_release(&host->instance);return result;
}

BOOL style_uwp_host_start(style_uwp_host *host,BOOL (*refresh)(void *))
{
    host->refresh=refresh;
    if(!style_instance_attach(&host->instance,host->owner,style_uwp_host_owner_proc,NULL)){
        host->instance.detached=TRUE;return FALSE;
    }
    if(host->title && !SetWindowSubclass(host->title,style_uwp_host_title_proc,(UINT_PTR)style_uwp_host_title_proc,(DWORD_PTR)host)){
        DWORD error=GetLastError();style_uwp_host_detach(host);SetLastError(error);return FALSE;
    }
    host->surface=CreateWindowExW(WS_EX_LAYERED|WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,L"STATIC",L"",
        WS_POPUP|SS_NOTIFY,0,0,1,1,host->owner,NULL,GetModuleHandleW(NULL),NULL);
    if(host->instance.detached && host->surface){DestroyWindow(host->surface);host->surface=NULL;}
    BOOL ok=!host->instance.detached && host->surface &&
        SetWindowSubclass(host->surface,style_uwp_host_surface_proc,(UINT_PTR)style_uwp_host_surface_proc,(DWORD_PTR)host) && refresh(host->instance.context);
    DWORD error=ok?0:GetLastError();
    if(!ok){style_uwp_host_detach(host);if(!error)error=ERROR_NOT_SUPPORTED;}
    SetLastError(error);return ok;
}
