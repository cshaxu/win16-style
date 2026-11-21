#ifndef NTWSHK_OBSERVED_H
#define NTWSHK_OBSERVED_H
#include <windows.h>

BOOL ntwshk_observed_control_attach(HWND window);
BOOL ntwshk_observed_control_attached(HWND window);
BOOL ntwshk_observed_control_remove(HWND window);
void ntwshk_observed_window_created(HWND window,DWORD style);
void ntwshk_observed_dialog_initialize(HWND dialog);
HRESULT ntwshk_observed_native_theme(HWND window,LPCWSTR application,LPCWSTR ids);

#endif
