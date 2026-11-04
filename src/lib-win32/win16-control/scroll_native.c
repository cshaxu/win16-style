#include "scroll_native.h"
BOOL win16_scroll_control_layout(HWND w,win16_scroll_info_fn get_info,win16_scroll_layout *layout)
{
    RECT r;SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS};
    UINT dpi=GetDpiForWindow(w);
    if(!GetClientRect(w,&r) || !get_info(w,SB_CTL,&info))return FALSE;
    return win16_scroll_measure_native(&r,(GetWindowLongPtrW(w,GWL_STYLE)&SBS_VERT)!=0,dpi,&info,layout);
}
BOOL win16_scroll_embedded_layout(HWND w,LONG object,win16_scroll_info_fn get_info,
    win16_scroll_layout *layout,POINT *screen_origin)
{
    BOOL vertical=object==OBJID_VSCROLL;
    SCROLLBARINFO bar={.cbSize=sizeof(bar)};SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_RANGE|SIF_PAGE|SIF_POS};
    RECT r;UINT dpi=GetDpiForWindow(w);
    if(!GetScrollBarInfo(w,vertical?OBJID_VSCROLL:OBJID_HSCROLL,&bar) || (bar.rgstate[0]&STATE_SYSTEM_INVISIBLE) ||
        !get_info(w,vertical?SB_VERT:SB_HORZ,&info))return FALSE;
    SetRect(&r,0,0,bar.rcScrollBar.right-bar.rcScrollBar.left,bar.rcScrollBar.bottom-bar.rcScrollBar.top);
    screen_origin->x=bar.rcScrollBar.left;screen_origin->y=bar.rcScrollBar.top;
    return win16_scroll_measure_native(&r,vertical,dpi,&info,layout);
}
BOOL win16_scroll_embedded_input(HWND w,const win16_scroll_embedded_view *v,UINT m,WPARAM a,LPARAM b)
{
    const win16_scroll_actions_view *s=&v->actions;
    win16_scroll_layout layout;win16_scroll_actions actions;POINT point,origin;
    BOOL multi=*s->list && (GetWindowLongPtrW(w,GWL_STYLE)&LBS_MULTICOLUMN)!=0;
    LONG object=multi?OBJID_HSCROLL:OBJID_VSCROLL;
    BOOL down=m==WM_NCLBUTTONDOWN || m==WM_NCLBUTTONDBLCLK;
    if(!*s->list && !*s->edit && !*s->frame)return FALSE;
    /* A dropped combo may already own capture for client clicks. */
    if(v->has_owner(s->context) && *s->list && !s->track->active && GetCapture()==w &&
        (m==WM_LBUTTONDOWN || m==WM_LBUTTONDBLCLK)){
        LRESULT hit;
        point.x=(short)LOWORD(b);point.y=(short)HIWORD(b);ClientToScreen(w,&point);
        hit=SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(point.x,point.y));
        if(*s->detached)return TRUE;
        if(hit==HTVSCROLL || hit==HTHSCROLL){down=TRUE;a=(WPARAM)hit;b=MAKELPARAM(point.x,point.y);}
    }
    if(s->track->active)object=*s->object;
    else if(down)object=a==HTHSCROLL?OBJID_HSCROLL:OBJID_VSCROLL;
    if(!down && m!=WM_MOUSEMOVE && m!=WM_LBUTTONUP && m!=WM_CANCELMODE &&
        m!=WM_CAPTURECHANGED && !(m==WM_TIMER && *s->timer && a==*s->timer))return FALSE;
    if((*s->edit && !win16_scroll_edit_thumb_messages(w,object,v->get_info)) ||
        (*s->list && !win16_scroll_list_axis_owned(w,object,v->get_info))){
        if(s->track->active)win16_scroll_finish(w,s,v->previous_capture,TRUE);
        return FALSE;
    }
    if(down && a==(WPARAM)(object==OBJID_HSCROLL?HTHSCROLL:HTVSCROLL)){RECT limit;unsigned epoch;
        if(s->track->active || !IsWindowEnabled(w) || !v->layout(w,object,&layout,&origin))return TRUE;
        point.x=(short)LOWORD(b)-origin.x;point.y=(short)HIWORD(b)-origin.y;
        limit=layout.bar;{int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));InflateRect(&limit,4*u,u);}
        if(!win16_scroll_begin(s->track,&layout,win16_scroll_disabled(w,object),point,&limit,&actions))return TRUE;
        epoch=++*s->epoch;*s->object=object;*v->previous_capture=GetCapture();
        if(*v->previous_capture!=w)SetCapture(w);
        if(*s->detached || *s->epoch!=epoch)return TRUE;
        if(GetCapture()!=w){win16_scroll_finish(w,s,v->previous_capture,TRUE);return TRUE;}
        win16_scroll_apply_actions(w,s,&actions,epoch);return TRUE;
    }
    if(!s->track->active)return FALSE;
    if(m==WM_MOUSEMOVE || (m==WM_TIMER && a==*s->timer)){
        if(!v->layout(w,object,&layout,&origin)){win16_scroll_finish(w,s,v->previous_capture,TRUE);return TRUE;}
        if(m==WM_TIMER){if(!GetCursorPos(&point))return TRUE;}
        else{point.x=(short)LOWORD(b);point.y=(short)HIWORD(b);ClientToScreen(w,&point);}
        point.x-=origin.x;point.y-=origin.y;
        win16_scroll_motion(s->track,&layout,win16_scroll_disabled(w,object),point,m==WM_TIMER,&actions);
        win16_scroll_apply_actions(w,s,&actions,*s->epoch);return TRUE;
    }
    if(m==WM_LBUTTONUP || m==WM_CANCELMODE || m==WM_CAPTURECHANGED){
        win16_scroll_finish(w,s,v->previous_capture,m!=WM_LBUTTONUP);return TRUE;}
    return FALSE;
}
BOOL win16_scroll_control_input(HWND w,const win16_scroll_input_view *v,UINT m,WPARAM a,LPARAM b)
{
    const win16_scroll_actions_view *s=&v->actions;
    win16_scroll_layout layout;win16_scroll_actions actions;POINT point;
    point.x=(short)LOWORD(b);point.y=(short)HIWORD(b);
    if(m==WM_LBUTTONDOWN || m==WM_LBUTTONDBLCLK){
        RECT limit;unsigned epoch;
        if(!IsWindowEnabled(w) || s->track->active || !v->layout(w,&layout))return TRUE;
        /* USER18:17AE uses 4*CXBORDER, CYBORDER as drag tolerance. */
        limit=layout.bar;{int unit=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));InflateRect(&limit,4*unit,unit);}
        if(!win16_scroll_begin(s->track,&layout,win16_scroll_disabled(w,OBJID_CLIENT),point,&limit,&actions))return TRUE;
        epoch=++*s->epoch;SetFocus(w);
        if(*s->detached || epoch!=*s->epoch || !s->track->active)return TRUE;
        SetCapture(w);
        if(*s->detached || epoch!=*s->epoch)return TRUE;
        if(GetCapture()!=w){win16_scroll_finish(w,s,v->previous_capture,TRUE);return TRUE;}
        win16_scroll_apply_actions(w,s,&actions,epoch);return TRUE;
    }
    if(m==WM_MOUSEMOVE || (m==WM_TIMER && *s->timer && a==*s->timer)){
        if(s->track->active && v->layout(w,&layout)){
            if(m==WM_TIMER){if(!GetCursorPos(&point) || !ScreenToClient(w,&point))return TRUE;}
            win16_scroll_motion(s->track,&layout,win16_scroll_disabled(w,OBJID_CLIENT),point,m==WM_TIMER,&actions);
            win16_scroll_apply_actions(w,s,&actions,*s->epoch);
        }return TRUE;
    }
    if(m==WM_LBUTTONUP || m==WM_CANCELMODE || m==WM_CAPTURECHANGED){
        win16_scroll_finish(w,s,v->previous_capture,m!=WM_LBUTTONUP);return TRUE;
    }
    if(m==WM_MOUSELEAVE)return TRUE;
    if(m==WM_KEYDOWN){UINT command;
        if(!IsWindowEnabled(w))return TRUE;
        switch(a){case VK_LEFT:case VK_UP:command=SB_LINEUP;break;
        case VK_RIGHT:case VK_DOWN:command=SB_LINEDOWN;break;
        case VK_PRIOR:command=SB_PAGEUP;break;case VK_NEXT:command=SB_PAGEDOWN;break;
        case VK_HOME:command=SB_TOP;break;case VK_END:command=SB_BOTTOM;break;
        default:return FALSE;}
        ZeroMemory(&actions,sizeof(actions));actions.timer_ms=WIN16_SCROLL_KEEP_TIMER;
        actions.count=1;actions.notices[0].command=command;actions.repaint=TRUE;*v->key=TRUE;
        win16_scroll_apply_actions(w,s,&actions,*s->epoch);return TRUE;
    }
    if(m==WM_KEYUP && *v->key){
        *v->key=FALSE;ZeroMemory(&actions,sizeof(actions));actions.timer_ms=WIN16_SCROLL_KEEP_TIMER;
        actions.count=1;actions.notices[0].command=SB_ENDSCROLL;
        win16_scroll_apply_actions(w,s,&actions,*s->epoch);return TRUE;
    }
    return FALSE;
}
void win16_scroll_stop_timer(HWND w,UINT_PTR *timer)
{if(*timer){KillTimer(w,*timer);*timer=0;}}
void win16_scroll_release_capture(HWND w,HWND *slot,BOOL list,BOOL edit,BOOL frame)
{
    HWND previous=*slot;*slot=NULL;
    /* Restore borrowed combo capture only while the original owner is valid;
     * never take capture back from an unrelated window. */
    if(GetCapture()==w){
        if((list || edit || frame) && previous && IsWindow(previous) &&
            GetWindowThreadProcessId(previous,NULL)==GetCurrentThreadId()){
            if(previous!=w)SetCapture(previous);
        }else ReleaseCapture();
    }
}
void win16_scroll_finish(HWND w,const win16_scroll_actions_view *s,HWND *previous,BOOL cancel)
{
    win16_scroll_actions actions;unsigned epoch=*s->epoch;
    if(!s->track->active)return;
    win16_scroll_end(s->track,cancel,&actions);win16_scroll_stop_timer(w,s->timer);
    win16_scroll_release_capture(w,previous,*s->list,*s->edit,*s->frame);
    win16_scroll_apply_actions(w,s,&actions,epoch);
}
void win16_scroll_apply_actions(HWND w,const win16_scroll_actions_view *s,
    const win16_scroll_actions *actions,unsigned epoch)
{
    UINT i;BOOL reporting=*s->reporting;LONG previous=*s->report;
    if(*s->detached || *s->epoch!=epoch)return;
    if(actions->timer_ms!=WIN16_SCROLL_KEEP_TIMER){
        win16_scroll_stop_timer(w,s->timer);
        if(actions->timer_ms && s->track->active)
            *s->timer=SetTimer(w,(UINT_PTR)s->track,actions->timer_ms,NULL);
    }
    for(i=0;i<actions->count;++i){
        HWND parent=GetParent(w);UINT message=(GetWindowLongPtrW(w,GWL_STYLE)&SBS_VERT)?WM_VSCROLL:WM_HSCROLL;
        *s->reporting=TRUE;*s->report=actions->notices[i].position;
        if(*s->frame){
            SendMessageW(w,*s->object==OBJID_HSCROLL?WM_HSCROLL:WM_VSCROLL,
                MAKEWPARAM(actions->notices[i].command,(WORD)*s->report),0);
        }else if(*s->edit){UINT command=actions->notices[i].command;
            SendMessageW(w,*s->object==OBJID_HSCROLL?WM_HSCROLL:WM_VSCROLL,MAKEWPARAM(command,(WORD)*s->report),0);
        }else if(*s->list){UINT command=actions->notices[i].command;
            BOOL multi=(GetWindowLongPtrW(w,GWL_STYLE)&LBS_MULTICOLUMN)!=0;
            if(!multi && *s->object==OBJID_HSCROLL)
                SendMessageW(w,WM_HSCROLL,MAKEWPARAM(command,(WORD)*s->report),0);
            else if(command==SB_THUMBTRACK || command==SB_THUMBPOSITION){
                LONGLONG index=*s->report;
                if(multi){RECT client;LRESULT height=SendMessageW(w,LB_GETITEMHEIGHT,0,0);
                    if(*s->detached || *s->epoch!=epoch)return;
                    if(height<=0 || !GetClientRect(w,&client))return;
                    index*=max(1,client.bottom/(LONG)height);
                }
                if(index>=0 && index<=MAXLONG)SendMessageW(w,LB_SETTOPINDEX,(WPARAM)index,0);
            }else SendMessageW(w,multi?WM_HSCROLL:WM_VSCROLL,MAKEWPARAM(command,0),0);
        }else if(parent)SendMessageW(parent,message,MAKEWPARAM(actions->notices[i].command,(WORD)*s->report),(LPARAM)w);
        *s->reporting=reporting;*s->report=previous;
        if(*s->detached || *s->epoch!=epoch)return;
    }
    if(actions->repaint && *s->frame){HDC dc=GetWindowDC(w);
        if(dc){s->draw(w,s->context,dc,*s->object,*s->object==OBJID_VSCROLL);ReleaseDC(w,dc);}}
    else if(actions->repaint){RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
        if(!*s->detached && (*s->list || *s->edit))s->nonclient(w,s->context,NULL);}
    s->caret(w,s->context);
}
BOOL win16_scroll_horizontal_thumb_messages(HWND w,win16_scroll_info_fn get_info)
{
    SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_RANGE|SIF_PAGE};
    return get_info(w,SB_HORZ,&info) && info.nMin>=0 &&
        (LONGLONG)info.nMax-(info.nPage?info.nPage-1:0)<=0xffff;
}
BOOL win16_scroll_list_axis_owned(HWND w,LONG object,win16_scroll_info_fn get_info)
{
    if(GetWindowLongPtrW(w,GWL_STYLE)&LBS_MULTICOLUMN)return object==OBJID_HSCROLL;
    return object==OBJID_VSCROLL || (object==OBJID_HSCROLL && win16_scroll_horizontal_thumb_messages(w,get_info));
}
BOOL win16_scroll_edit_thumb_messages(HWND w,LONG object,win16_scroll_info_fn get_info)
{
    /* Native EDIT reads its own nTrackPos when line count reaches this limit. */
    LRESULT lines=SendMessageW(w,EM_GETLINECOUNT,0,0);
    if(lines<=0 || lines>=0xffff)return FALSE;
    if(object==OBJID_HSCROLL && !win16_scroll_horizontal_thumb_messages(w,get_info))return FALSE;
    return TRUE;
}
UINT win16_scroll_disabled(HWND w,LONG object)
{
    SCROLLBARINFO info={.cbSize=sizeof(info)};UINT disabled=0;
    if(!IsWindowEnabled(w) || !GetScrollBarInfo(w,object,&info))return ESB_DISABLE_BOTH;
    if(info.rgstate[1]&STATE_SYSTEM_UNAVAILABLE)disabled|=ESB_DISABLE_LTUP;
    if(info.rgstate[5]&STATE_SYSTEM_UNAVAILABLE)disabled|=ESB_DISABLE_RTDN;
    return disabled;
}
void win16_scroll_caret_destroy(HWND w,const win16_scroll_caret_view *s)
{
    GUITHREADINFO info={.cbSize=sizeof(info)};
    if(*s->created && GetGUIThreadInfo(GetCurrentThreadId(),&info) && info.hwndCaret==w)DestroyCaret();
    *s->created=*s->shown=FALSE;
}
void win16_scroll_caret_sync(HWND w,const win16_scroll_caret_view *s)
{
    win16_scroll_layout layout;RECT r;int inset=max(1,MulDiv(2,(int)GetDpiForWindow(w),96));
    if(*s->detached || !s->is_scroll(w))return;
    if(GetFocus()!=w || !IsWindowEnabled(w) || !s->layout(w,&layout) || !layout.thumb_fits){
        win16_scroll_caret_destroy(w,s);return;}
    r=layout.thumb;
    if(s->track->active && s->track->part==WIN16_SCROLL_THUMB){
        if(layout.vertical)OffsetRect(&r,0,s->track->pixel-r.top);
        else OffsetRect(&r,s->track->pixel-r.left,0);}
    InflateRect(&r,-inset,-inset);
    if(IsRectEmpty(&r)){win16_scroll_caret_destroy(w,s);return;}
    if(*s->created && (s->size->cx!=r.right-r.left || s->size->cy!=r.bottom-r.top))
        win16_scroll_caret_destroy(w,s);
    if(!*s->created){
        if(!CreateCaret(w,(HBITMAP)1,r.right-r.left,r.bottom-r.top))return;
        *s->created=TRUE;s->size->cx=r.right-r.left;s->size->cy=r.bottom-r.top;}
    SetCaretPos(r.left,r.top);
    if(s->track->active && *s->shown){HideCaret(w);*s->shown=FALSE;}
    else if(!s->track->active && !*s->shown){ShowCaret(w);*s->shown=TRUE;}
}
BOOL win16_scroll_measure_native(const RECT *bar,BOOL vertical,UINT dpi,
    const SCROLLINFO *info,win16_scroll_layout *layout)
{
    if(!bar || !info)return FALSE;
    win16_scroll_metrics metrics={MulDiv(17,(int)dpi,96),MulDiv(17,(int)dpi,96),max(1,MulDiv(1,(int)dpi,96))};
    metrics.arrow=metrics.thumb=vertical?bar->right-bar->left:bar->bottom-bar->top;
    LONGLONG maximum=(LONGLONG)info->nMax-(info->nPage?info->nPage-1:0);
    if(maximum<info->nMin)maximum=info->nMin;
    return win16_scroll_measure(bar,vertical,&metrics,info->nMin,(LONG)maximum,info->nPos,layout);
}

void win16_control_managed_frame_scrollbars_palette_owned(HWND w,const win16_style_palette *palette,const void *owner,const win16_control_managed_services *v)
{
    DWORD_PTR data;
    if(GetWindowSubclass(w,v->proc,(UINT_PTR)v->proc,&data) &&
        ((control_style *)data)->frame_scrollbars && ((control_style *)data)->creation_owner==owner)
        ((control_style *)data)->palette=*palette;
}

void win16_control_managed_frame_scrollbars_remove(HWND w,const win16_control_managed_services *v)
{
    DWORD_PTR data;
    if(GetWindowSubclass(w,v->proc,(UINT_PTR)v->proc,&data) && ((control_style *)data)->frame_scrollbars)
        v->remove(w);
}
void win16_control_managed_frame_scrollbars_palette(HWND w,const win16_style_palette *palette,const win16_control_managed_services *v)
{
    DWORD_PTR data;
    if(GetWindowSubclass(w,v->proc,(UINT_PTR)v->proc,&data) && ((control_style *)data)->frame_scrollbars)
        ((control_style *)data)->palette=*palette;
}
BOOL win16_control_managed_active(HWND w,const win16_control_managed_services *v)
{
    DWORD_PTR data;
    return w && GetWindowThreadProcessId(w,NULL)==GetCurrentThreadId() &&
        GetWindowSubclass(w,v->proc,(UINT_PTR)v->proc,&data) &&
        !((control_style *)data)->detached &&
        (((control_style *)data)->frame_scrollbars || ((control_style *)data)->list_scrollbar || ((control_style *)data)->edit_scrollbars);
}
BOOL win16_control_managed_get_scroll_info(HWND w,int bar,LPSCROLLINFO info,const win16_control_managed_services *v)
{
    DWORD_PTR data;BOOL result=v->get_info(w,bar,info);DWORD error=GetLastError();
    if(result && (bar==SB_HORZ || bar==SB_VERT) && info && info->cbSize==sizeof(*info) &&
        (info->fMask&SIF_TRACKPOS) && GetWindowSubclass(w,v->proc,(UINT_PTR)v->proc,&data)){
        control_style *s=(control_style *)data;
        if((s->frame_scrollbars || s->list_scrollbar || s->edit_scrollbars) &&
            s->embedded_object==(bar==SB_HORZ?OBJID_HSCROLL:OBJID_VSCROLL)){
            if(s->scroll_reporting)info->nTrackPos=s->scroll_report;
            else if(s->scroll.active)info->nTrackPos=s->scroll.position;
        }
    }
    SetLastError(error);return result;
}
int win16_control_managed_set_scroll_info(HWND w,int bar,LPCSCROLLINFO info,BOOL redraw,const win16_control_managed_services *v)
{
    DWORD_PTR data;int result;DWORD error;
    BOOL styled=(bar==SB_HORZ || bar==SB_VERT) &&
        GetWindowSubclass(w,v->proc,(UINT_PTR)v->proc,&data) &&
        (((control_style *)data)->frame_scrollbars || ((control_style *)data)->list_scrollbar || ((control_style *)data)->edit_scrollbars);
    result=v->set_info(w,bar,info,styled?FALSE:redraw);
    error=GetLastError();
    if(styled && redraw){
        if(GetWindowSubclass(w,v->proc,(UINT_PTR)v->proc,&data)){
            HDC dc=GetWindowDC(w);
            if(dc){v->draw(w,(control_style *)data,dc,bar==SB_HORZ?OBJID_HSCROLL:OBJID_VSCROLL,bar==SB_VERT);ReleaseDC(w,dc);}
        }else RedrawWindow(w,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);
    }
    SetLastError(error);return result;
}
BOOL win16_control_managed_list_scrollbar_enable(HWND w,const win16_control_managed_services *v)
{
    DWORD_PTR data;
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId() || !v->is_list(w) ||
        !GetWindowSubclass(w,v->proc,(UINT_PTR)v->proc,&data)){
        SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    ((control_style *)data)->list_scrollbar=TRUE;
    RedrawWindow(w,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);return TRUE;
}

BOOL win16_control_managed_edit_scrollbars_enable(HWND w,const win16_control_managed_services *v)
{
    DWORD_PTR data;LONG_PTR style=GetWindowLongPtrW(w,GWL_STYLE);
    if(GetWindowThreadProcessId(w,NULL)!=GetCurrentThreadId() || !v->is_edit(w) ||
        !(style&ES_MULTILINE) ||
        !GetWindowSubclass(w,v->proc,(UINT_PTR)v->proc,&data)){
        SetLastError(ERROR_NOT_SUPPORTED);return FALSE;}
    ((control_style *)data)->edit_scrollbars=TRUE;
    RedrawWindow(w,NULL,NULL,RDW_FRAME|RDW_INVALIDATE);return TRUE;
}
