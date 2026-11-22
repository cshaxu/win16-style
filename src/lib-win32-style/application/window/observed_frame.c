#include "observed_frame.h"
#include "lib-win32-style/application/menu/native_system_menu.h"
#include "lib-win32-style/application/lifecycle/instance.h"
#include <stdlib.h>
#include "lib-win32-style/application/menu/menu_input.h"
#include "lib-win32-style/application/menu/tracking.h"
#include "lib-win32-style/application/menu/bar_controller.h"
#include "lib-win32-style/application/menu/host_menu.h"
#include "lib-win32-style/application/menu/row_text.h"
#include "lib-win32-style/application/window/native_frame.h"
#include "lib-win32-style/application/window/messages.h"
#include "lib-win32-style/application/lifecycle/install.h"
#include "lib-win32-style/application/window/caption_button.h"
#include "lib-win32-style/application/window/frame.h"
#include "lib-win32-style/application/graphics/bitmap.h"
#include "lib-win32-style/application/window/window_identity.h"
#include "lib-win32-style/application/window/window_icon.h"
#include "lib-win32-style/application/adapters/repaint.h"
#include "lib-win32-style/application/adapters/surface.h"
#include <commctrl.h>
#include <dwmapi.h>
#include <oleacc.h>
#include <stdint.h>
#include <windowsx.h>

typedef struct frame_window {
    style_instance instance;
    style_native_frame native;
    style_caption_button button;
    style_menu_owner menu;
    style_native_scroll scroll;
    style_uwp_repaint composed_repaint;
    style_bitmap caption_bitmap;
    HWND caption_surface;
    BOOL painting;
    BOOL composed_icon_update,system_slot,system_on_right,system_popup;
    style_observed_frame_theme theme;
    const style_observed_frame_provider *provider;
} frame_window;
static LRESULT CALLBACK frame_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static frame_window *frame_state(HWND window)
{return (frame_window *)style_instance_lookup(window,frame_proc);}

static int caption_height(HWND window)
{
    UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
    BOOL tool=(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_TOOLWINDOW)!=0;
    frame_window *frame=frame_state(window);
    if(!frame || !frame->theme.present)return style_native_caption_height(dpi,tool);
    return style_native_caption_measure_height(dpi,tool,&frame->theme.caption_metrics,
        frame->theme.border_presentation.geometry.caption_floor);
}

static int caption_client_height(HWND window,UINT dpi)
{
    const frame_window *frame=frame_state(window);
    unsigned separator=frame && frame->theme.present?frame->theme.border_presentation.geometry.separator_units:0;
    return caption_height(window)+(int)separator*max(1,MulDiv(1,(int)dpi,96));
}

/* Preserve the native path for menus needing application-owned rendering. */
static BOOL compact_menu(HWND window)
{
    HMENU menu=GetMenu(window);if(!menu)return TRUE;
    int count=GetMenuItemCount(menu);if(count<1)return FALSE;
    for(int i=0;i<count;++i){
        MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_FTYPE|MIIM_BITMAP};
        if(!GetMenuItemInfoW(menu,(UINT)i,TRUE,&item) ||
           (item.fType&(MFT_OWNERDRAW|MFT_MENUBREAK|MFT_MENUBARBREAK|MFT_SEPARATOR|MFT_RIGHTJUSTIFY)) || item.hbmpItem)
            return FALSE;
    }
    return TRUE;
}

static int menu_shift(HWND window)
{
    UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
    int metric=(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_TOOLWINDOW)?SM_CYSMCAPTION:SM_CYCAPTION;
    int difference=GetSystemMetricsForDpi(metric,dpi)-caption_client_height(window,dpi);
    const frame_window *frame=frame_state(window);
    return frame && frame->theme.sprite_resources?difference:max(0,difference);
}

static BOOL compact_enabled(HWND window)
{
    return !IsIconic(window) && !(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_LAYOUTRTL) && compact_menu(window);
}

static int frame_shift(HWND window,BOOL horizontal)
{
    LONG_PTR style=GetWindowLongPtrW(window,GWL_STYLE);
    if((style&WS_CAPTION)!=WS_CAPTION || IsZoomed(window) || !compact_enabled(window))return 0;
    UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
    const frame_window *frame=frame_state(window);
    const style_frame_geometry *geometry=frame && frame->theme.present?&frame->theme.border_presentation.geometry:NULL;
    if(!geometry || !geometry->scale)return 0;
    enum style_frame_border_kind kind=(style&WS_THICKFRAME)?STYLE_FRAME_SIZEABLE:
        (GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_DLGMODALFRAME)?STYLE_FRAME_DIALOG:STYLE_FRAME_FIXED;
    int target=style_frame_window_border_width(window,geometry,kind,horizontal,dpi);
    return max(0,style_frame_native_border_width(kind,horizontal,dpi)-target);
}

static void offset_menu(HWND window,RECT *rect)
{
    OffsetRect(rect,-frame_shift(window,TRUE),-menu_shift(window)-frame_shift(window,FALSE));
}

static style_menu_bar_inclusion compact_item(void *context,const MENUITEMINFOW *item,UINT *flags)
{
    (void)context;
    (void)flags;
    return (item->fType&(MFT_OWNERDRAW|MFT_MENUBREAK|MFT_MENUBARBREAK|MFT_SEPARATOR|MFT_RIGHTJUSTIFY)) || item->hbmpItem?
        STYLE_MENU_BAR_UNSUPPORTED:STYLE_MENU_BAR_INCLUDE;
}
static BOOL menu_snapshot(HWND window,style_menu_bar_snapshot *bar)
{
    const style_menu_bar_profile profile={.include=compact_item,.native_rects=TRUE};
    return style_menu_bar_read(window,GetMenu(window),(POINT){0,0},0,&profile,NULL,bar);
}
static int menu_item_at(HWND window,POINT point,RECT *bounds)
{
    if(!GetMenu(window) || !compact_enabled(window))return -1;
    style_menu_bar_snapshot bar;if(!menu_snapshot(window,&bar))return -1;
    int found=-1;
    for(UINT i=0;i<bar.count;++i){
        RECT r=bar.rects[i];offset_menu(window,&r);
        if(PtInRect(&r,point)){if(bounds)*bounds=r;found=(int)bar.items[i].position;break;}
    }
    style_menu_bar_free(&bar);return found;
}

static BOOL menu_bar_at(HWND window,POINT point)
{
    MENUBARINFO bar={.cbSize=sizeof(bar)};
    if(!GetMenu(window) || !compact_enabled(window) || !GetMenuBarInfo(window,OBJID_MENU,0,&bar))return FALSE;
    offset_menu(window,&bar.rcBar);return PtInRect(&bar.rcBar,point);
}

static void paint_menu(HWND window,HDC dc,const RECT *outer)
{
    HMENU menu=GetMenu(window);MENUBARINFO bar={.cbSize=sizeof(bar)};
    if(!menu || !compact_enabled(window) || !GetMenuBarInfo(window,OBJID_MENU,0,&bar))return;
    RECT area=bar.rcBar;offset_menu(window,&area);OffsetRect(&area,-outer->left,-outer->top);
    POINT client={0,0};RECT client_rect;
    if(ClientToScreen(window,&client) && client.y-outer->top>area.bottom)area.bottom=client.y-outer->top;
    if(GetClientRect(window,&client_rect)){
        POINT right={client_rect.right,0};
        if(ClientToScreen(window,&right))area.right=max(area.right,right.x-outer->left);
    }
    frame_window *frame=frame_state(window);
    const classic_menu_colors colors=style_menu_system_colors(frame && frame->theme.present?&frame->theme.menu_bar:NULL);
    HBRUSH face=CreateSolidBrush(colors.background);if(!face)return;
    FillRect(dc,&area,face);DeleteObject(face);
    UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
    HFONT font=style_menu_font_acquire(dpi,frame && frame->theme.present?&frame->theme.menu_font:NULL,false);
    unsigned text_flags=frame && frame->theme.present?frame->theme.menu_text_flags:0;
    BOOL centered=!text_flags || (text_flags&STYLE_MENU_TEXT_CENTER_BAR);
    int saved=SaveDC(dc);if(saved){
        if(font)SelectObject(dc,font);
        SetBkMode(dc,TRANSPARENT);
        style_menu_bar_snapshot snapshot;
        int selected=frame?frame->menu.bar.highlighted:-1;BOOL inactive=!frame || !frame->native.active;
        if(menu_snapshot(window,&snapshot)){
            for(UINT i=0;i<snapshot.count;++i){
                const style_menu_bar_item *item=&snapshot.items[i];RECT r=snapshot.rects[i];
                offset_menu(window,&r);OffsetRect(&r,-outer->left,-outer->top);
                BOOL highlighted=selected==(int)item->position;
                if(highlighted){HBRUSH active=CreateSolidBrush(colors.selection);if(active){FillRect(dc,&r,active);DeleteObject(active);}}
                BOOL disabled=inactive || (item->state&MFS_DISABLED);
                COLORREF text=disabled?colors.disabled_text:highlighted?colors.selected_text:colors.text;
                SetTextColor(dc,text);
                (void)style_menu_draw_label(dc,item->text,&r,centered,
                    disabled && (text_flags&STYLE_MENU_TEXT_STIPPLE_DISABLED),text);
            }
            style_menu_bar_free(&snapshot);
        }
        RestoreDC(dc,saved);
    }
    style_menu_font_release(font);
}

static void repaint_menu(HWND window)
{
    RECT outer;if(!GetWindowRect(window,&outer))return;
    HDC dc=GetWindowDC(window);if(!dc)return;
    paint_menu(window,dc,&outer);ReleaseDC(window,dc);
}

static BOOL anchored_caption_buttons(HWND window,const style_frame_sprites *sprites,
    const RECT *caption,const BOOL present[STYLE_NATIVE_CAPTION_BUTTONS],UINT dpi,
    RECT bounds[STYLE_NATIVE_CAPTION_BUTTONS])
{
    RECT outer;
    if(!GetWindowRect(window,&outer))return FALSE;
    RECT extent={max(outer.left,caption->left),outer.top,outer.right,min(caption->bottom,outer.bottom)};
    for(unsigned i=0;i<STYLE_NATIVE_CAPTION_BUTTONS;++i){
        if(!present[i])continue;
        const style_frame_sprite_button *button=&sprites->buttons[CLASSIC_BUTTON_MINIMIZE+i];
        if(!button->anchored)return FALSE;
        LONG x=outer.right+MulDiv(button->top_right.x,(int)dpi,96);
        LONG y=outer.top+MulDiv(button->top_right.y,(int)dpi,96);
        RECT button_rect={x,y,x+MulDiv((int)button->face.width,(int)dpi,96),
            y+MulDiv((int)button->face.height,(int)dpi,96)};
        IntersectRect(&bounds[i],&button_rect,&extent);
    }
    return TRUE;
}

/* TITLEBARINFOEX uses the same order as the shared roles, starting at index 1. */
_Static_assert(CLASSIC_BUTTON_SYSTEM==1 && CLASSIC_BUTTON_MINIMIZE==2 &&
    CLASSIC_BUTTON_MAXIMIZE==3 && CLASSIC_BUTTON_HELP==4 && CLASSIC_BUTTON_CLOSE==CCHILDREN_TITLEBAR,
    "Title bar roles must match TITLEBARINFOEX");

static void classic_buttons(HWND window,const frame_window *frame,TITLEBARINFOEX *info)
{
    LONG_PTR ex=GetWindowLongPtrW(window,GWL_EXSTYLE);
    if(!frame->theme.present || (ex&WS_EX_LAYOUTRTL))return;
    UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
    BOOL present[4];RECT bounds[4],system={0};
    for(unsigned i=0;i<ARRAYSIZE(bounds);++i){
        RECT r=info->rgrect[i+2];
        present[i]=!(info->rgstate[i+2]&STATE_SYSTEM_INVISIBLE) && r.right>r.left;
    }
    BOOL tool=(ex&WS_EX_TOOLWINDOW)!=0;
    BOOL synthetic=frame->system_slot && (GetWindowLongPtrW(window,GWL_STYLE)&WS_SYSMENU);
    if(synthetic){
        MENUBARINFO native={.cbSize=sizeof(native)};
        if(GetMenuBarInfo(window,OBJID_SYSMENU,0,&native) && !IsRectEmpty(&native.rcBar))
            info->rcTitleBar.left=min(info->rcTitleBar.left,native.rcBar.left-frame_shift(window,TRUE));
        SetRectEmpty(&info->rgrect[CLASSIC_BUTTON_SYSTEM]);
        info->rgstate[CLASSIC_BUTTON_SYSTEM]|=STATE_SYSTEM_INVISIBLE;
    }
    BOOL anchored=frame->theme.sprite_resources && !tool && !synthetic && !IsZoomed(window) &&
        anchored_caption_buttons(window,&frame->theme.sprites,&info->rcTitleBar,present,dpi,bounds);
    if(anchored || style_native_caption_buttons_with_system(&info->rcTitleBar,dpi,tool,present,
        &frame->theme.title_buttons,&frame->theme.caption_metrics,bounds,synthetic?&system:NULL)){
        if(!IsRectEmpty(&system)){
            info->rgrect[CLASSIC_BUTTON_SYSTEM]=system;
            info->rgstate[CLASSIC_BUTTON_SYSTEM]&=~STATE_SYSTEM_INVISIBLE;
        }
        for(unsigned i=0;i<ARRAYSIZE(bounds);++i){
            if(!present[i])continue;
            info->rgrect[i+2]=bounds[i];
            if(IsRectEmpty(&bounds[i]))info->rgstate[i+2]|=STATE_SYSTEM_INVISIBLE;
        }
    }
}

static BOOL caption_icon_rect(HWND window,const RECT *outer,RECT *icon)
{
    int dx=frame_shift(window,TRUE),dy=frame_shift(window,FALSE);
    MENUBARINFO info={.cbSize=sizeof(info)};
    if(!GetMenuBarInfo(window,OBJID_SYSMENU,0,&info))return FALSE;
    *icon=info.rcBar;OffsetRect(icon,-outer->left-dx,-outer->top-dy);
    TITLEBARINFOEX title={.cbSize=sizeof(title)};SendMessageW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)&title);
    icon->top=max(icon->top,title.rcTitleBar.top-outer->top);
    icon->bottom=min(icon->bottom,title.rcTitleBar.bottom-outer->top);
    return icon->right>icon->left && icon->bottom>icon->top;
}

static void paint_frame_sprites(HDC dc,RECT caption,int width,int height,UINT dpi,
    const style_frame_sprites *sprites,BOOL active)
{
    int bottom=MulDiv((int)sprites->bottom.height,(int)dpi,96);
    int left=MulDiv((int)sprites->left.width,(int)dpi,96);
    int right=MulDiv((int)sprites->right.width,(int)dpi,96);
    unsigned state=active?0u:1u;
    style_dib_strip_draw(dc,(RECT){0,caption.bottom,left,height-bottom},&sprites->left,state,dpi);
    style_dib_strip_draw(dc,(RECT){width-right,caption.bottom,width,height-bottom},&sprites->right,state,dpi);
    style_dib_strip_draw(dc,(RECT){0,height-bottom,width,height},&sprites->bottom,state,dpi);
}

static void paint(HWND window)
{
    frame_window *frame=frame_state(window);if(!frame || frame->painting)return;
    style_instance_retain(&frame->instance);
    frame->painting=TRUE;
    BOOL presented=FALSE;
    RECT outer;TITLEBARINFOEX info={0};WCHAR title[1024];
    if(IsIconic(window) || (frame->caption_surface && !IsWindowVisible(window)) ||
       !GetWindowRect(window,&outer))goto done;
    info.cbSize=sizeof(info);
    SendMessageW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
    if(frame->instance.detached)goto done;
    if(info.rcTitleBar.right<=info.rcTitleBar.left || info.rcTitleBar.bottom<=info.rcTitleBar.top)goto done;
    RECT caption=info.rcTitleBar;
    OffsetRect(&caption,-outer.left,-outer.top);
    RECT system=info.rgrect[CLASSIC_BUTTON_SYSTEM];
    BOOL synthetic=frame->system_slot && (GetWindowLongPtrW(window,GWL_STYLE)&WS_SYSMENU) &&
        !(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_LAYOUTRTL) &&
        !(info.rgstate[CLASSIC_BUTTON_SYSTEM]&STATE_SYSTEM_INVISIBLE) && !IsRectEmpty(&system);
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE),extended=(DWORD)GetWindowLongPtrW(window,GWL_EXSTYLE);
    RECT icon={0};
    BOOL icon_region=!synthetic && (style&WS_SYSMENU) && !(extended&WS_EX_TOOLWINDOW) &&
        caption_icon_rect(window,&outer,&icon);
    BOOL has_icon=icon_region && style_caption_icon_visible(frame->theme.caption_presentation.icon_mode,
        synthetic,style,extended);
    if(icon_region && icon.left<caption.left)caption.left=icon.left;
    int width=outer.right-outer.left,height=outer.bottom-outer.top;
    if(caption.left<0 || caption.top<0 || caption.right>width || caption.bottom>height)goto done;
    BOOL composed=(extended&WS_EX_NOREDIRECTIONBITMAP) && IsWindowVisible(window) &&
        style_frame_xaml_bridge(window,FALSE);
    HDC dc=GetWindowDC(window);if(!dc && !composed)goto done;
    const style_observed_frame_theme *theme=frame->theme.present?&frame->theme:NULL;
    const style_frame_colors *frame_colors=theme?&theme->colors:NULL;
    BOOL maximized=IsZoomed(window);
    UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
    if(dc && !maximized){
        int border_saved=SaveDC(dc);MENUBARINFO system={.cbSize=sizeof(system)};
        if(border_saved && !synthetic && GetMenuBarInfo(window,OBJID_SYSMENU,0,&system)){
            RECT icon=system.rcBar;OffsetRect(&icon,-outer.left-frame_shift(window,TRUE),-outer.top-frame_shift(window,FALSE));
            ExcludeClipRect(dc,icon.left,icon.top,icon.right,icon.bottom);
        }
        BOOL border_ready=TRUE;
        if(theme && (theme->sprite_resources || theme->border_presentation.fill==STYLE_BORDER_CLIPPED)){
            RECT client;POINT origin={0};
            border_ready=border_saved && GetClientRect(window,&client) && ClientToScreen(window,&origin);
            if(border_ready){
                OffsetRect(&client,origin.x-outer.left,origin.y-outer.top);
                border_ready=ExcludeClipRect(dc,client.left,client.top,client.right,client.bottom)!=ERROR;
            }
        }
        if(border_ready && theme)style_native_frame_border_paint(dc,(RECT){0,0,width,height},caption.top,
            &theme->border_presentation,frame_colors,frame->native.active?theme->active.left:theme->inactive.left,
            (GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_DLGMODALFRAME)!=0);
        if(border_ready && theme && theme->sprite_resources)
            paint_frame_sprites(dc,caption,width,height,dpi,&theme->sprites,frame->native.active);
        if(border_saved)RestoreDC(dc,border_saved);
    }
    if(dc)paint_menu(window,dc,&outer);
    GetWindowTextW(window,title,ARRAYSIZE(title));
    HICON image=has_icon && !frame->instance.detached?style_window_caption_icon(window):NULL;
    if(!frame->instance.detached){
        style_native_caption_layout layout={.band=caption,.icon=has_icon?icon:(RECT){0},
            .left_buttons=theme?style_native_caption_left_buttons(&theme->title_buttons):0,
            .sprites=theme && theme->sprite_resources?&theme->sprites:NULL,.active=frame->native.active};
        if(layout.sprites && !maximized && !composed)
            layout.sprite_band=(RECT){0,0,width,caption.bottom};
        if(synthetic){
            layout.system=system;
            OffsetRect(&layout.system,-outer.left,-outer.top);
            layout.system_on_right=frame->system_on_right;
            if(frame->system_popup || (frame->button.pressed==CLASSIC_BUTTON_SYSTEM && frame->button.hot))
                layout.system_flags=DFCS_PUSHED;
        }
        UINT flags[STYLE_NATIVE_CAPTION_BUTTONS]={0};
        for(unsigned i=0;i<STYLE_NATIVE_CAPTION_BUTTONS;++i){
            unsigned index=i+2;
            if(info.rgstate[index]&STATE_SYSTEM_INVISIBLE)continue;
            layout.buttons[i]=info.rgrect[index];
            OffsetRect(&layout.buttons[i],-outer.left,-outer.top);
            if(info.rgstate[index]&STATE_SYSTEM_UNAVAILABLE)flags[i]|=DFCS_INACTIVE;
            if(info.rgstate[index]&STATE_SYSTEM_HOTTRACKED)flags[i]|=DFCS_HOT;
            if(((unsigned)frame->button.pressed==index && frame->button.hot) ||
               (info.rgstate[index]&STATE_SYSTEM_PRESSED))flags[i]|=DFCS_PUSHED;
        }
        BOOL tool=(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_TOOLWINDOW)!=0;
        const style_font_descriptor *defined=frame->theme.present?
            (tool?&frame->theme.small_caption_font:&frame->theme.caption_font):NULL;
        HFONT font=frame->provider->caption_font(defined,dpi,tool);
        const style_native_caption_colors colors=theme?(frame->native.active?theme->active:theme->inactive):
            style_native_caption_palette(frame->native.active);
        HDC target=dc;
        RECT surface=IsRectEmpty(&layout.sprite_band)?caption:layout.sprite_band;
        SIZE size={surface.right-surface.left,surface.bottom-surface.top};
        POINT origin;
        BOOL buffered=dc && layout.sprites && !composed &&
            style_bitmap_ensure(&frame->caption_bitmap,dc,size) &&
            BitBlt(frame->caption_bitmap.dc,0,0,size.cx,size.cy,dc,surface.left,surface.top,SRCCOPY) &&
            SetViewportOrgEx(frame->caption_bitmap.dc,-surface.left,-surface.top,&origin);
        if(buffered)target=frame->caption_bitmap.dc;
        if(composed && !frame->instance.detached){
            if(!frame->caption_surface)
                frame->caption_surface=CreateWindowExW(WS_EX_LAYERED|WS_EX_TRANSPARENT|WS_EX_NOACTIVATE|WS_EX_TOOLWINDOW,
                    L"STATIC",L"",WS_POPUP,0,0,1,1,window,NULL,GetModuleHandleW(NULL),NULL);
            if(!frame->instance.detached && frame->caption_surface && style_bitmap_ensure(&frame->caption_bitmap,NULL,size) &&
               SetViewportOrgEx(frame->caption_bitmap.dc,-caption.left,-caption.top,&origin))
                target=frame->caption_bitmap.dc;
        }
        if(target && !frame->instance.detached)
            style_native_caption_render(target,&layout,title,image,font?font:GetStockObject(DEFAULT_GUI_FONT),
                dpi,colors,frame_colors,frame->theme.present?&frame->theme.caption_presentation:NULL,maximized,flags,&frame->theme.art);
        if(buffered){
            SetViewportOrgEx(target,origin.x,origin.y,NULL);
            BitBlt(dc,surface.left,surface.top,size.cx,size.cy,target,0,0,SRCCOPY);
        }else if(target && target!=dc){
            SetViewportOrgEx(target,origin.x,origin.y,NULL);
            RECT bounds=caption,hole={0};OffsetRect(&bounds,outer.left,outer.top);
            presented=style_uwp_surface_present(window,frame->caption_surface,&bounds,&hole,
                &frame->caption_bitmap,&frame->instance.detached);
        }
        if(font)DeleteObject(font);
    }
    if(dc)ReleaseDC(window,dc);
done:
    if(frame->caption_surface && !presented)ShowWindow(frame->caption_surface,SW_HIDE);
    frame->painting=FALSE;
    style_instance_release(&frame->instance);
}

static void paint_message(HWND window,void *context,HDC target)
{(void)context;(void)target;paint(window);}

static BOOL bar_read(const style_menu_bar_target *target,style_menu_bar_snapshot *bar)
{
    if(!GetMenu(target->window) || !compact_enabled(target->window) || !menu_snapshot(target->window,bar))return FALSE;
    for(UINT i=0;i<bar->count;++i)offset_menu(target->window,&bar->rects[i]);
    return TRUE;
}
static int bar_hit(const style_menu_bar_target *target,POINT point)
{int item=menu_item_at(target->window,point,NULL);return item<0?STYLE_MENU_NO_ITEM:item;}
static void bar_paint(const style_menu_bar_target *target)
{repaint_menu(target->window);}
static const style_menu_bar_ops bar_ops={.read=bar_read,.hit=bar_hit,.paint=bar_paint,.navigation=STYLE_MENU_TRACK_NATIVE_POPUPS};
static style_menu_bar_target menu_target(HWND w,frame_window *frame)
{return (style_menu_bar_target){w,&frame->instance,&frame->menu,frame,&bar_ops};}

static BOOL system_slot_rect(HWND window,const frame_window *frame,RECT *rect)
{
    if(!frame->system_slot || IsIconic(window) || !(GetWindowLongPtrW(window,GWL_STYLE)&WS_SYSMENU) ||
       (GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_LAYOUTRTL))return FALSE;
    TITLEBARINFOEX info={.cbSize=sizeof(info)};
    SendMessageW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
    *rect=info.rgrect[CLASSIC_BUTTON_SYSTEM];
    return !frame->instance.detached && !(info.rgstate[CLASSIC_BUTTON_SYSTEM]&STATE_SYSTEM_INVISIBLE) &&
        !IsRectEmpty(rect);
}

static void open_system_slot(HWND window,frame_window *frame,RECT rect)
{
    HMENU menu=GetSystemMenu(window,FALSE);
    if(!menu || frame->system_popup)return;
    frame->system_popup=TRUE;
    style_system_menu_session session;
    style_system_menu_begin(&session,window,menu);
    paint(window);
    UINT command=frame->instance.detached?0:
        style_system_menu_track(&session,window,(POINT){rect.left,rect.bottom});
    style_system_menu_end(&session);
    frame->system_popup=FALSE;
    if(!frame->instance.detached){
        paint(window);
        if(command && !frame->instance.detached)PostMessageW(window,WM_SYSCOMMAND,command,0);
    }
}

static BOOL button_input(HWND window,frame_window *frame,UINT message,WPARAM wp,LPARAM lp)
{
    style_caption_button *tracked=&frame->button;
    if((message==WM_NCLBUTTONDOWN || message==WM_NCLBUTTONDBLCLK) && !tracked->pressed){
        if(IsIconic(window))return FALSE;
        enum classic_button_role index=style_caption_role((int)wp);
        if(!index)return FALSE;
        if(index==CLASSIC_BUTTON_SYSTEM){
            RECT rect;if(!system_slot_rect(window,frame,&rect))return FALSE;
            if(message==WM_NCLBUTTONDBLCLK){
                if(style_caption_button_enabled(window,HTCLOSE))PostMessageW(window,WM_SYSCOMMAND,SC_CLOSE,0);
                return TRUE;
            }
        }
        TITLEBARINFOEX info={.cbSize=sizeof(info)};
        SendMessageW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
        if(info.rgstate[index]&(STATE_SYSTEM_INVISIBLE|STATE_SYSTEM_UNAVAILABLE))return FALSE;
        if(!style_caption_button_begin(window,tracked,(int)index,TRUE))return FALSE;
        paint(window);return TRUE;
    }
    if(!tracked->pressed)return FALSE;
    if(message==WM_MOUSEMOVE || message==WM_LBUTTONUP){
        unsigned index=(unsigned)tracked->pressed;TITLEBARINFOEX info={.cbSize=sizeof(info)};
        POINT point={GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
        ClientToScreen(window,&point);SendMessageW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
        BOOL inside=PtInRect(&info.rgrect[index],point);
        if(message==WM_LBUTTONUP){
            (void)style_caption_button_move(window,tracked,inside,TRUE,FALSE);
            if(IsWindow(window))paint(window);
            if(inside && IsWindow(window)){
                UINT command=style_caption_command((enum classic_button_role)index,IsZoomed(window));
                HMENU system=GetSystemMenu(window,FALSE);
                if(index==CLASSIC_BUTTON_SYSTEM)open_system_slot(window,frame,info.rgrect[index]);
                else if(command==SC_CONTEXTHELP || !(GetWindowLongPtrW(window,GWL_STYLE)&WS_SYSMENU) ||
                   (system && !(GetMenuState(system,command,MF_BYCOMMAND)&MFS_DISABLED)))
                    SendMessageW(window,WM_SYSCOMMAND,command,MAKELPARAM(point.x,point.y));
            }
        }else if(inside!=tracked->hot){
            (void)style_caption_button_move(window,tracked,inside,FALSE,FALSE);paint(window);
        }
        return TRUE;
    }
    if(message==WM_CAPTURECHANGED || message==WM_CANCELMODE || message==WM_NCDESTROY){
        (void)style_caption_button_cancel(window,tracked,message);
        if(message!=WM_NCDESTROY && IsWindow(window))paint(window);
    }
    return FALSE;
}

/* The bitmap caption owns hover painting. Resize edges and menus keep
 * their existing native paths. */
static BOOL sprite_caption_hover(HWND window,frame_window *frame,UINT message,WPARAM hit)
{
    if(message==WM_NCMOUSEMOVE &&
       (hit==HTCAPTION || style_caption_role((int)hit)>=CLASSIC_BUTTON_MINIMIZE)){
        WPARAM hover=hit+1;
        if(frame->native.hover!=hover){
            TRACKMOUSEEVENT track={.cbSize=sizeof(track),.dwFlags=TME_LEAVE|TME_NONCLIENT,.hwndTrack=window};
            TrackMouseEvent(&track);
            frame->native.hover=hover;
            paint(window);
        }
        return TRUE;
    }
    if(message==WM_NCMOUSELEAVE && frame->native.hover){
        frame->native.hover=0;
        paint(window);
        return TRUE;
    }
    return FALSE;
}

static void recalculate_frame(HWND window)
{
    SetWindowPos(window,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
    if(IsWindow(window))RedrawWindow(window,NULL,NULL,RDW_INVALIDATE|RDW_ERASE|RDW_ALLCHILDREN);
}

static void repaint_retain(void *context)
{style_instance_retain(&((frame_window *)context)->instance);}
static void repaint_release(void *context)
{style_instance_release(&((frame_window *)context)->instance);}
static void repaint_activate(HWND window,void *context)
{
    frame_window *frame=context;
    if(style_native_frame_take_refresh(&frame->native))recalculate_frame(window);
}
static const style_uwp_repaint_services repaint_services={repaint_retain,repaint_release,repaint_activate};

static void refresh_frame(HWND window,frame_window *frame)
{
    style_uwp_repaint *repaint=&frame->composed_repaint;
    if(!frame->native.refresh_pending || repaint->waiting)return;
    if(style_frame_xaml_bridge(window,FALSE) && !style_frame_xaml_bridge(window,TRUE)){
        repaint->waiting=TRUE;
        repaint->deadline=GetTickCount64()+2000;
        if(style_uwp_repaint_queue(window,repaint,&frame->instance.detached,frame,&repaint_services))return;
        repaint->waiting=FALSE;
    }
    if(!frame->instance.detached)repaint_activate(window,frame);
}

static LRESULT frame_body(HWND window,UINT message,WPARAM wp,LPARAM lp,style_instance *instance)
{
    DWORD incoming=GetLastError();frame_window *frame=(frame_window *)instance;
    if(frame->composed_icon_update &&
       (message==WM_STYLECHANGING || message==WM_STYLECHANGED || message==WM_NCPAINT || message==WM_NCCALCSIZE))
        return DefSubclassProc(window,message,wp,lp);
    if(message==WM_SETICON){
        LRESULT icon_result=0;
        if(style_frame_xaml_icon(window,message,wp,lp,&frame->composed_icon_update,&instance->detached,&icon_result))
            return icon_result;
    }
    if(!(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_LAYOUTRTL)){
        const style_scroll_ops scroll=style_native_scroll_operations(&frame->scroll);
        if(style_scroll_embedded_input(window,instance,NULL,&frame->scroll.controller,&scroll,message,wp,lp)){
            SetLastError(incoming);
            return 0;
        }
        if(message==WM_ENABLE && !wp)
            style_scroll_finish(window,instance,&frame->scroll.controller,&scroll,TRUE);
        if(instance->detached)return 0;
    }
    if(compact_enabled(window) && (message!=STYLE_NCUAHDRAWFRAME || frame->theme.border_presentation.fill ||
       frame->theme.sprite_resources) &&
       style_frame_suppress_native_theme(message,IsIconic(window),GetMenu(window)!=NULL)){
        SetLastError(incoming);return 0;
    }
    if(message==WM_SETTEXT || message==WM_THEMECHANGED || message==WM_DPICHANGED ||
       (!IsIconic(window) && compact_enabled(window) && message==WM_NCACTIVATE)){
        const style_frame_message_ops ops={.context=instance,.paint=paint_message};LRESULT result=0;
        if(style_frame_message(instance,&((frame_window *)instance)->native,message,wp,lp,&ops,&result)){
            if(message==WM_NCACTIVATE)SetLastError(incoming);
            return result;
        }
    }
    if(!IsIconic(window) && compact_enabled(window) && message==WM_NCPAINT){
        HRGN region=style_frame_native_region(window,wp);
        if(region){DefSubclassProc(window,message,(WPARAM)region,lp);DWORD error=GetLastError();
            DeleteObject(region);
            if(!instance->detached && IsWindow(window)){
                paint(window);
                style_native_scroll_paint_nc(window,&frame->scroll,NULL);
                if(IsWindow(window) && compact_enabled(window))style_invalidate_native_menu_overlap(window);
            }
            SetLastError(error);return 0;}
    }
    if(message && message==((frame_window *)instance)->native.refresh_message){
        refresh_frame(window,frame);
        SetLastError(incoming);return 0;
    }
    if((message==WM_NCLBUTTONDOWN || message==WM_NCLBUTTONDBLCLK) && wp==HTBORDER && compact_enabled(window)){
        POINT point={GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
        if(style_menu_route_click(message,wp,menu_bar_at(window,point))==HTMENU){
            if(message==WM_NCLBUTTONDOWN){const style_menu_bar_target t=menu_target(window,frame);style_menu_bar_input(&t,message,HTMENU,lp);}
            SetLastError(incoming);return 0;
        }
    }
    LRESULT menu_result=0;
    if(message==WM_CANCELMODE && frame->system_popup)EndMenu();
    if(message==WM_NCDESTROY)style_menu_owner_stop(&frame->menu);
    else if(style_menu_owner_message(&frame->menu,window,message,wp,
        message==WM_INITMENUPOPUP && frame->system_popup && (HMENU)wp==GetSystemMenu(window,FALSE)?
            MAKELPARAM(LOWORD(lp),TRUE):lp,&style_native_system_menu_ops,
        frame->theme.present?&frame->theme.menu:NULL,&menu_result)){
        SetLastError(incoming);return menu_result;
    }
    const style_menu_bar_target menu=menu_target(window,frame);
    if(message==WM_COMMAND && !lp && frame->menu.bar.keyboard)style_menu_bar_finish(&menu,TRUE);
    style_menu_tracking_update(window,message,wp,lp);
    if(style_menu_bar_input(&menu,message,wp,lp)){SetLastError(incoming);return 0;}
    if(message==WM_SYSCOMMAND && (wp&0xfff0)==SC_KEYMENU && lp==L' '){
        RECT rect;
        if(system_slot_rect(window,frame,&rect)){open_system_slot(window,frame,rect);SetLastError(incoming);return 0;}
    }
    if(button_input(window,(frame_window *)instance,message,wp,lp)){SetLastError(incoming);return 0;}
    if(frame->theme.sprite_resources && (message==WM_NCMOUSEMOVE || message==WM_NCMOUSELEAVE) &&
       compact_enabled(window) && sprite_caption_hover(window,frame,message,wp)){
        SetLastError(incoming);return 0;
    }
    BOOL repaint=FALSE;
    if(message==WM_NCACTIVATE)((frame_window *)instance)->native.active=(BOOL)wp;
    if(message==WM_NCMOUSEMOVE){
        WPARAM hit=wp+1;style_native_frame *native=&((frame_window *)instance)->native;
        if(native->hover!=hit){native->hover=hit;repaint=TRUE;}
    }else if(message==WM_NCMOUSELEAVE){((frame_window *)instance)->native.hover=0;repaint=TRUE;}
    if(message==WM_NCDESTROY){
        RemovePropW(window,frame->provider->attachment_property);
        style_uwp_repaint_cancel(&frame->composed_repaint);
        if(style_instance_detach(instance))style_native_scroll_retire(window,&frame->scroll,TRUE);
    }
    RECT *calculated=message==WM_NCCALCSIZE && lp?
        (wp?&((NCCALCSIZE_PARAMS *)lp)->rgrc[0]:(RECT *)lp):NULL;
    LONG proposed_top=calculated?calculated->top:0;
    SetLastError(incoming);
    LRESULT result=DefSubclassProc(window,message,wp,lp);
    DWORD outcome=GetLastError();
    if(instance->detached){SetLastError(outcome);return result;}
    if(message==WM_SHOWWINDOW && IsWindow(window)){
        if(wp)refresh_frame(window,frame);
        else style_uwp_repaint_cancel(&frame->composed_repaint);
    }
    if(!instance->detached && message==WM_ERASEBKGND && !frame->composed_repaint.repainting &&
       !frame->composed_repaint.pending && style_frame_xaml_bridge(window,FALSE))
        (void)style_uwp_repaint_queue(window,&frame->composed_repaint,&instance->detached,frame,&repaint_services);
    if(instance->detached){SetLastError(outcome);return result;}
    if(calculated && compact_enabled(window) &&
       (GetWindowLongPtrW(window,GWL_STYLE)&WS_CAPTION)==WS_CAPTION &&
       !(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_LAYOUTRTL)){
        UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
        int metric=(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_TOOLWINDOW)?SM_CYSMCAPTION:SM_CYCAPTION;
        int reserved=caption_client_height(window,dpi);
        int difference=GetSystemMetricsForDpi(metric,dpi)-reserved;
        int dx=frame_shift(window,TRUE),dy=frame_shift(window,FALSE);
        RECT client=*calculated;
        if(style_frame_xaml_bridge(window,FALSE)){
            /* Extended client applications can return the whole window here. Reserve
             * only the caption row and let the application lay out its children. */
            frame->native.keep_outer=TRUE;
            BOOL sizeable=(GetWindowLongPtrW(window,GWL_STYLE)&WS_THICKFRAME)!=0;
            LONG caption_end=proposed_top+style_frame_native_border_width(sizeable?STYLE_FRAME_SIZEABLE:STYLE_FRAME_FIXED,FALSE,dpi)-dy+reserved;
            if(caption_end<client.bottom)client.top=max(client.top,caption_end);
        }else{
            int shift=frame->theme.sprite_resources?difference:max(0,difference);
            client.left-=dx;client.right+=dx;client.top-=shift+dy;client.bottom+=dy;
        }
        if(!EqualRect(calculated,&client))result=style_frame_set_client(wp,lp,&client,result);
    }
    if(message==WM_GETTITLEBARINFOEX && lp){
        style_caption_button *tracked=&((frame_window *)instance)->button;
        TITLEBARINFOEX *info=(TITLEBARINFOEX *)lp;
        if(info->cbSize>=sizeof(*info)){
            int dx=frame_shift(window,TRUE),dy=frame_shift(window,FALSE);
            info->rcTitleBar.left-=dx;info->rcTitleBar.right+=dx;
            OffsetRect(&info->rcTitleBar,0,-dy);
            OffsetRect(&info->rgrect[1],-dx,-dy);
            int height=caption_height(window);
            if(compact_enabled(window) &&
               height>0 && (frame->theme.sprite_resources || info->rcTitleBar.bottom-info->rcTitleBar.top>height))
                info->rcTitleBar.bottom=info->rcTitleBar.top+height;
            classic_buttons(window,frame,info);
            if(frame->theme.sprite_resources){
                for(unsigned i=CLASSIC_BUTTON_MINIMIZE;i<=CLASSIC_BUTTON_CLOSE;++i)
                    info->rgstate[i]&=~(STATE_SYSTEM_PRESSED|STATE_SYSTEM_HOTTRACKED);
                if(frame->native.hover && !tracked->pressed){
                    enum classic_button_role role=style_caption_role((int)(frame->native.hover-1));
                    if(role>=CLASSIC_BUTTON_MINIMIZE)info->rgstate[role]|=STATE_SYSTEM_HOTTRACKED;
                }
            }
            if(tracked->pressed>=2 && tracked->pressed<=CCHILDREN_TITLEBAR && tracked->hot)info->rgstate[tracked->pressed]|=STATE_SYSTEM_PRESSED;
        }
    }
    BOOL preserve_hit=message==WM_NCHITTEST && (result==HTTRANSPARENT || result==HTERROR || result==HTNOWHERE || result==HTCLIENT);
    if(message==WM_NCHITTEST && !preserve_hit && compact_enabled(window)){
        RECT client;POINT point={GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
        if(menu_bar_at(window,point))result=style_menu_neutral_hit(HTMENU);
        else if(GetClientRect(window,&client) && ScreenToClient(window,&point) && PtInRect(&client,point))result=HTCLIENT;
        else result=style_menu_neutral_hit(result);
    }
    if(message==WM_NCHITTEST && !preserve_hit && !IsIconic(window) &&
       (GetWindowLongPtrW(window,GWL_STYLE)&WS_CAPTION)==WS_CAPTION){
        TITLEBARINFOEX info={.cbSize=sizeof(info)};POINT point={GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
        SendMessageW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
        unsigned visible=0;
        for(unsigned role=CLASSIC_BUTTON_MINIMIZE;role<=CLASSIC_BUTTON_CLOSE;++role)
            if(!(info.rgstate[role]&STATE_SYSTEM_INVISIBLE))visible|=1u<<role;
        int button=style_caption_hit(info.rgrect+CLASSIC_BUTTON_MINIMIZE,CLASSIC_BUTTON_MINIMIZE,visible,point);
        if(button)result=button;
        MENUBARINFO system={.cbSize=sizeof(system)};int dx=frame_shift(window,TRUE),dy=frame_shift(window,FALSE);
        BOOL icon=FALSE;
        BOOL system_slot=frame->system_slot && !(info.rgstate[CLASSIC_BUTTON_SYSTEM]&STATE_SYSTEM_INVISIBLE);
        BOOL show_icon=style_caption_icon_visible(frame->theme.caption_presentation.icon_mode,system_slot,
            (DWORD)GetWindowLongPtrW(window,GWL_STYLE),(DWORD)GetWindowLongPtrW(window,GWL_EXSTYLE));
        if(show_icon && (dx || dy) && GetMenuBarInfo(window,OBJID_SYSMENU,0,&system)){
            OffsetRect(&system.rcBar,-dx,-dy);
            system.rcBar.top=max(system.rcBar.top,info.rcTitleBar.top);
            system.rcBar.bottom=min(system.rcBar.bottom,info.rcTitleBar.bottom);
            icon=PtInRect(&system.rcBar,point);
        }
        if(!button && (icon || ((system_slot || show_icon) &&
            !(info.rgstate[1]&STATE_SYSTEM_INVISIBLE) && PtInRect(&info.rgrect[1],point))))result=HTSYSMENU;
        else if(!button && (PtInRect(&info.rcTitleBar,point) || (!show_icon && !system_slot && result==HTSYSMENU)))
            result=HTCAPTION;
    }
    if(message==WM_NCPAINT || message==WM_NCACTIVATE || message==WM_SETTEXT || message==WM_SIZE ||
       message==WM_NCLBUTTONDOWN || message==WM_NCLBUTTONUP || message==WM_SETICON ||
       message==WM_STYLECHANGED || message==WM_THEMECHANGED || message==WM_DPICHANGED || repaint ||
       (frame->caption_surface && (message==WM_WINDOWPOSCHANGED || message==WM_SHOWWINDOW)))
        if(IsWindow(window))paint(window);
    if(message==WM_NCPAINT || message==WM_NCACTIVATE || message==WM_SIZE ||
       message==WM_STYLECHANGED || message==WM_ENABLE || message==WM_VSCROLL ||
       message==WM_HSCROLL || message==WM_MOUSEWHEEL || message==WM_MOUSEHWHEEL)
        style_native_scroll_paint_nc(window,&frame->scroll,NULL);
    else if(message==WM_PRINT && wp && (lp&PRF_NONCLIENT) &&
        (!(lp&PRF_CHECKVISIBLE) || IsWindowVisible(window)))
        style_native_scroll_paint_nc(window,&frame->scroll,(HDC)wp);
    if((message==WM_NCPAINT || message==WM_NCACTIVATE) && IsWindow(window) && compact_enabled(window))
        style_invalidate_native_menu_overlap(window);
    SetLastError(outcome);
    return result;
}

static LRESULT CALLBACK frame_proc(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR ref)
{
    (void)id;style_instance *instance=(style_instance *)ref;
    style_instance_retain(instance);
    LRESULT result=frame_body(window,message,wp,lp,instance);DWORD error=GetLastError();
    style_instance_release(instance);SetLastError(error);return result;
}

static LRESULT CALLBACK pending_frame_proc(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR ref)
{
    (void)id;style_instance *instance=(style_instance *)ref;style_instance_retain(instance);
    if(message==WM_NCDESTROY)style_instance_detach(instance);
    LRESULT result=DefSubclassProc(window,message,wp,lp);
    DWORD error=GetLastError();
    if(message==WM_STYLECHANGED && wp==(WPARAM)GWL_STYLE && IsWindow(window)){
        LONG_PTR style=GetWindowLongPtrW(window,GWL_STYLE);
        if(!(style&WS_CHILD) && (style&WS_CAPTION)==WS_CAPTION){
            if(style_instance_detach(instance))style_observed_frame_attach(window,((const frame_window *)instance)->provider);
        }
    }
    style_instance_release(instance);SetLastError(error);return result;
}

static BOOL setup_frame(HWND window,void *context)
{
    frame_window *frame=context;
    /* Keep the existing externally observed attachment marker, not state. */
    if(!SetPropW(window,frame->provider->attachment_property,(HANDLE)1))return FALSE;
    frame->native.active=GetActiveWindow()==window;
    style_native_frame_capture(&frame->native,window,DwmGetWindowAttribute,TRUE);
    enum DWMNCRENDERINGPOLICY policy=DWMNCRP_DISABLED;
    HRESULT status=style_dwm_policy(window,DwmSetWindowAttribute,policy);
    if(FAILED(status)){SetLastError(HRESULT_CODE(status));return FALSE;}
    /* Avoid sending WM_SIZE before the creator can finish initializing its window. */
    UINT message=RegisterWindowMessageW(frame->provider->recalculate_message);
    style_native_frame_queue(&frame->native,window,message);
    return TRUE;
}
static void rollback_frame(HWND window,void *context)
{frame_window *frame=context;RemovePropW(window,frame->provider->attachment_property);style_instance_detach(&frame->instance);}
static void dispose_frame(void *context)
{
    frame_window *frame=context;
    if(frame->caption_surface)DestroyWindow(frame->caption_surface);
    style_bitmap_dispose(&frame->caption_bitmap);
    style_menu_owner_stop(&frame->menu);
    if(frame->theme.sprite_resources)FreeLibrary(frame->theme.sprite_resources);
    free(frame);
}
BOOL style_observed_frame_attach(HWND window,const style_observed_frame_provider *provider)
{
    if(!window || !win32_style_window_current_thread(window))return FALSE;
    LONG_PTR style=GetWindowLongPtrW(window,GWL_STYLE);
    if(style&WS_CHILD)return FALSE;
    if(frame_state(window)){SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
    BOOL pending=(style&WS_CAPTION)!=WS_CAPTION;
    if(pending && style_instance_lookup(window,pending_frame_proc))return TRUE;
    frame_window *frame=calloc(1,sizeof(*frame));
    if(!frame){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    frame->provider=provider;
    style_native_scroll_init(&frame->scroll,&frame->instance);
    DWORD theme_error=pending?ERROR_SUCCESS:provider->load(&frame->theme,&frame->scroll);
    if(theme_error){free(frame);SetLastError(theme_error);return FALSE;}
    if(frame->theme.present)
        frame->system_slot=style_native_caption_system_side(&frame->theme.title_buttons,&frame->system_on_right);
    frame->menu.bar.selected=frame->menu.bar.highlighted=-1;
    style_instance_init(&frame->instance,frame,dispose_frame);
    frame->scroll.controller.resources.frame=TRUE;
    const style_instance_install_ops ops={.context=frame,.setup=pending?NULL:setup_frame,.rollback=rollback_frame};
    return style_instance_install(&frame->instance,window,pending?pending_frame_proc:frame_proc,NULL,&ops);
}

BOOL style_observed_frame_attached(HWND window)
{
    if(!window || !win32_style_window_current_thread(window)){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    return frame_state(window)!=NULL;
}

BOOL style_observed_frame_remove(HWND window)
{
    if(!window || !win32_style_window_current_thread(window)){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    if(!frame_state(window)){
        style_instance *pending=style_instance_lookup(window,pending_frame_proc);
        if(pending){style_instance_retain(pending);BOOL result=style_instance_detach(pending);style_instance_release(pending);return result;}
        SetLastError(ERROR_NOT_FOUND);return FALSE;
    }
    style_instance *instance=style_instance_lookup(window,frame_proc);
    if(!instance)return FALSE;
    style_menu_tracking *tracking=style_menu_tracking_current();
    if((tracking && tracking->window==window) || ((frame_window *)instance)->button.pressed ||
       ((frame_window *)instance)->system_popup){
        SetLastError(ERROR_BUSY);return FALSE;
    }
    RECT client;BOOL normal=!IsIconic(window) && !IsZoomed(window);
    if(normal)style_client_screen(window,&client);
    style_instance_retain(instance);
    if(!style_instance_detach(instance)){style_instance_release(instance);return FALSE;}

    frame_window *frame=(frame_window *)instance;
    RemovePropW(window,frame->provider->attachment_property);
    style_uwp_repaint_cancel(&frame->composed_repaint);
    style_native_scroll_retire(window,&frame->scroll,TRUE);
    if(frame->menu.bar.keyboard){const style_menu_bar_target menu=menu_target(window,frame);style_menu_bar_finish(&menu,TRUE);}
    style_menu_owner_stop(&frame->menu);
    HRESULT status=style_native_frame_restore_policy(window,&frame->native,DwmSetWindowAttribute);
    if(FAILED(status)){style_instance_release(instance);SetLastError(HRESULT_CODE(status));return FALSE;}
    style_native_frame_restore_geometry(window,&frame->native,&client,normal);
    BOOL enabled=FALSE;
    if(frame->native.saved && frame->native.policy==DWMNCRP_ENABLED &&
       IsWindow(window) && !frame_state(window) &&
       SUCCEEDED(DwmGetWindowAttribute(window,DWMWA_NCRENDERING_ENABLED,&enabled,sizeof(enabled))) && !enabled){
        /* A classic nonclient paint can leave USER's theme state cached after
         * DWM accepts the original policy. Refresh only the detached window. */
        SendMessageW(window,WM_THEMECHANGED,0,0);
    }
    style_instance_release(instance);return TRUE;
}
