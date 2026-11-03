#define COBJMACROS
#include "hook_state.h"

void win16_dialog_hook_attach_base(win16_dialog_proxy *p,HWND w,BOOL events)
{p->active_window=w;win16_dialog_proxy_capture_folder(p,events);}
PCWSTR win16_dialog_hook_label(void *owner,int which)
{win16_dialog_proxy *p=owner;return which==0?p->ok_label:which==1?p->cancel_label:p->file_label;}
void win16_dialog_hook_detach(void *owner,HWND w,BOOL events)
{
    win16_dialog_proxy *p=owner;if(p->active_window==w)p->active_window=NULL;
    if(p->live_folder){IShellItem_Release(p->live_folder);p->live_folder=NULL;}
    if(events){
        if(p->live_selection){IShellItem_Release(p->live_selection);p->live_selection=NULL;}
        p->classic_ofn=NULL;p->classic_overwrite_suppressed=FALSE;
    }
    if(p->pending_folder){CoTaskMemFree(p->pending_folder);p->pending_folder=NULL;}
}
