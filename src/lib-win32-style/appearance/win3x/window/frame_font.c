#include "../common/font.h"
#include "frame_font.h"
void win16_control_release_frame_fonts(const style_font_pair *fonts)
{
    if(fonts->title)DeleteObject(fonts->title);
    win16_style_system_release(fonts->menu);
}
BOOL win16_control_refresh_frame_fonts(HWND w,style_font_pair *current,const msstyles_font *defined,
    const msstyles_font *menu_font)
{
    HFONT title,menu;
    /* USER virtualizes legacy windows separately; convert points once. */
    UINT dpi=GetDpiForWindow(w);if(!dpi)dpi=96;
    if(defined && defined->points){
        LOGFONTW logical=msstyles_font_at_dpi(defined,dpi);
        title=CreateFontIndirectW(&logical);
    }else title=CreateFontW(-MulDiv(10,(int)dpi,72),0,0,0,FW_BOLD,FALSE,FALSE,FALSE,
        DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,
        DEFAULT_PITCH,L"MS Sans Serif");
    menu=win16_style_menu_font_acquire(dpi,menu_font);
    /* Sandboxed helper processes may be unable to register a memory font.
     * A stock font keeps window creation working in that environment. */
    if(!menu)menu=(HFONT)GetStockObject(SYSTEM_FONT);
    return style_font_pair_commit(current,(style_font_pair){title,menu,dpi},win16_control_release_frame_fonts);
}
