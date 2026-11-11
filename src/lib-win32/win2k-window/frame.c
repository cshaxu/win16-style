#include "frame.h"
#include "caption.h"
#include "system_menu.h"
#include <commctrl.h>
#include <dwmapi.h>
#include <oleacc.h>
#include <stdint.h>
#include <windowsx.h>

static const WCHAR frame_property[]=L"Win2kStyle.Frame";
static const WCHAR hover_property[]=L"Win2kStyle.CaptionHover";
static const WCHAR button_property[]=L"Win2kStyle.CaptionButton";
static const WCHAR menu_property[]=L"Win2kStyle.Menu";
static const WCHAR menu_keyboard_property[]=L"Win2kStyle.MenuKeyboard";
static const WCHAR menu_focus_property[]=L"Win2kStyle.MenuFocus";

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

static int menu_item_at(HWND window,POINT point,RECT *bounds)
{
    HMENU menu=GetMenu(window);if(!menu || !compact_enabled(window))return -1;
    int count=GetMenuItemCount(menu),shift=menu_shift(window);
    for(int i=0;i<count;++i){
        RECT r;if(!GetMenuItemRect(window,menu,(UINT)i,&r))return -1;
        OffsetRect(&r,0,-shift);
        if(PtInRect(&r,point)){if(bounds)*bounds=r;return i;}
    }
    return -1;
}

static BOOL menu_bar_at(HWND window,POINT point)
{
    MENUBARINFO bar={.cbSize=sizeof(bar)};
    if(!GetMenu(window) || !compact_enabled(window) || !GetMenuBarInfo(window,OBJID_MENU,0,&bar))return FALSE;
    OffsetRect(&bar.rcBar,0,-menu_shift(window));return PtInRect(&bar.rcBar,point);
}

typedef struct menu_tracking { HWND window;HMENU root,active;int selected,target;BOOL armed;struct menu_tracking *previous; } menu_tracking;
static _Thread_local menu_tracking *active_menu;

static LRESULT CALLBACK menu_filter(int code,WPARAM wp,LPARAM lp)
{
    if(code==MSGF_MENU && active_menu && lp){
        MSG *message=(MSG *)lp;
        if(message->message==WM_LBUTTONUP || message->message==WM_NCLBUTTONUP)active_menu->armed=TRUE;
        if(message->message==WM_MOUSEMOVE || message->message==WM_NCMOUSEMOVE ||
           message->message==WM_LBUTTONDOWN || message->message==WM_NCLBUTTONDOWN){
            POINT point={GET_X_LPARAM(message->lParam),GET_Y_LPARAM(message->lParam)};
            int item=menu_item_at(active_menu->window,point,NULL);
            BOOL down=message->message==WM_LBUTTONDOWN || message->message==WM_NCLBUTTONDOWN;
            if(item>=0 && (item!=active_menu->selected || (down && active_menu->armed))){
                active_menu->target=item==active_menu->selected?-1:item;
                message->message=WM_NULL;EndMenu();
            }
        }
        if((message->message==WM_KEYDOWN || message->message==WM_SYSKEYDOWN) &&
           (message->wParam==VK_LEFT || message->wParam==VK_RIGHT) && active_menu->active==active_menu->root){
            BOOL child=FALSE;
            if(message->wParam==VK_RIGHT){
                int count=GetMenuItemCount(active_menu->root);
                for(int i=0;i<count;++i){MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_STATE|MIIM_SUBMENU};
                    if(GetMenuItemInfoW(active_menu->root,(UINT)i,TRUE,&item) && (item.fState&MFS_HILITE) && item.hSubMenu && !(item.fState&MFS_DISABLED)){child=TRUE;break;}}
            }
            if(!child){
                HMENU bar=GetMenu(active_menu->window);int count=bar?GetMenuItemCount(bar):0;
                int direction=message->wParam==VK_LEFT?-1:1;
                for(int step=1;step<count;++step){
                    int next=(active_menu->selected+direction*step+count*step)%count;
                    MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_STATE|MIIM_SUBMENU};
                    if(GetMenuItemInfoW(bar,(UINT)next,TRUE,&item) && item.hSubMenu && !(item.fState&MFS_DISABLED)){
                        active_menu->target=next;message->message=WM_NULL;EndMenu();break;
                    }
                }
            }
        }
    }
    return CallNextHookEx(NULL,code,wp,lp);
}

static void paint_menu(HWND window,HDC dc,const RECT *outer)
{
    HMENU menu=GetMenu(window);MENUBARINFO bar={.cbSize=sizeof(bar)};
    if(!menu || !compact_enabled(window) || !GetMenuBarInfo(window,OBJID_MENU,0,&bar))return;
    int shift=menu_shift(window);RECT area=bar.rcBar;OffsetRect(&area,-outer->left,-outer->top-shift);
    HBRUSH face=CreateSolidBrush(RGB(192,192,192));if(!face)return;
    FillRect(dc,&area,face);DeleteObject(face);
    UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
    HFONT font=CreateFontW(-MulDiv(8,(int)dpi,72),0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,
        DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");
    int saved=SaveDC(dc);if(saved){
        if(font)SelectObject(dc,font);
        SetBkMode(dc,TRANSPARENT);
        int count=GetMenuItemCount(menu);uintptr_t selected=(uintptr_t)GetPropW(window,menu_property);
        for(int i=0;i<count;++i){
            WCHAR text[512]={0};MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_STRING|MIIM_STATE,
                .dwTypeData=text,.cch=ARRAYSIZE(text)-1};RECT r;
            if(!GetMenuItemInfoW(menu,(UINT)i,TRUE,&item) || !GetMenuItemRect(window,menu,(UINT)i,&r))continue;
            OffsetRect(&r,-outer->left,-outer->top-shift);
            if(selected==(uintptr_t)i+1){HBRUSH active=CreateSolidBrush(RGB(0,0,128));if(active){FillRect(dc,&r,active);DeleteObject(active);}}
            SetTextColor(dc,(item.fState&MFS_DISABLED)?RGB(128,128,128):selected==(uintptr_t)i+1?RGB(255,255,255):RGB(0,0,0));
            DrawTextW(dc,text,-1,&r,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
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
    int size=MulDiv((ex&WS_EX_TOOLWINDOW)?15:18,(int)dpi,96);
    int gap=MulDiv(2,(int)dpi,96);
    RECT title=info->rcTitleBar;
    if(size<=0 || title.bottom-title.top<size)return;
    RECT result[CCHILDREN_TITLEBAR+1];
    for(unsigned i=0;i<ARRAYSIZE(result);++i)result[i]=info->rgrect[i];
    int right=title.right;BOOL close=FALSE;
    static const unsigned order[]={5,4,3,2};
    for(unsigned n=0;n<ARRAYSIZE(order);++n){
        unsigned i=order[n];
        if((info->rgstate[i]&STATE_SYSTEM_INVISIBLE) || info->rgrect[i].right<=info->rgrect[i].left)continue;
        if(close){right-=gap;close=FALSE;}
        if(right-size<title.left)return;
        result[i]=(RECT){right-size,title.top+(title.bottom-title.top-size)/2,right,title.top+(title.bottom-title.top-size)/2+size};
        right-=size;if(i==5)close=TRUE;
    }
    for(unsigned i=2;i<ARRAYSIZE(result);++i)info->rgrect[i]=result[i];
}

/* DrawFrameControl uses the host's palette; remap its classic shades locally. */
static void paint_button(HDC target,const RECT *rect,UINT state)
{
    int width=rect->right-rect->left,height=rect->bottom-rect->top;
    if(width<=0 || height<=0)return;
    BITMAPINFO bitmap={0};
    bitmap.bmiHeader.biSize=sizeof(bitmap.bmiHeader);
    bitmap.bmiHeader.biWidth=width;bitmap.bmiHeader.biHeight=-height;
    bitmap.bmiHeader.biPlanes=1;bitmap.bmiHeader.biBitCount=32;
    bitmap.bmiHeader.biCompression=BI_RGB;
    HDC memory=CreateCompatibleDC(target);void *bits=NULL;
    if(!memory)return;
    HBITMAP surface=CreateDIBSection(target,&bitmap,DIB_RGB_COLORS,&bits,NULL,0);
    if(surface && bits){
        HGDIOBJ old=SelectObject(memory,surface);
        RECT local={0,0,width,height};
        if(old && old!=HGDI_ERROR && DrawFrameControl(memory,&local,DFC_CAPTION,state)){
            static const int indices[]={COLOR_3DFACE,COLOR_3DSHADOW,COLOR_3DDKSHADOW,COLOR_3DLIGHT};
            static const COLORREF values[]={RGB(192,192,192),RGB(128,128,128),RGB(0,0,0),RGB(192,192,192)};
            DWORD from[ARRAYSIZE(indices)],to[ARRAYSIZE(indices)];
            for(unsigned i=0;i<ARRAYSIZE(indices);++i){
                COLORREF c=GetSysColor(indices[i]),v=values[i];
                from[i]=(DWORD)GetRValue(c)<<16 | (DWORD)GetGValue(c)<<8 | GetBValue(c);
                to[i]=(DWORD)GetRValue(v)<<16 | (DWORD)GetGValue(v)<<8 | GetBValue(v);
            }
            GdiFlush();DWORD *pixels=bits;
            for(int i=0;i<width*height;++i)for(unsigned j=0;j<ARRAYSIZE(indices);++j)
                if((pixels[i]&0xffffff)==from[j]){pixels[i]=to[j];break;}
            BitBlt(target,rect->left,rect->top,width,height,memory,0,0,SRCCOPY);
        }
        if(old && old!=HGDI_ERROR)SelectObject(memory,old);
    }
    if(surface)DeleteObject(surface);
    DeleteDC(memory);
}

static void paint_border(HDC dc,int width,int height,int thickness)
{
    static const COLORREF colors[]={RGB(255,255,255),RGB(0,0,0),RGB(192,192,192),RGB(128,128,128)};
    if(thickness>0 && width>2*thickness && height>2*thickness){
        HBRUSH face=CreateSolidBrush(RGB(192,192,192));
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

static void paint(HWND window)
{
    RECT outer;TITLEBARINFOEX info={0};WCHAR title[1024];
    if(IsIconic(window) || !GetWindowRect(window,&outer))return;
    info.cbSize=sizeof(info);
    SendMessageW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
    if(info.rcTitleBar.right<=info.rcTitleBar.left || info.rcTitleBar.bottom<=info.rcTitleBar.top)return;
    RECT caption=info.rcTitleBar;
    OffsetRect(&caption,-outer.left,-outer.top);
    int width=outer.right-outer.left,height=outer.bottom-outer.top;
    if(caption.left<0 || caption.top<0 || caption.right>width || caption.bottom>height)return;
    HDC dc=GetWindowDC(window);if(!dc)return;
    BOOL active=GetPropW(window,frame_property)==(HANDLE)(uintptr_t)2;
    win2k_caption_colors colors=active?
        (win2k_caption_colors){RGB(0,0,128),RGB(166,192,232),RGB(255,255,255),TRUE}:
        (win2k_caption_colors){RGB(128,128,128),RGB(181,181,181),RGB(192,192,192),TRUE};
    int saved=SaveDC(dc);
    if(saved){
        for(unsigned i=1;i<CCHILDREN_TITLEBAR+1;++i){
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
        UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
        BOOL tool=(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_TOOLWINDOW)!=0;
        int font_height=tool?MulDiv(8,(int)dpi,96):MulDiv(8,(int)dpi,72);
        HFONT owned_font=CreateFontW(-font_height,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,
            DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");
        HFONT font=owned_font?owned_font:(HFONT)GetStockObject(DEFAULT_GUI_FONT);
        win2k_caption_colors solid=colors;solid.gradient=FALSE;
        win2k_caption_paint(dc,&caption,L"",font,&solid);
        if(gradient.right>gradient.left)win2k_caption_paint(dc,&gradient,L"",font,&colors);
        RECT text=gradient;text.left+=2;
        if(text.right>text.left){
            SetBkMode(dc,TRANSPARENT);SetTextColor(dc,colors.text);SelectObject(dc,font);
            DrawTextW(dc,title,-1,&text,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOPREFIX);
        }
        RestoreDC(dc,saved);
        if(owned_font)DeleteObject(owned_font);
        if(!IsZoomed(window)){
            int border_saved=SaveDC(dc);MENUBARINFO system={.cbSize=sizeof(system)};
            if(border_saved && GetMenuBarInfo(window,OBJID_SYSMENU,0,&system)){
                RECT icon=system.rcBar;OffsetRect(&icon,-outer.left,-outer.top);
                ExcludeClipRect(dc,icon.left,icon.top,icon.right,icon.bottom);
            }
            paint_border(dc,width,height,caption.top);
            if(border_saved)RestoreDC(dc,border_saved);
        }
        paint_menu(window,dc,&outer);
        static const UINT buttons[]={0,0,DFCS_CAPTIONMIN,DFCS_CAPTIONMAX,DFCS_CAPTIONHELP,DFCS_CAPTIONCLOSE};
        for(unsigned i=2;i<CCHILDREN_TITLEBAR+1;++i){
            RECT r=info.rgrect[i];OffsetRect(&r,-outer.left,-outer.top);
            if(r.right<=r.left || r.bottom<=r.top || (info.rgstate[i]&STATE_SYSTEM_INVISIBLE))continue;
            UINT state=buttons[i];
            if(i==3 && IsZoomed(window))state=DFCS_CAPTIONRESTORE;
            if(info.rgstate[i]&STATE_SYSTEM_UNAVAILABLE)state|=DFCS_INACTIVE;
            uintptr_t tracked=(uintptr_t)GetPropW(window,button_property);
            if((tracked/2==i && (tracked&1)) || (info.rgstate[i]&STATE_SYSTEM_PRESSED))state|=DFCS_PUSHED;
            paint_button(dc,&r,state);
        }
    }
    ReleaseDC(window,dc);
}

static BOOL menu_input(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    if(message==WM_SYSCOMMAND && (wp&0xfff0)==SC_KEYMENU && lp && lp!=L' ' && compact_enabled(window)){
        HMENU menu=GetMenu(window);int count=menu?GetMenuItemCount(menu):0;WCHAR key=(WCHAR)lp;
        CharUpperBuffW(&key,1);
        for(int i=0;i<count;++i){
            WCHAR text[512]={0};MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_STRING|MIIM_STATE,
                .dwTypeData=text,.cch=ARRAYSIZE(text)-1};
            if(!GetMenuItemInfoW(menu,(UINT)i,TRUE,&item) || (item.fState&MFS_DISABLED))continue;
            for(WCHAR *p=text;*p;++p)if(*p==L'&' && p[1]){
                WCHAR candidate=*++p;if(candidate==L'&')continue;CharUpperBuffW(&candidate,1);
                if(candidate==key){
                    RECT r;if(!GetMenuItemRect(window,menu,(UINT)i,&r))return TRUE;
                    OffsetRect(&r,0,-menu_shift(window));
                    return menu_input(window,WM_NCLBUTTONDOWN,HTMENU,MAKELPARAM((r.left+r.right)/2,(r.top+r.bottom)/2));
                }
            }
        }
        return FALSE;
    }
    if(message!=WM_NCLBUTTONDOWN || wp!=HTMENU || !compact_enabled(window))return FALSE;
    POINT point={GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};RECT bounds;
    int index=menu_item_at(window,point,&bounds);if(index<0)return menu_bar_at(window,point);
    menu_tracking tracking={.window=window,.selected=index,.target=-1,
        .armed=(GetKeyState(VK_LBUTTON)&0x8000)==0,.previous=active_menu};
    active_menu=&tracking;HHOOK hook=SetWindowsHookExW(WH_MSGFILTER,menu_filter,NULL,GetCurrentThreadId());
    for(;;){
        HMENU menu=GetMenu(window);SendMessageW(window,WM_INITMENU,(WPARAM)menu,0);
        if(!IsWindow(window) || GetMenu(window)!=menu || !compact_enabled(window))break;
        MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_STATE|MIIM_ID|MIIM_SUBMENU};
        RECT native;if(!GetMenuItemInfoW(menu,(UINT)index,TRUE,&item) || (item.fState&MFS_DISABLED) ||
                       !GetMenuItemRect(window,menu,(UINT)index,&native))break;
        OffsetRect(&native,0,-menu_shift(window));bounds=native;
        if(!SetPropW(window,menu_property,(HANDLE)(uintptr_t)(index+1)))break;
        tracking.selected=index;tracking.target=-1;tracking.root=tracking.active=item.hSubMenu;repaint_menu(window);
        if(item.hSubMenu)TrackPopupMenuEx(item.hSubMenu,TPM_LEFTALIGN|TPM_TOPALIGN,bounds.left,bounds.bottom,window,NULL);
        else PostMessageW(window,WM_COMMAND,item.wID,0);
        if(!IsWindow(window))break;
        RemovePropW(window,menu_property);repaint_menu(window);
        if(tracking.target<0)break;
        index=tracking.target;
    }
    if(hook)UnhookWindowsHookEx(hook);
    active_menu=tracking.previous;
    if(IsWindow(window))RemovePropW(window,menu_property);
    return TRUE;
}

static void leave_menu_keyboard(HWND window,BOOL restore)
{
    HWND focus=(HWND)RemovePropW(window,menu_focus_property);
    RemovePropW(window,menu_keyboard_property);RemovePropW(window,menu_property);
    if(restore && focus && IsWindow(focus) && GetFocus()==window)SetFocus(focus);
    if(IsWindow(window))repaint_menu(window);
}

static BOOL menu_keyboard_input(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    HMENU menu=GetMenu(window);
    if(!menu || !compact_enabled(window))return FALSE;
    uintptr_t selected=(uintptr_t)GetPropW(window,menu_keyboard_property);
    if(message==WM_SYSCOMMAND && (wp&0xfff0)==SC_KEYMENU && !lp){
        if(selected)leave_menu_keyboard(window,TRUE);
        else if(GetMenuItemCount(menu)>0){
            HWND focus=GetFocus();if(focus)SetPropW(window,menu_focus_property,focus);
            if(!SetPropW(window,menu_keyboard_property,(HANDLE)(uintptr_t)1))return FALSE;
            SetPropW(window,menu_property,(HANDLE)(uintptr_t)1);SetFocus(window);repaint_menu(window);
        }
        return TRUE;
    }
    if(!selected)return FALSE;
    if(message==WM_KILLFOCUS){leave_menu_keyboard(window,FALSE);return FALSE;}
    if(message==WM_NCLBUTTONDOWN){leave_menu_keyboard(window,FALSE);return FALSE;}
    if(message!=WM_KEYDOWN && message!=WM_SYSKEYDOWN)return FALSE;
    if(wp==VK_ESCAPE || wp==VK_F10){leave_menu_keyboard(window,TRUE);return TRUE;}
    if(wp==VK_LEFT || wp==VK_RIGHT){
        int count=GetMenuItemCount(menu),direction=wp==VK_LEFT?-1:1,index=(int)selected-1;
        for(int step=1;step<=count;++step){int next=(index+direction*step+count*step)%count;
            MENUITEMINFOW item={.cbSize=sizeof(item),.fMask=MIIM_STATE};
            if(GetMenuItemInfoW(menu,(UINT)next,TRUE,&item) && !(item.fState&MFS_DISABLED)){
                SetPropW(window,menu_keyboard_property,(HANDLE)(uintptr_t)(next+1));
                SetPropW(window,menu_property,(HANDLE)(uintptr_t)(next+1));repaint_menu(window);break;
            }}
        return TRUE;
    }
    if(wp==VK_DOWN || wp==VK_RETURN){
        RECT r;if(GetMenuItemRect(window,menu,(UINT)(selected-1),&r)){
            OffsetRect(&r,0,-menu_shift(window));
            menu_input(window,WM_NCLBUTTONDOWN,HTMENU,MAKELPARAM((r.left+r.right)/2,(r.top+r.bottom)/2));
            if(IsWindow(window) && GetPropW(window,menu_keyboard_property)){
                SetPropW(window,menu_property,(HANDLE)selected);repaint_menu(window);
            }
        }
        return TRUE;
    }
    return FALSE;
}

static BOOL button_input(HWND window,UINT message,WPARAM wp,LPARAM lp)
{
    uintptr_t tracked=(uintptr_t)GetPropW(window,button_property);
    if(message==WM_NCLBUTTONDOWN && !tracked){
        if(IsIconic(window))return FALSE;
        unsigned index=wp==HTMINBUTTON?2:wp==HTMAXBUTTON?3:wp==HTHELP?4:wp==HTCLOSE?5:0;
        if(!index)return FALSE;
        TITLEBARINFOEX info={.cbSize=sizeof(info)};
        SendMessageW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
        if(info.rgstate[index]&(STATE_SYSTEM_INVISIBLE|STATE_SYSTEM_UNAVAILABLE))return FALSE;
        if(!SetPropW(window,button_property,(HANDLE)(uintptr_t)(index*2+1)))return FALSE;
        SetCapture(window);
        if(GetCapture()!=window){RemovePropW(window,button_property);return FALSE;}
        paint(window);return TRUE;
    }
    if(!tracked)return FALSE;
    if(message==WM_MOUSEMOVE || message==WM_LBUTTONUP){
        unsigned index=(unsigned)(tracked/2);TITLEBARINFOEX info={.cbSize=sizeof(info)};
        POINT point={GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
        ClientToScreen(window,&point);SendMessageW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
        BOOL inside=PtInRect(&info.rgrect[index],point);
        if(message==WM_LBUTTONUP){
            RemovePropW(window,button_property);if(GetCapture()==window)ReleaseCapture();
            if(IsWindow(window))paint(window);
            if(inside && IsWindow(window))PostMessageW(window,WM_SYSCOMMAND,
                index==2?SC_MINIMIZE:index==3?(IsZoomed(window)?SC_RESTORE:SC_MAXIMIZE):
                index==4?SC_CONTEXTHELP:SC_CLOSE,MAKELPARAM(point.x,point.y));
        }else if(inside!=(BOOL)(tracked&1)){
            SetPropW(window,button_property,(HANDLE)(uintptr_t)(index*2+(inside?1:0)));paint(window);
        }
        return TRUE;
    }
    if(message==WM_CAPTURECHANGED || message==WM_CANCELMODE || message==WM_NCDESTROY){
        RemovePropW(window,button_property);
        if(message==WM_CANCELMODE && GetCapture()==window)ReleaseCapture();
        if(message!=WM_NCDESTROY && IsWindow(window))paint(window);
    }
    return FALSE;
}

static LRESULT CALLBACK frame_proc(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR ref)
{
    (void)ref;
    DWORD incoming=GetLastError();
    if(message==WM_ENTERMENULOOP)win2k_system_menu_begin(window);
    else if(message==WM_INITMENUPOPUP && HIWORD(lp))win2k_system_menu_set(window,(HMENU)wp);
    else if(message==WM_EXITMENULOOP)win2k_system_menu_end(window,FALSE);
    else if(message==WM_NCDESTROY)win2k_system_menu_end(window,TRUE);
    if(message==WM_COMMAND && !lp && GetPropW(window,menu_keyboard_property))leave_menu_keyboard(window,TRUE);
    if(active_menu && active_menu->window==window){
        if(message==WM_INITMENUPOPUP)active_menu->active=(HMENU)wp;
        else if(message==WM_UNINITMENUPOPUP && active_menu->active==(HMENU)wp)active_menu->active=active_menu->root;
    }
    if(menu_keyboard_input(window,message,wp,lp)){SetLastError(incoming);return 0;}
    if(menu_input(window,message,wp,lp)){SetLastError(incoming);return 0;}
    if(button_input(window,message,wp,lp)){SetLastError(incoming);return 0;}
    BOOL repaint=FALSE;
    if(message==WM_NCACTIVATE)SetPropW(window,frame_property,(HANDLE)(uintptr_t)(wp?2:1));
    if(message==WM_NCMOUSEMOVE){
        HANDLE hit=(HANDLE)(uintptr_t)(wp+1);
        if(GetPropW(window,hover_property)!=hit){SetPropW(window,hover_property,hit);repaint=TRUE;}
    }else if(message==WM_NCMOUSELEAVE){RemovePropW(window,hover_property);repaint=TRUE;}
    if(message==WM_NCDESTROY){RemovePropW(window,menu_focus_property);RemovePropW(window,menu_keyboard_property);RemovePropW(window,menu_property);RemovePropW(window,hover_property);RemovePropW(window,frame_property);RemoveWindowSubclass(window,frame_proc,id);}
    SetLastError(incoming);
    LRESULT result=DefSubclassProc(window,message,wp,lp);
    DWORD outcome=GetLastError();
    if(message==WM_NCCALCSIZE && lp && compact_enabled(window) &&
       (GetWindowLongPtrW(window,GWL_STYLE)&WS_CAPTION)==WS_CAPTION &&
       !(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_LAYOUTRTL)){
        UINT dpi=GetDpiForWindow(window);if(!dpi)dpi=96;
        int metric=(GetWindowLongPtrW(window,GWL_EXSTYLE)&WS_EX_TOOLWINDOW)?SM_CYSMCAPTION:SM_CYCAPTION;
        int difference=GetSystemMetricsForDpi(metric,dpi)-caption_height(window);
        RECT *client=wp?&((NCCALCSIZE_PARAMS *)lp)->rgrc[0]:(RECT *)lp;
        if(difference>0){client->top-=difference;if(wp)result&=~WVR_VALIDRECTS;}
    }
    if(message==WM_GETTITLEBARINFOEX && lp){
        uintptr_t tracked=(uintptr_t)GetPropW(window,button_property);
        TITLEBARINFOEX *info=(TITLEBARINFOEX *)lp;
        if(info->cbSize>=sizeof(*info)){
            int height=caption_height(window);
            if(compact_enabled(window) &&
               height>0 && info->rcTitleBar.bottom-info->rcTitleBar.top>height)
                info->rcTitleBar.bottom=info->rcTitleBar.top+height;
            classic_buttons(window,info);
            if(tracked/2>=2 && tracked/2<=CCHILDREN_TITLEBAR && (tracked&1))info->rgstate[tracked/2]|=STATE_SYSTEM_PRESSED;
        }
    }
    if(message==WM_NCHITTEST && compact_enabled(window)){
        RECT client;POINT point={GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
        if(menu_bar_at(window,point))result=HTMENU;
        else if(GetClientRect(window,&client) && ScreenToClient(window,&point) && PtInRect(&client,point))result=HTCLIENT;
    }
    if(message==WM_NCHITTEST && !IsIconic(window) &&
       (result==HTCAPTION || result==HTMINBUTTON || result==HTMAXBUTTON || result==HTHELP || result==HTCLOSE)){
        TITLEBARINFOEX info={.cbSize=sizeof(info)};POINT point={GET_X_LPARAM(lp),GET_Y_LPARAM(lp)};
        SendMessageW(window,WM_GETTITLEBARINFOEX,0,(LPARAM)&info);
        static const LRESULT hits[]={0,0,HTMINBUTTON,HTMAXBUTTON,HTHELP,HTCLOSE};
        BOOL button=FALSE;
        for(unsigned i=2;i<ARRAYSIZE(hits);++i)
            if(!(info.rgstate[i]&STATE_SYSTEM_INVISIBLE) && PtInRect(&info.rgrect[i],point)){result=hits[i];button=TRUE;break;}
        if(!button && PtInRect(&info.rcTitleBar,point))result=HTCAPTION;
    }
    if(message==WM_NCPAINT || message==WM_NCACTIVATE || message==WM_SETTEXT || message==WM_SIZE ||
       message==WM_NCLBUTTONDOWN || message==WM_NCLBUTTONUP || message==WM_SETICON ||
       message==WM_STYLECHANGED || message==WM_THEMECHANGED || message==WM_DPICHANGED || repaint)
        if(IsWindow(window))paint(window);
    SetLastError(outcome);
    return result;
}

static LRESULT CALLBACK pending_frame_proc(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR ref)
{
    (void)ref;
    if(message==WM_NCDESTROY)RemoveWindowSubclass(window,pending_frame_proc,id);
    LRESULT result=DefSubclassProc(window,message,wp,lp);
    DWORD error=GetLastError();
    if(message==WM_STYLECHANGED && wp==(WPARAM)GWL_STYLE && IsWindow(window)){
        LONG_PTR style=GetWindowLongPtrW(window,GWL_STYLE);
        if(!(style&WS_CHILD) && (style&WS_CAPTION)==WS_CAPTION){
            RemoveWindowSubclass(window,pending_frame_proc,id);
            win2k_frame_attach(window);
        }
    }
    SetLastError(error);return result;
}

BOOL win2k_frame_attach(HWND window)
{
    DWORD pid=0;
    if(!window || GetWindowThreadProcessId(window,&pid)!=GetCurrentThreadId() || pid!=GetCurrentProcessId())return FALSE;
    LONG_PTR style=GetWindowLongPtrW(window,GWL_STYLE);
    if((style&WS_CHILD) || GetPropW(window,frame_property))return FALSE;
    if((style&WS_CAPTION)!=WS_CAPTION)
        return SetWindowSubclass(window,pending_frame_proc,(UINT_PTR)pending_frame_proc,0);
    if(!SetWindowSubclass(window,frame_proc,(UINT_PTR)frame_proc,0))return FALSE;
    if(!SetPropW(window,frame_property,(HANDLE)(uintptr_t)(GetActiveWindow()==window?2:1))){RemoveWindowSubclass(window,frame_proc,(UINT_PTR)frame_proc);return FALSE;}
    enum DWMNCRENDERINGPOLICY policy=DWMNCRP_DISABLED;
    if(FAILED(DwmSetWindowAttribute(window,DWMWA_NCRENDERING_POLICY,&policy,sizeof(policy)))){
        RemovePropW(window,frame_property);RemoveWindowSubclass(window,frame_proc,(UINT_PTR)frame_proc);return FALSE;
    }
    SetWindowPos(window,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
    return TRUE;
}
