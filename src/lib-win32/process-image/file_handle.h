#ifndef WIN32_PROCESS_IMAGE_FILE_HANDLE_H
#define WIN32_PROCESS_IMAGE_FILE_HANDLE_H
#include <windows.h>
/* Validates an open non-reparse DLL for the caller architecture; advances its file pointer. */
DWORD win32_process_image_validate_dll_handle(HANDLE file);
#endif
