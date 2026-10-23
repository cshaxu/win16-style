#include <windows.h>
#include <type_traits>

static_assert(std::is_pointer<decltype(&IsWow64Process2)>::value, "SDK process ABI");

extern "C" __declspec(dllexport) unsigned __cdecl probe_sdk(void)
{
    USHORT process = 0, native = 0;
    if (!IsWow64Process2(GetCurrentProcess(), &process, &native)) return 0;
    return native == IMAGE_FILE_MACHINE_AMD64 &&
        (process == IMAGE_FILE_MACHINE_I386 || process == IMAGE_FILE_MACHINE_UNKNOWN);
}
