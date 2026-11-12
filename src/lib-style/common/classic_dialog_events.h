#ifndef STYLE_CLASSIC_DIALOG_EVENTS_H
#define STYLE_CLASSIC_DIALOG_EVENTS_H
#include "lib-style/common/classic_dialog_proxy.h"
typedef HRESULT (*style_classic_dialog_changing_fn)(style_classic_dialog_proxy *,IShellItem *);
typedef FDE_OVERWRITE_RESPONSE (*style_classic_dialog_overwrite_fn)(style_classic_dialog_proxy *,IShellItem *);
BOOL style_classic_dialog_list_changing(HWND,style_classic_dialog_proxy *,style_classic_dialog_changing_fn);
BOOL style_classic_dialog_typed_changing(HWND,style_classic_dialog_proxy *,style_classic_dialog_changing_fn);
BOOL style_classic_dialog_drive_changing(HWND,style_classic_dialog_proxy *,style_classic_dialog_changing_fn);
BOOL style_classic_dialog_overwrite_check(HWND,style_classic_dialog_proxy *,style_classic_dialog_overwrite_fn);
typedef struct style_classic_dialog_message_services {
    BOOL events_enabled,hidden_enabled;
    PCWSTR property,hidden_message;
    BOOL (*list_changing)(HWND,style_classic_dialog_proxy *),(*typed_changing)(HWND,style_classic_dialog_proxy *);
    BOOL (*drive_changing)(HWND,style_classic_dialog_proxy *),(*overwrite_check)(HWND,style_classic_dialog_proxy *);
    HRESULT (*event_dispatch)(style_classic_dialog_proxy *,enum style_classic_event_kind,IShellItem *);
    UINT_PTR (*file_ok)(style_classic_dialog_proxy *,OPENFILENAMEW *);
    FDE_SHAREVIOLATION_RESPONSE (*share)(style_classic_dialog_proxy *,IShellItem *);
    void (*initial)(HWND,style_classic_dialog_proxy *,PCWSTR);
    void (*hidden_schedule)(HWND,style_classic_dialog_proxy *),(*hidden_files)(HWND,style_classic_dialog_proxy *);
} style_classic_dialog_message_services;
void style_classic_dialog_attach(style_classic_dialog_proxy *,HWND,OPENFILENAMEW *,const style_classic_dialog_message_services *);
BOOL style_classic_dialog_command(style_classic_dialog_proxy *,HWND,WPARAM,UINT_PTR *,const style_classic_dialog_message_services *);
BOOL style_classic_dialog_before(style_classic_dialog_proxy *,HWND,UINT,WPARAM,UINT_PTR *,const style_classic_dialog_message_services *);
void style_classic_dialog_selection(style_classic_dialog_proxy *,HWND,WPARAM,const style_classic_dialog_message_services *);
BOOL style_classic_dialog_after(style_classic_dialog_proxy *,HWND,UINT,LPARAM,UINT_PTR *,const style_classic_dialog_message_services *);
typedef HRESULT (*style_classic_dialog_owned_event_fn)(style_classic_dialog_proxy *,enum style_classic_event_kind,IShellItem *,WCHAR *);
typedef HRESULT (*style_classic_dialog_event_fn)(style_classic_dialog_proxy *,enum style_classic_event_kind,IShellItem *);
typedef void (*style_classic_dialog_navigate_fn)(HWND,style_classic_dialog_proxy *,PCWSTR);
typedef struct style_classic_dialog_navigation_scope {
    style_classic_dialog_proxy *proxy;WCHAR *folder,*filename;BOOL experimental;
} style_classic_dialog_navigation_scope;
BOOL style_classic_dialog_navigation_begin(style_classic_dialog_proxy *,BOOL,style_classic_dialog_navigation_scope *);
void style_classic_dialog_navigation_run(HWND,PCWSTR,style_classic_dialog_navigation_scope *,style_classic_dialog_owned_event_fn);
void style_classic_dialog_navigation_cleanup(void *);
typedef struct style_classic_dialog_initial_scope {
    style_classic_dialog_proxy *proxy;IShellItem *item;PWSTR path;BOOL initializing;
} style_classic_dialog_initial_scope;
void style_classic_dialog_initial_run(HWND,PCWSTR,style_classic_dialog_initial_scope *,style_classic_dialog_owned_event_fn,style_classic_dialog_navigate_fn,style_classic_dialog_event_fn);
void style_classic_dialog_initial_cleanup(void *);
typedef struct style_classic_dialog_result_scope {
    style_classic_dialog_proxy *proxy;IShellItem *item;IShellItemArray *items;BOOL revoke;
} style_classic_dialog_result_scope;
BOOL style_classic_dialog_prepare_result(style_classic_dialog_proxy *,OPENFILENAMEW *);
void style_classic_dialog_result_cleanup(void *);
#endif
