#ifndef STYLE_UWP_TARGET_H
#define STYLE_UWP_TARGET_H
#include <windows.h>

/* HWNDs and PID are observations, not ownership or durable identity. */
typedef struct style_uwp_target {DWORD pid;HWND core,frame;} style_uwp_target;
BOOL style_uwp_host_image(PCWSTR image,PCWSTR windows_directory);
BOOL style_uwp_target_find(DWORD pid,style_uwp_target *target);
BOOL style_uwp_target_window(HWND window,style_uwp_target *target);
/* Revalidate the observed association before a caller acts on it. */
BOOL style_uwp_target_valid(const style_uwp_target *target);
#endif
