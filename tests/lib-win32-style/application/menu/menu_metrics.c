#include "lib-win32-style/application/menu/menu_metrics.h"
#include "lib-win32-style/application/menu/row_text.h"
#include <stdio.h>
int main(void)
{
    HDC dc=CreateCompatibleDC(NULL);if(!dc)return 1;
    HGDIOBJ original=GetCurrentObject(dc,OBJ_FONT);
    if(!original || style_menu_native_text_margin(dc,96)<=0 || GetCurrentObject(dc,OBJ_FONT)!=original || !DeleteDC(dc))return 2;
    for(UINT dpi=96;dpi<=192;dpi+=48){
        int mark=style_menu_native_mark_width(dpi);
        style_menu_popup_columns native=style_menu_popup_spacing(dpi,0,0,0,0);
        if(native.check!=mark || native.gap!=0 || native.arrow!=mark || native.plain!=mark)return 3;
    }
    style_menu_popup_columns declared=style_menu_popup_spacing(144,18,8,12,4);
    if(declared.check!=27 || declared.gap!=12 || declared.arrow!=18 || declared.plain!=6)return 4;
    unsigned defaults=STYLE_MENU_TEXT_SHORTCUT_RIGHT|STYLE_MENU_TEXT_BOLD_DEFAULT|STYLE_MENU_COMMAND_GLYPHS;
    if(style_menu_text_options(0)!=defaults ||
        style_menu_text_options(STYLE_MENU_TEXT_DEFINED)!=STYLE_MENU_TEXT_DEFINED)return 5;
    if(style_menu_row_height(96,10,14,1)!=16 ||
        style_menu_row_height(144,18,14,1)!=25 ||
        style_menu_row_height(192,36,14,1)!=40 ||
        style_menu_row_height(96,0,0,0)!=GetSystemMetricsForDpi(SM_CYMENU,96))return 6;
    puts("MENU_METRICS_OK native_menu=1 font_restored=1 popup_defaults=1");return 0;
}
