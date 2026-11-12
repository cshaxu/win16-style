#include "control_scope.h"
#include "control_class.h"
#define class_is ntw16s_class_is

typedef struct control_scope_data {
    win16_style_palette palette;
    win16_style_control_counts counts;
} control_scope_data;
void win16_control_scope_observe(WPARAM ignored,LPARAM b,win16_style_control_scope *scopes,const win16_control_scope_services *v)
{
    (void)ignored;
    const CWPRETSTRUCT *m=(const CWPRETSTRUCT *)b;win16_style_control_scope *s;
    if(m->message==WM_NCDESTROY){
        style_control_scope_root_destroyed(scopes,m->hwnd);
    }else if(m->message==WM_CREATE && m->lResult!=-1){
        HWND ancestor;const CREATESTRUCTW *create=(const CREATESTRUCTW *)m->lParam;
        BOOL edit=class_is(m->hwnd,L"EDIT"),internal=FALSE;
        if(!edit && !class_is(m->hwnd,L"BUTTON") && !class_is(m->hwnd,L"STATIC") &&
            !class_is(m->hwnd,L"LISTBOX") && !class_is(m->hwnd,L"COMBOBOX") &&
            !class_is(m->hwnd,L"SCROLLBAR") && !class_is(m->hwnd,L"SysLink"))goto done;
        /* A combo owns installation of its internal list; do not attach it
         * prematurely while the native combo is still constructing itself. */
        for(ancestor=GetParent(m->hwnd);ancestor;ancestor=GetParent(ancestor))
            if(class_is(ancestor,L"COMBOBOX")){internal=TRUE;break;}
        if(internal)goto done;
        s=style_control_scope_find(scopes,m->hwnd);
        if(s){BOOL installed;DWORD failure;control_scope_data *data=style_control_scope_data(s);
            style_control_scope_retain(s);
            installed=edit?v->edit_install(m->hwnd,(create->style&WS_BORDER)!=0,&data->palette):
                v->control_install(m->hwnd,&data->palette);
            failure=GetLastError();
            if(installed)++data->counts.installed;
            else if(failure==ERROR_NOT_SUPPORTED)++data->counts.unsupported;
            else ++data->counts.failed;
            style_control_scope_release(s);
        }
    }
done:
    return;
}

win16_style_control_scope *win16_control_scope_begin(HWND root,const win16_style_palette *palette,
    win16_style_control_scope **scopes,HHOOK *creation_hook,HOOKPROC observer)
{
    win16_style_control_scope *s=style_control_scope_begin(root,sizeof(control_scope_data),scopes,creation_hook,observer);
    if(!s)return NULL;
    control_scope_data *data=style_control_scope_data(s);
    if(palette)data->palette=*palette;else win16_style_default_palette(&data->palette);
    return s;
}

BOOL win16_control_scope_end(win16_style_control_scope *s,win16_style_control_counts *counts,
    win16_style_control_scope **scopes,HHOOK *creation_hook)
{
    if(!style_control_scope_contains(*scopes,s))return style_control_scope_end(s,scopes,creation_hook);
    win16_style_control_counts value=((control_scope_data *)style_control_scope_data(s))->counts;
    if(!style_control_scope_end(s,scopes,creation_hook))return FALSE;
    if(counts)*counts=value;
    return TRUE;
}
