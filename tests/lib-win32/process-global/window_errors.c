#include <windows.h>
#include <stdio.h>
static DWORD enum_error;static BOOL unhook_failure;static unsigned unhooks;
static HWINEVENTHOOK WINAPI fake_hook(DWORD first,DWORD last,HMODULE module,WINEVENTPROC proc,DWORD process,DWORD thread,DWORD flags)
{(void)first;(void)last;(void)module;(void)proc;(void)process;(void)thread;(void)flags;return (HWINEVENTHOOK)(UINT_PTR)1;}
static BOOL WINAPI fake_unhook(HWINEVENTHOOK hook)
{(void)hook;++unhooks;if(unhook_failure){SetLastError(ERROR_BUSY);return FALSE;}return TRUE;}
static BOOL WINAPI fake_enum(WNDENUMPROC proc,LPARAM context)
{(void)proc;(void)context;SetLastError(enum_error);return FALSE;}
#define SetWinEventHook fake_hook
#define UnhookWinEvent fake_unhook
#define EnumWindows fake_enum
#include "../../../src/lib-win32/process-global/window.c"
static void found(HWND window,DWORD process,DWORD thread,void *context)
{(void)window;(void)process;(void)thread;(void)context;}
int main(void)
{
    win32_process_global_window watcher={0};
    enum_error=0;
    if(win32_process_global_window_start(&watcher,found,NULL) || !watcher.hook)return 1;
    if(win32_process_global_window_stop(&watcher) || watcher.hook)return 2;
    enum_error=ERROR_ACCESS_DENIED;unhooks=0;
    if(win32_process_global_window_start(&watcher,found,NULL)!=ERROR_ACCESS_DENIED || watcher.hook || unhooks!=1)return 3;
    unhook_failure=TRUE;unhooks=0;
    if(win32_process_global_window_start(&watcher,found,NULL)!=ERROR_BUSY || !watcher.hook || unhooks!=1)return 4;
    if(win32_process_global_window_start(&watcher,found,NULL)!=ERROR_BUSY)return 5;
    unhook_failure=FALSE;
    if(win32_process_global_window_stop(&watcher) || watcher.hook || unhooks!=2)return 6;
    puts("WINDOW_START_ERRORS_OK empty=1 enum=1 rollback=1 retry=1");return 0;
}
