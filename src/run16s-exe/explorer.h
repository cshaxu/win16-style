#ifndef RUN16S_EXPLORER_H
#define RUN16S_EXPLORER_H
#include <windows.h>

typedef struct run16s_explorer_snapshot run16s_explorer_snapshot;
BOOL run16s_explorer_default(PCWSTR image,PCWSTR arguments);
run16s_explorer_snapshot *run16s_explorer_snapshot_create(void);
DWORD run16s_explorer_attach_new(run16s_explorer_snapshot *,PCWSTR image,PCWSTR directory);
void run16s_explorer_snapshot_free(run16s_explorer_snapshot *);
#endif
