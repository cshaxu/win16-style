#ifndef NTW16S_WINDOW_THREAD_H
#define NTW16S_WINDOW_THREAD_H

#include <windows.h>

/* The hook is temporary and targets one caller-selected window. The result
 * property holds one plus the Win32 error (one means success). A successful
 * attachment pins the DLL while window callbacks may still use its code. */
#define NTW16S_WINDOW_THREAD_MESSAGE L"Ntw16s.WindowThread.v1"
#define NTW16S_WINDOW_THREAD_RESULT L"Ntw16s.WindowThread.Result.v1"
#define NTW16S_WINDOW_THREAD_ATTACH 1
#define NTW16S_WINDOW_THREAD_REMOVE 2
LRESULT CALLBACK Ntw16sWindowThreadHook(int,WPARAM,LPARAM);

#endif
