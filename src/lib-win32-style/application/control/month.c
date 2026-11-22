#include "month.h"
#include "control_chrome.h"
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

/* The witnessed western date strings may contain U+200E bidi marks.
 * Other scripts and directional controls retain the native renderer. */
BOOL style_control_month_text_length(LPCWSTR text,int count,int *length)
{
    int n=0;if(!text || !length || count < -1 || count > 128)return FALSE;
    if(count==-1){while(n<128 && text[n])++n;if(n==128)return FALSE;}else n=count;
    if(!n)return FALSE;
    for(int i=0;i<n;++i)if((text[i]<32 || text[i]>126) && text[i]!=0x200e)return FALSE;
    *length=n;return TRUE;
}

void style_control_month_fill(HDC dc,const RECT *bounds,const RECT *clip,COLORREF color)
{
    int saved=SaveDC(dc);if(!saved)return;
    if(clip && IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom)==ERROR){RestoreDC(dc,saved);return;}
    HBRUSH brush=(HBRUSH)GetStockObject(DC_BRUSH);SetDCBrushColor(dc,color);
    FillRect(dc,bounds,brush);RestoreDC(dc,saved);
}

BOOL style_control_month_text(HTHEME theme,HDC dc,int part,int state,HFONT font,LPCWSTR text,int count,DWORD flags,const RECT *bounds,
    int (WINAPI *draw)(HDC,LPCWSTR,int,LPRECT,UINT),BOOL (*room)(void *,int),void *context)
{
    RECT output=*bounds,measure=*bounds;COLORREF color;HGDIOBJ previous=SelectObject(dc,font);
    if(SUCCEEDED(GetThemeColor(theme,part,state,TMT_TEXTCOLOR,&color)) && previous && previous!=HGDI_ERROR){
        SetTextColor(dc,color);SetBkMode(dc,TRANSPARENT);
        int height=draw(dc,text,count,&measure,flags|DT_CALCRECT);
        if(height>0 && measure.right-measure.left<=bounds->right-bounds->left &&
           measure.bottom-measure.top<=bounds->bottom-bounds->top && room(context,measure.right-measure.left) &&
           IntersectClipRect(dc,bounds->left,bounds->top,bounds->right,bounds->bottom)!=ERROR)
            return draw(dc,text,count,&output,flags)!=0;
    }
    return FALSE;
}

void style_control_month_paint_arrow(HDC dc,const RECT *r,const RECT *clip,int part,int state,const style_window_palette *palette,
    const style_scroll_arrow_art *art)
{
    if(!dc || !r || IsRectEmpty(r) || r->right-r->left>128 || r->right-r->left!=r->bottom-r->top)return;
    int saved=SaveDC(dc);
    if(!saved)return;
    BOOL previous=part==MC_NAVPREV;
    UINT flags=previous?DFCS_SCROLLLEFT:DFCS_SCROLLRIGHT;
    if(state==(previous?MCNP_DISABLED:MCNN_DISABLED))flags|=DFCS_INACTIVE;
    else if(state==(previous?MCNP_PRESSED:MCNN_PRESSED))flags|=DFCS_PUSHED;
    if(!clip || IntersectClipRect(dc,clip->left,clip->top,clip->right,clip->bottom)!=ERROR)
        style_control_outline_scroll_arrow(dc,*r,flags,palette,art);
    RestoreDC(dc,saved);
}

void style_control_month_arrow(const style_control_month_view *v,HTHEME theme,HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address)
{
    if(!v || !r || (part!=MC_NAVNEXT && part!=MC_NAVPREV) || state<MCNN_NORMAL || state>MCNN_DISABLED || !style_control_month_draw_identity(&v->native,theme,dc,address))return;
    style_control_month_paint_arrow(dc,r,clip,part,state,v->palette,v->arrows);
}
static BOOL month_text_allowed(const style_control_month_view *s,HTHEME theme,HDC dc,int part,int state,DWORD flags,DWORD flags2,const RECT *r,const void *address)
{return s && r && (part==MC_GRIDCELL || part==MC_GRIDCELLUPPER) && state>=0 && state<=7 && flags2==0 && (flags==0x925 || flags==0x926 || flags==0x924) && IsWindowEnabled(s->native.window) && style_control_month_draw_identity(&s->native,theme,dc,address);}
HFONT style_control_month_text_borrow(const style_control_month_view *v,HTHEME theme,HDC dc,int part,int state,DWORD flags,DWORD flags2,const RECT *r,const void *address)
{
    if(!v || !v->font_dpi || !month_text_allowed(v,theme,dc,part,state,flags,flags2,r,address) ||
       GetDpiForWindow(v->native.window)!=96)return NULL;
    if(!*v->cached_font)*v->cached_font=v->acquire_font(v->font_dpi,v->font_points);
    if(!v->native.current(v->native.context) || !month_text_allowed(v,theme,dc,part,state,flags,flags2,r,address) ||
       !*v->cached_font || !v->retain_font(*v->cached_font))return NULL;
    return *v->cached_font;
}
BOOL style_control_month_text_room(const style_control_month_view *v,HTHEME theme,HDC dc,int part,int state,DWORD flags,DWORD flags2,const RECT *r,int width,const void *address)
{const style_control_month_view *s=v;if(!month_text_allowed(s,theme,dc,part,state,flags,flags2,r,address) || !s->native.state->header_known)return FALSE;
 if(EqualRect(r,&s->native.state->header))return s->native.state->nav_valid && style_control_month_title_fits(r,&s->native.state->prev,&s->native.state->next,width);
 return TRUE;}
void style_control_month_header_text(const style_control_month_view *v,HTHEME theme,HDC dc,int part,int state,const RECT *r,DWORD flags,const void *address)
{const style_control_month_view *s=v;COLORREF text;if(!s || !s->native.state->header_valid || !IsWindowEnabled(s->native.window) || part!=MC_GRIDCELL || state!=0 || !r || (flags&DT_CALCRECT) || !EqualRect(r,&s->native.state->header) ||
 !style_control_month_draw_identity(&s->native,theme,dc,address) || FAILED(GetThemeColor(theme,part,state,TMT_TEXTCOLOR,&text)) || text!=RGB(0,0,0))return;
 s->native.state->header_valid=FALSE;style_control_month_fill(dc,r,NULL,s->palette->active);}
void style_control_month_selection_text(const style_control_month_view *v,HTHEME theme,HDC dc,int part,int state,const RECT *r,DWORD flags,const void *address)
{
    style_control_month_selection selected;COLORREF foreground;
    if(!v || !style_control_month_selection_take(&v->native,theme,dc,part,state,r,flags,address,&selected) ||
        FAILED(GetThemeColor(theme,part,state,TMT_TEXTCOLOR,&foreground)) || foreground!=RGB(0,0,0))return;
    style_control_month_fill(dc,&selected.rect,selected.has_clip?&selected.clip:NULL,v->selection);
}
