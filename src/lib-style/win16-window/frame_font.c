#include "../win16-common/font.h"
BOOL win16_control_refresh_frame_fonts(HWND w,UINT *current_dpi,HFONT *current_title,HFONT *current_menu)
{
    HFONT title,menu;
    /* USER virtualizes legacy windows separately; convert points once. */
    UINT dpi=GetDpiForWindow(w);if(!dpi)dpi=96;
    title=CreateFontW(-MulDiv(10,(int)dpi,72),0,0,0,FW_BOLD,FALSE,FALSE,FALSE,
        DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,NONANTIALIASED_QUALITY,
        DEFAULT_PITCH,L"MS Sans Serif");
    menu=win16_style_menu_font_acquire(dpi);
    /* Sandboxed helper processes may be unable to register a memory font.
     * A stock font keeps window creation working in that environment. */
    if(!menu)menu=(HFONT)GetStockObject(SYSTEM_FONT);
    if(!title || !menu){if(title)DeleteObject(title);win16_style_system_release(menu);return FALSE;}
    if(*current_title)DeleteObject(*current_title);
    win16_style_system_release(*current_menu);
    *current_dpi=dpi;*current_title=title;*current_menu=menu;return TRUE;
}
