#ifndef NTW16S_UWP_HOOK_H
#define NTW16S_UWP_HOOK_H
#include <windows.h>
#define NTW16S_UWP_MESSAGE L"Win16Style.UwpFrame.v1"
#define NTW16S_UWP_RESULT L"Win16Style.UwpFrame.Result.v1"
enum { NTW16S_UWP_APPLY=1, NTW16S_UWP_REMOVE=2, NTW16S_UWP_APPLY_FULLSCREEN=3 };
/* WH_CALLWNDPROC, selected host GUI thread only. Message HWND is the frame,
 * LPARAM its current CoreWindow, WPARAM an action above. Result property is
 * Win32 error+1 (1=success); the coordinator removes it after reading.
 * The module remains mapped for the host lifetime because subclass callbacks
 * can outlive the coordinator's thread hook. Removal is not module unloading. */
LRESULT CALLBACK Ntw16sUwpFrameHook(int code,WPARAM parameter,LPARAM message);
#endif
