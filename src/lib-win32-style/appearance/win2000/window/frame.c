#include "frame.h"
#include "lib-win32-style/application/lifecycle/instance.h"
#include <stdlib.h>
#include "caption.h"
#include "system_menu.h"
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
#include <commctrl.h>
#include <dwmapi.h>
#include <oleacc.h>
#include <shellscalingapi.h>
#include <stdint.h>
#include <windowsx.h>

static const WCHAR frame_property[]=L"Win2kStyle.Frame";
typedef struct frame_window {
    style_instance instance;
    style_native_frame native;
    style_caption_button button;
    style_menu_owner menu;
} frame_window;
static LRESULT CALLBACK frame_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static frame_window *frame_state(HWND window)
{return (frame_window *)style_instance_lookup(window,frame_proc);}

static int caption_height(HWND window)
{
    UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
    return MulDiv((GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_TOOLWINDOW)?16:19,(int)dpi,96);
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
    int metric=GetSystemMetricsForDpi(sizeable?(horizontal?SM_CXSIZEFRAME:SM_CYSIZEFRAME):
        (horizontal?SM_CXFIXEDFRAME:SM_CYFIXEDFRAME),dpi);
    if(sizeable)metric+=GetSystemMetricsForDpi(SM_CXPADDEDBORDER,dpi);
    int target=max(1,MulDiv(sizeable?4:3,(int)dpi,(int)(horizontal?x:y)));
    return max(0,metric-target);
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
    HBRUSH face=CreateSolidBrush(RGB(212,208,200));if(!face)return;
    FillRect(dc,&area,face);DeleteObject(face);
    UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
    HFONT font=CreateFontW(-MulDiv(8,(int)dpi,72),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
        DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");
    int saved=SaveDC(dc);if(saved){
        if(font)SelectObject(dc,font);
        SetBkMode(dc,TRANSPARENT);
        style_menu_bar_snapshot snapshot;frame_window *frame=frame_state(window);
        int selected=frame?frame->menu.bar.highlighted:-1;BOOL inactive=!frame || !frame->native.active;
        if(menu_snapshot(window,&snapshot)){
            for(UINT i=0;i<snapshot.count;++i){
                const style_menu_bar_item *item=&snapshot.items[i];RECT r=snapshot.rects[i];
                offset_menu(window,&r);OffsetRect(&r,-outer->left,-outer->top);
                BOOL highlighted=selected==(int)item->position;
                if(highlighted){HBRUSH active=CreateSolidBrush(RGB(0,0,128));if(active){FillRect(dc,&r,active);DeleteObject(active);}}
                SetTextColor(dc,(inactive || (item->state&MFS_DISABLED))?RGB(128,128,128):highlighted?RGB(255,255,255):RGB(0,0,0));
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

static void classic_buttons(HWND window,TITLEBARINFOEX *info)
{
    LONG_PTR ex=GetWindowLongPtrW(window,GWL_EXSTYLE);
    if(ex&WS_EX_LAYOUTRTL)return;
    UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
    int width=MulDiv((ex&WS_EX_TOOLWINDOW)?15:16,(int)dpi,96);
    int height=MulDiv((ex&WS_EX_TOOLWINDOW)?15:14,(int)dpi,96);
    int gap=MulDiv(2,(int)dpi,96);
    RECT title=info->rcTitleBar;
    if(width<=0 || height<=0 || title.bottom-title.top<height)return;
    RECT result[CCHILDREN_TITLEBAR+1];
    for(unsigned i=0;i<ARRAYSIZE(result);++i)result[i]=info->rgrect[i];
    int right=title.right-gap;BOOL close=FALSE;
    static const unsigned order[]={5,4,3,2};
    for(unsigned n=0;n<ARRAYSIZE(order);++n){
        unsigned i=order[n];
        if((info->rgstate[i]&STATE_SYSTEM_INVISIBLE) || info->rgrect[i].right<=info->rgrect[i].left)continue;
        if(close){right-=gap;close=FALSE;}
        if(right-width<title.left)return;
        result[i]=(RECT){right-width,title.top+(title.bottom-title.top-height)/2,right,title.top+(title.bottom-title.top-height)/2+height};
        right-=width;if(i==5)close=TRUE;
    }
    for(unsigned i=2;i<ARRAYSIZE(result);++i)info->rgrect[i]=result[i];
}

/* DrawFrameControl uses the host's palette; remap its classic shades locally. */
static void paint_button(HDC target,const RECT *rect,UINT state)
{
    int width=rect->right-rect->left,height=rect->bottom-rect->top;
    if(width<=0 || height<=0)return;
    style_bitmap bitmap={0};SIZE size={width,height};
    if(style_bitmap_ensure(&bitmap,target,size)){
        HDC memory=bitmap.dc;
        RECT local={0,0,width,height};
        if(DrawFrameControl(memory,&local,DFC_CAPTION,state)){
            static const int indices[]={COLOR_3DFACE,COLOR_3DSHADOW,COLOR_3DDKSHADOW,COLOR_3DLIGHT};
            static const COLORREF values[]={RGB(212,208,200),RGB(128,128,128),RGB(0,0,0),RGB(212,208,200)};
            DWORD from[ARRAYSIZE(indices)],to[ARRAYSIZE(indices)];
            for(unsigned i=0;i<ARRAYSIZE(indices);++i){
                COLORREF c=GetSysColor(indices[i]),v=values[i];
                from[i]=(DWORD)GetRValue(c)<<16 | (DWORD)GetGValue(c)<<8 | GetBValue(c);
                to[i]=(DWORD)GetRValue(v)<<16 | (DWORD)GetGValue(v)<<8 | GetBValue(v);
            }
            GdiFlush();DWORD *pixels=(DWORD *)bitmap.pixels;
            for(int i=0;i<width*height;++i)for(unsigned j=0;j<ARRAYSIZE(indices);++j)
                if((pixels[i]&0xffffff)==from[j]){pixels[i]=to[j];break;}
            BitBlt(target,rect->left,rect->top,width,height,memory,0,0,SRCCOPY);
        }
    }
    style_bitmap_dispose(&bitmap);
}

static void paint_border(HDC dc,int width,int height,int thickness)
{
    static const COLORREF colors[]={RGB(212,208,200),RGB(0,0,0),RGB(255,255,255),RGB(128,128,128)};
    if(thickness>0 && width>2*thickness && height>2*thickness){
        HBRUSH face=CreateSolidBrush(RGB(212,208,200));
        if(face){
            RECT top={0,0,width,thickness},bottom={0,height-thickness,width,height};
            RECT left={0,thickness,thickness,height-thickness},right={width-thickness,thickness,width,height-thickness};
            FillRect(dc,&top,face);FillRect(dc,&bottom,face);FillRect(dc,&left,face);FillRect(dc,&right,face);
            DeleteObject(face);
        }
    }
    for(int inset=0;inset<2;++inset){
        HBRUSH light=CreateSolidBrush(colors[inset*2]),dark=CreateSolidBrush(colors[inset*2+1]);
        if(light && dark && width>2*inset && height>2*inset){
            RECT top={inset,inset,width-inset,inset+1},left={inset,inset+1,inset+1,height-inset};
            RECT bottom={inset,height-inset-1,width-inset,height-inset},right={width-inset-1,inset,width-inset,height-inset};
            FillRect(dc,&top,light);FillRect(dc,&left,light);
            FillRect(dc,&bottom,dark);FillRect(dc,&right,dark);
        }
        if(light)DeleteObject(light);
        if(dark)DeleteObject(dark);
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

static void paint_caption_icon(HWND window,HDC dc,const RECT *icon)
{
    HICON image=(HICON)SendMessageW(window,WM_GETICON,ICON_SMALL2,0);
    if(!image)image=(HICON)SendMessageW(window,WM_GETICON,ICON_SMALL,0);
    if(!image)image=(HICON)SendMessageW(window,WM_GETICON,ICON_BIG,0);
    if(!image)image=(HICON)GetClassLongPtrW(window,GCLP_HICONSM);
    if(!image)image=(HICON)GetClassLongPtrW(window,GCLP_HICON);
    if(!image)image=LoadIconW(NULL,IDI_WINLOGO);
    UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
    int width=min(icon->right-icon->left,GetSystemMetricsForDpi(SM_CXSMICON,dpi));
    int height=min(icon->bottom-icon->top,GetSystemMetricsForDpi(SM_CYSMICON,dpi));
    if(image && width>0 && height>0)DrawIconEx(dc,icon->left+(icon->right-icon->left-width)/2,
        icon->top+(icon->bottom-icon->top-height)/2,image,width,height,0,NULL,DI_NORMAL);
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
    BOOL active=frame->native.active;
    win2k_caption_colors colors=active?
        (win2k_caption_colors){RGB(0,0,128),RGB(166,202,240),RGB(255,255,255),TRUE}:
        (win2k_caption_colors){RGB(128,128,128),RGB(181,181,181),RGB(192,192,192),TRUE};
    int saved=SaveDC(dc);
    if(saved){
        UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
        int separator_height=max(1,MulDiv(1,(int)dpi,96));
        if(caption.bottom-caption.top>separator_height){
            RECT separator=caption;separator.top=separator.bottom-separator_height;
            HBRUSH face=CreateSolidBrush(RGB(212,208,200));
            if(face){FillRect(dc,&separator,face);DeleteObject(face);}
            caption.bottom-=separator_height;
        }
        for(unsigned i=2;i<CCHILDREN_TITLEBAR+1;++i){
            RECT r=info.rgrect[i];OffsetRect(&r,-outer.left,-outer.top);
            if(r.right>r.left && r.bottom>r.top)ExcludeClipRect(dc,r.left,r.top,r.right,r.bottom);
        }
        GetWindowTextW(window,title,ARRAYSIZE(title));
        RECT gradient=caption;
        for(unsigned i=2;i<CCHILDREN_TITLEBAR+1;++i){
            RECT r=info.rgrect[i];OffsetRect(&r,-outer.left,-outer.top);
            if(r.left>gradient.left && r.left<gradient.right && r.right>r.left &&
               !(info.rgstate[i]&STATE_SYSTEM_INVISIBLE))gradient.right=r.left;
        }
        BOOL tool=(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_TOOLWINDOW)!=0;
        int font_height=tool?MulDiv(8,(int)dpi,96):MulDiv(8,(int)dpi,72);
        HFONT owned_font=CreateFontW(-font_height,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,
            DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");
        HFONT font=owned_font?owned_font:(HFONT)GetStockObject(DEFAULT_GUI_FONT);
        if(gradient.right>gradient.left)win2k_caption_paint(dc,&gradient,L"",font,&colors);
        RECT tail=caption;tail.left=gradient.right;
        win2k_caption_colors solid=colors;solid.left=colors.right;solid.gradient=FALSE;
        if(tail.right>tail.left)win2k_caption_paint(dc,&tail,L"",font,&solid);
        RECT text=gradient;text.left=has_icon?max(text.left,icon.right)+2:text.left+2;
        if(text.right>text.left){
            SetBkMode(dc,TRANSPARENT);SetTextColor(dc,colors.text);SelectObject(dc,font);
            DrawTextW(dc,title,-1,&text,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOPREFIX);
        }
        RestoreDC(dc,saved);
        if(owned_font)DeleteObject(owned_font);
        if(!IsZoomed(window)){
            int border_saved=SaveDC(dc);MENUBARINFO system={.cbSize=sizeof(system)};
            if(border_saved && GetMenuBarInfo(window,OBJID_SYSMENU,0,&system)){
                RECT icon=system.rcBar;OffsetRect(&icon,-outer.left-frame_shift(window,TRUE),-outer.top-frame_shift(window,FALSE));
                ExcludeClipRect(dc,icon.left,icon.top,icon.right,icon.bottom);
            }
            paint_border(dc,width,height,caption.top);
            if(border_saved)RestoreDC(dc,border_saved);
        }
        paint_menu(window,dc,&outer);
        if(has_icon)paint_caption_icon(window,dc,&icon);
        static const UINT buttons[]={0,0,DFCS_CAPTIONMIN,DFCS_CAPTIONMAX,DFCS_CAPTIONHELP,DFCS_CAPTIONCLOSE};
        for(unsigned i=2;i<CCHILDREN_TITLEBAR+1;++i){
            RECT r=info.rgrect[i];OffsetRect(&r,-outer.left,-outer.top);
            if(r.right<=r.left || r.bottom<=r.top || (info.rgstate[i]&STATE_SYSTEM_INVISIBLE))continue;
            UINT state=buttons[i];
            if(i==3 && IsZoomed(window))state=DFCS_CAPTIONRESTORE;
            if(info.rgstate[i]&STATE_SYSTEM_UNAVAILABLE)state|=DFCS_INACTIVE;
            style_caption_button *tracked=&frame->button;
            if(((unsigned)tracked->pressed==i && tracked->hot) || (info.rgstate[i]&STATE_SYSTEM_PRESSED))state|=DFCS_PUSHED;
            paint_button(dc,&r,state);
        }
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

static LRESULT frame_body(HWND window,UINT message,WPARAM wp,LPARAM lp,style_instance *instance)
{
    DWORD incoming=GetLastError();frame_window *frame=(frame_window *)instance;
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
                if(IsWindow(window) && compact_enabled(window))style_invalidate_native_menu_overlap(window);
            }
            SetLastError(error);return 0;}
    }
    if(message && message==((frame_window *)instance)->native.refresh_message){
        if(style_native_frame_take_refresh(&((frame_window *)instance)->native))recalculate_frame(window);
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
    else if(style_menu_owner_message(&frame->menu,window,message,wp,lp,&win2k_system_menu_operations,NULL,&menu_result)){
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
    if(message==WM_NCDESTROY){RemovePropW(window,frame_property);style_instance_detach(instance);}
    SetLastError(incoming);
    LRESULT result=DefSubclassProc(window,message,wp,lp);
    DWORD outcome=GetLastError();
    if(instance->detached){SetLastError(outcome);return result;}
    if(message==WM_SHOWWINDOW && wp && IsWindow(window) && style_native_frame_take_refresh(&((frame_window *)instance)->native))recalculate_frame(window);
    if(message==WM_NCCALCSIZE && lp && compact_enabled(window) &&
       (GetWindowLongPtrW(window,GWL_STYLE)&WS_CAPTION)==WS_CAPTION &&
       !(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_LAYOUTRTL)){
        UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
        int metric=(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_TOOLWINDOW)?SM_CYSMCAPTION:SM_CYCAPTION;
        int difference=GetSystemMetricsForDpi(metric,dpi)-caption_height(window);
        RECT *client=wp?&((NCCALCSIZE_PARAMS *)lp)->rgrc[0]:(RECT *)lp;
        int dx=frame_shift(window,TRUE),dy=frame_shift(window,FALSE);
        if(difference>0 || dx || dy){
            client->left-=dx;client->right+=dx;client->top-=max(0,difference)+dy;client->bottom+=dy;
            result=style_frame_set_client(wp,lp,client,result);
        }
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
            classic_buttons(window,info);
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
    frame->menu.bar.selected=frame->menu.bar.highlighted=-1;
    style_instance_init(&frame->instance,frame,dispose_frame);
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
    if(frame->menu.bar.keyboard){const style_menu_bar_target menu=menu_target(window,frame);style_menu_bar_finish(&menu,TRUE);}
    style_menu_owner_stop(&frame->menu);
    HRESULT status=style_native_frame_restore_policy(window,&frame->native,DwmSetWindowAttribute);
    if(FAILED(status)){style_instance_release(instance);SetLastError(HRESULT_CODE(status));return FALSE;}
    style_native_frame_restore_geometry(window,&frame->native,&client,normal);
    style_instance_release(instance);return TRUE;
}
