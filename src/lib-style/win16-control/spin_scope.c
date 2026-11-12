#include "spin_scope.h"
#include "spin_identity.h"
#include "special_paint.h"
#include "../common/control_protocol.h"
#include <string.h>

void win16_control_spin_buffers_clear(spin_draw_scope *s)
{if(s->buffers){HeapFree(GetProcessHeap(),0,s->buffers);s->buffers=NULL;}}

void win16_control_spin_dc_created(spin_draw_scope *s,HDC source,HDC result)
{

    if(!s || !*s->modern_spin || !result || (WindowFromDC(source)!=s->window && (!s->dc || source!=s->dc)))return;
    for(unsigned i=0;i<16;i++)if(!s->clones[i]){s->clones[i]=result;return;}
}
static void spin_snapshot_invalidate(spin_draw_scope *s)
{
    s->snapshot_dc=NULL;s->snapshot_bitmap=NULL;s->states[0]=s->states[1]=0;
    if(s->buffers)memset(s->buffers->valid,0,sizeof(s->buffers->valid));
}
/* Invalidate before forwarding every new theme draw on the witnessed DC.
 * An unknown theme or a failed draw must never leave its old pixels eligible. */
void win16_control_spin_theme_begin(spin_draw_scope *s,HDC dc,const RECT *rect)
{
    RECT client,safe;SIZE_T got=0;POINT pt[2];
    if(!s || !s->buffers || s->snapshot_dc!=dc)return;
    if(!rect || !ReadProcessMemory(GetCurrentProcess(),rect,&safe,sizeof(safe),&got) || got!=sizeof(safe) ||
       !GetClientRect(s->window,&client) || client.right<=0 || client.bottom<=0 || client.right>64 || client.bottom>64){
        spin_snapshot_invalidate(s);return;
    }
    pt[0]=(POINT){safe.left,safe.top};pt[1]=(POINT){safe.right,safe.bottom};
    if(!LPtoDP(dc,pt,2) || pt[0].x!=0 || pt[1].x!=client.right || pt[0].y<0 || pt[1].y>client.bottom ||
       pt[1].y<=pt[0].y || !((pt[0].y==0 && pt[1].y<client.bottom) || (pt[0].y>0 && pt[1].y==client.bottom))){
        spin_snapshot_invalidate(s);return;
    }
    s->states[pt[0].y==0?0:1]=0;
    for(int y=pt[0].y;y<pt[1].y;y++)for(int x=0;x<client.right;x++)s->buffers->valid[y*64+x]=0;
}
void win16_control_spin_dc_deleted(spin_draw_scope *stack,HDC dc)
{
    for(spin_draw_scope *s=stack;s;s=s->previous_stack){
        for(unsigned i=0;i<16;i++)if(s->clones[i]==dc)s->clones[i]=NULL;
        if(s->snapshot_dc==dc)spin_snapshot_invalidate(s);
    }
}
void win16_control_spin_selected(spin_draw_scope *stack,HDC dc)
{
    for(spin_draw_scope *s=stack;s;s=s->previous_stack)
        if(s->snapshot_dc==dc && GetCurrentObject(dc,OBJ_BITMAP)!=s->snapshot_bitmap)spin_snapshot_invalidate(s);
}
void win16_control_spin_object_deleted(spin_draw_scope *stack,HGDIOBJ object)
{
    for(spin_draw_scope *s=stack;s;s=s->previous_stack)
        if(s->snapshot_bitmap==object)spin_snapshot_invalidate(s);
}
void win16_control_spin_theme(spin_draw_scope *s,HDC dc,int part,int state,const RECT *rect,const RECT *clip,const void *caller)
{
    HMODULE module=NULL;BOOL owned=FALSE;RECT client;POINT pt[2];
    if(!s || !*s->modern_spin || *s->detached || !rect || clip || part!=1 || state<1 || state>4 ||
       !win16_control_modern_spin_shape(s->window) || GetParent(s->window)!=s->parent || !style_control_protocol_is(s->parent,L"EDIT") ||
       GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)caller,&module) ||
       module!=(HMODULE)GetClassLongPtrW(s->window,GCLP_HMODULE) || !GetClientRect(s->window,&client))return;
    if(WindowFromDC(dc)==s->window)owned=TRUE;
    for(unsigned i=0;i<16;i++)if(s->clones[i]==dc)owned=TRUE;
    if(!owned || rect->left!=0 || rect->top!=0 || rect->right!=client.right || rect->bottom<=0 ||
       client.right<=0 || client.bottom<=0 || client.right>64 || client.bottom>64)return;
    pt[0]=(POINT){rect->left,rect->top};pt[1]=(POINT){rect->right,rect->bottom};
    if(!LPtoDP(dc,pt,2) || pt[0].x!=0 || pt[1].x!=client.right || pt[0].y<0 || pt[1].y>client.bottom ||
       pt[1].y<=pt[0].y || !((pt[0].y==0 && pt[1].y<client.bottom) || (pt[0].y>0 && pt[1].y==client.bottom)))return;
    if(!win16_control_spin_full_clip(dc,pt))return;
    if(!s->buffers){s->buffers=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*s->buffers));if(!s->buffers)return;}
    if(s->snapshot_dc!=dc || s->snapshot_bitmap!=GetCurrentObject(dc,OBJ_BITMAP)){
        s->snapshot_dc=dc;s->snapshot_bitmap=GetCurrentObject(dc,OBJ_BITMAP);
        memset(s->buffers->valid,0,sizeof(s->buffers->valid));s->states[0]=s->states[1]=0;
    }
    int half=pt[0].y==0?0:1;s->states[half]=0;
    for(int y=pt[0].y;y<pt[1].y;y++)for(int x=0;x<client.right;x++)s->buffers->valid[y*64+x]=0;
    if(!win16_control_spin_snapshot(dc,client.right,client.bottom,pt,s->buffers->background,s->buffers->valid))return;
    s->halves[half]=(RECT){pt[0].x,pt[0].y,pt[1].x,pt[1].y};s->states[half]=state;
}
void win16_control_spin_blit(spin_draw_scope *s,HDC dest,int x,int y,int width,int height,HDC source,int sx,int sy,DWORD rop)
{
    RECT r;
    if(!s || !s->buffers || !*s->modern_spin || *s->detached || !win16_control_modern_spin_shape(s->window) ||
       GetParent(s->window)!=s->parent || !style_control_protocol_is(s->parent,L"EDIT") || !GetClientRect(s->window,&r) ||
       (WindowFromDC(dest)!=s->window && (!s->dc || dest!=s->dc)) || source!=s->snapshot_dc ||
       GetCurrentObject(source,OBJ_BITMAP)!=s->snapshot_bitmap || x<0 || y<0 || sx!=x || sy!=y ||
       width<=0 || height<=0 || x>r.right-width || y>r.bottom-height || r.right>64 || r.bottom>64 ||
       rop!=SRCCOPY || !s->states[0] || !s->states[1])return;
    for(int j=y;j<y+height;j++)for(int i=x;i<x+width;i++)if(!s->buffers->valid[j*64+i])return;
    BOOL copied=win16_control_spin_compose(s->window,dest,x,y,width,height,source,r.right,r.bottom,
        s->halves,s->states,s->buffers->background,s->buffers->original);
    if(copied && !*s->detached)*s->verified=TRUE;
}

/* Forward the native primitive first. Its adjusted rectangle, return value and
 * error remain caller-owned; this overlay never draws the application arrow. */
void win16_control_spin_frame(spin_draw_scope *s,HDC dc,const RECT *rect,UINT type,UINT flags,const void *caller)
{
    HMODULE module=NULL;RECT client,clip;POINT points[2];
    const UINT allowed=DFCS_BUTTONPUSH|DFCS_PUSHED|DFCS_ADJUSTRECT;
    if(!s || *s->detached || !dc || !rect || type!=DFC_BUTTON ||
       (flags&~allowed) || (flags&(DFCS_BUTTONPUSH|DFCS_ADJUSTRECT))!=(DFCS_BUTTONPUSH|DFCS_ADJUSTRECT) ||
       !win16_control_spin_identity(s->window) || GetParent(s->window)!=s->parent || WindowFromDC(dc)!=s->window ||
       (s->dc && s->dc!=dc) || GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)caller,&module) || module!=GetModuleHandleW(NULL) || !GetClientRect(s->window,&client) ||
       client.right<=0 || client.bottom<=0 || client.right>256 || client.bottom>256 ||
       rect->left!=0 || rect->top!=0 || rect->right!=client.right || rect->bottom<=0 ||
       GetClipBox(dc,&clip)==ERROR || clip.left<rect->left || clip.top<rect->top ||
       clip.right>rect->right || clip.bottom>rect->bottom)return;
    points[0]=(POINT){rect->left,rect->top};points[1]=(POINT){rect->right,rect->bottom};
    if(!LPtoDP(dc,points,2) || points[0].x!=0 || points[1].x!=client.right ||
       points[0].y<0 || points[1].y>client.bottom || points[1].y<=points[0].y ||
       (points[0].y!=0 && points[1].y!=client.bottom) || *s->detached)return;
    win16_control_black_frame(dc,rect,NULL);
}
