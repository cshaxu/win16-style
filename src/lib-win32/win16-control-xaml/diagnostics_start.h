#ifndef WIN16_XAML_DIAGNOSTICS_START_H
#define WIN16_XAML_DIAGNOSTICS_START_H
#include <windows.h>

/* Run on a worker thread. Paths and endpoint are chosen by the caller from
 * the target runtime; no product or target names are built into this API. */
HRESULT win16_xaml_diagnostics_start(DWORD process_id, const wchar_t *module_path,
                                     const wchar_t *diagnostics_path, const wchar_t *tap_path,
                                     const wchar_t *endpoint, REFCLSID class_id);
#endif
