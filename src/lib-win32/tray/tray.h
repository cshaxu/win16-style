#ifndef LIBTRAY_H
#define LIBTRAY_H

#include <windows.h>

typedef struct tray_icon tray_icon;
/* The caller creates a fresh popup menu; the library destroys it after use. */
typedef HMENU (*tray_menu_factory)(void *context);
typedef void (*tray_menu_command)(UINT command, void *context);

/* Icon and context are borrowed and must outlive the tray object. */
DWORD tray_icon_create(HICON icon, const WCHAR *tooltip,
                       tray_menu_factory menu, tray_menu_command command,
                       void *context, tray_icon **result);
void tray_icon_destroy(tray_icon *tray);

#ifdef TRAY_TESTING
/* Test-only messages exercise the real window procedure without Explorer. */
LRESULT tray_test_mouse(tray_icon *tray, UINT mouse_message);
void tray_test_selection(UINT command);
typedef LRESULT (*tray_test_control)(WPARAM request, void *context);
void tray_test_set_control(tray_icon *tray, tray_test_control control);
#endif

#endif
