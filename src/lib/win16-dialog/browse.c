/* Shell folder layout selection; unsupported callback contracts stay native. */
#include "browse.h"
#define BRIDGE_FLAGS (BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_VALIDATE)
win16_dialog_browse_plan win16_dialog_plan_browse(UINT flags,BOOL has_callback)
{
    win16_dialog_browse_plan plan={WIN16_BROWSE_NATIVE,flags};
    if(has_callback && (flags==BRIDGE_FLAGS || flags==(BRIDGE_FLAGS & ~BIF_VALIDATE)))return plan;
    if(!has_callback && (flags==(BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE) ||
        flags==(BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE|BIF_NONEWFOLDERBUTTON))){
        plan.mode=WIN16_BROWSE_REWRITE;plan.flags=BIF_RETURNONLYFSDIRS;return plan;
    }
    if(!has_callback && flags==(BRIDGE_FLAGS|BIF_NONEWFOLDERBUTTON)){
        plan.mode=WIN16_BROWSE_VALIDATE;plan.flags=flags & ~(BIF_NEWDIALOGSTYLE|BIF_NONEWFOLDERBUTTON);return plan;
    }
    if(flags!=BRIDGE_FLAGS && flags!=(BRIDGE_FLAGS & ~BIF_VALIDATE))return plan;
    plan.mode=WIN16_BROWSE_REWRITE;plan.flags=has_callback?(flags & ~BIF_NEWDIALOGSTYLE):BIF_RETURNONLYFSDIRS;
    return plan;
}
int CALLBACK win16_dialog_folder_selection_callback(HWND window,UINT message,LPARAM value,LPARAM data)
{
    (void)window;win16_dialog_folder_selection *ctx=(win16_dialog_folder_selection *)data;
    if(message==BFFM_SELCHANGED){PIDLIST_ABSOLUTE copy=value?ILCloneFull((PCIDLIST_ABSOLUTE)value):NULL;if(ctx->selected)CoTaskMemFree(ctx->selected);ctx->selected=copy;}
    if(message==BFFM_VALIDATEFAILEDA || message==BFFM_VALIDATEFAILEDW)ctx->invalid=TRUE;
    return 0;
}
