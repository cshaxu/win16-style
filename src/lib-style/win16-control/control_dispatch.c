#include "control_dispatch.h"
#include "basic_message.h"
#include "../common/control_protocol.h"
#include <commctrl.h>

LRESULT win16_control_dispatch_message(HWND w,control_style *s,UINT *message,WPARAM *wparam,LPARAM *lparam,BOOL tab,BOOL header,BOOL progress,const win16_control_dispatch_services *v)
{
    UINT m=*message;WPARAM a=*wparam;LPARAM b=*lparam;LRESULT result;
    if(m==WM_NCDESTROY){(void)v->terminal(w);
        result=DefSubclassProc(w,m,a,b);}
    else if(tab)result=win16_control_tab_message(w,s,m,a,b,v->present,v->tab);
    else if(header)result=win16_control_header_message(w,s,m,a,b,v->present,v->header);
    else if(progress)result=win16_control_progress_message(w,s,m,a,b,v->present,v->progress);
    else if(v->class_is(w,UPDOWN_CLASSW)){result=DefSubclassProc(w,m,a,b);}
    else if(v->class_is(w,TRACKBAR_CLASSW) || style_control_protocol_is(w,TRACKBAR_CLASSW)){
        if(style_control_protocol_runtime_trackbar(w) && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT))
            result=v->track_dispatch(s,w,m,a,b);
        else result=DefSubclassProc(w,m,a,b);
        if(!s->detached && m==WM_STYLECHANGED)SendMessageW(w,WM_SYSCOLORCHANGE,0,0);
        if(!s->detached && s->track_parent && GetParent(w)!=s->track_parent){
            v->track_unbind(s);SendMessageW(w,WM_SYSCOLORCHANGE,0,0);
        }
    }
    else if(v->is_link(w)){
        /* Preserve native markup, hit testing, focus and NM_CLICK/NM_RETURN.
         * Only the shared WM_SETFONT path above changes link typography. */
        result=DefSubclassProc(w,m,a,b);
    }
    else if(s->frame_scrollbars){
        const win16_control_scroll_services *services=v->scroll;
        result=win16_control_frame_scroll_message(w,s,m,a,b,services);
    }
    else if(v->is_scroll(w)){
        const win16_control_scroll_services *services=v->scroll;
        if(GetWindowLongPtrW(w,GWL_STYLE)&SBS_SIZEBOX)result=win16_control_sizebox_message(w,s,m,a,b,services);
        else result=win16_control_scroll_message(w,s,&m,&a,&b,services);
    }
    else if(s->file_owner){
        const win16_control_fileowner_services *services=v->fileowner;
        result=win16_control_fileowner_message(w,s,m,a,b,services);
    }
    else if(v->is_combo(w))result=win16_control_combo_message(w,s,m,a,b,v->present,v->combo);
    else if(v->is_list(w)){
        const win16_control_list_services *services=v->list;
        result=win16_control_list_message(w,s,m,a,b,services);
    }
    else if(v->is_edit(w)){
        const win16_control_edit_services *services=v->edit;
        result=win16_control_edit_message(w,s,m,a,b,services);
    }
    else result=win16_control_basic_message(w,s,m,a,b,v->present,v->basic);
    *message=m;*wparam=a;*lparam=b;return result;
}
