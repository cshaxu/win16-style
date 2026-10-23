#include <windows.h>
#include <stdint.h>

_Static_assert(sizeof(WCHAR) == 2, "Windows UTF-16 ABI");
_Static_assert(sizeof(uintptr_t) == sizeof(void *), "pointer-width ABI");
static __declspec(thread) unsigned calls;

__declspec(dllexport) unsigned __cdecl probe_tls(void)
{
    return ++calls;
}

__declspec(dllexport) unsigned __cdecl probe_width(void)
{
    return (unsigned)(sizeof(void *) * 8);
}

BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID reserved)
{
    (void)module;
    (void)reason;
    (void)reserved;
    return TRUE;
}
