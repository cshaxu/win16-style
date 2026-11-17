#include "edge.h"
#include "control_vcl_identity.h"
#include "control_class.h"
void style_control_edge_settle(HWND w,UINT message,style_instance *instance,style_control_edge_state *state,style_control_edge_kind kind,const style_control_edge_ops *ops)
{
    if(message!=WM_PAINT || !state->candidate || state->attempted || instance->detached)return;
    DWORD error=GetLastError();
    if(kind==STYLE_EDGE_GRID){
        SCROLLINFO info={.cbSize=sizeof(info),.fMask=SIF_ALL};DWORD style=(DWORD)GetWindowLongPtrW(w,GWL_STYLE);state->attempted=TRUE;
        if(style_control_vcl_grid_identity(w,state->owner,state->parent) &&
            ((style_control_class_is(w,L"TValueListEditor") && !(style&(WS_HSCROLL|WS_VSCROLL))) ||
             (ops->scroll_info(w,(style&WS_HSCROLL)?SB_HORZ:SB_VERT,&info) &&
              info.nMin<=info.nMax && info.nPos>=info.nMin && info.nPos<=info.nMax)) && !instance->detached){
            BOOL installed=ops->install(w,ops->context,&instance->detached);
            if(installed && !instance->detached){state->installed=TRUE;RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);}
            else if(!instance->detached)ops->remove(w,ops->context);
        }
    }else {
        HWND parent=GetParent(w);
        if(ops->eligible(w,parent,kind,ops->context)){
            state->attempted=TRUE;state->parent=parent;state->installed=TRUE;
            BOOL installed=ops->install(w,ops->context,&instance->detached);
            if(installed && !instance->detached && (kind!=STYLE_EDGE_DATA || ops->eligible(w,parent,kind,ops->context)))
                RedrawWindow(w,NULL,NULL,RDW_INVALIDATE|RDW_FRAME);
            else if(!instance->detached){state->installed=FALSE;ops->remove(w,ops->context);}
        }
    }
    SetLastError(error);
}
