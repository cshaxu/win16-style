#ifndef WIN16_DIALOG_EVENTS_H
#define WIN16_DIALOG_EVENTS_H
#include <windows.h>
#include <shobjidl.h>
typedef struct win16_dialog_event_services { ULONG (*add)(void *); ULONG (*release)(void *); } win16_dialog_event_services;
typedef struct event_adapter {
    IFileDialogEvents iface; LONG refs; void *owner; IFileDialogEvents *target;
    DWORD cookie; struct event_adapter *next;
    const win16_dialog_event_services *services;
} event_adapter;
typedef struct win16_dialog_event_snapshot { event_adapter **items;size_t count; } win16_dialog_event_snapshot;
enum classic_event_kind { CLASSIC_OK,CLASSIC_FOLDER_CHANGING,CLASSIC_FOLDER_CHANGED,CLASSIC_SELECTION_CHANGED,CLASSIC_TYPE_CHANGED };
BOOL win16_dialog_event_snapshot_init(event_adapter *events,win16_dialog_event_snapshot *snapshot);
void win16_dialog_event_snapshot_free(win16_dialog_event_snapshot *snapshot);
HRESULT win16_dialog_event_dispatch(const win16_dialog_event_snapshot *snapshot,void *owner,enum classic_event_kind kind,IShellItem *item);
FDE_OVERWRITE_RESPONSE win16_dialog_event_overwrite(const win16_dialog_event_snapshot *snapshot,void *owner,IShellItem *item);
FDE_SHAREVIOLATION_RESPONSE win16_dialog_event_share(const win16_dialog_event_snapshot *snapshot,void *owner,IShellItem *item);
event_adapter *win16_dialog_event_new(void *owner,IFileDialogEvents *target,const win16_dialog_event_services *services);
ULONG STDMETHODCALLTYPE win16_dialog_event_add(IFileDialogEvents *self);
ULONG STDMETHODCALLTYPE win16_dialog_event_release(IFileDialogEvents *self);
HRESULT win16_dialog_events_advise(IFileDialog *native,event_adapter **events,void *owner,
    IFileDialogEvents *target,DWORD *cookie,const win16_dialog_event_services *services);
HRESULT win16_dialog_events_unadvise(IFileDialog *native,event_adapter **events,DWORD cookie);
void win16_dialog_events_release(IFileDialog *native,event_adapter **events);
#endif
