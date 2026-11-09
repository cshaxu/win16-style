#include "diagnostics_start.h"
#include <roapi.h>
#include <wchar.h>

typedef HRESULT (WINAPI *initialize_fn)(PCWSTR,DWORD,PCWSTR,PCWSTR,CLSID,PCWSTR);

HRESULT win16_xaml_diagnostics_start(DWORD process_id, const wchar_t *module_path,
                                     const wchar_t *diagnostics_path, const wchar_t *tap_path,
                                     const wchar_t *endpoint, REFCLSID class_id)
{
    if (!process_id || !module_path || !*module_path || !tap_path || !*tap_path ||
        !endpoint || !*endpoint || !class_id) return E_INVALIDARG;
    HRESULT hr=RoInitialize(RO_INIT_MULTITHREADED);
    if (FAILED(hr)) return hr;
    DWORD search=(wcschr(module_path,L'\\') || wcschr(module_path,L'/'))
        ? LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS : LOAD_LIBRARY_SEARCH_SYSTEM32;
    HMODULE module=LoadLibraryExW(module_path,NULL,search);
    if (!module) hr=HRESULT_FROM_WIN32(GetLastError());
    else {
        initialize_fn initialize=(initialize_fn)(void *)GetProcAddress(module,"InitializeXamlDiagnosticsEx");
        hr=initialize ? initialize(endpoint,process_id,diagnostics_path,tap_path,*class_id,NULL)
                      : HRESULT_FROM_WIN32(ERROR_PROC_NOT_FOUND);
        FreeLibrary(module);
    }
    RoUninitialize();
    return hr;
}
