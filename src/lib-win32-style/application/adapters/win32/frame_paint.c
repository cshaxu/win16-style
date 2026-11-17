#include "../../control/control_vcl_identity.h"
#include "frame_paint.h"
#include "lib-win32-style/application/graphics/dc_geometry.h"
#include <string.h>
#include <wchar.h>

typedef struct frame_result {style_instance *instance;style_runtime_frame_state *state;HWND parent;} frame_result;
static void frame_drawn(void *context)
{frame_result *result=context;if(!result->instance->detached){result->state->parent=result->parent;result->state->verified=TRUE;}}

/* Only the newly created group's own verified paint may draw this frame. */
void style_runtime_group_frame(const style_runtime_frame_paint *v,const style_runtime_frame_renderer *renderer,HDC dc,int part,int state,const RECT *rect,const RECT *clip,LPCWSTR text,int count,DWORD flags,const void *caller,BOOL is_text)
{
 style_runtime_frame_scope *scope=v->scope;void *a=v->owner;HWND w=v->window,parent=w?GetParent(w):NULL;
 WCHAR cls[64],real[64],caption[512];HMODULE module=NULL;RECT client={0},frame={0};DWORD pid=0;
 if(!a||v->instance->detached||!v->state->candidate||v->state->retired||!v->owned(w,a)||!parent||part!=4||!rect||!((state==1&&IsWindowEnabled(w))||(state==2&&!IsWindowEnabled(w)))||
    GetWindowThreadProcessId(w,&pid)!=GetCurrentThreadId()||pid!=GetCurrentProcessId()||GetWindowThreadProcessId(parent,NULL)!=GetCurrentThreadId()||
    (v->state->parent&&v->state->parent!=parent)||
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
 if(v->instance->detached||v->state->retired||!v->current(scope)||!v->owned(w,a)||GetParent(w)!=parent||IsWindowEnabled(w)!=(state==1))return;
 frame_result result={v->instance,v->state,parent};renderer->group(dc,&frame,clip,is_text?rect:NULL,frame_drawn,&result);
}
/* Accept only paired outer bevel strokes from the panel owner in its paint. */
void style_runtime_panel_polyline(const style_runtime_frame_paint *v,const style_runtime_frame_renderer *renderer,HDC dc,const POINT *points,int count,const void *caller,BOOL (WINAPI *draw)(HDC,const POINT*,int))
{
 style_runtime_frame_scope *scope=v->scope;void *a=v->owner;HWND w=v->window,parent=w?GetParent(w):NULL;
 HMODULE module=NULL;RECT client={0},clip={0};LOGPEN pen={0};DWORD pid=0;
 if(!a||v->instance->detached||!v->state->candidate||v->state->retired||!v->owned(w,a)||!parent||
    (v->state->parent&&v->state->parent!=parent)||GetWindowThreadProcessId(w,&pid)!=GetCurrentThreadId()||pid!=GetCurrentProcessId()||
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
 const POINT *shadow=scope->panel_color==lo?scope->panel_points:points;frame_result result={v->instance,v->state,parent};
 renderer->panel(dc,shadow,draw,frame_drawn,&result);
}

BOOL style_runtime_controlbar_edge_allowed(UINT edge)
{return edge==BDR_RAISEDOUTER || edge==BDR_SUNKENOUTER || edge==BDR_RAISEDINNER || edge==BDR_SUNKENINNER || edge==EDGE_RAISED || edge==EDGE_SUNKEN || edge==EDGE_ETCHED || edge==EDGE_BUMP;}

void style_runtime_controlbar_edge(HWND window,style_instance *instance,style_runtime_frame_state *state,HDC dc,const RECT *rect,UINT edge,UINT flags,const void *address,BOOL (*render)(HDC,const RECT *,const RECT *))
{
 HMODULE caller=NULL;RECT bounds;POINT origin={0};
 if(!state || !state->candidate || !style_runtime_controlbar_identity(window,instance,state) || !dc || !rect || !style_runtime_controlbar_edge_allowed(edge) || flags!=(BF_RECT|BF_ADJUST) || WindowFromDC(dc)!=window || !style_dc_pixel_viewport(dc) || !GetWindowOrgEx(dc,&origin) || origin.x || origin.y || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) || caller!=state->owner || !GetWindowRect(window,&bounds))return;
 OffsetRect(&bounds,-bounds.left,-bounds.top);if(!EqualRect(rect,&bounds))return;
 BOOL drew=render(dc,rect,NULL);if(drew && style_runtime_controlbar_identity(window,instance,state))state->verified=TRUE;
}

void style_runtime_bit_frame(const style_runtime_bit_frame_paint *s,HDC dc,int part,const RECT *rect,const RECT *clip,const void *address,BOOL (*paint)(HWND,HDC,const RECT *,const RECT *,UINT))
{
    HMODULE caller=NULL;RECT client;
    if(!s || !s->instance || s->instance->detached || !dc || dc!=s->dc || part!=1 || !rect ||
        !style_control_vcl_bit_identity(s->window,s->report->module) || GetParent(s->window)!=s->report->parent ||
        !GetClientRect(s->window,&client) || !EqualRect(rect,&client) || !EqualRect(&s->item,&client) ||
        !style_dc_pixel_viewport(dc) ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)address,&caller) || caller!=s->report->module)return;
    if(paint(s->window,dc,rect,clip,s->state) && !s->instance->detached)s->verification->verified=TRUE;
}
