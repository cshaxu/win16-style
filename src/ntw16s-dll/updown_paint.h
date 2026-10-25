#ifndef NTW16S_UPDOWN_PAINT_H
#define NTW16S_UPDOWN_PAINT_H
#include "win16_style.h"
#include <uxtheme.h>
typedef const char ntw16s_updown_glyph[17][18];
typedef struct ntw16s_updown_paint {
    struct ntw16s_updown_paint *next;
    HWND window;
    HMODULE native;
    const win16_style_palette *palette;
    ntw16s_updown_glyph *const *glyphs;
    void *owner;
    BOOL (*acquire)(void *);
    void (*release)(void *);
} ntw16s_updown_paint;
void ntw16s_updown_api_enable(void);
BOOL ntw16s_updown_api_enabled(void);
void ntw16s_updown_register(ntw16s_updown_paint *,HWND,HMODULE,const win16_style_palette *,
    ntw16s_updown_glyph *const *,void *,BOOL (*)(void *),void (*)(void *));
void ntw16s_updown_retire(ntw16s_updown_paint *);
void ntw16s_updown_dc_created(HDC,HDC,void *);
void ntw16s_updown_dc_deleted(HDC);
BOOL ntw16s_updown_draw_frame(HDC,RECT *,UINT,UINT,void *);
BOOL ntw16s_updown_draw_theme(HTHEME,HDC,int,int,const RECT *,const RECT *,void *);
#endif
