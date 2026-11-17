/* Classic COM dialog window dispatch; the caller owns proxy state. */
#include "classic_com_hook.h"
#include "classic_dialog_state.h"
#include <dlgs.h>
UINT_PTR style_classic_com_hook(HWND w,UINT message,WPARAM wp,LPARAM lp,const style_classic_com_hook_services *s)
{
    const style_classic_dialog_message_services *messages=s->messages;
    PCWSTR key=messages->property;
    if(message==WM_INITDIALOG && lp){OPENFILENAMEW *ofn=(OPENFILENAMEW *)lp;style_classic_dialog_proxy *p=(style_classic_dialog_proxy *)ofn->lCustData;
        if(p && SetPropW(w,key,p))style_classic_dialog_attach(p,w,ofn,messages);
        if(p){PCWSTR label=style_classic_dialog_hook_label(p,0);if(label)SetDlgItemTextW(w,IDOK,label);
            label=style_classic_dialog_hook_label(p,1);if(label)SetDlgItemTextW(w,IDCANCEL,label);
            label=style_classic_dialog_hook_label(p,2);if(label)SetDlgItemTextW(w,stc3,label);}
    }else if(message==WM_COMMAND){UINT_PTR result=0;
        if(style_classic_dialog_command(GetPropW(w,key),w,wp,&result,messages))return result;
    }else if(message==WM_NCDESTROY){style_classic_dialog_proxy *p=GetPropW(w,key);
        if(p){RemovePropW(w,key);style_classic_dialog_hook_detach(p,w,messages->events_enabled);}
    }else if(message>=0xc000 && message<=0xffff){
        UINT selection=RegisterWindowMessageW(LBSELCHSTRINGW);UINT_PTR result=0;
        style_classic_dialog_proxy *p=GetPropW(w,key);
        if(style_classic_dialog_before(p,w,message,wp,&result,messages))return result;
        if(selection && message==selection && (wp==lst2 || wp==cmb2 || wp==lst1 || wp==cmb1))style_classic_dialog_selection(p,w,wp,messages);
        UINT navigate=RegisterWindowMessageW(s->navigation_message);
        if(navigate && message==navigate){if(p)s->navigate(w,p,key);return 1;}
        if(style_classic_dialog_after(p,w,message,lp,&result,messages))return result;
    }
    return 0;
}
