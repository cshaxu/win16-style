#ifndef STYLE_FILE_EVENTS_H
#define STYLE_FILE_EVENTS_H
#include <windows.h>
#include <shobjidl.h>
/* Owner is an IFileDialog-compatible identity; services outlive each adapter.
 * Adapters retain targets; snapshots retain listeners across reentrant callbacks.
 * STYLE_CLASSIC_* names identify the old-dialog event ordering policy. */
typedef struct style_dialog_event_services { ULONG (*add)(void *); ULONG (*release)(void *); } style_dialog_event_services;
typedef struct style_dialog_event_adapter {
    IFileDialogEvents iface; LONG refs; void *owner; IFileDialogEvents *target;
    DWORD cookie; struct style_dialog_event_adapter *next;
    const style_dialog_event_services *services;
} style_dialog_event_adapter;
typedef struct style_dialog_event_snapshot { style_dialog_event_adapter **items;size_t count; } style_dialog_event_snapshot;
enum style_classic_event_kind { STYLE_CLASSIC_OK,STYLE_CLASSIC_FOLDER_CHANGING,STYLE_CLASSIC_FOLDER_CHANGED,STYLE_CLASSIC_SELECTION_CHANGED,STYLE_CLASSIC_TYPE_CHANGED };
BOOL style_dialog_event_snapshot_init(style_dialog_event_adapter *events,style_dialog_event_snapshot *snapshot);
void style_dialog_event_snapshot_free(style_dialog_event_snapshot *snapshot);
HRESULT style_dialog_event_dispatch(const style_dialog_event_snapshot *snapshot,void *owner,enum style_classic_event_kind kind,IShellItem *item);
FDE_OVERWRITE_RESPONSE style_dialog_event_overwrite(const style_dialog_event_snapshot *snapshot,void *owner,IShellItem *item);
FDE_SHAREVIOLATION_RESPONSE style_dialog_event_share(const style_dialog_event_snapshot *snapshot,void *owner,IShellItem *item);
style_dialog_event_adapter *style_dialog_event_new(void *owner,IFileDialogEvents *target,const style_dialog_event_services *services);
ULONG STDMETHODCALLTYPE style_dialog_event_add(IFileDialogEvents *self);
ULONG STDMETHODCALLTYPE style_dialog_event_release(IFileDialogEvents *self);
HRESULT style_dialog_events_advise(IFileDialog *native,style_dialog_event_adapter **events,void *owner,
    IFileDialogEvents *target,DWORD *cookie,const style_dialog_event_services *services);
HRESULT style_dialog_events_unadvise(IFileDialog *native,style_dialog_event_adapter **events,DWORD cookie);
void style_dialog_events_release(IFileDialog *native,style_dialog_event_adapter **events);
#endif
