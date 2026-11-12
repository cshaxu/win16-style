#ifndef WIN16_CONTROL_MONTH_PAINT_H
#define WIN16_CONTROL_MONTH_PAINT_H
#include <windows.h>
#include <uxtheme.h>
typedef struct win16_control_month_state {
    BOOL header_valid,header_known,nav_valid;RECT header,prev,next;BOOL selected;HTHEME theme;HDC dc;RECT rect,clip;BOOL has_clip;
} win16_control_month_state;
typedef struct win16_control_month_view {
    void *scope,*owner;HWND window;win16_control_month_state *state;const BOOL *rich_month,*rich_witness,*detached;HFONT *cached_font;
    BOOL (*identity)(HWND,void *);BOOL (*current)(void *);
} win16_control_month_view;
void win16_control_month_query(HWND,const win16_control_month_view *);
void win16_control_month_arrow(const win16_control_month_view *,HTHEME,HDC,int,int,const RECT *,const RECT *,const void *);
HFONT win16_control_month_text_borrow(const win16_control_month_view *,HTHEME,HDC,int,int,DWORD,DWORD,const RECT *,const void *);
BOOL win16_control_month_text_room(const win16_control_month_view *,HTHEME,HDC,int,int,DWORD,DWORD,const RECT *,int,const void *);
void win16_control_month_header_text(const win16_control_month_view *,HTHEME,HDC,int,int,const RECT *,DWORD,const void *);
void win16_control_month_selection_background(const win16_control_month_view *,HTHEME,HDC,int,int,const RECT *,const RECT *,const void *);
void win16_control_month_selection_text(const win16_control_month_view *,HTHEME,HDC,int,int,const RECT *,DWORD,const void *);
#endif
