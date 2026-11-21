#include <stdio.h>
#include "lib-win32-style/appearance/msstyles/raster_font.h"
#include "lib-win32-style/application/window/full_client_layout.h"
#include "lib-win32-style/application/menu/classic_menu_layout.h"
#include "lib-win32-style/appearance/win3x/common/palette.h"

int main(void)
{
    win16_style_palette palette;
    HFONT font;
    SIZE size={30,16};UINT flags=0;RECT item={0};int height=0;
    style_window_palette_default(&palette);
    if(palette.face!=GetSysColor(COLOR_BTNFACE) || palette.edge!=GetSysColor(COLOR_WINDOWTEXT))return 10;
    font=style_menu_font_acquire_with_bitmap_default(96,NULL);
    if(!font)return 11;
    if(!style_classic_menu_arrange((POINT){0,0},100,&size,&flags,1,&item,&height) ||
       item.right<=item.left || height<=0){
        style_raster_font_release(font);return 12;
    }
    style_raster_font_release(font);
    puts("WIN16_COMMON_LINK_OK palette=1 font=1 menu_layout=1");
    return 0;
}
