#include "scroll_controller.h"

BOOL style_scroll_embedded_input(HWND w,style_instance *instance,const style_control_state *owner,style_scroll_controller *c,const style_scroll_ops *ops,UINT m,WPARAM a,LPARAM b)
{
    style_scroll_layout layout;style_scroll_actions actions;POINT point,origin;
    BOOL multi=c->resources.list && (GetWindowLongPtrW(w,GWL_STYLE)&LBS_MULTICOLUMN)!=0;
    LONG object=multi?OBJID_HSCROLL:OBJID_VSCROLL;
    BOOL down=m==WM_NCLBUTTONDOWN || m==WM_NCLBUTTONDBLCLK;
    if(!c->resources.list && !c->resources.edit && !c->resources.frame)return FALSE;
    /* A dropped combo may already own capture for client clicks. */
    if(owner->owner!=NULL && c->resources.list && !c->track.active && GetCapture()==w &&
        (m==WM_LBUTTONDOWN || m==WM_LBUTTONDBLCLK)){
        LRESULT hit;
        point.x=(short)LOWORD(b);point.y=(short)HIWORD(b);ClientToScreen(w,&point);
        hit=SendMessageW(w,WM_NCHITTEST,0,MAKELPARAM(point.x,point.y));
        if(instance->detached)return TRUE;
        if(hit==HTVSCROLL || hit==HTHSCROLL){down=TRUE;a=(WPARAM)hit;b=MAKELPARAM(point.x,point.y);}
    }
    if(c->track.active)object=c->object;
    else if(down)object=a==HTHSCROLL?OBJID_HSCROLL:OBJID_VSCROLL;
    if(!down && m!=WM_MOUSEMOVE && m!=WM_LBUTTONUP && m!=WM_CANCELMODE &&
        m!=WM_CAPTURECHANGED && !(m==WM_TIMER && c->resources.timer && a==c->resources.timer))return FALSE;
    if((c->resources.edit && !style_scroll_edit_thumb_messages(w,object,ops->get_info)) ||
        (c->resources.list && !style_scroll_list_axis_owned(w,object,ops->get_info))){
        if(c->track.active)style_scroll_finish(w,instance,c,ops,TRUE);
        return FALSE;
    }
    if(down && a==(WPARAM)(object==OBJID_HSCROLL?HTHSCROLL:HTVSCROLL)){RECT limit;unsigned epoch;
        if(c->track.active || !IsWindowEnabled(w) || !ops->layout(w,ops->context,object,&layout,&origin))return TRUE;
        point.x=(short)LOWORD(b)-origin.x;point.y=(short)HIWORD(b)-origin.y;
        limit=layout.bar;{int u=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));InflateRect(&limit,ops->policy->tolerance_x*u,ops->policy->tolerance_y*u);}
        if(!style_scroll_begin(&c->track,&layout,style_scroll_disabled(w,object),point,&limit,&actions,ops->policy))return TRUE;
        epoch=++c->epoch;c->object=object;c->resources.previous_capture=GetCapture();
        if(c->resources.previous_capture!=w)SetCapture(w);
        if(instance->detached || c->epoch!=epoch)return TRUE;
        if(GetCapture()!=w){style_scroll_finish(w,instance,c,ops,TRUE);return TRUE;}
        style_scroll_apply_actions(w,instance,c,ops,&actions,epoch);return TRUE;
    }
    if(!c->track.active)return FALSE;
    if(m==WM_MOUSEMOVE || (m==WM_TIMER && a==c->resources.timer)){
        if(!ops->layout(w,ops->context,object,&layout,&origin)){style_scroll_finish(w,instance,c,ops,TRUE);return TRUE;}
        if(m==WM_TIMER){if(!GetCursorPos(&point))return TRUE;}
        else{point.x=(short)LOWORD(b);point.y=(short)HIWORD(b);ClientToScreen(w,&point);}
        point.x-=origin.x;point.y-=origin.y;
        style_scroll_motion(&c->track,&layout,style_scroll_disabled(w,object),point,m==WM_TIMER,&actions,ops->policy);
        style_scroll_apply_actions(w,instance,c,ops,&actions,c->epoch);return TRUE;
    }
    if(m==WM_LBUTTONUP || m==WM_CANCELMODE || m==WM_CAPTURECHANGED){
        style_scroll_finish(w,instance,c,ops,m!=WM_LBUTTONUP);return TRUE;}
    return FALSE;
}
BOOL style_scroll_control_input(HWND w,style_instance *instance,style_scroll_controller *c,const style_scroll_ops *ops,UINT m,WPARAM a,LPARAM b)
{
    style_scroll_layout layout;style_scroll_actions actions;POINT point;
    point.x=(short)LOWORD(b);point.y=(short)HIWORD(b);
    if(m==WM_LBUTTONDOWN || m==WM_LBUTTONDBLCLK){
        RECT limit;unsigned epoch;
        if(!IsWindowEnabled(w) || c->track.active || !ops->layout(w,ops->context,OBJID_CLIENT,&layout,NULL))return TRUE;
        /* USER18:17AE uses 4*CXBORDER, CYBORDER as drag tolerance. */
        limit=layout.bar;{int unit=max(1,MulDiv(1,(int)GetDpiForWindow(w),96));InflateRect(&limit,ops->policy->tolerance_x*unit,ops->policy->tolerance_y*unit);}
        if(!style_scroll_begin(&c->track,&layout,style_scroll_disabled(w,OBJID_CLIENT),point,&limit,&actions,ops->policy))return TRUE;
        epoch=++c->epoch;SetFocus(w);
        if(instance->detached || epoch!=c->epoch || !c->track.active)return TRUE;
        SetCapture(w);
        if(instance->detached || epoch!=c->epoch)return TRUE;
        if(GetCapture()!=w){style_scroll_finish(w,instance,c,ops,TRUE);return TRUE;}
        style_scroll_apply_actions(w,instance,c,ops,&actions,epoch);return TRUE;
    }
    if(m==WM_MOUSEMOVE || (m==WM_TIMER && c->resources.timer && a==c->resources.timer)){
        if(c->track.active && ops->layout(w,ops->context,OBJID_CLIENT,&layout,NULL)){
            if(m==WM_TIMER){if(!GetCursorPos(&point) || !ScreenToClient(w,&point))return TRUE;}
            style_scroll_motion(&c->track,&layout,style_scroll_disabled(w,OBJID_CLIENT),point,m==WM_TIMER,&actions,ops->policy);
            style_scroll_apply_actions(w,instance,c,ops,&actions,c->epoch);
        }return TRUE;
    }
    if(m==WM_LBUTTONUP || m==WM_CANCELMODE || m==WM_CAPTURECHANGED){
        style_scroll_finish(w,instance,c,ops,m!=WM_LBUTTONUP);return TRUE;
    }
    if(m==WM_MOUSELEAVE)return TRUE;
    if(m==WM_KEYDOWN){UINT command;
        if(!IsWindowEnabled(w))return TRUE;
        if(!style_scroll_key_command(a,&command))return FALSE;
        ZeroMemory(&actions,sizeof(actions));actions.timer_ms=STYLE_SCROLL_KEEP_TIMER;
        actions.count=1;actions.notices[0].command=command;actions.repaint=TRUE;c->key=TRUE;
        style_scroll_apply_actions(w,instance,c,ops,&actions,c->epoch);return TRUE;
    }
    if(m==WM_KEYUP && c->key){
        c->key=FALSE;ZeroMemory(&actions,sizeof(actions));actions.timer_ms=STYLE_SCROLL_KEEP_TIMER;
        actions.count=1;actions.notices[0].command=SB_ENDSCROLL;
        style_scroll_apply_actions(w,instance,c,ops,&actions,c->epoch);return TRUE;
    }
    return FALSE;
}
void style_scroll_finish(HWND w,style_instance *instance,style_scroll_controller *c,const style_scroll_ops *ops,BOOL cancel)
{
    style_scroll_actions actions;unsigned epoch=c->epoch;
    if(!c->track.active)return;
    style_scroll_end(&c->track,cancel,&actions);style_scroll_stop_timer(w,&c->resources.timer);
    style_scroll_release_capture(w,&c->resources.previous_capture,c->resources.list || c->resources.edit || c->resources.frame);
    style_scroll_apply_actions(w,instance,c,ops,&actions,epoch);
}
void style_scroll_apply_actions(HWND w,style_instance *instance,style_scroll_controller *c,const style_scroll_ops *ops,
    const style_scroll_actions *actions,unsigned epoch)
{
    UINT i;BOOL reporting=c->reporting;LONG previous=c->report;
    if(instance->detached || c->epoch!=epoch)return;
    if(actions->timer_ms!=STYLE_SCROLL_KEEP_TIMER){
        style_scroll_stop_timer(w,&c->resources.timer);
        if(actions->timer_ms && c->track.active)
            c->resources.timer=SetTimer(w,(UINT_PTR)&c->track,actions->timer_ms,NULL);
    }
    for(i=0;i<actions->count;++i){
        HWND parent=GetParent(w);UINT message=(GetWindowLongPtrW(w,GWL_STYLE)&SBS_VERT)?WM_VSCROLL:WM_HSCROLL;
        c->reporting=TRUE;c->report=actions->notices[i].position;
        if(c->resources.frame){
            SendMessageW(w,c->object==OBJID_HSCROLL?WM_HSCROLL:WM_VSCROLL,
                MAKEWPARAM(actions->notices[i].command,(WORD)c->report),0);
        }else if(c->resources.edit){UINT command=actions->notices[i].command;
            SendMessageW(w,c->object==OBJID_HSCROLL?WM_HSCROLL:WM_VSCROLL,MAKEWPARAM(command,(WORD)c->report),0);
        }else if(c->resources.list){UINT command=actions->notices[i].command;
            BOOL multi=(GetWindowLongPtrW(w,GWL_STYLE)&LBS_MULTICOLUMN)!=0;
            if(!multi && c->object==OBJID_HSCROLL)
                SendMessageW(w,WM_HSCROLL,MAKEWPARAM(command,(WORD)c->report),0);
            else if(command==SB_THUMBTRACK || command==SB_THUMBPOSITION){
                LONGLONG index=c->report;
                if(multi){RECT client;LRESULT height=SendMessageW(w,LB_GETITEMHEIGHT,0,0);
                    if(instance->detached || c->epoch!=epoch)return;
                    if(height<=0 || !GetClientRect(w,&client))return;
                    index*=max(1,client.bottom/(LONG)height);
                }
                if(index>=0 && index<=MAXLONG)SendMessageW(w,LB_SETTOPINDEX,(WPARAM)index,0);
            }else SendMessageW(w,multi?WM_HSCROLL:WM_VSCROLL,MAKEWPARAM(command,0),0);
        }else if(parent)SendMessageW(parent,message,MAKEWPARAM(actions->notices[i].command,(WORD)c->report),(LPARAM)w);
        c->reporting=reporting;c->report=previous;
        if(instance->detached || c->epoch!=epoch)return;
    }
    if(actions->repaint && c->resources.frame){HDC dc=GetWindowDC(w);
        if(dc){ops->draw(w,ops->context,dc,c->object,c->object==OBJID_VSCROLL);ReleaseDC(w,dc);}}
    else if(actions->repaint){RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW);
        if(!instance->detached && (c->resources.list || c->resources.edit))ops->nonclient(w,ops->context,NULL);}
    ops->caret(w,ops->context);
}
