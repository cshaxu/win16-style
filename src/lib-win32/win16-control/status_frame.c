#include "status_frame.h"
#include "special_paint.h"
#include "vb_text.h"
#include <commctrl.h>

/* The themed native status control omits the final pane background when it
 * owns a sizing grip. Only a native text call for an icon-free,
 * ordinary final pane supplies a verified pre-content drawing boundary. */
void win16_control_status_text_frame(const win16_control_status_frame_view *s,HDC dc,const RECT *text,UINT flags,const void *address)
{
    HMODULE module=NULL;POINT origin;RECT pane;HWND window;
    LRESULT parts,icon,type;
    if(!s || !s->owner || *s->detached || s->epoch!=*s->owner_epoch ||
        s->count<2 || !text || (flags&(DT_CALCRECT|DT_RTLREADING)) ||
        !(GetWindowLongPtrW(s->window,GWL_STYLE)&SBARS_SIZEGRIP) ||
        !s->identity(s->window,s->owner,NULL) ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)address,&module) || module!=s->native || !dc ||
        GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) ||
        !GetViewportOrgEx(dc,&origin) || origin.x || origin.y)return;
    window=WindowFromDC(dc);if(s->dc?dc!=s->dc:window!=s->window)return;
    pane=s->panes[s->count-1];
    if(text->left<=pane.left || text->left>=pane.right || text->right>pane.right ||
        text->right<=text->left || text->top>=pane.bottom || text->bottom<=pane.top)return;
    parts=SendMessageW(s->window,SB_GETPARTS,0,0);
    if(*s->detached || s->epoch!=*s->owner_epoch || parts!=(LRESULT)s->count)return;
    type=SendMessageW(s->window,SB_GETTEXTLENGTHW,s->count-1,0);
    if(*s->detached || s->epoch!=*s->owner_epoch || HIWORD(type)!=0)return;
    icon=SendMessageW(s->window,SB_GETICON,s->count-1,0);
    if(*s->detached || s->epoch!=*s->owner_epoch || icon)return;
    win16_control_black_frame(dc,&pane,NULL);
}

/* For a size-grip pane, native comctl draws its icon before its text.
 * Frame only the verified native pane before the same icon is painted. */
void win16_control_status_icon_frame(const win16_control_status_frame_view *s,HDC dc,int x,int y,int cx,int cy,HICON icon,const void *address)
{
    HMODULE module=NULL;POINT origin;RECT pane;HWND window;
    LRESULT parts,type,current;
    if(!s || !s->owner || *s->detached || s->epoch!=*s->owner_epoch || s->count<2 ||
        !(GetWindowLongPtrW(s->window,GWL_STYLE)&SBARS_SIZEGRIP) || !s->identity(s->window,s->owner,NULL) ||
        !dc || !icon || cx<=0 || cy<=0 ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)address,&module) || module!=s->native || GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) ||
        !GetViewportOrgEx(dc,&origin) || origin.x || origin.y)return;
    window=WindowFromDC(dc);if(s->dc?dc!=s->dc:window!=s->window)return;
    pane=s->panes[s->count-1];
    if(x<pane.left || x>=pane.right || (LONGLONG)y+cy<=pane.top || y>=pane.bottom)return;
    parts=SendMessageW(s->window,SB_GETPARTS,0,0);
    if(*s->detached || s->epoch!=*s->owner_epoch || parts!=(LRESULT)s->count)return;
    type=SendMessageW(s->window,SB_GETTEXTLENGTHW,s->count-1,0);
    if(*s->detached || s->epoch!=*s->owner_epoch || HIWORD(type)!=0)return;
    current=SendMessageW(s->window,SB_GETICON,s->count-1,0);
    if(*s->detached || s->epoch!=*s->owner_epoch || (HICON)current!=icon)return;
    win16_control_black_frame(dc,&pane,NULL);
}

/* Called only after the original native drawing call has completed. */
void win16_control_status_frame(const win16_control_status_frame_view *s,HDC dc,const RECT *rect,const RECT *clip,const void *caller)
{
    HMODULE module=NULL;HWND w,scoped=NULL;BOOL matches=FALSE;POINT origin;
    if(!s || !s->owner || *s->detached || s->epoch!=*s->owner_epoch || !dc || !rect ||
        !s->identity(s->window,s->owner,NULL) || GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) ||
        !GetViewportOrgEx(dc,&origin) || origin.x || origin.y ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)caller,&module) || module!=s->native)return;
    if(ntw16s_native_paint_target(dc,&scoped) && scoped!=s->window)return;
    w=WindowFromDC(dc);if(s->dc?dc!=s->dc:w!=s->window)return;
    for(unsigned i=0;i<s->count;++i)if(EqualRect(rect,&s->panes[i])){matches=TRUE;break;}
    if(!matches || *s->detached || s->epoch!=*s->owner_epoch)return;
    win16_control_black_frame(dc,rect,clip);
}

void win16_control_status_invalidate(UINT m,BOOL *font_invalid,unsigned *epoch)
{
    if(m==WM_SETFONT || m==WM_FONTCHANGE || m==WM_SETTINGCHANGE || m==WM_THEMECHANGED || m==WM_DPICHANGED)*font_invalid=TRUE;
    if(m==WM_SETTEXT || m==SB_SETPARTS || m==SB_SETTEXTA || m==SB_SETTEXTW || m==SB_SIMPLE ||
        m==SB_SETICON || m==SB_SETMINHEIGHT || m==WM_SETFONT || m==WM_STYLECHANGED ||
        m==WM_SIZE || m==WM_ENABLE || m==WM_SETTINGCHANGE || m==WM_DESTROY || m==WM_NCDESTROY)++*epoch;
}
void win16_control_status_snapshot(HWND w,const win16_control_status_snapshot_view *s)
{
    *s->simple=s->identity(w,s->owner,s->native) && SendMessageW(w,SB_ISSIMPLE,0,0) && !*s->detached && s->epoch==*s->owner_epoch;
    if(*s->simple){
        LRESULT length=SendMessageW(w,SB_GETTEXTLENGTHW,0,0);
        if(*s->detached || s->epoch!=*s->owner_epoch || HIWORD(length)!=0 || LOWORD(length)==0 || LOWORD(length)>512)*s->simple=FALSE;
        else {
            int copied=GetWindowTextW(w,s->simple_text,(int)LOWORD(length)+1);
            if(*s->detached || s->epoch!=*s->owner_epoch || copied!=(int)LOWORD(length) || !s->identity(w,s->owner,NULL))*s->simple=FALSE;
            else *s->simple_length=copied;
        }
    }
    if(!*s->simple && s->identity(w,s->owner,s->native) && !SendMessageW(w,SB_ISSIMPLE,0,0) && !*s->detached && s->epoch==*s->owner_epoch){
        LRESULT count=SendMessageW(w,SB_GETPARTS,0,0);
        if(count>0 && count<=64 && !*s->detached && s->epoch==*s->owner_epoch){
            for(unsigned i=0;i<(unsigned)count;++i){RECT r={0};LRESULT type=SendMessageW(w,SB_GETTEXTLENGTHW,i,0);
                if(*s->detached || s->epoch!=*s->owner_epoch)break;
                if((HIWORD(type)&~(SBT_OWNERDRAW|SBT_NOBORDERS)) || !SendMessageW(w,SB_GETRECT,i,(LPARAM)&r))continue;
                if(*s->detached || s->epoch!=*s->owner_epoch)break;
                if(!(HIWORD(type)&SBT_OWNERDRAW))s->text_panes[(*s->text_count)++]=r;
                if(!(HIWORD(type)&SBT_NOBORDERS))s->panes[(*s->count)++]=r;
            }
        }
    }
}
