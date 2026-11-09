#ifndef NTW16S_XAML_SITE_H
#define NTW16S_XAML_SITE_H
#include <windows.h>
/* Start only if the process has already loaded the WinUI runtime. */
void ntw16s_xaml_start(void);
/* Stop styling on its owning UI thread; disconnection runs on a worker. */
HRESULT ntw16s_xaml_stop(void);
HRESULT ntw16s_xaml_remove_window(HWND window);
#endif
