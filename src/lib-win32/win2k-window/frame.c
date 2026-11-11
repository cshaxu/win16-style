#include "frame.h"
#include "caption.h"
#include <commctrl.h>
#include <dwmapi.h>
#include <oleacc.h>
#include <stdint.h>
#include <windowsx.h>

static const WCHAR frame_property[]=L"Win2kStyle.Frame";
static const WCHAR hover_property[]=L"Win2kStyle.CaptionHover";
static const WCHAR button_property[]=L"Win2kStyle.CaptionButton";

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
    if(button_input(window,message,wp,lp)){SetLastError(incoming);return 0;}
    BOOL repaint=FALSE;
    if(message==WM_NCACTIVATE)SetPropW(window,frame_property,(HANDLE)(uintptr_t)(wp?2:1));
    if(message==WM_NCMOUSEMOVE){
        HANDLE hit=(HANDLE)(uintptr_t)(wp+1);
        if(GetPropW(window,hover_property)!=hit){SetPropW(window,hover_property,hit);repaint=TRUE;}
    }else if(message==WM_NCMOUSELEAVE){RemovePropW(window,hover_property);repaint=TRUE;}
    if(message==WM_NCDESTROY){RemovePropW(window,hover_property);RemovePropW(window,frame_property);RemoveWindowSubclass(window,frame_proc,id);}
    SetLastError(incoming);
    LRESULT result=DefSubclassProc(window,message,wp,lp);
    DWORD outcome=GetLastError();
    if(message==WM_GETTITLEBARINFOEX && lp){
        uintptr_t tracked=(uintptr_t)GetPropW(window,button_property);
        TITLEBARINFOEX *info=(TITLEBARINFOEX *)lp;
        if(info->cbSize>=sizeof(*info)){
            classic_buttons(window,info);
            if(tracked/2>=2 && tracked/2<=CCHILDREN_TITLEBAR && (tracked&1))info->rgstate[tracked/2]|=STATE_SYSTEM_PRESSED;
        }
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

BOOL win2k_frame_attach(HWND window)
{
    DWORD pid=0;
    if(!window || GetWindowThreadProcessId(window,&pid)!=GetCurrentThreadId() || pid!=GetCurrentProcessId())return FALSE;
    LONG_PTR style=GetWindowLongPtrW(window,GWL_STYLE);
    if((style&WS_CHILD) || (style&WS_CAPTION)!=WS_CAPTION || GetPropW(window,frame_property))return FALSE;
    if(!SetWindowSubclass(window,frame_proc,(UINT_PTR)frame_proc,0))return FALSE;
    if(!SetPropW(window,frame_property,(HANDLE)(uintptr_t)(GetActiveWindow()==window?2:1))){RemoveWindowSubclass(window,frame_proc,(UINT_PTR)frame_proc);return FALSE;}
    enum DWMNCRENDERINGPOLICY policy=DWMNCRP_DISABLED;
    if(FAILED(DwmSetWindowAttribute(window,DWMWA_NCRENDERING_POLICY,&policy,sizeof(policy)))){
        RemovePropW(window,frame_property);RemoveWindowSubclass(window,frame_proc,(UINT_PTR)frame_proc);return FALSE;
    }
    SetWindowPos(window,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
    return TRUE;
}
