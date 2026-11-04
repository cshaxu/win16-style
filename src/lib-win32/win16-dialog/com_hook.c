/* Classic COM dialog window dispatch; the caller owns proxy state. */
#include "com_hook.h"
#include <dlgs.h>
UINT_PTR win16_dialog_com_hook(HWND w,UINT message,WPARAM wp,LPARAM lp,const win16_dialog_com_hook_services *s)
{
    static const WCHAR key[]=L"Win16Style.T20.ComDialog";
    if(message==WM_INITDIALOG && lp){OPENFILENAMEW *ofn=(OPENFILENAMEW *)lp;void *p=(void *)ofn->lCustData;
        if(p && SetPropW(w,key,p))s->attach(p,w,ofn);
        if(p){PCWSTR label=s->label(p,0);if(label)SetDlgItemTextW(w,IDOK,label);
            label=s->label(p,1);if(label)SetDlgItemTextW(w,IDCANCEL,label);
            label=s->label(p,2);if(label)SetDlgItemTextW(w,stc3,label);}
    }else if(message==WM_COMMAND){UINT_PTR result=0;
        if(s->command(GetPropW(w,key),w,wp,&result))return result;
    }else if(message==WM_NCDESTROY){void *p=GetPropW(w,key);
        if(p){RemovePropW(w,key);s->detach(p,w);}
    }else if(message>=0xc000 && message<=0xffff){
        UINT selection=RegisterWindowMessageW(LBSELCHSTRINGW);UINT_PTR result=0;
        void *p=GetPropW(w,key);
        if(s->before_registered(p,w,message,wp,&result))return result;
        if(selection && message==selection && (wp==lst2 || wp==cmb2 || wp==lst1 || wp==cmb1))s->selection(p,w,wp);
        UINT navigate=RegisterWindowMessageW(s->navigation_message);
        if(navigate && message==navigate){if(p)s->navigate(p,w,key);return 1;}
        if(s->after_registered(p,w,message,lp,&result))return result;
    }
    return 0;
}
