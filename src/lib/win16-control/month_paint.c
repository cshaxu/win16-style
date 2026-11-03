#include "month_paint.h"
#include "month_text.h"
#include "special_paint.h"
#include "../win16-common/font.h"
#include <vssym32.h>
#include <commctrl.h>

void win16_control_month_arrow(const win16_control_month_view *v,HTHEME theme,HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address)
{const win16_control_month_view *s=v;HMODULE caller=NULL,native=GetModuleHandleW(L"comctl32.dll");
 if(!s || !*s->rich_month || !*s->rich_witness || *s->detached || !r || (part!=MC_NAVNEXT && part!=MC_NAVPREV) || state<MCNN_NORMAL || state>MCNN_DISABLED ||
    !s->identity(s->window,s->owner) || !native || GetWindowTheme(s->window)!=theme ||
    !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) || caller!=native)return;
 win16_style_month_arrow(dc,r,clip,part,state);
}
static BOOL month_draw_identity(const win16_control_month_view *s,HTHEME theme,HDC dc,const void *address)
{HMODULE caller=NULL,native=GetModuleHandleW(L"comctl32.dll");return s && *s->rich_month && *s->rich_witness && !*s->detached && dc && native &&
 s->identity(s->window,s->owner) && GetWindowTheme(s->window)==theme &&
 GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) && caller==native;}
static BOOL month_text_allowed(const win16_control_month_view *s,HTHEME theme,HDC dc,int part,int state,DWORD flags,DWORD flags2,const RECT *r,const void *address)
{return s && r && (part==MC_GRIDCELL || part==MC_GRIDCELLUPPER) && state>=0 && state<=7 && flags2==0 && (flags==0x925 || flags==0x926 || flags==0x924) && IsWindowEnabled(s->window) && month_draw_identity(s,theme,dc,address);}
HFONT win16_control_month_text_borrow(const win16_control_month_view *v,HTHEME theme,HDC dc,int part,int state,DWORD flags,DWORD flags2,const RECT *r,const void *address)
{const win16_control_month_view *s=v;if(!month_text_allowed(s,theme,dc,part,state,flags,flags2,r,address))return NULL;if(GetDpiForWindow(s->window)!=96)return NULL;if(!*s->cached_font)*s->cached_font=win16_style_calendar_font_acquire();if(!s->current(s->scope) || !month_text_allowed(s,theme,dc,part,state,flags,flags2,r,address) || !*s->cached_font || !win16_style_system_try_retain(*s->cached_font))return NULL;return *s->cached_font;}
BOOL win16_control_month_text_room(const win16_control_month_view *v,HTHEME theme,HDC dc,int part,int state,DWORD flags,DWORD flags2,const RECT *r,int width,const void *address)
{const win16_control_month_view *s=v;if(!month_text_allowed(s,theme,dc,part,state,flags,flags2,r,address) || !s->state->header_known)return FALSE;
 if(EqualRect(r,&s->state->header))return s->state->nav_valid && ntw16s_month_title_fits(r,&s->state->prev,&s->state->next,width);
 return TRUE;}
void win16_control_month_header_text(const win16_control_month_view *v,HTHEME theme,HDC dc,int part,int state,const RECT *r,DWORD flags,const void *address)
{const win16_control_month_view *s=v;COLORREF text;if(!s || !s->state->header_valid || !IsWindowEnabled(s->window) || part!=MC_GRIDCELL || state!=0 || !r || (flags&DT_CALCRECT) || !EqualRect(r,&s->state->header) ||
 !month_draw_identity(s,theme,dc,address) || FAILED(GetThemeColor(theme,part,state,TMT_TEXTCOLOR,&text)) || text!=RGB(0,0,0))return;
 s->state->header_valid=FALSE;win16_control_month_fill(dc,r,NULL);}
void win16_control_month_selection_background(const win16_control_month_view *v,HTHEME theme,HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address)
{const win16_control_month_view *s=v;if(!s)return;s->state->selected=FALSE;if(part!=MC_GRIDCELLBACKGROUND || (state!=MCGCB_SELECTED && state!=MCGCB_SELECTEDNOTFOCUSED) || !r || r->right<=r->left || r->bottom<=r->top || !month_draw_identity(s,theme,dc,address))return;
 s->state->selected=TRUE;s->state->theme=theme;s->state->dc=dc;s->state->rect=*r;s->state->has_clip=clip!=NULL;if(clip)s->state->clip=*clip;}
void win16_control_month_selection_text(const win16_control_month_view *v,HTHEME theme,HDC dc,int part,int state,const RECT *r,DWORD flags,const void *address)
{const win16_control_month_view *s=v;if(!s||!s->state->selected)return;s->state->selected=FALSE;COLORREF foreground;RECT *cell=&s->state->rect;
 if(part!=MC_GRIDCELL || state!=6 || !r || !IsWindowEnabled(s->window) || (flags&DT_CALCRECT) || theme!=s->state->theme || dc!=s->state->dc ||
    r->left!=cell->left || r->top!=cell->top || r->right>cell->right || r->bottom!=cell->bottom ||
    !month_draw_identity(s,theme,dc,address) || FAILED(GetThemeColor(theme,part,state,TMT_TEXTCOLOR,&foreground)) || foreground!=RGB(0,0,0))return;
 win16_control_month_fill(dc,cell,s->state->has_clip?&s->state->clip:NULL);
}

void win16_control_month_query(HWND w,const win16_control_month_view *v)
{
 MCGRIDINFO grid={.cbSize=sizeof(grid),.dwPart=MCGIP_CALENDARHEADER,.dwFlags=MCGIF_RECT};
 LRESULT count=SendMessageW(w,MCM_GETCALENDARCOUNT,0,0);
 if(count==1 && v->current(v->scope) && !*v->detached && v->identity(w,v->owner) && SendMessageW(w,MCM_GETCALENDARGRIDINFO,0,(LPARAM)&grid) && v->current(v->scope) && !*v->detached && v->identity(w,v->owner) && grid.rc.right>grid.rc.left && grid.rc.bottom>grid.rc.top){v->state->header=grid.rc;v->state->header_valid=TRUE;v->state->header_known=TRUE;
 MCGRIDINFO prev={.cbSize=sizeof(prev),.dwPart=MCGIP_PREV,.dwFlags=MCGIF_RECT},next={.cbSize=sizeof(next),.dwPart=MCGIP_NEXT,.dwFlags=MCGIF_RECT};
 if(SendMessageW(w,MCM_GETCALENDARGRIDINFO,0,(LPARAM)&prev) && v->current(v->scope) && !*v->detached && v->identity(w,v->owner) && SendMessageW(w,MCM_GETCALENDARGRIDINFO,0,(LPARAM)&next) && v->current(v->scope) && !*v->detached && v->identity(w,v->owner) && prev.rc.right>prev.rc.left && next.rc.right>next.rc.left){v->state->prev=prev.rc;v->state->next=next.rc;v->state->nav_valid=TRUE;}}
}
