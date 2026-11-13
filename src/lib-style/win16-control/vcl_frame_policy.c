#include "vcl_frame_policy.h"
#include "special_paint.h"
#include "lib-style/common/dc_geometry.h"
#include <string.h>
#include <wchar.h>

/* Only the newly created group's own verified paint may draw this frame. */
void win16_control_vcl_group_frame(const win16_control_vcl_frame_view *v,HDC dc,int part,int state,const RECT *rect,const RECT *clip,LPCWSTR text,int count,DWORD flags,const void *caller,BOOL is_text)
{
 win16_control_vcl_frame_scope *scope=v->scope;void *a=v->owner;HWND w=v->window,parent=w?GetParent(w):NULL;
 WCHAR cls[64],real[64],caption[512];HMODULE module=NULL;RECT client={0},frame={0};DWORD pid=0;
 if(!a||*v->detached||!*v->candidate||*v->retired||!v->owned(w,a)||!parent||part!=4||!rect||!((state==1&&IsWindowEnabled(w))||(state==2&&!IsWindowEnabled(w)))||
    GetWindowThreadProcessId(w,&pid)!=GetCurrentThreadId()||pid!=GetCurrentProcessId()||GetWindowThreadProcessId(parent,NULL)!=GetCurrentThreadId()||
    (*v->parent&&*v->parent!=parent)||
    !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)caller,&module)||
    module!=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)||module!=(HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)||WindowFromDC(dc)!=w||
    !GetClassNameW(w,cls,64)||(lstrcmpW(cls,L"TGroupBox")&&lstrcmpW(cls,L"TRadioGroup"))||!RealGetWindowClassW(w,real,64)||lstrcmpW(real,cls)||!(GetWindowLongPtrW(w,GWL_STYLE)&WS_CHILD)||
    (GetWindowLongPtrW(w,GWL_EXSTYLE)&(WS_EX_LAYOUTRTL|WS_EX_RTLREADING))||!style_dc_origin_pixels(dc)||
    !GetClientRect(w,&client)||client.right<4||client.bottom<4)return;
 if(is_text){int n;if(!text||count<=0||count>=512||flags!=0||(n=GetWindowTextW(w,caption,512))!=count||wcsncmp(caption,text,(size_t)count)||
      rect->left<0||rect->right>client.right||rect->top!=0||rect->bottom<=0||rect->bottom>=client.bottom)return;
    frame=(RECT){0,rect->top+(rect->bottom-rect->top)/2,client.right,client.bottom};
    if(!scope->group_background||scope->group_state!=state||!EqualRect(&frame,&scope->group_rect))return;
 }else {if(rect->left!=0||rect->top<0||rect->right!=client.right||rect->bottom!=client.bottom)return;scope->group_background=TRUE;scope->group_state=state;scope->group_rect=*rect;
    if(GetWindowTextLengthW(w)!=0||rect->top!=0)return;
    frame=*rect;}
 if(*v->detached||*v->retired||!v->current(scope)||!v->owned(w,a)||GetParent(w)!=parent||IsWindowEnabled(w)!=(state==1))return;
 win16_control_vcl_frame_result result={a,parent};win16_control_group_frame(dc,&frame,clip,is_text?rect:NULL,v->drawn,&result);
}
/* Accept only paired outer bevel strokes from the panel owner in its paint. */
void win16_control_vcl_panel_polyline(const win16_control_vcl_frame_view *v,HDC dc,const POINT *points,int count,const void *caller,BOOL (WINAPI *draw)(HDC,const POINT*,int))
{
 win16_control_vcl_frame_scope *scope=v->scope;void *a=v->owner;HWND w=v->window,parent=w?GetParent(w):NULL;
 HMODULE module=NULL;RECT client={0},clip={0};LOGPEN pen={0};DWORD pid=0;
 if(!a||*v->detached||!*v->candidate||*v->retired||!v->owned(w,a)||!parent||
    (*v->parent&&*v->parent!=parent)||GetWindowThreadProcessId(w,&pid)!=GetCurrentThreadId()||pid!=GetCurrentProcessId()||
    GetWindowThreadProcessId(parent,NULL)!=GetCurrentThreadId()||
    !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)caller,&module)||
    module!=(HMODULE)GetClassLongPtrW(w,GCLP_HMODULE)||module!=(HMODULE)GetClassLongPtrW(parent,GCLP_HMODULE)||
    WindowFromDC(dc)!=w||count!=3||!points||!style_dc_origin_pixels(dc)||
    !GetClientRect(w,&client)||client.right<4||client.bottom<4||GetClipBox(dc,&clip)<=NULLREGION||
    (clip.left>0&&clip.top>0&&clip.right<client.right&&clip.bottom<client.bottom)||
    GetObjectW(GetCurrentObject(dc,OBJ_PEN),sizeof(pen),&pen)!=sizeof(pen)||pen.lopnStyle!=PS_SOLID||pen.lopnWidth.x!=1){if(scope)scope->panel_first=FALSE;return;}
 COLORREF hi=GetSysColor(COLOR_BTNHIGHLIGHT),lo=GetSysColor(COLOR_BTNSHADOW);if(hi==lo){scope->panel_first=FALSE;return;}
 if(points[0].x==0&&points[0].y==client.bottom-1&&points[1].x==0&&points[1].y==0&&points[2].x==client.right-1&&points[2].y==0&&(pen.lopnColor==hi||pen.lopnColor==lo)){
    scope->panel_first=TRUE;scope->panel_enabled=IsWindowEnabled(w);scope->panel_parent=parent;scope->panel_dc=dc;scope->panel_color=pen.lopnColor;memcpy(scope->panel_points,points,sizeof(scope->panel_points));return;}
 if(!scope->panel_first||scope->panel_enabled!=IsWindowEnabled(w)||scope->panel_parent!=parent||scope->panel_dc!=dc||points[0].x!=client.right-1||points[0].y!=0||points[1].x!=client.right-1||points[1].y!=client.bottom-1||points[2].x!=-1||points[2].y!=client.bottom-1||
    !((scope->panel_color==hi&&pen.lopnColor==lo)||(scope->panel_color==lo&&pen.lopnColor==hi))){scope->panel_first=FALSE;return;}
 scope->panel_first=FALSE;if(scope->panel_enabled!=IsWindowEnabled(w)||GetParent(w)!=parent)return;
 const POINT *shadow=scope->panel_color==lo?scope->panel_points:points;win16_control_vcl_frame_result result={a,parent};
 win16_control_panel_shadow(dc,shadow,draw,v->drawn,&result);
}
