#include "frame.h"
#include "lib-win32-style/application/lifecycle/instance.h"
#include <stdlib.h>
#include "caption.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "system_menu.h"
#include "../control/scroll.h"
#include "lib-win32-style/application/menu/menu_input.h"
#include "lib-win32-style/application/menu/tracking.h"
#include "lib-win32-style/application/menu/bar_controller.h"
#include "lib-win32-style/application/window/native_frame.h"
#include "lib-win32-style/application/window/messages.h"
#include "lib-win32-style/application/lifecycle/install.h"
#include "lib-win32-style/application/window/caption_button.h"
#include "lib-win32-style/application/window/frame.h"
#include "lib-win32-style/application/graphics/bitmap.h"
#include "lib-win32-style/application/window/window_identity.h"
#include "lib-win32-style/application/window/window_icon.h"
#include "lib-win32-style/application/adapters/repaint.h"
#include <commctrl.h>
#include <dwmapi.h>
#include <oleacc.h>
#include <shellscalingapi.h>
#include <stdint.h>
#include <windowsx.h>

static const WCHAR frame_property[]=L"Win2kStyle.Frame";
extern IMAGE_DOS_HEADER __ImageBase;
typedef struct frame_window {
    style_instance instance;
    style_native_frame native;
    style_caption_button button;
    style_menu_owner menu;
    win2k_scroll scroll;
    style_uwp_repaint composed_repaint;
    BOOL composed_icon_update;
    BOOL themed;
    classic_theme theme;
} frame_window;
static LRESULT CALLBACK frame_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static frame_window *frame_state(HWND window)
{return (frame_window *)style_instance_lookup(window,frame_proc);}

static int caption_height(HWND window)
{
    UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
    BOOL tool=(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_TOOLWINDOW)!=0;
    frame_window *frame=frame_state(window);
    return win2k_caption_theme_height(dpi,tool,frame && frame->themed?&frame->theme:NULL);
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
    return max(0,GetSystemMetricsForDpi(metric,dpi)-caption_height(window));
}

static BOOL compact_enabled(HWND window)
{
    return !IsIconic(window) && !(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_LAYOUTRTL) && compact_menu(window);
}

static int frame_metric(BOOL sizeable,BOOL horizontal,UINT dpi)
{
    int metric=GetSystemMetricsForDpi(sizeable?(horizontal?SM_CXSIZEFRAME:SM_CYSIZEFRAME):
        (horizontal?SM_CXFIXEDFRAME:SM_CYFIXEDFRAME),dpi);
    return sizeable?metric+GetSystemMetricsForDpi(SM_CXPADDEDBORDER,dpi):metric;
}

static int frame_shift(HWND window,BOOL horizontal)
{
    LONG_PTR style=GetWindowLongPtrW(window,GWL_STYLE);
    if((style&WS_CAPTION)!=WS_CAPTION || IsZoomed(window) || !compact_enabled(window))return 0;
    UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
    UINT x=dpi,y=dpi;DWORD error=GetLastError();
    DPI_AWARENESS_CONTEXT prior=SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    if(prior){
        HMONITOR monitor=MonitorFromWindow(window,MONITOR_DEFAULTTONEAREST);
        if(FAILED(GetDpiForMonitor(monitor,MDT_EFFECTIVE_DPI,&x,&y)) || !x || !y)x=y=dpi;
        SetThreadDpiAwarenessContext(prior);
    }
    SetLastError(error);
    BOOL sizeable=(style&WS_THICKFRAME)!=0;
    int target=max(1,MulDiv(sizeable?4:3,(int)dpi,(int)(horizontal?x:y)));
    return max(0,frame_metric(sizeable,horizontal,dpi)-target);
}

static void offset_menu(HWND window,RECT *rect)
{
    OffsetRect(rect,-frame_shift(window,TRUE),-menu_shift(window)-frame_shift(window,FALSE));
}

static style_menu_bar_inclusion compact_item(const MENUITEMINFOW *item,UINT *flags)
{
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
    const classic_menu_colors *colors=win2k_menu_colors(frame && frame->themed?&frame->theme.menu_bar:NULL);
    HBRUSH face=CreateSolidBrush(colors->background);if(!face)return;
    FillRect(dc,&area,face);DeleteObject(face);
    UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
    HFONT font=win2k_menu_font(dpi,frame && frame->themed?&frame->theme.menu_font:NULL,FALSE);
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
                if(highlighted){HBRUSH active=CreateSolidBrush(colors->selection);if(active){FillRect(dc,&r,active);DeleteObject(active);}}
                SetTextColor(dc,(inactive || (item->state&MFS_DISABLED))?colors->disabled_text:
                    highlighted?colors->selected_text:colors->text);
                DrawTextW(dc,item->text,min(511,lstrlenW(item->text)),&r,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
            }
            style_menu_bar_free(&snapshot);
        }
        RestoreDC(dc,saved);
    }
    if(font)DeleteObject(font);
}

static void repaint_menu(HWND window)
{
    RECT outer;if(!GetWindowRect(window,&outer))return;
    HDC dc=GetWindowDC(window);if(!dc)return;
    paint_menu(window,dc,&outer);ReleaseDC(window,dc);
}

static void classic_buttons(HWND window,const frame_window *frame,TITLEBARINFOEX *info)
{
    LONG_PTR ex=GetWindowLongPtrW(window,GWL_EXSTYLE);
    if(ex&WS_EX_LAYOUTRTL)return;
    UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
    BOOL present[4];RECT bounds[4];
    for(unsigned i=0;i<ARRAYSIZE(bounds);++i){
        RECT r=info->rgrect[i+2];
        present[i]=!(info->rgstate[i+2]&STATE_SYSTEM_INVISIBLE) && r.right>r.left;
    }
    BOOL tool=(ex&WS_EX_TOOLWINDOW)!=0;
    BOOL result=frame->themed?win2k_caption_buttons_with_group(&info->rcTitleBar,dpi,tool,present,
        &frame->theme.title_buttons,&frame->theme.caption_metrics,bounds):
        win2k_caption_buttons(&info->rcTitleBar,dpi,tool,present,bounds);
    if(result)
        for(unsigned i=0;i<ARRAYSIZE(bounds);++i)if(present[i])info->rgrect[i+2]=bounds[i];
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

static void paint(HWND window)
{
    frame_window *frame=frame_state(window);if(!frame)return;
    style_instance_retain(&frame->instance);
    RECT outer;TITLEBARINFOEX info={0};WCHAR title[1024];
    if(IsIconic(window) || !GetWindowRect(window,&outer))goto done;
    info.cbSize=sizeof(info);
    SendMessageW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
    if(frame->instance.detached)goto done;
    if(info.rcTitleBar.right<=info.rcTitleBar.left || info.rcTitleBar.bottom<=info.rcTitleBar.top)goto done;
    RECT caption=info.rcTitleBar;
    OffsetRect(&caption,-outer.left,-outer.top);
    RECT icon={0};BOOL has_icon=(GetWindowLongPtrW(window,GWL_STYLE)&WS_SYSMENU) &&
        !(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_TOOLWINDOW) && caption_icon_rect(window,&outer,&icon);
    if(has_icon && icon.left<caption.left)caption.left=icon.left;
    int width=outer.right-outer.left,height=outer.bottom-outer.top;
    if(caption.left<0 || caption.top<0 || caption.right>width || caption.bottom>height)goto done;
    HDC dc=GetWindowDC(window);if(!dc)goto done;
    const classic_frame_colors *frame_colors=frame->themed?&frame->theme.frame:NULL;
    BOOL maximized=IsZoomed(window);
    if(!maximized){
        int border_saved=SaveDC(dc);MENUBARINFO system={.cbSize=sizeof(system)};
        if(border_saved && GetMenuBarInfo(window,OBJID_SYSMENU,0,&system)){
            RECT icon=system.rcBar;OffsetRect(&icon,-outer.left-frame_shift(window,TRUE),-outer.top-frame_shift(window,FALSE));
            ExcludeClipRect(dc,icon.left,icon.top,icon.right,icon.bottom);
        }
        win2k_window_border_paint(dc,width,height,caption.top,frame_colors);
        if(border_saved)RestoreDC(dc,border_saved);
    }
    paint_menu(window,dc,&outer);
    GetWindowTextW(window,title,ARRAYSIZE(title));
    HICON image=has_icon && !frame->instance.detached?style_window_caption_icon(window):NULL;
    if(!frame->instance.detached){
        win2k_caption_layout layout={.band=caption,.icon=has_icon?icon:(RECT){0}};
        UINT flags[WIN2K_CAPTION_PARTS]={0};
        for(unsigned i=0;i<WIN2K_CAPTION_PARTS;++i){
            unsigned index=i+2;
            if(info.rgstate[index]&STATE_SYSTEM_INVISIBLE)continue;
            layout.buttons[i]=info.rgrect[index];
            OffsetRect(&layout.buttons[i],-outer.left,-outer.top);
            if(info.rgstate[index]&STATE_SYSTEM_UNAVAILABLE)flags[i]|=DFCS_INACTIVE;
            if(((unsigned)frame->button.pressed==index && frame->button.hot) ||
               (info.rgstate[index]&STATE_SYSTEM_PRESSED))flags[i]|=DFCS_PUSHED;
        }
        UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
        BOOL tool=(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_TOOLWINDOW)!=0;
        const msstyles_font *defined=frame->themed?
            (tool?&frame->theme.small_caption_font:&frame->theme.caption_font):NULL;
        HFONT font=win2k_caption_font(dpi,tool,defined);
        win2k_caption_colors colors=win2k_caption_theme_palette(frame->native.active,
            frame->themed?&frame->theme:NULL);
        win2k_caption_render_with_colors(dc,&layout,title,image,font?font:GetStockObject(DEFAULT_GUI_FONT),
            dpi,colors,frame_colors,maximized,flags);
        if(font)DeleteObject(font);
    }
    ReleaseDC(window,dc);
done:
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

static BOOL button_input(HWND window,frame_window *frame,UINT message,WPARAM wp,LPARAM lp)
{
    style_caption_button *tracked=&frame->button;
    if((message==WM_NCLBUTTONDOWN || message==WM_NCLBUTTONDBLCLK) && !tracked->pressed){
        if(IsIconic(window))return FALSE;
        unsigned index=wp==HTMINBUTTON?2:wp==HTMAXBUTTON?3:wp==HTHELP?4:wp==HTCLOSE?5:0;
        if(!index)return FALSE;
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
                UINT command=index==2?SC_MINIMIZE:index==3?(IsZoomed(window)?SC_RESTORE:SC_MAXIMIZE):
                    index==4?SC_CONTEXTHELP:SC_CLOSE;
                HMENU system=GetSystemMenu(window,FALSE);
                if(command==SC_CONTEXTHELP || !(GetWindowLongPtrW(window,GWL_STYLE)&WS_SYSMENU) ||
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
        const style_scroll_ops scroll=win2k_scroll_operations(&frame->scroll);
        if(style_scroll_embedded_input(window,instance,NULL,&frame->scroll.controller,&scroll,message,wp,lp)){
            SetLastError(incoming);
            return 0;
        }
        if(message==WM_ENABLE && !wp)
            style_scroll_finish(window,instance,&frame->scroll.controller,&scroll,TRUE);
        if(instance->detached)return 0;
    }
    if(compact_enabled(window) && style_frame_suppress_native_theme(message,IsIconic(window),GetMenu(window)!=NULL)){
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
                win2k_scroll_paint_nc(window,&frame->scroll,NULL);
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
    if(message==WM_NCDESTROY)style_menu_owner_stop(&frame->menu);
    else if(style_menu_owner_message(&frame->menu,window,message,wp,lp,&win2k_system_menu_operations,
        frame->themed?&frame->theme:NULL,&menu_result)){
        SetLastError(incoming);return menu_result;
    }
    const style_menu_bar_target menu=menu_target(window,frame);
    if(message==WM_COMMAND && !lp && frame->menu.bar.keyboard)style_menu_bar_finish(&menu,TRUE);
    style_menu_tracking_update(window,message,wp,lp);
    if(style_menu_bar_input(&menu,message,wp,lp)){SetLastError(incoming);return 0;}
    if(button_input(window,(frame_window *)instance,message,wp,lp)){SetLastError(incoming);return 0;}
    BOOL repaint=FALSE;
    if(message==WM_NCACTIVATE)((frame_window *)instance)->native.active=(BOOL)wp;
    if(message==WM_NCMOUSEMOVE){
        WPARAM hit=wp+1;style_native_frame *native=&((frame_window *)instance)->native;
        if(native->hover!=hit){native->hover=hit;repaint=TRUE;}
    }else if(message==WM_NCMOUSELEAVE){((frame_window *)instance)->native.hover=0;repaint=TRUE;}
    if(message==WM_NCDESTROY){
        RemovePropW(window,frame_property);
        style_uwp_repaint_cancel(&frame->composed_repaint);
        if(style_instance_detach(instance))win2k_scroll_retire(window,&frame->scroll,TRUE);
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
        int difference=GetSystemMetricsForDpi(metric,dpi)-caption_height(window);
        int dx=frame_shift(window,TRUE),dy=frame_shift(window,FALSE);
        RECT client=*calculated;
        if(style_frame_xaml_bridge(window,FALSE)){
            /* Extended client applications can return the whole window here. Reserve
             * only the caption row and let the application lay out its children. */
            frame->native.keep_outer=TRUE;
            BOOL sizeable=(GetWindowLongPtrW(window,GWL_STYLE)&WS_THICKFRAME)!=0;
            LONG caption_end=proposed_top+frame_metric(sizeable,FALSE,dpi)-dy+caption_height(window);
            if(caption_end<client.bottom)client.top=max(client.top,caption_end);
        }else{
            client.left-=dx;client.right+=dx;client.top-=max(0,difference)+dy;client.bottom+=dy;
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
               height>0 && info->rcTitleBar.bottom-info->rcTitleBar.top>height)
                info->rcTitleBar.bottom=info->rcTitleBar.top+height;
            classic_buttons(window,frame,info);
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
        static const LRESULT hits[]={0,0,HTMINBUTTON,HTMAXBUTTON,HTHELP,HTCLOSE};
        BOOL button=FALSE;
        for(unsigned i=2;i<ARRAYSIZE(hits);++i)
            if(!(info.rgstate[i]&STATE_SYSTEM_INVISIBLE) && PtInRect(&info.rgrect[i],point)){result=hits[i];button=TRUE;break;}
        MENUBARINFO system={.cbSize=sizeof(system)};int dx=frame_shift(window,TRUE),dy=frame_shift(window,FALSE);
        BOOL icon=FALSE;
        if((dx || dy) && GetMenuBarInfo(window,OBJID_SYSMENU,0,&system)){
            OffsetRect(&system.rcBar,-dx,-dy);
            system.rcBar.top=max(system.rcBar.top,info.rcTitleBar.top);
            system.rcBar.bottom=min(system.rcBar.bottom,info.rcTitleBar.bottom);
            icon=PtInRect(&system.rcBar,point);
        }
        if(!button && (icon || (!(info.rgstate[1]&STATE_SYSTEM_INVISIBLE) && PtInRect(&info.rgrect[1],point))))result=HTSYSMENU;
        else if(!button && PtInRect(&info.rcTitleBar,point))result=HTCAPTION;
    }
    if(message==WM_NCPAINT || message==WM_NCACTIVATE || message==WM_SETTEXT || message==WM_SIZE ||
       message==WM_NCLBUTTONDOWN || message==WM_NCLBUTTONUP || message==WM_SETICON ||
       message==WM_STYLECHANGED || message==WM_THEMECHANGED || message==WM_DPICHANGED || repaint)
        if(IsWindow(window))paint(window);
    if(message==WM_NCPAINT || message==WM_NCACTIVATE || message==WM_SIZE ||
       message==WM_STYLECHANGED || message==WM_ENABLE || message==WM_VSCROLL ||
       message==WM_HSCROLL || message==WM_MOUSEWHEEL || message==WM_MOUSEHWHEEL)
        win2k_scroll_paint_nc(window,&frame->scroll,NULL);
    else if(message==WM_PRINT && wp && (lp&PRF_NONCLIENT) &&
        (!(lp&PRF_CHECKVISIBLE) || IsWindowVisible(window)))
        win2k_scroll_paint_nc(window,&frame->scroll,(HDC)wp);
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
            if(style_instance_detach(instance))win2k_frame_attach(window);
        }
    }
    style_instance_release(instance);SetLastError(error);return result;
}

static BOOL setup_frame(HWND window,void *context)
{
    frame_window *frame=context;
    /* Keep the existing externally observed attachment marker, not state. */
    if(!SetPropW(window,frame_property,(HANDLE)1))return FALSE;
    frame->native.active=GetActiveWindow()==window;
    style_native_frame_capture(&frame->native,window,DwmGetWindowAttribute,TRUE);
    enum DWMNCRENDERINGPOLICY policy=DWMNCRP_DISABLED;
    HRESULT status=style_dwm_policy(window,DwmSetWindowAttribute,policy);
    if(FAILED(status)){SetLastError(HRESULT_CODE(status));return FALSE;}
    /* Avoid sending WM_SIZE before the creator can finish initializing its window. */
    UINT message=RegisterWindowMessageW(L"Win2kStyle.RecalculateFrame");
    style_native_frame_queue(&frame->native,window,message);
    return TRUE;
}
static void rollback_frame(HWND window,void *context)
{frame_window *frame=context;RemovePropW(window,frame_property);style_instance_detach(&frame->instance);}
static void dispose_frame(void *context)
{
    frame_window *frame=context;style_menu_owner_stop(&frame->menu);free(frame);
}
BOOL win2k_frame_attach(HWND window)
{
    if(!window || !win32_style_window_current_thread(window))return FALSE;
    LONG_PTR style=GetWindowLongPtrW(window,GWL_STYLE);
    if(style&WS_CHILD)return FALSE;
    if(frame_state(window)){SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
    BOOL pending=(style&WS_CAPTION)!=WS_CAPTION;
    if(pending && style_instance_lookup(window,pending_frame_proc))return TRUE;
    frame_window *frame=calloc(1,sizeof(*frame));
    if(!frame){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    frame->themed=!pending && classic_theme_default((HMODULE)&__ImageBase,&frame->theme);
    frame->menu.bar.selected=frame->menu.bar.highlighted=-1;
    style_instance_init(&frame->instance,frame,dispose_frame);
    win2k_scroll_init(&frame->scroll,&frame->instance);
    if(frame->themed)win2k_scroll_theme(&frame->scroll,&frame->theme);
    frame->scroll.controller.resources.frame=TRUE;
    const style_instance_install_ops ops={.context=frame,.setup=pending?NULL:setup_frame,.rollback=rollback_frame};
    return style_instance_install(&frame->instance,window,pending?pending_frame_proc:frame_proc,NULL,&ops);
}

BOOL win2k_frame_attached(HWND window)
{
    if(!window || !win32_style_window_current_thread(window)){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    return frame_state(window)!=NULL;
}

BOOL win2k_frame_remove(HWND window)
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
    if((tracking && tracking->window==window) || ((frame_window *)instance)->button.pressed){
        SetLastError(ERROR_BUSY);return FALSE;
    }
    RECT client;BOOL normal=!IsIconic(window) && !IsZoomed(window);
    if(normal)style_client_screen(window,&client);
    style_instance_retain(instance);
    if(!style_instance_detach(instance)){style_instance_release(instance);return FALSE;}

    RemovePropW(window,frame_property);
    frame_window *frame=(frame_window *)instance;
    style_uwp_repaint_cancel(&frame->composed_repaint);
    win2k_scroll_retire(window,&frame->scroll,TRUE);
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
