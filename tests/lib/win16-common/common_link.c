#include <stdio.h>
#include "lib-win32/win16-common/font.h"
#include "lib-win32/win16-common/layout.h"
#include "lib-win32/win16-common/menu_layout.h"
#include "lib-win32/win16-common/palette.h"

int main(void)
{
    win16_style_palette palette;
    HFONT font;
    SIZE size={30,16};UINT flags=0;RECT item={0};int height=0;
    win16_style_default_palette(&palette);
    if(palette.face!=RGB(192,192,192) || palette.edge!=RGB(0,0,0))return 10;
    font=win16_style_menu_font_acquire(96);
    if(!font)return 11;
    if(!win16_style_menu_arrange((POINT){0,0},100,&size,&flags,1,&item,&height) ||
       item.right<=item.left || height<=0){
        DeleteObject(font);return 12;
    }
    DeleteObject(font);
    puts("WIN16_COMMON_LINK_OK palette=1 font=1 menu_layout=1");
    return 0;
}
