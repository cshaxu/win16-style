/* Shell folder layout selection; unsupported callback contracts stay native. */
#include "browse.h"
#define BRIDGE_FLAGS (BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE | BIF_EDITBOX | BIF_VALIDATE)
win16_dialog_browse_plan win16_dialog_plan_browse(UINT flags,BOOL has_callback)
{
    win16_dialog_browse_plan plan={WIN16_BROWSE_NATIVE,flags};
    if(has_callback)return plan;
    if(flags==(BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE) ||
        flags==(BIF_RETURNONLYFSDIRS|BIF_NEWDIALOGSTYLE|BIF_NONEWFOLDERBUTTON)){
        plan.mode=WIN16_BROWSE_REWRITE;plan.flags=BIF_RETURNONLYFSDIRS;return plan;
    }
    if(flags==(BRIDGE_FLAGS|BIF_NONEWFOLDERBUTTON)){
        plan.mode=WIN16_BROWSE_VALIDATE;plan.flags=flags & ~(BIF_NEWDIALOGSTYLE|BIF_NONEWFOLDERBUTTON);return plan;
    }
    if(flags!=BRIDGE_FLAGS && flags!=(BRIDGE_FLAGS & ~BIF_VALIDATE))return plan;
    plan.mode=WIN16_BROWSE_REWRITE;plan.flags=BIF_RETURNONLYFSDIRS;
    return plan;
}
int CALLBACK win16_dialog_folder_selection_callback(HWND window,UINT message,LPARAM value,LPARAM data)
{
    (void)window;win16_dialog_folder_selection *ctx=(win16_dialog_folder_selection *)data;
    if(message==BFFM_SELCHANGED){PIDLIST_ABSOLUTE copy=value?ILCloneFull((PCIDLIST_ABSOLUTE)value):NULL;if(ctx->selected)CoTaskMemFree(ctx->selected);ctx->selected=copy;}
    if(message==BFFM_VALIDATEFAILEDA || message==BFFM_VALIDATEFAILEDW)ctx->invalid=TRUE;
    return 0;
}
#define BROWSE_ADAPT(name,type,native_type) \
PIDLIST_ABSOLUTE name(type *info,native_type native,BOOL enabled) { \
    if(!enabled || !info)return native(info); \
    win16_dialog_browse_plan plan=win16_dialog_plan_browse(info->ulFlags,info->lpfn!=NULL); \
    if(plan.mode==WIN16_BROWSE_NATIVE)return native(info); \
    type copy=*info;copy.ulFlags=plan.flags; \
    if(plan.mode==WIN16_BROWSE_VALIDATE){win16_dialog_folder_selection ctx={0}; \
        copy.lpfn=win16_dialog_folder_selection_callback;copy.lParam=(LPARAM)&ctx; \
        PIDLIST_ABSOLUTE result=native(&copy); \
        if(!result && ctx.invalid && ctx.selected){result=ctx.selected;ctx.selected=NULL;} \
        if(ctx.selected){CoTaskMemFree(ctx.selected);}return result;} \
    return native(&copy); \
}
BROWSE_ADAPT(win16_dialog_browse_a,BROWSEINFOA,win16_dialog_browse_native_a)
BROWSE_ADAPT(win16_dialog_browse_w,BROWSEINFOW,win16_dialog_browse_native_w)
