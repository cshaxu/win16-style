#include "control_scope.h"

BOOL win16_control_scope_install(style_control_scope *scope,const CWPRETSTRUCT *message,BOOL edit,void *context)
{
    const win16_control_scope_services *services=context;
    const CREATESTRUCTW *create=(const CREATESTRUCTW *)message->lParam;
    const win16_style_palette *palette=style_control_scope_data(scope);
    return edit?services->edit_install(message->hwnd,(create->style&WS_BORDER)!=0,palette):
        services->control_install(message->hwnd,palette);
}
win16_style_control_scope *win16_control_scope_begin(HWND root,const win16_style_palette *palette,
    style_control_scope_manager *manager,HOOKPROC observer)
{
    win16_style_control_scope *scope=style_control_scope_begin(root,sizeof(win16_style_palette),manager,observer);
    if(!scope)return NULL;
    win16_style_palette *data=style_control_scope_data(scope);
    if(palette)*data=*palette;else style_window_palette_default(data);
    return scope;
}
