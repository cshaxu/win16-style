#include "frame.h"
#include "input.h"
#include "surface.h"
#include "../menu/host_menu.h"
#include "../window/window_identity.h"

static BOOL local(HWND w)
{
    if(!win32_style_window_current_thread(w)){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    return TRUE;
}
static style_uwp_host *lookup(HWND w)
{
    style_instance *instance=style_instance_lookup(w,style_uwp_host_owner_proc);
    return (style_uwp_host *)instance;
}
static void dispose(void *context)
{
    style_uwp_frame *s=context;
    style_bitmap_dispose(&s->bitmap);
    style_menu_owner_stop(&s->menu);
    s->ops->dispose(s->context);
}
static void before_title_remove(void *context)
{
    style_uwp_frame *s=context;
    if(s->tracking)EndMenu();
}
static void after_title_remove(void *context)
{
    style_uwp_frame *s=context;
    style_menu_owner_stop(&s->menu);
}
static BOOL refresh(void *context)
{return style_uwp_surface_refresh(context);}
static void menu_present(style_uwp_frame *s,HWND window)
{
    if(s->host.instance.detached || !s->tracking || s->menu_window)return;
    style_uwp_menu_present(window,GetSystemMenu(s->host.owner,FALSE),&s->menu_window);
}
static void system_menu(style_uwp_frame *s)
{
    if(s->tracking || s->host.instance.detached)return;
    HMENU menu=GetSystemMenu(s->host.owner,FALSE);RECT surface;
    if(!menu || !GetWindowRect(s->host.surface,&surface))return;
    s->tracking=TRUE;
    SendMessageW(s->host.owner,WM_INITMENU,(WPARAM)menu,0);
    if(s->host.instance.detached || GetSystemMenu(s->host.owner,FALSE)!=menu){s->tracking=FALSE;return;}
    /* TrackPopupMenu does not apply the root window's system-command states.
     * Add only temporary restrictions; retain application-disabled commands. */
    style_system_menu_session session;
    style_system_menu_begin(&session,s->host.owner,menu);
    BOOL actions_ready=TRUE;
    for(unsigned i=0;i<s->action_count;++i){
        style_uwp_frame_action_entry *action=&s->actions[i];
        action->command=style_uwp_menu_add(menu,action->value.label,action);
        if(!action->command){actions_ready=FALSE;break;}
    }
    UINT command=0;unsigned selected=s->action_count;
    /* Never present a replacement menu with a required action missing. */
    if(actions_ready){
        s->no_menu=TRUE;refresh(s);
        if(!s->host.instance.detached)command=style_system_menu_track(&session,s->host.surface,
            (POINT){surface.left+s->menu_anchor.x,surface.top+s->menu_anchor.y});
    }
    if(s->menu_window){
        style_uwp_menu_unpresent(s->menu_window,menu);
        s->menu_window=NULL;
    }
    style_system_menu_end(&session);
    for(unsigned i=0;i<s->action_count;++i){
        style_uwp_frame_action_entry *action=&s->actions[i];
        if(!action->command)continue;
        if(command==action->command)selected=i;
        style_uwp_menu_remove(menu,action->command,action);
        action->command=0;
    }
    s->tracking=FALSE;s->no_menu=FALSE;
    if(!s->host.instance.detached){
        refresh(s);
        if(!s->host.instance.detached && command){
            if(selected<s->action_count)s->actions[selected].value.invoke(s->host.owner,s->actions[selected].value.context);
            else PostMessageW(s->host.owner,WM_SYSCOMMAND,command,0);
        }
    }
}
static BOOL surface_message(void *context,HWND w,UINT message,WPARAM a,LPARAM b,LRESULT *output)
{
    style_uwp_frame *s=context;
    if(s->ops->menu_message(s->context,w,message,a,b,FALSE,output))return TRUE;
    if(message==WM_ENTERIDLE){if(a==MSGF_MENU)menu_present(s,(HWND)b);*output=0;return TRUE;}
    return style_uwp_input_message(s,message,a,b,output);
}
void style_uwp_frame_menu_selection(HWND w,void *context,BOOL selected)
{
    (void)w;style_uwp_frame *s=context;
    if(!s->host.instance.detached){s->no_menu=selected;refresh(s);}
}
static BOOL owner_message(void *context,HWND w,UINT message,WPARAM a,LPARAM b,LRESULT *result)
{
    style_uwp_frame *s=context;
    if(!s->host.instance.detached && message==WM_CANCELMODE && s->tracking)EndMenu();
    if(!s->host.instance.detached && message==WM_SYSCOMMAND &&
       (((a&0xfff0)==SC_KEYMENU && (b==L' '||b==0)) || (a&0xfff0)==SC_MOUSEMENU)){
        system_menu(s);*result=0;return TRUE;
    }
    return !s->host.instance.detached &&
        s->ops->menu_message(s->context,w,message,a,b,TRUE,result);
}
static void owner_activation(void *context,UINT message,WPARAM value)
{style_uwp_frame *s=context;s->active=message==WM_NCACTIVATE?(BOOL)value:LOWORD(value)!=WA_INACTIVE;}
BOOL style_uwp_frame_validate(HWND w,HWND title,const style_uwp_frame_action *actions,unsigned count)
{
    if(count>256 || (count&&!actions)){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    for(unsigned i=0;i<count;++i)if(!actions[i].label || !*actions[i].label || !actions[i].invoke){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    if(!local(w) || (title && !local(title)))return FALSE;
    if(lookup(w)){SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
    if(title && GetParent(title)!=w){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    return TRUE;
}
BOOL style_uwp_frame_start(style_uwp_frame *s,HWND w,HWND title,const style_uwp_frame_ops *ops,void *context,
    const style_uwp_frame_action *actions,unsigned count,style_uwp_frame_action_entry *storage)
{
    s->ops=ops;s->context=context;s->actions=storage;s->action_count=count;
    for(unsigned i=0;i<count;++i)s->actions[i].value=actions[i];
    s->host=(style_uwp_host){.owner=w,.title=title,.surface_message=surface_message,
        .owner_message=owner_message,.owner_activation=owner_activation,
        .before_title_remove=before_title_remove,.after_title_remove=after_title_remove,
        .instance={.context=s,.dispose=dispose,.calls=1}};
    s->active=GetForegroundWindow()==w;
    BOOL ok=style_uwp_host_start(&s->host,refresh);
    DWORD error=GetLastError();
    style_instance_release(&s->host.instance);SetLastError(error);return ok;
}
BOOL style_uwp_frame_remove(HWND w)
{
    if(!local(w))return FALSE;
    style_uwp_host *s=lookup(w);if(!s){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    style_instance_retain(&s->instance);BOOL ok=style_uwp_host_detach(s);style_instance_release(&s->instance);return ok;
}
HWND style_uwp_frame_surface(HWND w)
{
    if(!local(w))return NULL;
    style_uwp_host *s=lookup(w);return s&&!s->instance.detached?s->surface:NULL;
}
static style_uwp_frame *installed_frame(HWND w)
{
    if(!local(w))return NULL;
    style_uwp_host *host=lookup(w);
    return host && !host->instance.detached?host->instance.context:NULL;
}
unsigned style_uwp_frame_profile(HWND w)
{
    const style_uwp_frame *frame=installed_frame(w);
    return frame?frame->profile_key:0;
}
void style_uwp_frame_set_profile(HWND w,unsigned profile)
{
    style_uwp_frame *frame=installed_frame(w);
    if(frame)frame->profile_key=profile;
}
