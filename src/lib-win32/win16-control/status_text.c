#include "status_text.h"
#include <string.h>

BOOL win16_control_status_theme_prepare(ntw16s_status_theme_scope *t,const win16_control_status_text_view *s,HDC dc,int part,int state,LPCWSTR text,int count,DWORD flags,const RECT *rect,const void *address)
{
    HMODULE caller=NULL;POINT origin;HWND from;BOOL pane=FALSE;
    if(!s || !s->owner || !*s->candidate || s->simple || *s->detached || s->epoch!=*s->owner_epoch ||
       !dc || !text || !rect || part || state || (flags&(DT_RTLREADING|DT_MODIFYSTRING|DT_CALCRECT)) ||
       !IsWindowEnabled(s->window) || !s->identity(s->window,s->owner,NULL) || GetMapMode(dc)!=MM_TEXT ||
       (GetLayout(dc)&LAYOUT_RTL) || !GetViewportOrgEx(dc,&origin) || origin.x || origin.y ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) || caller!=s->native)return TRUE;
    from=WindowFromDC(dc);if(s->dc?dc!=s->dc:from!=s->window)return TRUE;
    for(unsigned i=0;i<s->text_count;++i){RECT p=s->text_panes[i];if(rect->left>=p.left && rect->left<p.right && rect->right<=p.right && rect->right>rect->left && rect->top<p.bottom && rect->bottom>p.top){pane=TRUE;break;}}
    if(!pane || count< -1)return TRUE;
    if(count==-1){count=0;while(count<513 && text[count])++count;}
    if(count<=0 || count>512)return TRUE;
    for(int i=0;i<count;++i)if(text[i]>0x7f)return TRUE;
    memcpy(t->text,text,(size_t)count*sizeof(WCHAR));t->length=count;t->dc=dc;t->paint=s->paint_identity;t->valid=TRUE;return TRUE;
}
HFONT win16_control_status_text_font(const win16_control_status_text_view *s,ntw16s_status_theme_scope *theme,HDC dc,LPCWSTR text,int count,UINT flags,const void *address,void **token)
{
    HMODULE caller=NULL;POINT origin;HWND from;
    if(token)*token=NULL;
    if(!token || !s || !s->owner || !*s->candidate || *s->detached || s->epoch!=*s->owner_epoch ||
       !dc || !text || (flags&(DT_RTLREADING|DT_MODIFYSTRING)) || GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL) ||
       !GetViewportOrgEx(dc,&origin) || origin.x || origin.y || !s->identity(s->window,s->owner,NULL) || !IsWindowEnabled(s->window) ||
       !GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,(LPCWSTR)address,&caller) || caller!=GetModuleHandleW(L"uxtheme.dll"))return NULL;
    from=WindowFromDC(dc);if(s->dc?dc!=s->dc:from!=s->window)return NULL;
    if(count< -1)return NULL;
    if(count==-1){count=0;while(count<4096 && text[count])++count;if(count==4096)return NULL;}
    if(count<=0 || count>4096)return NULL;
    for(int i=0;i<count;++i)if(text[i]>0x7f)return NULL;
    if(s->simple){if(s->simple_length!=count || memcmp(s->simple_text,text,(size_t)count*sizeof(WCHAR)))return NULL;}
    else {ntw16s_status_theme_scope *t=theme;if(!t || !t->valid || t->paint!=s->paint_identity || t->dc!=dc || t->length!=count || memcmp(t->text,text,(size_t)count*sizeof(WCHAR)))return NULL;}
    if(!s->identity(s->window,s->owner,NULL) || *s->detached || s->epoch!=*s->owner_epoch)return NULL;
    return win16_control_status_font_borrow(s->font_cache,dc,s->owner,token);
}
