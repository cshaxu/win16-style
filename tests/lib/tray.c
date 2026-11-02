#include "tray.h"
#include <stdio.h>

typedef struct fixture { unsigned menus, commands; UINT last; } fixture;

static HMENU make_menu(void *context)
{
    fixture *state = (fixture *)context;
    HMENU menu = CreatePopupMenu();
    if (!menu) return NULL;
    if (!AppendMenuW(menu, MF_STRING, 37, L"First") ||
        !AppendMenuW(menu, MF_STRING, 42, L"Second")) {
        DestroyMenu(menu);
        return NULL;
    }
    ++state->menus;
    return menu;
}

static void on_command(UINT command, void *context)
{
    fixture *state = (fixture *)context;
    ++state->commands;
    state->last = command;
}

int main(void)
{
    fixture state = {0};
    tray_icon *tray = NULL;
    HICON icon = LoadIconW(NULL, IDI_APPLICATION);
    if (!icon || tray_icon_create(icon, L"Test", make_menu, on_command,
                                  &state, &tray)) return 1;
    const UINT mouse[] = {WM_LBUTTONUP, WM_RBUTTONUP,
                          WM_LBUTTONDBLCLK, WM_RBUTTONDBLCLK};
    tray_test_selection(42);
    for (unsigned i = 0; i < ARRAYSIZE(mouse); ++i)
        tray_test_mouse(tray, mouse[i]);
    if (state.menus != 4 || state.commands != 4 || state.last != 42) return 2;
    tray_test_selection(0);
    tray_test_mouse(tray, WM_LBUTTONUP);
    if (state.menus != 5 || state.commands != 4) return 3;
    tray_test_mouse(tray, WM_MOUSEMOVE);
    if (state.menus != 5 || state.commands != 4) return 4;
    tray_icon_destroy(tray);
    puts("COMMON TRAY PASS mouse=4 cancel=1 context=1");
    return 0;
}
