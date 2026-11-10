#include "tray.h"
#include <shellapi.h>
#include <stdlib.h>
#include <wchar.h>

#define TRAY_MESSAGE (WM_APP + 53)
#define TRAY_ID 1

struct tray_icon {
    HWND window;
    HICON icon;
    WCHAR tooltip[128];
    tray_menu_factory menu;
    tray_menu_command command;
    void *context;
    UINT restart;
    BOOL added;
#ifdef TRAY_TESTING
    tray_test_control test_control;
#endif
};

#ifdef TRAY_TESTING
static UINT test_selection;
void tray_test_selection(UINT command) { test_selection = command; }
void tray_test_set_control(tray_icon *tray, tray_test_control control)
{ tray->test_control = control; }
LRESULT tray_test_mouse(tray_icon *tray, UINT message)
{
    return SendMessageW(tray->window, TRAY_MESSAGE, TRAY_ID, message);
}
#endif

static BOOL notify(tray_icon *tray, DWORD operation)
{
#ifdef TRAY_TESTING
    (void)tray;
    (void)operation;
    return TRUE;
#else
    NOTIFYICONDATAW data = {0};
    data.cbSize = sizeof(data);
    data.hWnd = tray->window;
    data.uID = TRAY_ID;
    data.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    data.uCallbackMessage = TRAY_MESSAGE;
    data.hIcon = tray->icon;
    lstrcpynW(data.szTip, tray->tooltip, ARRAYSIZE(data.szTip));
    SetLastError(ERROR_SUCCESS);
    return Shell_NotifyIconW(operation, &data);
#endif
}

static void show_menu(tray_icon *tray)
{
    HMENU menu = tray->menu(tray->context);
    POINT point;
    if (!menu) return;
    if (!GetCursorPos(&point)) {
        DestroyMenu(menu);
        return;
    }
    SetForegroundWindow(tray->window);
#ifdef TRAY_TESTING
    UINT command = test_selection;
#else
    UINT command = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_NONOTIFY |
                                 TPM_RIGHTBUTTON, point.x, point.y, 0,
                                 tray->window, NULL);
#endif
    PostMessageW(tray->window, WM_NULL, 0, 0);
    DestroyMenu(menu);
    if (command) tray->command(command, tray->context);
}

static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM a, LPARAM b)
{
    tray_icon *tray = (tray_icon *)(void *)GetWindowLongPtrW(window, GWLP_USERDATA);
    if (message == WM_NCCREATE) {
        CREATESTRUCTW *create = (CREATESTRUCTW *)(void *)b;
        tray = (tray_icon *)create->lpCreateParams;
        SetWindowLongPtrW(window, GWLP_USERDATA, (LONG_PTR)(void *)tray);
        tray->window = window;
    }
    if (tray && tray->restart && message == tray->restart) {
        tray->added = notify(tray, NIM_ADD);
        return 0;
    }
#ifdef TRAY_TESTING
    if (tray && message == WM_APP + 54 && tray->test_control)
        return tray->test_control(a, tray->context);
#endif
    if (tray && message == TRAY_MESSAGE && a == TRAY_ID &&
        (b == WM_LBUTTONUP || b == WM_RBUTTONUP ||
         b == WM_LBUTTONDBLCLK || b == WM_RBUTTONDBLCLK)) {
        show_menu(tray);
        return 0;
    }
    if (message == WM_CLOSE) return 0;
    return DefWindowProcW(window, message, a, b);
}

DWORD tray_icon_create(HICON icon, const WCHAR *tooltip,
                       tray_menu_factory menu, tray_menu_command command,
                       void *context, tray_icon **result)
{
    if (result) *result = NULL;
    if (!icon || !tooltip || !menu || !command || !result ||
        wcslen(tooltip) >= ARRAYSIZE(((tray_icon *)0)->tooltip))
        return ERROR_INVALID_PARAMETER;
    tray_icon *tray = (tray_icon *)calloc(1, sizeof(*tray));
    if (!tray) return ERROR_NOT_ENOUGH_MEMORY;
    tray->icon = icon;
    lstrcpynW(tray->tooltip, tooltip, ARRAYSIZE(tray->tooltip));
    tray->menu = menu;
    tray->command = command;
    tray->context = context;
    tray->restart = RegisterWindowMessageW(L"TaskbarCreated");
    HINSTANCE instance = GetModuleHandleW(NULL);
    WNDCLASSW wc = {0};
    wc.hInstance = instance;
    wc.lpfnWndProc = window_proc;
    wc.lpszClassName = L"LibTrayWindow";
    if (!RegisterClassW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        DWORD error = GetLastError();
        free(tray);
        return error;
    }
    HWND window = CreateWindowW(wc.lpszClassName, tooltip, WS_OVERLAPPED,
                               0, 0, 0, 0, NULL, NULL, instance, tray);
    if (!window) {
        DWORD error = GetLastError();
        free(tray);
        return error;
    }
    tray->added = notify(tray, NIM_ADD);
    if (!tray->added) {
        DWORD error = GetLastError();
        DestroyWindow(window);
        free(tray);
        return error ? error : ERROR_NOT_READY;
    }
    *result = tray;
    return ERROR_SUCCESS;
}

void tray_icon_destroy(tray_icon *tray)
{
    if (!tray) return;
    if (tray->added) notify(tray, NIM_DELETE);
    if (tray->window) DestroyWindow(tray->window);
    free(tray);
}
