#ifndef NTW16S_IMPORT_SCAN_H
#define NTW16S_IMPORT_SCAN_H
#include <windows.h>
#include <stddef.h>

typedef struct ntw16s_import_query {
    const char *library;
    const char *symbol;
} ntw16s_import_query;

/* Resolve slots only. The caller holds a reference to image and decides whether
 * a slot may be changed. No memory protection or import is modified here. */
DWORD ntw16s_import_slots(HMODULE image,const ntw16s_import_query *queries,
                          size_t count,void ***slots);

typedef struct ntw16s_import_change {
    void **slot;
    void *expected;
    void *replacement;
} ntw16s_import_change;

/* Atomically replace only expected slots; leave pre-existing replacements in
 * place, and never overwrite a foreign pointer. The caller holds image and pins
 * the module containing replacements until no other thread can call them. */
DWORD ntw16s_import_apply(HMODULE image,const ntw16s_import_change *changes,
                          size_t count,unsigned *changed);
#endif
