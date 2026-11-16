#ifndef STYLE_RUNTIME_MODULE_H
#define STYLE_RUNTIME_MODULE_H
#include <windows.h>
/* Same-bitness targets only. The caller supplies an absolute package path and
 * a callback in the loaded module. Existing same-named mappings must match
 * both that mapped file and the callback's first 64 bytes at the same offset. */
DWORD win32_style_module_compatible(DWORD,HMODULE,HOOKPROC,PCWSTR);
/* On success the caller owns *module until all callback use has ended, then
 * releases it with FreeLibrary. This does not pin or unload the target image. */
DWORD win32_style_module_open(DWORD,PCWSTR,const char *,HMODULE *,HOOKPROC *);
#endif
