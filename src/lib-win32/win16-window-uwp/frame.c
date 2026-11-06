#include "frame.h"
#include "frame_layout.h"
#include "lib-win32/win16-window/frame_view.h"
#include "lib-win32/win16-common/font.h"
#include "lib-win32/win16-window/menu_message.h"
#include <commctrl.h>
#include <stdint.h>
#include <stdlib.h>
#include <windowsx.h>

typedef struct uwp_frame {
    HWND window,title,surface;
    unsigned calls;
    BOOL detached,updating,active,hot,tracking;
    BOOL no_menu,no_keyboard;
    int pressed,no_selection;
    UINT dpi;
    HFONT title_font,menu_font;
    HDC dc;
    HBITMAP bitmap;
    HGDIOBJ old_bitmap;
    BYTE *pixels;
    SIZE size;
    win16_style_layout layout;
    style_window *menu;
    HWND menu_window;
} uwp_frame;

static LRESULT CALLBACK owner_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static LRESULT CALLBACK surface_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);
static LRESULT CALLBACK title_proc(HWND,UINT,WPARAM,LPARAM,UINT_PTR,DWORD_PTR);

static BOOL local(HWND w)
{
    DWORD pid=0;
    if(GetWindowThreadProcessId(w,&pid)!=GetCurrentThreadId() || pid!=GetCurrentProcessId()){
        SetLastError(ERROR_INVALID_THREAD_ID);return FALSE;
    }
    return TRUE;
}
static uwp_frame *lookup(HWND w)
{
    DWORD_PTR value=0;
    return GetWindowSubclass(w,owner_proc,(UINT_PTR)owner_proc,&value)?(uwp_frame *)value:NULL;
}
static void free_bitmap(uwp_frame *s)
{
    if(s->old_bitmap)SelectObject(s->dc,s->old_bitmap);
    if(s->bitmap)DeleteObject(s->bitmap);
    if(s->dc)DeleteDC(s->dc);
    s->dc=NULL;s->bitmap=NULL;s->old_bitmap=NULL;s->pixels=NULL;
}
static void release(uwp_frame *s)
{
    if(--s->calls || !s->detached)return;
    free_bitmap(s);
    if(s->title_font)DeleteObject(s->title_font);
    s->menu->menu_font=NULL; /* The frame owns the font lease. */
    win16_control_frame_state_dispose(s->menu);
    win16_style_system_release(s->menu_font);
    free(s);
}
static BOOL detach(uwp_frame *s)
{
    if(s->detached)return TRUE;
    if(!RemoveWindowSubclass(s->window,owner_proc,(UINT_PTR)owner_proc))return FALSE;
    s->detached=TRUE;
    if(s->tracking)EndMenu();
    RemoveWindowSubclass(s->title,title_proc,(UINT_PTR)title_proc);
    s->menu->detached=TRUE;
    while(s->menu->menus)win16_control_frame_state_end_menu(s->menu);
    win16_style_menu_restore(&s->menu->menu_items);
    HWND surface=s->surface;s->surface=NULL;
    if(surface){
        if(GetCapture()==surface)ReleaseCapture();
        RemoveWindowSubclass(surface,surface_proc,(UINT_PTR)surface_proc);
        DestroyWindow(surface);
    }
    return TRUE;
}
static BOOL geometry(uwp_frame *s,RECT *outer)
{
    RECT content,title;
    if(!IsWindow(s->title) || GetParent(s->title)!=s->window || !IsWindowVisible(s->title) ||
       !GetWindowRect(s->window,outer) || !GetWindowRect(s->title,&title) ||
       !GetClientRect(s->window,&content))return FALSE;
    if(IsZoomed(s->window)){
        MONITORINFO monitor={.cbSize=sizeof(monitor)};RECT visible;
        if(!GetMonitorInfoW(MonitorFromWindow(s->window,MONITOR_DEFAULTTONEAREST),&monitor) ||
           !IntersectRect(&visible,outer,&monitor.rcWork))return FALSE;
        *outer=visible;
    }
    POINT origin={0,0};ClientToScreen(s->window,&origin);OffsetRect(&content,origin.x-outer->left,origin.y-outer->top);
    /* This adapter covers only the native title band. Arbitrary XAML controls
     * elsewhere remain outside its region and input path. */
    /* The host can restrict its title child to the right-hand caption buttons;
     * its bottom still defines the shared title strip above XAML content. */
    if(title.left<origin.x || title.right!=outer->left+content.right || title.bottom<=origin.y)return FALSE;
    content.top=title.bottom-outer->top;
    SIZE size={outer->right-outer->left,outer->bottom-outer->top};
    UINT dpi=GetDpiForWindow(s->window);
    if(!win16_uwp_frame_layout(size,dpi,(DWORD)GetWindowLongPtrW(s->window,GWL_STYLE),IsZoomed(s->window),content,&s->layout))return FALSE;
    if(dpi!=s->dpi && !win16_control_refresh_frame_fonts(s->window,&s->dpi,&s->title_font,&s->menu_font))return FALSE;
    s->menu->dpi=s->dpi;s->menu->menu_font=s->menu_font;
    return !s->detached;
}
static BOOL cached_layout(HWND w,void *context,win16_style_layout *layout)
{(void)w;*layout=((uwp_frame *)context)->layout;return TRUE;}
static void colors(void *context,win16_style_palette *palette)
{(void)context;win16_style_default_palette(palette);}
static void no_scrollbars(HWND w,HDC dc,const win16_style_palette *palette)
{(void)w;(void)dc;(void)palette;}
static BOOL bitmap(uwp_frame *s,SIZE size)
{
    if(s->dc && size.cx==s->size.cx && size.cy==s->size.cy)return TRUE;
    if(size.cx<=0 || size.cy<=0 || (SIZE_T)size.cx>SIZE_MAX/4/(SIZE_T)size.cy){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    free_bitmap(s);
    BITMAPINFO info={0};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth=size.cx;info.bmiHeader.biHeight=-size.cy;
    info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
    s->dc=CreateCompatibleDC(NULL);
    if(s->dc)s->bitmap=CreateDIBSection(s->dc,&info,DIB_RGB_COLORS,(void **)&s->pixels,NULL,0);
    if(!s->bitmap){free_bitmap(s);return FALSE;}
    s->old_bitmap=SelectObject(s->dc,s->bitmap);s->size=size;
    if(!s->old_bitmap || s->old_bitmap==HGDI_ERROR){s->old_bitmap=NULL;free_bitmap(s);return FALSE;}
    return TRUE;
}
static BOOL refresh(uwp_frame *s)
{
    if(s->detached || s->updating)return !s->detached;
    if(!s->surface)return TRUE;
    if(IsIconic(s->window) || !IsWindowVisible(s->window)){ShowWindow(s->surface,SW_HIDE);return TRUE;}
    s->updating=TRUE;
    RECT outer;BOOL ok=geometry(s,&outer);
    if(ok){
        SIZE size={outer.right-outer.left,outer.bottom-outer.top};ok=bitmap(s,size);
        if(ok){
            ZeroMemory(s->pixels,(SIZE_T)size.cx*(SIZE_T)size.cy*4);
            win16_control_frame_view view={s,&s->detached,&s->active,&s->menu_font,&s->title_font,&s->dpi,
                &s->no_selection,&s->no_menu,&s->no_keyboard,&s->pressed,&s->hot,cached_layout,colors,no_scrollbars};
            win16_control_paint_frame(s->window,&view,s->dc);GdiFlush();
            RECT hole=s->layout.client;
            for(int y=0;y<size.cy;++y){
                BYTE *row=s->pixels+(SIZE_T)y*(SIZE_T)size.cx*4;
                if(y<hole.top || y>=hole.bottom){for(int x=0;x<size.cx;++x)row[(SIZE_T)x*4+3]=255;}
                else{
                    for(int x=0;x<hole.left;++x)row[(SIZE_T)x*4+3]=255;
                    for(int x=hole.right;x<size.cx;++x)row[(SIZE_T)x*4+3]=255;
                }
            }
            if(!s->detached){
                HRGN region=CreateRectRgn(0,0,size.cx,size.cy),inside=CreateRectRgnIndirect(&hole);
                ok=region && inside && CombineRgn(region,region,inside,RGN_DIFF)!=ERROR;
                if(inside)DeleteObject(inside);
                if(ok && SetWindowRgn(s->surface,region,FALSE))region=NULL;else ok=FALSE;
                if(region)DeleteObject(region);
                POINT at={outer.left,outer.top},source={0,0};BLENDFUNCTION blend={AC_SRC_OVER,0,255,AC_SRC_ALPHA};
                if(ok)ok=UpdateLayeredWindow(s->surface,NULL,&at,&size,s->dc,&source,0,&blend,ULW_ALPHA);
                if(ok){
                    HWND above=GetWindow(s->window,GW_HWNDPREV);
                    if(above==s->surface)above=GetWindow(s->surface,GW_HWNDPREV);
                    ok=SetWindowPos(s->surface,above?above:HWND_TOP,0,0,0,0,
                        SWP_NOMOVE|SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOOWNERZORDER|SWP_SHOWWINDOW);
                }
            }else ok=FALSE;
        }
    }
    if(!ok && !s->detached)ShowWindow(s->surface,SW_HIDE);
    s->updating=FALSE;return ok&&!s->detached;
}
static LRESULT point_hit(uwp_frame *s,LPARAM point)
{
    POINT p={GET_X_LPARAM(point),GET_Y_LPARAM(point)};
    return win16_uwp_frame_hit(&s->layout,p);
}
static LPARAM screen_point(uwp_frame *s,LPARAM point)
{
    POINT p={GET_X_LPARAM(point),GET_Y_LPARAM(point)};ClientToScreen(s->surface,&p);
    return MAKELPARAM(p.x,p.y);
}
static void invoke(uwp_frame *s,int hit)
{
    UINT command=hit==HTMINBUTTON?SC_MINIMIZE:hit==HTMAXBUTTON?
        (IsZoomed(s->window)?SC_RESTORE:SC_MAXIMIZE):SC_KEYMENU;
    PostMessageW(s->window,WM_SYSCOMMAND,command,command==SC_KEYMENU?L' ':0);
}
static void menu_present(uwp_frame *s,HWND window)
{
    if(s->detached || !s->tracking || s->menu_window)return;
    WCHAR name[16];MENUBARINFO info={.cbSize=sizeof(info)};
    if(GetWindowThreadProcessId(window,NULL)!=GetCurrentThreadId() ||
       !GetClassNameW(window,name,ARRAYSIZE(name)) || wcscmp(name,L"#32768") ||
       !GetMenuBarInfo(window,OBJID_CLIENT,0,&info) || info.hMenu!=GetSystemMenu(s->window,FALSE))return;
    LONG_PTR ex=GetWindowLongPtrW(window,GWL_EXSTYLE);
    if(ex&WS_EX_LAYERED)return; /* USER may still own its menu animation. */
    s->menu_window=window;
    SetLastError(0);SetWindowLongPtrW(window,GWL_EXSTYLE,ex|WS_EX_LAYERED);
    if(GetLastError() || !SetLayeredWindowAttributes(window,0,255,LWA_ALPHA)){
        SetWindowLongPtrW(window,GWL_EXSTYLE,ex);s->menu_window=NULL;return;
    }
    RedrawWindow(window,NULL,NULL,RDW_INVALIDATE|RDW_FRAME|RDW_UPDATENOW);
}
static void system_menu(uwp_frame *s)
{
    if(s->tracking || s->detached)return;
    HMENU menu=GetSystemMenu(s->window,FALSE);RECT surface;
    if(!menu || !GetWindowRect(s->surface,&surface))return;
    s->tracking=TRUE;s->no_menu=TRUE;refresh(s);
    UINT command=0;
    if(!s->detached)command=TrackPopupMenuEx(menu,TPM_LEFTALIGN|TPM_TOPALIGN|TPM_RETURNCMD|TPM_NOANIMATION,
        surface.left+s->layout.system.left,surface.top+s->layout.system.bottom,
        s->surface,NULL);
    if(s->menu_window){
        MENUBARINFO info={.cbSize=sizeof(info)};
        if(GetMenuBarInfo(s->menu_window,OBJID_CLIENT,0,&info) && info.hMenu==menu)
            SetWindowLongPtrW(s->menu_window,GWL_EXSTYLE,GetWindowLongPtrW(s->menu_window,GWL_EXSTYLE)&~WS_EX_LAYERED);
        s->menu_window=NULL;
    }
    s->tracking=FALSE;s->no_menu=FALSE;
    if(!s->detached){refresh(s);if(command)PostMessageW(s->window,WM_SYSCOMMAND,command,0);}
}
static void surface_menu_button(HWND w,style_window *menu,int hit)
{(void)w;(void)menu;(void)hit;}
static LRESULT CALLBACK surface_proc(HWND w,UINT message,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    uwp_frame *s=(uwp_frame *)data;LRESULT result=0;BOOL handled=TRUE;(void)id;++s->calls;
    if(s->detached){result=DefSubclassProc(w,message,a,b);release(s);return result;}
    if(message==WM_ENTERMENULOOP)win16_control_frame_state_enter_menu(s->menu);
    if(win16_control_menu_message(w,s->menu,message,a,b,surface_menu_button,&result)){
        release(s);return result;
    }
    switch(message){
    case WM_ENTERIDLE:if(a==MSGF_MENU)menu_present(s,(HWND)b);break;
    case WM_MOUSEACTIVATE:result=MA_NOACTIVATE;break;
    case WM_NCHITTEST:result=HTCLIENT;break; /* The window region excludes client content. */
    case WM_LBUTTONDOWN: {
        int hit=(int)point_hit(s,b);SetForegroundWindow(s->window);
        if(s->detached)break;
        if(hit==HTSYSMENU || hit==HTMINBUTTON || hit==HTMAXBUTTON){s->pressed=hit;s->hot=TRUE;SetCapture(w);refresh(s);}
        else if(hit==HTCAPTION || (hit>=HTLEFT && hit<=HTBOTTOMRIGHT))
            SendMessageW(s->window,WM_NCLBUTTONDOWN,(WPARAM)hit,screen_point(s,b));
        break;
    }
    case WM_LBUTTONDBLCLK: {
        int hit=(int)point_hit(s,b);
        if(hit==HTSYSMENU)PostMessageW(s->window,WM_SYSCOMMAND,SC_CLOSE,0);
        else if(hit==HTCAPTION && (GetWindowLongPtrW(s->window,GWL_STYLE)&WS_MAXIMIZEBOX))invoke(s,HTMAXBUTTON);
        break;
    }
    case WM_RBUTTONUP:
        if(point_hit(s,b)==HTCAPTION || point_hit(s,b)==HTSYSMENU)invoke(s,HTSYSMENU);
        break;
    case WM_MOUSEMOVE:
        if(s->pressed){BOOL hot=point_hit(s,b)==s->pressed;if(hot!=s->hot){s->hot=hot;refresh(s);}}
        break;
    case WM_LBUTTONUP:
        if(s->pressed){int hit=s->pressed;BOOL act=point_hit(s,b)==hit;s->pressed=0;s->hot=FALSE;
            ReleaseCapture();if(!s->detached){refresh(s);if(act&&!s->detached)invoke(s,hit);}}
        break;
    case WM_CANCELMODE:
    case WM_CAPTURECHANGED:
        if(s->pressed){s->pressed=0;s->hot=FALSE;if(GetCapture()==w)ReleaseCapture();refresh(s);}
        break;
    case WM_NCDESTROY:s->surface=NULL;detach(s);handled=FALSE;break;
    default:handled=FALSE;break;
    }
    if(!handled)result=DefSubclassProc(w,message,a,b);
    release(s);return result;
}
static void menu_button(HWND w,style_window *menu,int hit)
{
    (void)hit;uwp_frame *s=lookup(w);
    if(s&&!s->detached){s->no_menu=menu->system_menu;refresh(s);}
}
static LRESULT CALLBACK title_proc(HWND w,UINT message,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    uwp_frame *s=(uwp_frame *)data;(void)id;++s->calls;
    if(message==WM_NCDESTROY)detach(s);
    LRESULT result=DefSubclassProc(w,message,a,b);
    if(!s->detached&&(message==WM_WINDOWPOSCHANGED||message==WM_SHOWWINDOW))refresh(s);
    release(s);return result;
}
static LRESULT CALLBACK owner_proc(HWND w,UINT message,WPARAM a,LPARAM b,UINT_PTR id,DWORD_PTR data)
{
    uwp_frame *s=(uwp_frame *)data;(void)id;++s->calls;
    if(message==WM_NCDESTROY)detach(s);
    LRESULT result=0;BOOL handled=FALSE;
    if(!s->detached && message==WM_CANCELMODE && s->tracking)EndMenu();
    if(!s->detached && message==WM_SYSCOMMAND &&
       (((a&0xfff0)==SC_KEYMENU && (b==L' '||b==0)) || (a&0xfff0)==SC_MOUSEMENU)){
        system_menu(s);release(s);return 0;
    }
    if(!s->detached){
        if(message==WM_ENTERMENULOOP)win16_control_frame_state_enter_menu(s->menu);
        handled=win16_control_menu_message(w,s->menu,message,a,b,menu_button,&result);
    }
    if(!handled)result=DefSubclassProc(w,message,a,b);
    if(!s->detached){
        switch(message){
        case WM_NCACTIVATE:s->active=(BOOL)a;refresh(s);break;
        case WM_ACTIVATE:s->active=LOWORD(a)!=WA_INACTIVE;refresh(s);break;
        case WM_WINDOWPOSCHANGED:case WM_SIZE:case WM_SHOWWINDOW:
        case WM_DPICHANGED:case WM_SETTEXT:case WM_THEMECHANGED:refresh(s);break;
        default:break;
        }
    }
    release(s);return result;
}
BOOL win16_uwp_frame_apply(HWND w,HWND title)
{
    if(!local(w) || !local(title))return FALSE;
    if(lookup(w)){SetLastError(ERROR_ALREADY_EXISTS);return FALSE;}
    if(GetParent(title)!=w){SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    uwp_frame *s=calloc(1,sizeof(*s));if(!s){SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    s->menu=win16_control_frame_state_create();
    if(!s->menu){free(s);SetLastError(ERROR_NOT_ENOUGH_MEMORY);return FALSE;}
    win16_style_default_palette(&s->menu->palette);
    s->window=w;s->title=title;s->calls=1;s->no_selection=-1;s->active=GetForegroundWindow()==w;
    if(!SetWindowSubclass(w,owner_proc,(UINT_PTR)owner_proc,(DWORD_PTR)s)){win16_control_frame_state_dispose(s->menu);free(s);return FALSE;}
    if(!SetWindowSubclass(title,title_proc,(UINT_PTR)title_proc,(DWORD_PTR)s)){
        DWORD error=GetLastError();detach(s);release(s);SetLastError(error);return FALSE;
    }
    s->surface=CreateWindowExW(WS_EX_LAYERED|WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE,L"STATIC",L"",
        WS_POPUP|SS_NOTIFY,0,0,1,1,w,NULL,GetModuleHandleW(NULL),NULL);
    if(s->detached && s->surface){DestroyWindow(s->surface);s->surface=NULL;}
    BOOL ok=!s->detached && s->surface && SetWindowSubclass(s->surface,surface_proc,(UINT_PTR)surface_proc,(DWORD_PTR)s) && refresh(s);
    DWORD error=ok?0:GetLastError();
    if(!ok){detach(s);if(!error)error=ERROR_NOT_SUPPORTED;}
    release(s);SetLastError(error);return ok;
}
BOOL win16_uwp_frame_remove(HWND w)
{
    if(!local(w))return FALSE;
    uwp_frame *s=lookup(w);if(!s){SetLastError(ERROR_NOT_FOUND);return FALSE;}
    ++s->calls;BOOL ok=detach(s);release(s);return ok;
}
HWND win16_uwp_frame_surface(HWND w)
{
    if(!local(w))return NULL;
    uwp_frame *s=lookup(w);return s&&!s->detached?s->surface:NULL;
}
