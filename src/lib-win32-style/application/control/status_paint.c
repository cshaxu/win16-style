#include "lib-win32-style/application/graphics/dc_geometry.h"
#include "status_paint.h"
#include <commctrl.h>
#include <string.h>

static BOOL final_pane_valid(const style_control_status_scope *s,HICON expected)
{
    LRESULT parts=SendMessageW(s->window,SB_GETPARTS,0,0);
    if(s->instance->detached || s->snapshot.epoch!=s->state->epoch || parts!=(LRESULT)s->snapshot.count)return FALSE;
    LRESULT type=SendMessageW(s->window,SB_GETTEXTLENGTHW,s->snapshot.count-1,0);
    if(s->instance->detached || s->snapshot.epoch!=s->state->epoch || HIWORD(type)!=0)return FALSE;
    LRESULT icon=SendMessageW(s->window,SB_GETICON,s->snapshot.count-1,0);
    return !s->instance->detached && s->snapshot.epoch==s->state->epoch && (HICON)icon==expected;
}

/* The themed native status control omits the final pane background when it
 * owns a sizing grip. Only a native text call for an icon-free,
 * ordinary final pane supplies a verified pre-content drawing boundary. */
BOOL style_control_status_text_frame(const style_control_status_scope *s,HDC dc,const RECT *text,UINT flags,const void *address,RECT *result)
{
    HMODULE module=NULL;RECT pane;HWND window;
    if(!s || !s->context || s->instance->detached || s->snapshot.epoch!=s->state->epoch ||
        s->snapshot.count<2 || !text || (flags&(DT_CALCRECT|DT_RTLREADING)) ||
        !(GetWindowLongPtrW(s->window,GWL_STYLE)&SBARS_SIZEGRIP) ||
        !style_control_status_identity(s->window,s->instance,s->state,s->class_name,NULL) ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)address,&module) || module!=s->snapshot.native || !dc ||
        !style_dc_pixel_viewport(dc))return FALSE;
    window=WindowFromDC(dc);if(s->dc?dc!=s->dc:window!=s->window)return FALSE;
    pane=s->snapshot.panes[s->snapshot.count-1];
    if(text->left<=pane.left || text->left>=pane.right || text->right>pane.right ||
        text->right<=text->left || text->top>=pane.bottom || text->bottom<=pane.top)return FALSE;
    if(!final_pane_valid(s,NULL))return FALSE;
    *result=pane;return TRUE;
}

/* For a size-grip pane, native comctl draws its icon before its text.
 * Frame only the verified native pane before the same icon is painted. */
BOOL style_control_status_icon_frame(const style_control_status_scope *s,HDC dc,int x,int y,int cx,int cy,HICON icon,const void *address,RECT *result)
{
    HMODULE module=NULL;RECT pane;HWND window;
    if(!s || !s->context || s->instance->detached || s->snapshot.epoch!=s->state->epoch || s->snapshot.count<2 ||
        !(GetWindowLongPtrW(s->window,GWL_STYLE)&SBARS_SIZEGRIP) || !style_control_status_identity(s->window,s->instance,s->state,s->class_name,NULL) ||
        !dc || !icon || cx<=0 || cy<=0 ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)address,&module) || module!=s->snapshot.native || !style_dc_pixel_viewport(dc))return FALSE;
    window=WindowFromDC(dc);if(s->dc?dc!=s->dc:window!=s->window)return FALSE;
    pane=s->snapshot.panes[s->snapshot.count-1];
    if(x<pane.left || x>=pane.right || (LONGLONG)y+cy<=pane.top || y>=pane.bottom)return FALSE;
    if(!final_pane_valid(s,icon))return FALSE;
    *result=pane;return TRUE;
}

/* Called only after the original native drawing call has completed. */
BOOL style_control_status_frame(const style_control_status_scope *s,HDC dc,const RECT *rect,const void *caller,BOOL observed,HWND scoped)
{
    HMODULE module=NULL;HWND w;BOOL matches=FALSE;
    if(!s || !s->context || s->instance->detached || s->snapshot.epoch!=s->state->epoch || !dc || !rect ||
        !style_control_status_identity(s->window,s->instance,s->state,s->class_name,NULL) || !style_dc_pixel_viewport(dc) ||
        !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
            (LPCWSTR)caller,&module) || module!=s->snapshot.native)return FALSE;
    if(observed && scoped!=s->window)return FALSE;
    w=WindowFromDC(dc);if(s->dc?dc!=s->dc:w!=s->window)return FALSE;
    for(unsigned i=0;i<s->snapshot.count;++i)if(EqualRect(rect,&s->snapshot.panes[i])){matches=TRUE;break;}
    if(!matches || s->instance->detached || s->snapshot.epoch!=s->state->epoch)return FALSE;
    return TRUE;
}

static void theme_prepare(style_control_status_theme *t,const style_control_status_scope *s,HDC dc,int part,int state,LPCWSTR text,int count,DWORD flags,const RECT *rect,const void *address,style_control_status_text_policy accept)
{
    HMODULE caller=NULL;HWND from;BOOL pane=FALSE;
    if(!s || !s->context || !s->state->candidate || s->snapshot.simple || s->instance->detached || s->snapshot.epoch!=s->state->epoch ||
       !dc || !text || !rect || part || state || (flags&(DT_RTLREADING|DT_MODIFYSTRING|DT_CALCRECT)) ||
       !IsWindowEnabled(s->window) || !style_control_status_identity(s->window,s->instance,s->state,s->class_name,NULL) || !style_dc_pixel_viewport(dc) ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) || caller!=s->snapshot.native)return;
    from=WindowFromDC(dc);if(s->dc?dc!=s->dc:from!=s->window)return;
    for(unsigned i=0;i<s->snapshot.text_count;++i){RECT p=s->snapshot.text_panes[i];if(rect->left>=p.left && rect->left<p.right && rect->right<=p.right && rect->right>rect->left && rect->top<p.bottom && rect->bottom>p.top){pane=TRUE;break;}}
    if(!pane || count< -1)return;
    if(count==-1){count=0;while(count<513 && text[count])++count;}
    if(count<=0 || count>512)return;
    if(!accept(text,count))return;
    memcpy(t->text,text,(size_t)count*sizeof(WCHAR));t->length=count;t->dc=dc;t->paint=s;t->valid=TRUE;return;
}
BOOL style_control_status_text_matches(const style_control_status_scope *s,const style_control_status_theme *theme,HDC dc,LPCWSTR text,int count,UINT flags,const void *address,style_control_status_text_policy accept)
{
    HMODULE caller=NULL;HWND from;
    if(!s || !s->context || !s->state->candidate || s->instance->detached || s->snapshot.epoch!=s->state->epoch ||
       !dc || !text || (flags&(DT_RTLREADING|DT_MODIFYSTRING)) || !style_dc_pixel_viewport(dc) || !style_control_status_identity(s->window,s->instance,s->state,s->class_name,NULL) || !IsWindowEnabled(s->window) ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) || caller!=GetModuleHandleW(L"uxtheme.dll"))return FALSE;
    from=WindowFromDC(dc);if(s->dc?dc!=s->dc:from!=s->window)return FALSE;
    if(count< -1)return FALSE;
    if(count==-1){count=0;while(count<4096 && text[count])++count;if(count==4096)return FALSE;}
    if(count<=0 || count>4096)return FALSE;
    if(!accept(text,count))return FALSE;
    if(s->snapshot.simple){if(s->snapshot.simple_length!=count || memcmp(s->snapshot.simple_text,text,(size_t)count*sizeof(WCHAR)))return FALSE;}
    else {const style_control_status_theme *t=theme;if(!t || !t->valid || t->paint!=s || t->dc!=dc || t->length!=count || memcmp(t->text,text,(size_t)count*sizeof(WCHAR)))return FALSE;}
    if(!style_control_status_identity(s->window,s->instance,s->state,s->class_name,NULL) || s->instance->detached || s->snapshot.epoch!=s->state->epoch)return FALSE;
    return TRUE;
}

BOOL style_control_status_theme_begin(style_control_status_themes *themes,style_control_status_theme *t,const style_control_status_scope *s,HDC dc,int part,int state,LPCWSTR text,int count,DWORD flags,const RECT *rect,const void *address,style_control_status_text_policy accept)
{
    if(!t || (!s && !themes->current))return FALSE;
    memset(t,0,sizeof(*t));t->previous=themes->current;themes->current=t;
    if(s)theme_prepare(t,s,dc,part,state,text,count,flags,rect,address,accept);
    return TRUE;
}
void style_control_status_theme_end(style_control_status_themes *themes,style_control_status_theme *t)
{if(themes->current==t)themes->current=t->previous;}
