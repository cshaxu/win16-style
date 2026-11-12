#ifndef STYLE_PRINT_RANGES_H
#define STYLE_PRINT_RANGES_H
#include <windows.h>
#include <commdlg.h>
/* Strict numeric subset of the system page-range grammar. Failure never writes
 * to the destination; callers must not silently reinterpret unsupported input. */
BOOL style_print_ranges_parse(const WCHAR *text,DWORD minimum,DWORD maximum,
                               PRINTPAGERANGE *output,DWORD capacity,DWORD *count);
BOOL style_print_ranges_format(const PRINTPAGERANGE *ranges,DWORD count,DWORD minimum,DWORD maximum,
                                WCHAR *output,DWORD capacity);
#endif
