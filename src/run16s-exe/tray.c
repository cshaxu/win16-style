#include "tray.h"
#include <shellapi.h>
#include <stdlib.h>

#define TRAY_MESSAGE (WM_APP+53)
#define MENU_ENABLE 1001
#define MENU_EXIT 1002
struct run16s_tray {HWND window;HICON icon;run16s_tray_action action;void *context;BOOL enabled,busy,added;UINT restart;};
static BOOL notify(run16s_tray *tray,DWORD command)
{
#ifdef RUN16S_TRAY_TEST
    (void)tray;(void)command;return TRUE;
#else
    NOTIFYICONDATAW data={0};data.cbSize=sizeof(data);data.hWnd=tray->window;data.uID=1;
    data.uFlags=NIF_ICON|NIF_MESSAGE|NIF_TIP;data.uCallbackMessage=TRAY_MESSAGE;data.hIcon=tray->icon;
    lstrcpynW(data.szTip,L"Win16 Style",ARRAYSIZE(data.szTip));
    SetLastError(ERROR_SUCCESS);
    return Shell_NotifyIconW(command,&data);
#endif
}
static HMENU make_menu(const run16s_tray *tray)
{
    HMENU menu=CreatePopupMenu();if(!menu)return NULL;
    UINT flags=MF_STRING|(tray->enabled?MF_CHECKED:MF_UNCHECKED)|(tray->busy?MF_GRAYED:0);
    if(!AppendMenuW(menu,flags,MENU_ENABLE,L"Enable Win16 Style") || !AppendMenuW(menu,MF_STRING,MENU_EXIT,L"Exit")){
        DestroyMenu(menu);return NULL;
    }
    return menu;
}
static void show_menu(run16s_tray *tray)
{
    HMENU menu=make_menu(tray);if(!menu)return;POINT point;
    if(GetCursorPos(&point)){SetForegroundWindow(tray->window);
        UINT selected=TrackPopupMenu(menu,TPM_RETURNCMD|TPM_NONOTIFY|TPM_RIGHTBUTTON,point.x,point.y,0,tray->window,NULL);
        PostMessageW(tray->window,WM_NULL,0,0);
        if(selected==MENU_ENABLE)tray->action(RUN16S_TRAY_TOGGLE,tray->context);
        else if(selected==MENU_EXIT)tray->action(RUN16S_TRAY_EXIT,tray->context);
    }
    DestroyMenu(menu);
}
static LRESULT CALLBACK window_proc(HWND window,UINT message,WPARAM a,LPARAM b)
{
    run16s_tray *tray=(run16s_tray *)(void *)GetWindowLongPtrW(window,GWLP_USERDATA);
    if(message==WM_NCCREATE){CREATESTRUCTW *create=(CREATESTRUCTW *)(void *)b;tray=(run16s_tray *)create->lpCreateParams;
        SetWindowLongPtrW(window,GWLP_USERDATA,(LONG_PTR)(void *)tray);tray->window=window;}
    if(tray && tray->restart && message==tray->restart){tray->added=notify(tray,NIM_ADD);return 0;}
    if(tray && message==TRAY_MESSAGE && a==1 && (b==WM_LBUTTONUP || b==WM_RBUTTONUP)){show_menu(tray);return 0;}
#ifdef RUN16S_TRAY_TEST
    if(tray && message==WM_APP+54){if(a==1 && !tray->busy)tray->action(RUN16S_TRAY_TOGGLE,tray->context);
        else if(a==2)tray->action(RUN16S_TRAY_EXIT,tray->context);
        else if(a==3)return (tray->enabled?1:0)|(tray->busy?2:0);
        else if(a==4){WCHAR first[64]={0},second[64]={0};HMENU menu=make_menu(tray);if(!menu)return 0;
            BOOL valid=GetMenuItemCount(menu)==2 && GetMenuStringW(menu,0,first,ARRAYSIZE(first),MF_BYPOSITION) &&
                GetMenuStringW(menu,1,second,ARRAYSIZE(second),MF_BYPOSITION) && !lstrcmpW(first,L"Enable Win16 Style") &&
                !lstrcmpW(second,L"Exit") && !!(GetMenuState(menu,MENU_ENABLE,MF_BYCOMMAND)&MF_CHECKED)==!!tray->enabled;
            DestroyMenu(menu);return valid;}
        else if(a==5){ICONINFO info={0};BITMAP bitmap={0};if(!GetIconInfo(tray->icon,&info))return 0;
            BOOL valid=info.hbmColor && GetObjectW(info.hbmColor,sizeof(bitmap),&bitmap)==sizeof(bitmap) &&
                bitmap.bmWidth==32 && bitmap.bmHeight==32;
            if(info.hbmColor)DeleteObject(info.hbmColor);
            if(info.hbmMask)DeleteObject(info.hbmMask);
            return valid;}
        return 0;}
#endif
    if(message==WM_CLOSE)return 0;
    return DefWindowProcW(window,message,a,b);
}
DWORD run16s_tray_create(HICON icon,run16s_tray_action action,void *context,run16s_tray **result)
{
    if(result)*result=NULL;
    if(!icon||!action||!result)return ERROR_INVALID_PARAMETER;
    run16s_tray *tray=(run16s_tray *)calloc(1,sizeof(*tray));if(!tray)return ERROR_NOT_ENOUGH_MEMORY;
    tray->icon=icon;tray->action=action;tray->context=context;tray->restart=RegisterWindowMessageW(L"TaskbarCreated");
    HINSTANCE instance=GetModuleHandleW(NULL);WNDCLASSW wc={0};wc.hInstance=instance;wc.lpfnWndProc=window_proc;wc.lpszClassName=L"Run16sTrayWindow";
    if(!RegisterClassW(&wc) && GetLastError()!=ERROR_CLASS_ALREADY_EXISTS){DWORD error=GetLastError();free(tray);return error;}
    HWND window=CreateWindowW(wc.lpszClassName,L"Win16 Style",WS_OVERLAPPED,0,0,0,0,NULL,NULL,instance,tray);
    if(!window){DWORD error=GetLastError();free(tray);return error;}
    tray->added=notify(tray,NIM_ADD);if(!tray->added){DWORD error=GetLastError();DestroyWindow(window);free(tray);return error?error:ERROR_NOT_READY;}
    *result=tray;return ERROR_SUCCESS;
}
void run16s_tray_state(run16s_tray *tray,BOOL enabled,BOOL busy){if(tray){tray->enabled=enabled;tray->busy=busy;}}
void run16s_tray_destroy(run16s_tray *tray)
{if(tray){if(tray->added)notify(tray,NIM_DELETE);if(tray->window)DestroyWindow(tray->window);free(tray);}}
