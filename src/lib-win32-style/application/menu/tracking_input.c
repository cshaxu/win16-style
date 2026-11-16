#include "tracking.h"
#include "menu_input.h"
#include <windowsx.h>

static void consume(style_menu_tracking *frame,MSG *message)
{
    message->message=WM_NULL;
    if(frame->kind==WH_GETMESSAGE){message->wParam=0;message->lParam=0;}
    EndMenu();
}
static int next_popup(HWND window,int selected,int direction)
{
    int next=style_menu_next_enabled(GetMenu(window),selected,direction,TRUE);
    return next<0?STYLE_MENU_NO_ITEM:next;
}
void style_menu_tracking_input(style_menu_tracking *frame,MSG *message)
{
    BOOL native=frame->navigation==STYLE_MENU_TRACK_NATIVE_POPUPS;
    BOOL key=message->message==WM_KEYDOWN || message->message==WM_SYSKEYDOWN;
    if(key && message->wParam==VK_ESCAPE && frame->active==frame->root)frame->escape=TRUE;
    if(message->message==WM_LBUTTONUP || message->message==WM_NCLBUTTONUP)frame->armed=TRUE;
    if(frame->hit && (message->message==WM_MOUSEMOVE || message->message==WM_NCMOUSEMOVE ||
       message->message==WM_LBUTTONDOWN || message->message==WM_NCLBUTTONDOWN)){
        POINT point={GET_X_LPARAM(message->lParam),GET_Y_LPARAM(message->lParam)};
        int hit=frame->hit(frame,point);
        if(style_menu_pointer(message->message,hit,frame->selected,STYLE_MENU_NO_ITEM,STYLE_MENU_NO_ITEM,frame->armed,&frame->target))
            consume(frame,message);
    }
    if(key && (message->wParam==VK_LEFT || message->wParam==VK_RIGHT) &&
       (!native || frame->active==frame->root) &&
       ((native && !frame->root) || style_menu_bar_arrow(frame->window,frame->root,frame->active,(UINT)message->wParam,!native))){
        int direction=message->wParam==VK_LEFT?-1:1;
        if(native){int next=next_popup(frame->window,frame->selected,direction);
            if(next!=STYLE_MENU_NO_ITEM){frame->target=next;consume(frame,message);}}
        else {frame->direction=direction;consume(frame,message);}
    }
    if(!native && message->hwnd==frame->window &&
       (message->message==WM_NCLBUTTONDBLCLK || message->message==WM_LBUTTONDBLCLK)){
        POINT point={GET_X_LPARAM(message->lParam),GET_Y_LPARAM(message->lParam)};
        if(PtInRect(&frame->button,point)){
            frame->double_click=TRUE;frame->point=MAKELPARAM(point.x,point.y);consume(frame,message);
        }
    }
}
