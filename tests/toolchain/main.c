#include <windows.h>
#include <stdio.h>

__declspec(dllimport) unsigned __cdecl probe_tls(void);
__declspec(dllimport) unsigned __cdecl probe_width(void);
__declspec(dllimport) unsigned __cdecl probe_sdk(void);

static DWORD WINAPI thread_test(void *context)
{
    (void)context;
    return probe_tls() == 1 && probe_tls() == 2 ? 0 : 1;
}

int main(void)
{
    HANDLE thread;
    DWORD result = 1;
    if (probe_width() != sizeof(void *) * 8 || !probe_sdk()) return 1;
    if (probe_tls() != 1 || probe_tls() != 2) return 2;
    thread = CreateThread(NULL, 0, thread_test, NULL, 0, NULL);
    if (!thread) return 3;
    if (WaitForSingleObject(thread, 5000) != WAIT_OBJECT_0 ||
        !GetExitCodeThread(thread, &result)) {
        CloseHandle(thread);
        return 4;
    }
    CloseHandle(thread);
    if (result || probe_tls() != 3) return 5;
    printf("Toolchain PASS: width=%u MSVC=%d C17/C++17 SDK DLL /MT TLS\n",
        probe_width(), _MSC_FULL_VER);
    return 0;
}
