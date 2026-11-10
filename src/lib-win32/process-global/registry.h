#ifndef WIN32_PROCESS_GLOBAL_REGISTRY_H
#define WIN32_PROCESS_GLOBAL_REGISTRY_H
#include <windows.h>
/* Intrusive PID/TID index. Supply a nonzero count of zeroed buckets; the
 * caller owns entries and synchronizes every access. */
typedef struct win32_process_global_registry_node {struct win32_process_global_registry_node *next;DWORD pid,tid;void *owner;} win32_process_global_registry_node;
typedef struct win32_process_global_registry {win32_process_global_registry_node **buckets;unsigned bucket_count,cursor;} win32_process_global_registry;
typedef BOOL (*win32_process_global_registry_remove_fn)(void *owner,void *context);
typedef void (*win32_process_global_registry_release_fn)(void *owner,void *context);
void win32_process_global_registry_init(win32_process_global_registry *,win32_process_global_registry_node **,unsigned);
void *win32_process_global_registry_find(const win32_process_global_registry *,DWORD,DWORD);
void win32_process_global_registry_insert(win32_process_global_registry *,win32_process_global_registry_node *,DWORD,DWORD,void *);
unsigned win32_process_global_registry_reap(win32_process_global_registry *,unsigned,win32_process_global_registry_remove_fn,win32_process_global_registry_release_fn,void *);
#endif
