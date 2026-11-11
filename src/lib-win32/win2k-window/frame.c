#include "frame.h"
#include "caption.h"
#include <commctrl.h>
#include <dwmapi.h>
#include <oleacc.h>
#include <stdlib.h>

static const WCHAR frame_property[]=L"Win2kStyle.Frame";

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
    BOOL active=GetForegroundWindow()==window;
    win2k_caption_colors colors=active?
        (win2k_caption_colors){RGB(0,0,128),RGB(16,132,208),RGB(255,255,255),TRUE}:
        (win2k_caption_colors){RGB(128,128,128),RGB(192,192,192),RGB(192,192,192),TRUE};
    int saved=SaveDC(dc);
    if(saved){
        for(unsigned i=1;i<CCHILDREN_TITLEBAR+1;++i){
            RECT r=info.rgrect[i];OffsetRect(&r,-outer.left,-outer.top);
            if(r.right>r.left && r.bottom>r.top)ExcludeClipRect(dc,r.left,r.top,r.right,r.bottom);
        }
        GetWindowTextW(window,title,ARRAYSIZE(title));
        RECT text=caption;text.left+=GetSystemMetrics(SM_CXSIZE)+4;
        for(unsigned i=1;i<CCHILDREN_TITLEBAR+1;++i){
            RECT r=info.rgrect[i];OffsetRect(&r,-outer.left,-outer.top);
            if(r.left>text.left && r.left<text.right && r.right>r.left)text.right=r.left;
        }
        HFONT font=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
        win2k_caption_paint(dc,&caption,L"",font,&colors);
        if(text.right>text.left){
            SetBkMode(dc,TRANSPARENT);SetTextColor(dc,colors.text);SelectObject(dc,font);
            DrawTextW(dc,title,-1,&text,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOPREFIX);
        }
        RestoreDC(dc,saved);
        static const UINT buttons[]={0,0,DFCS_CAPTIONMIN,DFCS_CAPTIONMAX,DFCS_CAPTIONHELP,DFCS_CAPTIONCLOSE};
        for(unsigned i=2;i<CCHILDREN_TITLEBAR+1;++i){
            RECT r=info.rgrect[i];OffsetRect(&r,-outer.left,-outer.top);
            if(r.right<=r.left || r.bottom<=r.top || (info.rgstate[i]&STATE_SYSTEM_INVISIBLE))continue;
            UINT state=buttons[i];
            if(i==3 && IsZoomed(window))state=DFCS_CAPTIONRESTORE;
            if(info.rgstate[i]&STATE_SYSTEM_UNAVAILABLE)state|=DFCS_INACTIVE;
            if(info.rgstate[i]&STATE_SYSTEM_PRESSED)state|=DFCS_PUSHED;
            DrawFrameControl(dc,&r,DFC_CAPTION,state);
        }
        if(!IsZoomed(window)){
            RECT border={0,0,width,height};
            DrawEdge(dc,&border,EDGE_RAISED,BF_RECT);
        }
    }
    ReleaseDC(window,dc);
}

static LRESULT CALLBACK frame_proc(HWND window,UINT message,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR ref)
{
    (void)ref;
    if(message==WM_NCDESTROY){RemovePropW(window,frame_property);RemoveWindowSubclass(window,frame_proc,id);}
    LRESULT result=DefSubclassProc(window,message,wp,lp);
    if(message==WM_NCPAINT || message==WM_NCACTIVATE || message==WM_SETTEXT || message==WM_SIZE ||
       message==WM_NCMOUSEMOVE || message==WM_NCLBUTTONDOWN || message==WM_NCLBUTTONUP)
        if(IsWindow(window))paint(window);
    return result;
}

BOOL win2k_frame_attach(HWND window)
{
    DWORD pid=0;
    if(!window || GetWindowThreadProcessId(window,&pid)!=GetCurrentThreadId() || pid!=GetCurrentProcessId())return FALSE;
    LONG_PTR style=GetWindowLongPtrW(window,GWL_STYLE);
    if((style&WS_CHILD) || (style&WS_CAPTION)!=WS_CAPTION || GetPropW(window,frame_property))return FALSE;
    if(!SetWindowSubclass(window,frame_proc,(UINT_PTR)frame_proc,0))return FALSE;
    if(!SetPropW(window,frame_property,(HANDLE)1)){RemoveWindowSubclass(window,frame_proc,(UINT_PTR)frame_proc);return FALSE;}
    enum DWMNCRENDERINGPOLICY policy=DWMNCRP_DISABLED;
    if(FAILED(DwmSetWindowAttribute(window,DWMWA_NCRENDERING_POLICY,&policy,sizeof(policy)))){
        RemovePropW(window,frame_property);RemoveWindowSubclass(window,frame_proc,(UINT_PTR)frame_proc);return FALSE;
    }
    SetWindowPos(window,NULL,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
    return TRUE;
}
