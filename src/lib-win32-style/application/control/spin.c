#include "spin.h"
#include "lib-win32-style/application/control/control_vcl_spin.h"
#include "../graphics/bitmap.h"
#include "lib-win32-style/application/control/control_protocol.h"
#include "../graphics/clip.h"
#include <string.h>

static BOOL spin_dib_open(HDC source,int width,int height,style_bitmap *d)
{
    SIZE size={width,height};if(!style_bitmap_ensure(d,source,size))return FALSE;
    int saved=SaveDC(source);BOOL ok=FALSE;
    if(saved){SetViewportOrgEx(source,0,0,NULL);SetWindowOrgEx(source,0,0,NULL);SelectClipRgn(source,NULL);
        ok=BitBlt(d->dc,0,0,width,height,source,0,0,SRCCOPY);GdiFlush();RestoreDC(source,saved);}
    if(!ok){style_bitmap_dispose(d);return FALSE;}return TRUE;
}
static BOOL spin_snapshot(HDC dc,int width,int height,const POINT pt[2],DWORD *background,BYTE *valid)
{
    style_bitmap d={0};if(!spin_dib_open(dc,width,height,&d))return FALSE;
    const DWORD *pixels=(const DWORD *)d.pixels;
    for(int y=pt[0].y;y<pt[1].y;y++)for(int x=0;x<width;x++){
        int at=y*64+x;background[at]=pixels[y*width+x]&0x00ffffff;valid[at]=1;
    }
    style_bitmap_dispose(&d);return TRUE;
}
static BOOL spin_compose(HWND window,HDC dest,int x,int y,int width,int height,HDC source,
    int source_width,int source_height,const RECT halves[2],const int states[2],const DWORD *background,DWORD *original,BOOL (*face)(HWND,HDC,const RECT *,const RECT *,UINT))
{
    style_bitmap d={0};if(!spin_dib_open(source,source_width,source_height,&d))return FALSE;
    DWORD *pixels=(DWORD *)d.pixels;
    for(int j=y;j<y+height;j++)for(int i=x;i<x+width;i++)original[j*64+i]=pixels[j*source_width+i];
    for(int half=0;half<2;half++)if(!face(window,d.dc,&halves[half],NULL,states[half]==3?ODS_SELECTED:0)){style_bitmap_dispose(&d);return FALSE;}
    GdiFlush();for(int j=y;j<y+height;j++)for(int i=x;i<x+width;i++){
        int at=j*64+i;if((original[at]&0x00ffffff)!=background[at])pixels[j*source_width+i]=original[at];
    }
    BOOL copied=BitBlt(dest,x,y,width,height,d.dc,x,y,SRCCOPY);style_bitmap_dispose(&d);return copied;
}

BOOL style_control_spin_message(const style_instance *instance,const style_control_spin_state *state,UINT m)
{
    return state->candidate && !instance->detached && (m==WM_PAINT || m==WM_PRINT || m==WM_PRINTCLIENT ||
        m==WM_LBUTTONDOWN || m==WM_LBUTTONUP || m==WM_MOUSEMOVE || m==WM_ENABLE || m==WM_CANCELMODE ||
        m==WM_SETFOCUS || m==WM_KILLFOCUS);
}
void style_control_spin_buffers_clear(style_control_spin_scope *s)
{if(s->buffers){HeapFree(GetProcessHeap(),0,s->buffers);s->buffers=NULL;}}

void style_control_spin_dc_created(style_control_spin_scope *s,HDC source,HDC result)
{

    if(!s || !s->state->modern || !result || (WindowFromDC(source)!=s->window && (!s->dc || source!=s->dc)))return;
    for(unsigned i=0;i<16;i++)if(!s->clones[i]){s->clones[i]=result;return;}
}
static void spin_snapshot_invalidate(style_control_spin_scope *s)
{
    s->snapshot_dc=NULL;s->snapshot_bitmap=NULL;s->states[0]=s->states[1]=0;
    if(s->buffers)memset(s->buffers->valid,0,sizeof(s->buffers->valid));
}
/* Invalidate before forwarding every new theme draw on the witnessed DC.
 * An unknown theme or a failed draw must never leave its old pixels eligible. */
void style_control_spin_theme_begin(style_control_spin_scope *s,HDC dc,const RECT *rect)
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
void style_control_spin_dc_deleted(style_control_spin_scope *stack,HDC dc)
{
    for(style_control_spin_scope *s=stack;s;s=s->previous_stack){
        for(unsigned i=0;i<16;i++)if(s->clones[i]==dc)s->clones[i]=NULL;
        if(s->snapshot_dc==dc)spin_snapshot_invalidate(s);
    }
}
void style_control_spin_selected(style_control_spin_scope *stack,HDC dc)
{
    for(style_control_spin_scope *s=stack;s;s=s->previous_stack)
        if(s->snapshot_dc==dc && GetCurrentObject(dc,OBJ_BITMAP)!=s->snapshot_bitmap)spin_snapshot_invalidate(s);
}
void style_control_spin_object_deleted(style_control_spin_scope *stack,HGDIOBJ object)
{
    for(style_control_spin_scope *s=stack;s;s=s->previous_stack)
        if(s->snapshot_bitmap==object)spin_snapshot_invalidate(s);
}
void style_control_spin_theme(style_control_spin_scope *s,HDC dc,int part,int state,const RECT *rect,const RECT *clip,const void *caller)
{
    HMODULE module=NULL;BOOL owned=FALSE;RECT client;POINT pt[2];
    if(!s || !s->state->modern || s->instance->detached || !rect || clip || part!=1 || state<1 || state>4 ||
       !style_control_vcl_modern_spin_shape(s->window) || GetParent(s->window)!=s->parent || !style_control_protocol_is(s->parent,L"EDIT") ||
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
    if(!style_clip_covers(dc,pt))return;
    if(!s->buffers){s->buffers=HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,sizeof(*s->buffers));if(!s->buffers)return;}
    if(s->snapshot_dc!=dc || s->snapshot_bitmap!=GetCurrentObject(dc,OBJ_BITMAP)){
        s->snapshot_dc=dc;s->snapshot_bitmap=GetCurrentObject(dc,OBJ_BITMAP);
        memset(s->buffers->valid,0,sizeof(s->buffers->valid));s->states[0]=s->states[1]=0;
    }
    int half=pt[0].y==0?0:1;s->states[half]=0;
    for(int y=pt[0].y;y<pt[1].y;y++)for(int x=0;x<client.right;x++)s->buffers->valid[y*64+x]=0;
    if(!spin_snapshot(dc,client.right,client.bottom,pt,s->buffers->background,s->buffers->valid))return;
    s->halves[half]=(RECT){pt[0].x,pt[0].y,pt[1].x,pt[1].y};s->states[half]=state;
}
void style_control_spin_blit(style_control_spin_scope *s,HDC dest,int x,int y,int width,int height,HDC source,int sx,int sy,DWORD rop,BOOL (*face)(HWND,HDC,const RECT *,const RECT *,UINT))
{
    RECT r;
    if(!s || !s->buffers || !s->state->modern || s->instance->detached || !style_control_vcl_modern_spin_shape(s->window) ||
       GetParent(s->window)!=s->parent || !style_control_protocol_is(s->parent,L"EDIT") || !GetClientRect(s->window,&r) ||
       (WindowFromDC(dest)!=s->window && (!s->dc || dest!=s->dc)) || source!=s->snapshot_dc ||
       GetCurrentObject(source,OBJ_BITMAP)!=s->snapshot_bitmap || x<0 || y<0 || sx!=x || sy!=y ||
       width<=0 || height<=0 || x>r.right-width || y>r.bottom-height || r.right>64 || r.bottom>64 ||
       rop!=SRCCOPY || !s->states[0] || !s->states[1])return;
    for(int j=y;j<y+height;j++)for(int i=x;i<x+width;i++)if(!s->buffers->valid[j*64+i])return;
    BOOL copied=spin_compose(s->window,dest,x,y,width,height,source,r.right,r.bottom,
        s->halves,s->states,s->buffers->background,s->buffers->original,face);
    if(copied && !s->instance->detached)s->state->verified=TRUE;
}

/* Forward the native primitive first. Its adjusted rectangle, return value and
 * error remain caller-owned; this overlay never draws the application arrow. */
void style_control_spin_frame(style_control_spin_scope *s,HDC dc,const RECT *rect,UINT type,UINT flags,const void *caller,BOOL (*frame)(HDC,const RECT *,const RECT *))
{
    HMODULE module=NULL;RECT client,clip;POINT points[2];
    const UINT allowed=DFCS_BUTTONPUSH|DFCS_PUSHED|DFCS_ADJUSTRECT;
    if(!s || s->instance->detached || !dc || !rect || type!=DFC_BUTTON ||
       (flags&~allowed) || (flags&(DFCS_BUTTONPUSH|DFCS_ADJUSTRECT))!=(DFCS_BUTTONPUSH|DFCS_ADJUSTRECT) ||
       !style_control_vcl_spin_identity(s->window) || GetParent(s->window)!=s->parent || WindowFromDC(dc)!=s->window ||
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
       (points[0].y!=0 && points[1].y!=client.bottom) || s->instance->detached)return;
    frame(dc,rect,NULL);
}
