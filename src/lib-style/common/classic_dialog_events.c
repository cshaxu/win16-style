#define COBJMACROS
#include "classic_dialog_events.h"
#include "lib-style/common/classic_file_helpers.h"
#include "lib-style/common/classic_shell_item.h"
#include "classic_dialog_state.h"
#include <wchar.h>
#include <string.h>
#include <dlgs.h>

BOOL style_classic_dialog_list_changing(HWND w,style_classic_dialog_proxy *p,style_classic_dialog_changing_fn changing)
{
    IShellItem *target=style_classic_file_list_target(w,p->live_folder);
    if(!target)return TRUE;
    HRESULT hr=changing(p,target);
    return hr==S_OK && !p->pending_folder;
}
BOOL style_classic_dialog_typed_changing(HWND w,style_classic_dialog_proxy *p,style_classic_dialog_changing_fn changing)
{
    IShellItem *target=NULL;
    if(!style_classic_file_typed_target(w,p->live_folder,&target))return FALSE;
    if(!target)return TRUE;
    HRESULT hr=changing(p,target);
    return hr==S_OK && !p->pending_folder;
}
BOOL style_classic_dialog_drive_changing(HWND w,style_classic_dialog_proxy *p,style_classic_dialog_changing_fn changing)
{
    LRESULT index;HWND combo;IShellItem *target=style_classic_file_drive_target(w,&index,&combo);
    if(!target)return TRUE;
    HRESULT hr=changing(p,target);
    if(hr!=S_OK || p->pending_folder){if(p->classic_drive_index!=CB_ERR)SendMessageW(combo,CB_SETCURSEL,(WPARAM)p->classic_drive_index,0);return FALSE;}
    p->classic_drive_index=index;return TRUE;
}
BOOL style_classic_dialog_overwrite_check(HWND w,style_classic_dialog_proxy *p,style_classic_dialog_overwrite_fn overwrite)
{
    OPENFILENAMEW *ofn=p->classic_ofn;if(!ofn)return TRUE;
    if(p->classic_overwrite_suppressed){ofn->Flags|=OFN_OVERWRITEPROMPT;p->classic_overwrite_suppressed=FALSE;}
    if(!p->save || !(ofn->Flags&OFN_OVERWRITEPROMPT))return TRUE;
    IShellItem *item=style_classic_file_overwrite_target(w,p->live_folder,p->extension);
    if(!item)return TRUE;
    FDE_OVERWRITE_RESPONSE response=overwrite(p,item);
    if(!IsWindow(w) || p->active_window!=w || p->classic_ofn!=ofn)return FALSE;
    if(response==FDEOR_REFUSE)return FALSE;
    if(response==FDEOR_ACCEPT){ofn->Flags&=~OFN_OVERWRITEPROMPT;p->classic_overwrite_suppressed=TRUE;}
    return TRUE;
}

BOOL style_classic_dialog_prepare_result(style_classic_dialog_proxy *p,OPENFILENAMEW *ofn)
{
    if(!ofn || !ofn->lpstrFile)return FALSE;
    if(p->result){IShellItem_Release(p->result);p->result=NULL;}
    if(p->results){IShellItemArray_Release(p->results);p->results=NULL;}
    p->completed=FALSE;IShellItem *item=NULL;HRESULT hr;
    if(ofn->Flags&OFN_ALLOWMULTISELECT){size_t length=wcsnlen(ofn->lpstrFile,ofn->nMaxFile);
        if(length==ofn->nMaxFile)return FALSE;
        WCHAR *copy=CoTaskMemAlloc((length+1)*sizeof(WCHAR));if(!copy)return FALSE;
        memcpy(copy,ofn->lpstrFile,(length+1)*sizeof(WCHAR));
        hr=style_classic_multi_result(copy,ofn->nFileOffset,(ofn->Flags&OFN_NODEREFERENCELINKS)!=0,&p->result,&p->results,&p->completed);CoTaskMemFree(copy);
        item=p->result;
    }else hr=p->save?SHCreateItemFromParsingName(ofn->lpstrFile,NULL,&IID_IShellItem,(void **)&item):
        style_classic_open_item(ofn->lpstrFile,(ofn->Flags&OFN_NODEREFERENCELINKS)!=0,&item);
    if(p->save && hr==HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))hr=style_classic_save_item(ofn->lpstrFile,&item);
    if(FAILED(hr))return FALSE;
    p->result=item;p->completed=TRUE;return TRUE;
}
void style_classic_dialog_result_cleanup(void *data)
{
    style_classic_dialog_result_scope *scope=data;
    if(scope->revoke && scope->item && scope->proxy->result==scope->item){IShellItem_Release(scope->item);
        scope->proxy->result=NULL;scope->proxy->completed=FALSE;}
    if(scope->revoke && scope->items && scope->proxy->results==scope->items){IShellItemArray_Release(scope->items);
        scope->proxy->results=NULL;scope->proxy->completed=FALSE;}
}

void style_classic_dialog_attach(style_classic_dialog_proxy *p,HWND w,OPENFILENAMEW *ofn,const style_classic_dialog_message_services *v)
{
    style_classic_dialog_hook_attach_base(p,w,v->events_enabled);
    if(v->hidden_enabled)v->hidden_schedule(w,p);
    if(v->events_enabled){p->classic_ofn=ofn;HWND drives=GetDlgItem(w,cmb2);
        p->classic_drive_index=drives?SendMessageW(drives,CB_GETCURSEL,0,0):CB_ERR;
        if(p->events && p->live_folder)v->initial(w,p,v->property);}
}
BOOL style_classic_dialog_command(style_classic_dialog_proxy *p,HWND w,WPARAM wp,UINT_PTR *result,const style_classic_dialog_message_services *v)
{
    if(v->events_enabled){
        if(LOWORD(wp)==lst2 && HIWORD(wp)==LBN_DBLCLK){if(p && p->events && !v->list_changing(w,p)){*result=1;return TRUE;}return FALSE;}
        if(LOWORD(wp)==IDOK){if(p && p->events && !p->classic_navigating && (!v->typed_changing(w,p)||!v->overwrite_check(w,p))){*result=1;return TRUE;}return FALSE;}
        if(LOWORD(wp)==cmb2 && HIWORD(wp)==CBN_SELCHANGE){if(p && p->events && !v->drive_changing(w,p)){*result=1;return TRUE;}return FALSE;}
    }
    if(v->hidden_enabled && LOWORD(wp)==cmb1 && HIWORD(wp)==CBN_SELCHANGE)v->hidden_schedule(w,p);
    return FALSE;
}
BOOL style_classic_dialog_before(style_classic_dialog_proxy *p,HWND w,UINT message,WPARAM wp,UINT_PTR *result,const style_classic_dialog_message_services *v)
{
    if(v->hidden_enabled){UINT refresh=RegisterWindowMessageW(v->hidden_message);
        if(refresh && message==refresh){if(p && p->classic_hidden)v->hidden_files(w,p);*result=1;return TRUE;}
        UINT selection=RegisterWindowMessageW(LBSELCHSTRINGW);
        if(selection && message==selection && (wp==lst2 || wp==cmb2))v->hidden_schedule(w,p);}
    return FALSE;
}
void style_classic_dialog_selection(style_classic_dialog_proxy *p,HWND w,WPARAM wp,const style_classic_dialog_message_services *v)
{
    if(!p)return;
    if(wp==lst2 || wp==cmb2){style_classic_dialog_proxy_capture_folder(p,v->events_enabled);
        if(v->events_enabled){if(wp==cmb2){HWND drives=GetDlgItem(w,cmb2);p->classic_drive_index=drives?SendMessageW(drives,CB_GETCURSEL,0,0):CB_ERR;}
            if(p->events)(void)v->event_dispatch(p,STYLE_CLASSIC_FOLDER_CHANGED,NULL);}
    }else if(v->events_enabled && wp==lst1){style_classic_dialog_proxy_capture_selection(w,p);if(p->events)(void)v->event_dispatch(p,STYLE_CLASSIC_SELECTION_CHANGED,NULL);}
    else if(v->events_enabled && wp==cmb1){FILEOPENDIALOGOPTIONS options=0;
        if(p->events && (p->filter || FAILED(IFileDialog_GetOptions(p->native,&options)) || !(options&FOS_PICKFOLDERS)))
            (void)v->event_dispatch(p,STYLE_CLASSIC_TYPE_CHANGED,NULL);}
}
BOOL style_classic_dialog_after(style_classic_dialog_proxy *p,HWND w,UINT message,LPARAM lp,UINT_PTR *result,const style_classic_dialog_message_services *v)
{
    (void)w;if(!v->events_enabled)return FALSE;
    UINT fileok=RegisterWindowMessageW(FILEOKSTRINGW);
    if(fileok && message==fileok && p && p->events){*result=v->file_ok(p,(OPENFILENAMEW *)lp);return TRUE;}
    UINT share=RegisterWindowMessageW(SHAREVISTRINGW);
    if(share && message==share){FILEOPENDIALOGOPTIONS options=0;IShellItem *item=NULL;
        if(p && p->events && lp && SUCCEEDED(IFileDialog_GetOptions(p->native,&options)) && (options&FOS_SHAREAWARE) &&
            SUCCEEDED(SHCreateItemFromParsingName((PCWSTR)lp,NULL,&IID_IShellItem,(void **)&item))){
            FDE_SHAREVIOLATION_RESPONSE response=v->share(p,item);
            if(response==FDESVR_ACCEPT){*result=OFN_SHAREFALLTHROUGH;return TRUE;}
            if(response==FDESVR_REFUSE){*result=OFN_SHARENOWARN;return TRUE;}}}
    return FALSE;
}

BOOL style_classic_dialog_navigation_begin(style_classic_dialog_proxy *p,BOOL experimental,style_classic_dialog_navigation_scope *scope)
{
    WCHAR *folder=p->pending_folder;p->pending_folder=NULL;
    if(!folder)return FALSE;
    *scope=(style_classic_dialog_navigation_scope){p,folder,NULL,experimental};return TRUE;
}
void style_classic_dialog_navigation_run(HWND w,PCWSTR key,style_classic_dialog_navigation_scope *scope,style_classic_dialog_owned_event_fn changing)
{
    style_classic_dialog_proxy *p=scope->proxy;
    if(scope->experimental && p->events){IShellItem *destination=NULL;
        HRESULT hr=SHCreateItemFromParsingName(scope->folder,NULL,&IID_IShellItem,(void **)&destination);
        if(SUCCEEDED(hr))hr=changing(p,STYLE_CLASSIC_FOLDER_CHANGING,destination,NULL);
        if(hr!=S_OK || p->pending_folder || !IsWindow(w) || GetPropW(w,key)!=p)return;
    }
    style_classic_dialog_proxy_navigate_contents(w,p,key,scope->folder,&scope->filename,scope->experimental);
}
void style_classic_dialog_navigation_cleanup(void *data)
{
    style_classic_dialog_navigation_scope *scope=data;
    if(scope->experimental){scope->proxy->classic_navigating=FALSE;
        if(scope->folder)CoTaskMemFree(scope->folder);
        if(scope->filename)CoTaskMemFree(scope->filename);
    }else{
        if(scope->filename)CoTaskMemFree(scope->filename);
        if(scope->folder)CoTaskMemFree(scope->folder);
    }
}
void style_classic_dialog_initial_run(HWND w,PCWSTR key,style_classic_dialog_initial_scope *scope,
    style_classic_dialog_owned_event_fn changing_owned,style_classic_dialog_navigate_fn navigate,style_classic_dialog_event_fn dispatch)
{
    style_classic_dialog_proxy *p=scope->proxy;
    (void)IShellItem_GetDisplayName(scope->item,SIGDN_FILESYSPATH,&scope->path);
    IShellItem_AddRef(scope->item);p->classic_initializing=TRUE;
    HRESULT changing=changing_owned(p,STYLE_CLASSIC_FOLDER_CHANGING,scope->item,NULL);
    p->classic_initializing=scope->initializing;
    if(changing!=S_OK && !p->pending_folder && p->initial_default && IsWindow(w) && GetPropW(w,key)==p){
        PWSTR fallback=NULL;
        if(SUCCEEDED(IShellItem_GetDisplayName(p->initial_default,SIGDN_FILESYSPATH,&fallback)))p->pending_folder=fallback;
    }
    if(IsWindow(w) && GetPropW(w,key)==p && p->pending_folder)navigate(w,p,key);
    if(IsWindow(w) && GetPropW(w,key)==p && scope->path && p->live_folder!=scope->item){
        if(p->pending_folder)CoTaskMemFree(p->pending_folder);
        p->pending_folder=scope->path;scope->path=NULL;navigate(w,p,key);
    }else if(IsWindow(w) && GetPropW(w,key)==p && p->events)
        (void)dispatch(p,STYLE_CLASSIC_FOLDER_CHANGED,NULL);
    if(IsWindow(w) && GetPropW(w,key)==p && p->events && p->filter)
        (void)dispatch(p,STYLE_CLASSIC_TYPE_CHANGED,NULL);
}
void style_classic_dialog_initial_cleanup(void *data)
{
    style_classic_dialog_initial_scope *scope=data;scope->proxy->classic_initializing=scope->initializing;
    if(scope->path)CoTaskMemFree(scope->path);
    if(scope->item)IShellItem_Release(scope->item);
}
