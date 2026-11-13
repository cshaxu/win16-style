#ifndef STYLE_COMMON_SCROLL_STATE_H
#define STYLE_COMMON_SCROLL_STATE_H
#include <windows.h>
#include <oleacc.h>
#include <stdint.h>

typedef BOOL (*style_scroll_info_fn)(HWND,int,LPSCROLLINFO);
typedef void (*style_scroll_painter)(HWND,void *,HDC,LONG,BOOL);

/* Native list and edit controls use 16-bit positions in scroll messages.
 * Callers retain the decision to fall back to native handling. */
static inline BOOL style_scroll_horizontal_thumb_messages(HWND window,style_scroll_info_fn get_info)
{
    SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_RANGE|SIF_PAGE};
    return get_info(window,SB_HORZ,&info) && info.nMin>=0 &&
        (LONGLONG)info.nMax-(info.nPage?info.nPage-1:0)<=0xffff;
}
static inline BOOL style_scroll_list_axis_owned(HWND window,LONG object,style_scroll_info_fn get_info)
{
    if(GetWindowLongPtrW(window,GWL_STYLE)&LBS_MULTICOLUMN)return object==OBJID_HSCROLL;
    return object==OBJID_VSCROLL || (object==OBJID_HSCROLL && style_scroll_horizontal_thumb_messages(window,get_info));
}
static inline BOOL style_scroll_edit_thumb_messages(HWND window,LONG object,style_scroll_info_fn get_info)
{
    LRESULT lines=SendMessageW(window,EM_GETLINECOUNT,0,0);
    if(lines<=0 || lines>=0xffff)return FALSE;
    if(object==OBJID_HSCROLL && !style_scroll_horizontal_thumb_messages(window,get_info))return FALSE;
    return TRUE;
}

/* Native scrollbar keyboard commands are shared by both axes. */
static inline BOOL style_scroll_key_command(WPARAM key,UINT *command)
{
    switch(key){
    case VK_LEFT:case VK_UP:*command=SB_LINEUP;return TRUE;
    case VK_RIGHT:case VK_DOWN:*command=SB_LINEDOWN;return TRUE;
    case VK_PRIOR:*command=SB_PAGEUP;return TRUE;
    case VK_NEXT:*command=SB_PAGEDOWN;return TRUE;
    case VK_HOME:*command=SB_TOP;return TRUE;
    case VK_END:*command=SB_BOTTOM;return TRUE;
    default:return FALSE;
    }
}

/* Supply only the present axes; the painter owns the style-specific pixels. */
static inline void style_scroll_nonclient_paint(HWND window,void *context,HDC target,style_scroll_painter paint)
{
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    if(!(style&(WS_HSCROLL|WS_VSCROLL)))return;
    HDC dc=target?target:GetWindowDC(window);if(!dc)return;
    if(style&WS_HSCROLL)paint(window,context,dc,OBJID_HSCROLL,FALSE);
    if(style&WS_VSCROLL)paint(window,context,dc,OBJID_VSCROLL,TRUE);
    if(!target)ReleaseDC(window,dc);
}

/* Map a thumb origin to the caller's effective range; thumb size and travel
 * are supplied by the style, so fixed and proportional thumbs can share it. */
static inline LONG style_scroll_position(int origin,int travel,LONG minimum,LONG maximum,int pixel)
{
    if(travel<=0)return minimum;
    int64_t distance=(int64_t)pixel-origin;
    distance=max(0,min((int64_t)travel,distance));
    int64_t range=(int64_t)maximum-minimum;
    return (LONG)(minimum+(distance*range+travel/2)/travel);
}

static inline void style_scroll_stop_timer(HWND window,UINT_PTR *timer)
{if(*timer){KillTimer(window,*timer);*timer=0;}}

/* Release only our own capture; optionally return a borrowed same-thread capture. */
static inline void style_scroll_release_capture(HWND window,HWND *slot,BOOL restore_previous)
{
    HWND previous=*slot;*slot=NULL;
    if(GetCapture()==window){
        if(restore_previous && previous && IsWindow(previous) &&
            GetWindowThreadProcessId(previous,NULL)==GetCurrentThreadId()){
            if(previous!=window)SetCapture(previous);
        }else ReleaseCapture();
    }
}

/* Read native state and geometry; the style chooses how to lay it out. */
static inline BOOL style_scroll_control_read(HWND window,style_scroll_info_fn get_info,
    RECT *bar,SCROLLINFO *info,UINT *dpi)
{
    *info=(SCROLLINFO){.cbSize=sizeof(*info),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS};
    *dpi=GetDpiForWindow(window);
    return GetClientRect(window,bar) && get_info(window,SB_CTL,info);
}
static inline BOOL style_scroll_embedded_read(HWND window,LONG object,style_scroll_info_fn get_info,
    RECT *bar,POINT *screen_origin,SCROLLINFO *info,UINT *dpi)
{
    BOOL vertical=object==OBJID_VSCROLL;
    SCROLLBARINFO native={.cbSize=sizeof(native)};
    *info=(SCROLLINFO){.cbSize=sizeof(*info),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS};
    *dpi=GetDpiForWindow(window);
    if(!GetScrollBarInfo(window,vertical?OBJID_VSCROLL:OBJID_HSCROLL,&native) ||
       (native.rgstate[0]&STATE_SYSTEM_INVISIBLE) ||
       !get_info(window,vertical?SB_VERT:SB_HORZ,info))return FALSE;
    SetRect(bar,0,0,native.rcScrollBar.right-native.rcScrollBar.left,
        native.rcScrollBar.bottom-native.rcScrollBar.top);
    screen_origin->x=native.rcScrollBar.left;screen_origin->y=native.rcScrollBar.top;
    return TRUE;
}

/* Query native arrow availability independently of scrollbar geometry. */
static inline UINT style_scroll_disabled(HWND window,LONG object)
{
    SCROLLBARINFO info={.cbSize=sizeof(info)};UINT disabled=0;
    if(!IsWindowEnabled(window) || !GetScrollBarInfo(window,object,&info))return ESB_DISABLE_BOTH;
    if(info.rgstate[1]&STATE_SYSTEM_UNAVAILABLE)disabled|=ESB_DISABLE_LTUP;
    if(info.rgstate[5]&STATE_SYSTEM_UNAVAILABLE)disabled|=ESB_DISABLE_RTDN;
    return disabled;
}
#endif
