#ifndef WIN32_STYLE_COMPANION_H
#define WIN32_STYLE_COMPANION_H
#include <windows.h>
#include <stddef.h>
/* Resolves exports from a sibling of the module containing anchor, verifies
 * its loaded image against that file and pins the module on success. The
 * caller supplies the module filename, export names and pinning entry. */
BOOL win32_style_companion(const void *anchor,PCWSTR filename,const char *const *names,
    FARPROC *exports,size_t count,size_t pin_index);
#endif
