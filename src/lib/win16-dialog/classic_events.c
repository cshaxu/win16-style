#define COBJMACROS
#include "classic_events.h"
#include "file_helpers.h"
#include <dlgs.h>

BOOL win16_dialog_list_changing(HWND w,win16_dialog_proxy *p,win16_dialog_changing_fn changing)
{
    IShellItem *target=win16_dialog_classic_list_target(w,p->live_folder);
    if(!target)return TRUE;
    HRESULT hr=changing(p,target);
    return hr==S_OK && !p->pending_folder;
}
BOOL win16_dialog_typed_changing(HWND w,win16_dialog_proxy *p,win16_dialog_changing_fn changing)
{
    IShellItem *target=NULL;
    if(!win16_dialog_classic_typed_target(w,p->live_folder,&target))return FALSE;
    if(!target)return TRUE;
    HRESULT hr=changing(p,target);
    return hr==S_OK && !p->pending_folder;
}
BOOL win16_dialog_drive_changing(HWND w,win16_dialog_proxy *p,win16_dialog_changing_fn changing)
{
    LRESULT index;HWND combo;IShellItem *target=win16_dialog_classic_drive_target(w,&index,&combo);
    if(!target)return TRUE;
    HRESULT hr=changing(p,target);
    if(hr!=S_OK || p->pending_folder){if(p->classic_drive_index!=CB_ERR)SendMessageW(combo,CB_SETCURSEL,(WPARAM)p->classic_drive_index,0);return FALSE;}
    p->classic_drive_index=index;return TRUE;
}
BOOL win16_dialog_overwrite_check(HWND w,win16_dialog_proxy *p,win16_dialog_overwrite_fn overwrite)
{
    OPENFILENAMEW *ofn=p->classic_ofn;if(!ofn)return TRUE;
    if(p->classic_overwrite_suppressed){ofn->Flags|=OFN_OVERWRITEPROMPT;p->classic_overwrite_suppressed=FALSE;}
    if(!p->save || !(ofn->Flags&OFN_OVERWRITEPROMPT))return TRUE;
    IShellItem *item=win16_dialog_classic_overwrite_target(w,p->live_folder,p->extension);
    if(!item)return TRUE;
    FDE_OVERWRITE_RESPONSE response=overwrite(p,item);
    if(!IsWindow(w) || p->active_window!=w || p->classic_ofn!=ofn)return FALSE;
    if(response==FDEOR_REFUSE)return FALSE;
    if(response==FDEOR_ACCEPT){ofn->Flags&=~OFN_OVERWRITEPROMPT;p->classic_overwrite_suppressed=TRUE;}
    return TRUE;
}
