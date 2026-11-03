#include "controls.h"
#include "../lib/win16-control/control_scope.h"

static _Thread_local win16_style_control_scope *scopes;
static _Thread_local HHOOK creation_hook;
static const win16_control_scope_services services={win16_style_edit_install,win16_style_control_install};
static LRESULT CALLBACK created(int code,WPARAM a,LPARAM b)
{
    LRESULT result=CallNextHookEx(NULL,code,a,b);DWORD error=GetLastError();
    if(code<0)return result;
    win16_control_scope_observe(a,b,scopes,&services);
    SetLastError(error);return result;
}
win16_style_control_scope *win16_style_controls_begin(HWND root,const win16_style_palette *palette)
{return win16_control_scope_begin(root,palette,&scopes,&creation_hook,created);}
BOOL win16_style_controls_end(win16_style_control_scope *scope,win16_style_control_counts *counts)
{return win16_control_scope_end(scope,counts,&scopes,&creation_hook);}
