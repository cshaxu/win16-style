#include "lib-style/common/menu_metrics.h"
#include <stdio.h>
int main(void)
{
    HDC dc=CreateCompatibleDC(NULL);if(!dc)return 1;
    HGDIOBJ original=GetCurrentObject(dc,OBJ_FONT);
    if(!original || style_menu_native_text_margin(dc,96)<=0 || GetCurrentObject(dc,OBJ_FONT)!=original || !DeleteDC(dc))return 2;
    puts("MENU_METRICS_OK native_menu=1 font_restored=1");return 0;
}
