#ifndef WIN16_CONTROL_STATUS_TEXT_H
#define WIN16_CONTROL_STATUS_TEXT_H
#include "status_font.h"
typedef struct ntw16s_status_theme_scope {struct ntw16s_status_theme_scope *previous;void *paint;HDC dc;int length;BOOL valid;WCHAR text[513];} ntw16s_status_theme_scope;
typedef struct win16_control_status_text_view {
    void *owner,*paint_identity; HWND window; HDC dc; HMODULE native;
    unsigned epoch,text_count; BOOL simple; int simple_length; const WCHAR *simple_text; const RECT *text_panes;
    const BOOL *candidate,*detached; const unsigned *owner_epoch; win16_control_status_font_cache *font_cache;
    BOOL (*identity)(HWND,void *,HMODULE *);
} win16_control_status_text_view;
BOOL win16_control_status_theme_prepare(ntw16s_status_theme_scope *,const win16_control_status_text_view *,HDC,int,int,LPCWSTR,int,DWORD,const RECT *,const void *);
HFONT win16_control_status_text_font(const win16_control_status_text_view *,ntw16s_status_theme_scope *,HDC,LPCWSTR,int,UINT,const void *,void **);
#endif
