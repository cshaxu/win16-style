#ifndef NTW16S_PRINT_RANGES_H
#define NTW16S_PRINT_RANGES_H
#include <windows.h>
#include <commdlg.h>
/* Strict numeric subset of the system page-range grammar. Failure never writes
 * to the destination; callers must not silently reinterpret unsupported input. */
BOOL ntw16s_parse_print_ranges(const WCHAR *text,DWORD minimum,DWORD maximum,
                               PRINTPAGERANGE *output,DWORD capacity,DWORD *count);
#endif
