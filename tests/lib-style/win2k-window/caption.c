#include "lib-style/win2k-window/caption.h"
#include <stdio.h>

int main(void)
{
    HDC screen=GetDC(NULL),dc=screen?CreateCompatibleDC(screen):NULL;
    HBITMAP bitmap=screen?CreateCompatibleBitmap(screen,96,24):NULL;
    if(!dc || !bitmap)return 1;
    HGDIOBJ previous=SelectObject(dc,bitmap);
    RECT rect={0,0,96,24};
    win2k_caption_colors colors={RGB(0,0,128),RGB(16,132,208),RGB(255,255,255),FALSE};
    HFONT font=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
    BOOL ok=previous && win2k_caption_paint(dc,&rect,L"",font,&colors) &&
        GetPixel(dc,5,5)==colors.left;
    colors.gradient=TRUE;
    ok=ok && win2k_caption_paint(dc,&rect,L"",font,&colors) &&
        GetPixel(dc,5,5)!=GetPixel(dc,90,5) &&
        win2k_caption_paint(dc,&rect,L"Sample",font,&colors) &&
        !win2k_caption_paint(NULL,&rect,L"",font,&colors);
    SelectObject(dc,previous);DeleteObject(bitmap);DeleteDC(dc);ReleaseDC(NULL,screen);
    if(!ok)return 1;
    puts("WIN2K CAPTION PASS solid gradient text invalid-input");return 0;
}
