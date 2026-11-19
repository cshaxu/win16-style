#ifndef STYLE_PRINT_RESOURCES_H
#define STYLE_PRINT_RESOURCES_H
#include <windows.h>
#include <commdlg.h>
/* Global-memory clones and returned device contexts belong to the caller.
 * Device-mode and DEVNAMES pointers passed to the low-level helpers must
 * already be valid, locked and in the requested ANSI or Unicode format. */
BOOL style_print_global_available(HGLOBAL);
HGLOBAL style_print_clone_global(HGLOBAL);
HGLOBAL style_print_clone_file_devnames(HGLOBAL,BOOL);
HDC style_print_create_dc(void *,DEVNAMES *,BOOL,DWORD);
void style_print_set_copies(void *,BOOL,short,BOOL);
HDC style_print_create_print_dc(HGLOBAL,HGLOBAL,BOOL,DWORD,DWORD);
BOOL style_print_normalize_driver_copies(HGLOBAL,BOOL,WORD *,DWORD *);

typedef struct style_print_snapshot {
    HGLOBAL mode,names;
    HDC dc;
    DWORD copies,flags;
} style_print_snapshot;

/* Input handles contain the active printer's valid settings and stay owned
 * by the caller. Success returns an independent result in an empty slot.
 * Failure leaves the slot untouched. */
BOOL style_print_snapshot_create(HGLOBAL mode,HGLOBAL names,BOOL ansi,DWORD copies,DWORD flags,
    style_print_snapshot *result);
void style_print_snapshot_dispose(style_print_snapshot *);
#endif
