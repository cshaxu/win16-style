/* Test-only controller for a self-owned passive XAML target. */
#include <windows.h>
#include <roapi.h>
#include <stdio.h>
#include <wchar.h>
#include <stdlib.h>
#include "lib-win32/win16-control-xaml/diagnostics_start.h"
#include "lib-win32/win16-control-xaml/runtime_find.h"

typedef HRESULT (WINAPI *initialize_fn)(PCWSTR,DWORD,PCWSTR,PCWSTR,CLSID,PCWSTR);
static const CLSID tap_clsid = {0x7132a744,0x60fb,0x4301,{0xa8,0x9a,0x40,0xd0,0x44,0x31,0x93,0x85}};

int wmain(int argc, wchar_t **argv)
{
    BOOL probe = argc == 3 && !wcscmp(argv[1],L"--probe");
    BOOL winui = argc == 4 && !wcscmp(argv[1],L"--winui");
    if (!probe && !winui && argc != 3 && argc != 6) return 2;
    DWORD pid = probe ? 0 : wcstoul(argv[winui ? 2 : 1],NULL,10);
    if (!probe && !pid) return 2;
    if (!probe) {
        WCHAR loader[32752],xaml[32752];
        if (winui) {
            HANDLE process=OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,FALSE,pid);
            if (!process) { printf("open=%lu\n",(unsigned long)GetLastError()); return 4; }
            DWORD error=win16_xaml_runtime_find(process,loader,ARRAYSIZE(loader),xaml,ARRAYSIZE(xaml));
            CloseHandle(process);
            if (error) { printf("runtime=%lu\n",(unsigned long)error); return 4; }
        }
        PCWSTR module_path=winui ? loader : argc==6 ? argv[3] : L"Windows.UI.Xaml.dll";
        PCWSTR endpoint=winui ? L"WinUIVisualDiagConnection1" : argc==6 ? argv[5] : L"VisualDiagConnection1";
        PCWSTR diagnostics_path=winui ? xaml : argc==6 ? argv[4] : NULL;
        WCHAR done_name[160];
        DWORD length=GetEnvironmentVariableW(L"WIN16_XAML_CONTROLLER_DONE_EVENT",done_name,ARRAYSIZE(done_name));
        HMODULE held=NULL;
        HRESULT held_apartment=E_FAIL;
        if (length && length<ARRAYSIZE(done_name) && wcschr(module_path,L'\\'))
            held=LoadLibraryExW(module_path,NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (length && length<ARRAYSIZE(done_name)) held_apartment=RoInitialize(RO_INIT_MULTITHREADED);
        HRESULT result=win16_xaml_diagnostics_start(pid,module_path,diagnostics_path,argv[winui ? 3 : 2],endpoint,&tap_clsid);
        printf("initialize=%08lx\n",(unsigned long)result);
        if (SUCCEEDED(result) && length && length<ARRAYSIZE(done_name)) {
            HANDLE done=OpenEventW(SYNCHRONIZE,FALSE,done_name);
            if (done) { (void)WaitForSingleObject(done,7000); CloseHandle(done); }
        }
        if (held) FreeLibrary(held);
        if (SUCCEEDED(held_apartment)) RoUninitialize();
        return FAILED(result) ? 5 : 0;
    }
    HRESULT hr = RoInitialize(RO_INIT_MULTITHREADED);
    if (FAILED(hr)) { printf("ro=%08lx\n",(unsigned long)hr); return 3; }
    PCWSTR module_path = argv[2];
    HMODULE module = LoadLibraryExW(module_path,NULL,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
    if (!module) { printf("load=%lu\n",(unsigned long)GetLastError()); RoUninitialize(); return 4; }
    initialize_fn initialize = (initialize_fn)(void *)GetProcAddress(module,"InitializeXamlDiagnosticsEx");
    printf("diagnostics_export=%d\n",initialize!=NULL);
    FreeLibrary(module); RoUninitialize(); return initialize ? 0 : 5;
}
