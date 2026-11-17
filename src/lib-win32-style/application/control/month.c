#include "month.h"
#include "lib-win32/process-hook/unwind.h"
#include <vssym32.h>
static void query_end(void *context){style_control_month_queries *queries=context;queries->active=FALSE;}
static BOOL query_current(HWND w,const style_control_month_scope *scope)
{return scope->current(scope->context) && !scope->instance->detached && style_runtime_rich_identity(w,scope->instance,scope->rich,scope->rejected_styles);}
void PROCESS_HOOK_UNWIND_FUNCTION style_control_month_query(const style_control_month_scope *scope,style_control_month_queries *queries)
{
    if(queries->active)return;
    HWND w=scope->window;scope->state->header_valid=FALSE;queries->active=TRUE;process_hook_unwind_scope unwind;PROCESS_HOOK_UNWIND_BEGIN(unwind,query_end,queries);
    MCGRIDINFO grid={.cbSize=sizeof(grid),.dwPart=MCGIP_CALENDARHEADER,.dwFlags=MCGIF_RECT};
    LRESULT count=SendMessageW(w,MCM_GETCALENDARCOUNT,0,0);
    if(count==1 && query_current(w,scope) && SendMessageW(w,MCM_GETCALENDARGRIDINFO,0,(LPARAM)&grid) && query_current(w,scope) && grid.rc.right>grid.rc.left && grid.rc.bottom>grid.rc.top){
        scope->state->header=grid.rc;scope->state->header_valid=TRUE;scope->state->header_known=TRUE;
        MCGRIDINFO prev={.cbSize=sizeof(prev),.dwPart=MCGIP_PREV,.dwFlags=MCGIF_RECT},next={.cbSize=sizeof(next),.dwPart=MCGIP_NEXT,.dwFlags=MCGIF_RECT};
        if(SendMessageW(w,MCM_GETCALENDARGRIDINFO,0,(LPARAM)&prev) && query_current(w,scope) && SendMessageW(w,MCM_GETCALENDARGRIDINFO,0,(LPARAM)&next) && query_current(w,scope) && prev.rc.right>prev.rc.left && next.rc.right>next.rc.left){scope->state->prev=prev.rc;scope->state->next=next.rc;scope->state->nav_valid=TRUE;}
    }
    PROCESS_HOOK_UNWIND_END(unwind);
}
BOOL style_control_month_draw_identity(const style_control_month_scope *scope,HTHEME theme,HDC dc,const void *address)
{
    HMODULE caller=NULL,native=GetModuleHandleW(L"comctl32.dll");return scope && scope->rich->month && scope->rich->witness && !scope->instance->detached && dc && native &&
        style_runtime_rich_identity(scope->window,scope->instance,scope->rich,scope->rejected_styles) && GetWindowTheme(scope->window)==theme &&
        GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) && caller==native;
}
void style_control_month_selection_begin(const style_control_month_scope *scope,HTHEME theme,HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address)
{
    if(!scope)return;
    scope->state->selected=FALSE;
    if(part!=MC_GRIDCELLBACKGROUND || (state!=MCGCB_SELECTED && state!=MCGCB_SELECTEDNOTFOCUSED) || !r || r->right<=r->left || r->bottom<=r->top || !style_control_month_draw_identity(scope,theme,dc,address))return;
    scope->state->selected=TRUE;scope->state->theme=theme;scope->state->dc=dc;scope->state->rect=*r;scope->state->has_clip=clip!=NULL;if(clip)scope->state->clip=*clip;
}
BOOL style_control_month_selection_take(const style_control_month_scope *scope,HTHEME theme,HDC dc,int part,int state,const RECT *r,DWORD flags,const void *address,style_control_month_selection *selection)
{
    if(!scope || !scope->state->selected)return FALSE;
    scope->state->selected=FALSE;const RECT *cell=&scope->state->rect;
    if(part!=MC_GRIDCELL || state!=6 || !r || !IsWindowEnabled(scope->window) || (flags&DT_CALCRECT) || theme!=scope->state->theme || dc!=scope->state->dc ||
        r->left!=cell->left || r->top!=cell->top || r->right>cell->right || r->bottom!=cell->bottom || !style_control_month_draw_identity(scope,theme,dc,address))return FALSE;
    *selection=(style_control_month_selection){.rect=*cell,.has_clip=scope->state->has_clip};
    if(selection->has_clip)selection->clip=scope->state->clip;
    return TRUE;
}
BOOL style_control_month_title_fits(const RECT *r,const RECT *prev,const RECT *next,int width)
{
    if(!r || !prev || !next || width<0 || width>r->right-r->left)return FALSE;
    int left=r->left+(r->right-r->left-width)/2,right=left+width;
    return (right<=prev->left || left>=prev->right) && (right<=next->left || left>=next->right);
}
