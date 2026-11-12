#include "lib-style/common/dc_geometry.h"
#include "vcl_extra_frame.h"
#include "vcl_identity.h"
#include "special_paint.h"

void win16_control_bit_theme_frame(const win16_control_bit_frame_view *s,HDC dc,int part,const RECT *rect,const RECT *clip,const void *address)
{
    HMODULE caller=NULL;RECT client;
    if(!s || !s->owner || *s->detached || !dc || dc!=s->dc || part!=1 || !rect ||
        !win16_control_bit_identity(s->window,*s->module) || GetParent(s->window)!=*s->parent ||
        !GetClientRect(s->window,&client) || !EqualRect(rect,&client) || !EqualRect(&s->item,&client) ||
        !style_dc_pixel_viewport(dc) ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)address,&caller) || caller!=*s->module)return;
    if(win16_style_bitbtn_face(s->window,dc,rect,clip,s->state) && !*s->detached)*s->verified=TRUE;
}

BOOL win16_control_controlbar_edge_allowed(UINT edge)
{return edge==BDR_RAISEDOUTER || edge==BDR_SUNKENOUTER || edge==BDR_RAISEDINNER || edge==BDR_SUNKENINNER || edge==EDGE_RAISED || edge==EDGE_SUNKEN || edge==EDGE_ETCHED || edge==EDGE_BUMP;}

void win16_control_controlbar_edge(const win16_control_controlbar_paint_view *s,HDC dc,const RECT *rect,UINT edge,UINT flags,const void *address)
{
 HMODULE caller=NULL;RECT bounds;POINT origin={0};
 if(!s || !*s->candidate || !s->identity(s->window,s->owner) || !dc || !rect || !win16_control_controlbar_edge_allowed(edge) || flags!=(BF_RECT|BF_ADJUST) || WindowFromDC(dc)!=s->window || !style_dc_pixel_viewport(dc) || !GetWindowOrgEx(dc,&origin) || origin.x || origin.y || !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) || caller!=*s->module || !GetWindowRect(s->window,&bounds))return;
 OffsetRect(&bounds,-bounds.left,-bounds.top);if(!EqualRect(rect,&bounds))return;
 BOOL drew=win16_control_black_frame(dc,rect,NULL);if(drew && s->identity(s->window,s->owner))*s->verified=TRUE;
}
