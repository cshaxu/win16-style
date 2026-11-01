#ifndef RUN16S_TRAY_H
#define RUN16S_TRAY_H
#include <windows.h>

enum { RUN16S_TRAY_TOGGLE=1, RUN16S_TRAY_EXIT=2 };
typedef void (*run16s_tray_action)(unsigned action,void *context);
typedef struct run16s_tray run16s_tray;
DWORD run16s_tray_create(HICON icon,run16s_tray_action action,void *context,run16s_tray **result);
void run16s_tray_state(run16s_tray *tray,BOOL enabled,BOOL busy);
void run16s_tray_destroy(run16s_tray *tray);
int run16s_tray_global(HANDLE parent);
#endif
