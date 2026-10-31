#ifndef NTW16S_LATE_GUARD_H
#define NTW16S_LATE_GUARD_H
#include <windows.h>
#include <stddef.h>
/* Internal experimental primitive. It never alters instruction pointers.
 * Caller must finish or abort its patch before releasing the guard.
 * This does not prevent thread creation or prove callback quiescence. */
typedef struct ntw16s_late_region { const void *address; SIZE_T bytes; } ntw16s_late_region;
typedef struct ntw16s_late_guard { HANDLE *threads; DWORD count; DWORD suspended; } ntw16s_late_guard;
DWORD ntw16s_late_guard_begin(ntw16s_late_guard *,const ntw16s_late_region *,DWORD);
/* On a resume failure the guard retains that handle for a later retry. */
DWORD ntw16s_late_guard_end(ntw16s_late_guard *);
#ifdef NTW16S_LATE_GUARD_TEST
void ntw16s_late_guard_test_fail_after(DWORD);
void ntw16s_late_guard_test_after_suspend(void (*)(void));
#endif
#endif
