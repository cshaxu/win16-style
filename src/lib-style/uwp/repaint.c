#include "repaint.h"
#include "lib-style/common/frame.h"
#include <commctrl.h>

static LRESULT CALLBACK repaint_proc(HWND timer,UINT msg,WPARAM wp,LPARAM lp,UINT_PTR id,DWORD_PTR data)
{
    style_uwp_repaint *s=(style_uwp_repaint *)data;(void)id;
    if(msg==WM_TIMER && wp==(UINT_PTR)s){
        KillTimer(timer,(UINT_PTR)s);s->pending=FALSE;s->services->retain(s->context);
        if(s->waiting){
            HWND owner=s->owner;
            if(!style_frame_xaml_bridge(owner,TRUE) && GetTickCount64()<s->deadline &&
                style_uwp_repaint_queue(owner,s,s->detached,s->context,s->services)){
                s->services->release(s->context);return 0;
            }
            s->waiting=FALSE;s->services->activate(owner,s->context);
        }else{
            s->repainting=TRUE;SendMessageW(s->owner,WM_NCPAINT,1,0);s->repainting=FALSE;
        }
        s->services->release(s->context);return 0;
    }
    return DefSubclassProc(timer,msg,wp,lp);
}
void style_uwp_repaint_cancel(style_uwp_repaint *s)
{
    if(s->timer){KillTimer(s->timer,(UINT_PTR)s);DestroyWindow(s->timer);
        s->timer=NULL;s->pending=FALSE;s->waiting=FALSE;}
}
BOOL style_uwp_repaint_queue(HWND w,style_uwp_repaint *s,const BOOL *detached,void *context,const style_uwp_repaint_services *services)
{
    if(!s->timer){
        HWND timer=CreateWindowExW(0,L"STATIC",L"",0,0,0,0,0,HWND_MESSAGE,NULL,GetModuleHandleW(NULL),NULL);
        if(!timer)return FALSE;
        if(*detached || !IsWindow(w)){DestroyWindow(timer);return FALSE;}
        if(!SetWindowSubclass(timer,repaint_proc,(UINT_PTR)repaint_proc,(DWORD_PTR)s)){DestroyWindow(timer);return FALSE;}
        if(*detached || !IsWindow(w)){DestroyWindow(timer);return FALSE;}
        s->owner=w;s->timer=timer;s->detached=detached;s->context=context;s->services=services;
    }
    if(!SetTimer(s->timer,(UINT_PTR)s,30,NULL))return FALSE;
    s->pending=TRUE;return TRUE;
}
