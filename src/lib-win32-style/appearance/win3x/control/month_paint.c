#include "month_paint.h"
#include "month_text.h"
#include "special_paint.h"
#include "../common/font.h"
#include <vssym32.h>
#include <commctrl.h>

COLORREF win16_control_month_selected_color(const classic_theme *theme,COLORREF fallback)
{
    if(theme->calendar.present&(1u<<CLASSIC_CALENDAR_SELECTED))
        return theme->calendar.values[CLASSIC_CALENDAR_SELECTED];
    return theme->list.present?theme->list.selection:fallback;
}

void win16_control_month_arrow(const win16_control_month_view *v,HTHEME theme,HDC dc,int part,int state,const RECT *r,const RECT *clip,const void *address)
{
    if(!v || !r || (part!=MC_NAVNEXT && part!=MC_NAVPREV) || state<MCNN_NORMAL || state>MCNN_DISABLED || !style_control_month_draw_identity(&v->native,theme,dc,address))return;
    win16_style_month_arrow(dc,r,clip,part,state,v->palette,v->arrows);
}
static BOOL month_text_allowed(const win16_control_month_view *s,HTHEME theme,HDC dc,int part,int state,DWORD flags,DWORD flags2,const RECT *r,const void *address)
{return s && r && (part==MC_GRIDCELL || part==MC_GRIDCELLUPPER) && state>=0 && state<=7 && flags2==0 && (flags==0x925 || flags==0x926 || flags==0x924) && IsWindowEnabled(s->native.window) && style_control_month_draw_identity(&s->native,theme,dc,address);}
HFONT win16_control_month_text_borrow(const win16_control_month_view *v,HTHEME theme,HDC dc,int part,int state,DWORD flags,DWORD flags2,const RECT *r,const void *address)
{
    if(!v || !v->font_dpi || !month_text_allowed(v,theme,dc,part,state,flags,flags2,r,address) ||
       GetDpiForWindow(v->native.window)!=96)return NULL;
    if(!*v->cached_font)*v->cached_font=win16_style_calendar_font_acquire(v->font_dpi,v->font_points);
    if(!v->native.current(v->native.context) || !month_text_allowed(v,theme,dc,part,state,flags,flags2,r,address) ||
       !*v->cached_font || !win16_style_system_try_retain(*v->cached_font))return NULL;
    return *v->cached_font;
}
BOOL win16_control_month_text_room(const win16_control_month_view *v,HTHEME theme,HDC dc,int part,int state,DWORD flags,DWORD flags2,const RECT *r,int width,const void *address)
{const win16_control_month_view *s=v;if(!month_text_allowed(s,theme,dc,part,state,flags,flags2,r,address) || !s->native.state->header_known)return FALSE;
 if(EqualRect(r,&s->native.state->header))return s->native.state->nav_valid && style_control_month_title_fits(r,&s->native.state->prev,&s->native.state->next,width);
 return TRUE;}
void win16_control_month_header_text(const win16_control_month_view *v,HTHEME theme,HDC dc,int part,int state,const RECT *r,DWORD flags,const void *address)
{const win16_control_month_view *s=v;COLORREF text;if(!s || !s->native.state->header_valid || !IsWindowEnabled(s->native.window) || part!=MC_GRIDCELL || state!=0 || !r || (flags&DT_CALCRECT) || !EqualRect(r,&s->native.state->header) ||
 !style_control_month_draw_identity(&s->native,theme,dc,address) || FAILED(GetThemeColor(theme,part,state,TMT_TEXTCOLOR,&text)) || text!=RGB(0,0,0))return;
 s->native.state->header_valid=FALSE;win16_control_month_fill(dc,r,NULL,s->palette->active);}
void win16_control_month_selection_text(const win16_control_month_view *v,HTHEME theme,HDC dc,int part,int state,const RECT *r,DWORD flags,const void *address)
{
    style_control_month_selection selected;COLORREF foreground;
    if(!v || !style_control_month_selection_take(&v->native,theme,dc,part,state,r,flags,address,&selected) ||
        FAILED(GetThemeColor(theme,part,state,TMT_TEXTCOLOR,&foreground)) || foreground!=RGB(0,0,0))return;
    win16_control_month_fill(dc,&selected.rect,selected.has_clip?&selected.clip:NULL,v->selection);
}
