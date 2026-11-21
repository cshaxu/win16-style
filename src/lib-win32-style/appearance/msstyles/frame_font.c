#include "lib-win32-style/appearance/msstyles/raster_font.h"
#include "frame_font.h"
HFONT style_frame_menu_font_acquire(UINT dpi,const msstyles_font *defined)
{
#ifdef STYLE_THEME_EXTERNAL_ONLY
    return style_menu_font_acquire(dpi,defined,false);
#else
    return style_menu_font_acquire_with_bitmap_default(dpi,defined);
#endif
}

void style_frame_fonts_release(const style_font_pair *fonts)
{
    if(fonts->title)DeleteObject(fonts->title);
    style_menu_font_release(fonts->menu);
}
BOOL style_frame_fonts_refresh(HWND w,style_font_pair *current,const msstyles_font *defined,
    const msstyles_font *menu_font)
{
    HFONT title,menu;
    /* USER virtualizes legacy windows separately; convert points once. */
    UINT dpi=GetDpiForWindow(w);if(!dpi)dpi=96;
    title=msstyles_font_create(defined,dpi,MSSTYLES_FONT_CAPTION);
    menu=style_frame_menu_font_acquire(dpi,menu_font);
    /* Sandboxed helper processes may be unable to register a memory font.
     * A stock font keeps window creation working in that environment. */
    if(!menu)menu=(HFONT)GetStockObject(SYSTEM_FONT);
    return style_font_pair_commit(current,(style_font_pair){title,menu,dpi},style_frame_fonts_release);
}
