#ifndef WIN16_DIALOG_CLASSIC_EVENTS_H
#define WIN16_DIALOG_CLASSIC_EVENTS_H
#include "proxy.h"
typedef HRESULT (*win16_dialog_changing_fn)(win16_dialog_proxy *,IShellItem *);
typedef FDE_OVERWRITE_RESPONSE (*win16_dialog_overwrite_fn)(win16_dialog_proxy *,IShellItem *);
BOOL win16_dialog_list_changing(HWND,win16_dialog_proxy *,win16_dialog_changing_fn);
BOOL win16_dialog_typed_changing(HWND,win16_dialog_proxy *,win16_dialog_changing_fn);
BOOL win16_dialog_drive_changing(HWND,win16_dialog_proxy *,win16_dialog_changing_fn);
BOOL win16_dialog_overwrite_check(HWND,win16_dialog_proxy *,win16_dialog_overwrite_fn);
typedef struct win16_dialog_classic_message_services {
    BOOL events_enabled,hidden_enabled;
    BOOL (*list_changing)(HWND,win16_dialog_proxy *),(*typed_changing)(HWND,win16_dialog_proxy *);
    BOOL (*drive_changing)(HWND,win16_dialog_proxy *),(*overwrite_check)(HWND,win16_dialog_proxy *);
    HRESULT (*event_dispatch)(win16_dialog_proxy *,enum classic_event_kind,IShellItem *);
    UINT_PTR (*file_ok)(win16_dialog_proxy *,OPENFILENAMEW *);
    FDE_SHAREVIOLATION_RESPONSE (*share)(win16_dialog_proxy *,IShellItem *);
    void (*initial)(HWND,win16_dialog_proxy *,PCWSTR);
    void (*hidden_schedule)(HWND,win16_dialog_proxy *),(*hidden_files)(HWND,win16_dialog_proxy *);
} win16_dialog_classic_message_services;
void win16_dialog_classic_attach(win16_dialog_proxy *,HWND,OPENFILENAMEW *,const win16_dialog_classic_message_services *);
BOOL win16_dialog_classic_command(win16_dialog_proxy *,HWND,WPARAM,UINT_PTR *,const win16_dialog_classic_message_services *);
BOOL win16_dialog_classic_before(win16_dialog_proxy *,HWND,UINT,WPARAM,UINT_PTR *,const win16_dialog_classic_message_services *);
void win16_dialog_classic_selection(win16_dialog_proxy *,HWND,WPARAM,const win16_dialog_classic_message_services *);
BOOL win16_dialog_classic_after(win16_dialog_proxy *,HWND,UINT,LPARAM,UINT_PTR *,const win16_dialog_classic_message_services *);
typedef struct win16_dialog_classic_result_scope {
    win16_dialog_proxy *proxy;IShellItem *item;IShellItemArray *items;BOOL revoke;
} win16_dialog_classic_result_scope;
BOOL win16_dialog_classic_folder_adjust(const OPENFILENAMEW *,WCHAR *,size_t,OPENFILENAMEW *);
BOOL win16_dialog_classic_prepare_result(win16_dialog_proxy *,OPENFILENAMEW *);
void win16_dialog_classic_result_cleanup(void *);
#endif
