#include <windows.h>
#include <tlhelp32.h>
#include "lib-win32/process-global/enumerate.h"

static BOOL WINAPI failing_next(HANDLE snapshot, LPTHREADENTRY32 entry)
{
    (void)snapshot;
    (void)entry;
    SetLastError(ERROR_ACCESS_DENIED);
    return FALSE;
}

#define Thread32Next failing_next
#include "../../../src/lib-win32/process-global/enumerate.c"

static BOOL observe(DWORD pid, DWORD tid, void *context)
{
    (void)pid;
    (void)tid;
    ++*(unsigned *)context;
    return TRUE;
}

static BOOL stop(DWORD pid, DWORD tid, void *context)
{
    (void)pid;
    (void)tid;
    (void)context;
    SetLastError(ERROR_ACCESS_DENIED);
    return FALSE;
}

int main(void)
{
    unsigned count = 0;
    DWORD before = 0, after = 0;
    if (!GetProcessHandleCount(GetCurrentProcess(), &before)) return 1;
    if (win32_process_global_threads(0, observe, &count) != ERROR_ACCESS_DENIED || count != 1) return 2;
    if (win32_process_global_threads(0, stop, NULL) != ERROR_SUCCESS) return 3;
    if (!GetProcessHandleCount(GetCurrentProcess(), &after) || before != after) return 4;
    return 0;
}
