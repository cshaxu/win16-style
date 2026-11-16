#ifndef NTWSHK_SYSTEM_PROCESS_H
#define NTWSHK_SYSTEM_PROCESS_H
#include <windows.h>
/* Existing exported names alias the shared startup target policy. */
BOOL ntwshk_system_print_host_w(LPCWSTR,LPCWSTR);
BOOL ntwshk_system_print_host_a(LPCSTR,LPCSTR);
#endif
