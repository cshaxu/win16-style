#include "lib-win32-style/appearance/win2000/window/caption.h"
#include <stdio.h>

static COLORREF classic_color(COLORREF pixel)
{
    if(pixel==GetSysColor(COLOR_3DFACE))return RGB(212,208,200);
    if(pixel==GetSysColor(COLOR_3DSHADOW))return RGB(128,128,128);
    if(pixel==GetSysColor(COLOR_3DDKSHADOW))return RGB(0,0,0);
    if(pixel==GetSysColor(COLOR_3DLIGHT))return RGB(212,208,200);
    return pixel;
}

static BOOL compare_buttons(HDC dc,HFONT font,UINT dpi,BOOL active)
{
    const BOOL present[]={TRUE,TRUE,FALSE,TRUE};
    const UINT flags[]={0,DFCS_PUSHED,0,0};
    const UINT glyphs[]={DFCS_CAPTIONMIN,DFCS_CAPTIONRESTORE,0,DFCS_CAPTIONCLOSE};
    win2k_caption_layout layout={.band={0,0,192,win2k_caption_height(dpi,FALSE)}};
    if(!win2k_caption_buttons(&layout.band,dpi,FALSE,present,layout.buttons))return FALSE;
    win2k_caption_render(dc,&layout,L"",NULL,font,dpi,active,TRUE,flags);
    for(unsigned i=0;i<WIN2K_CAPTION_PARTS;++i){
        if(!present[i])continue;
        RECT bounds=layout.buttons[i];
        RECT reference={bounds.left,40,bounds.right,40+bounds.bottom-bounds.top};
        if(!DrawFrameControl(dc,&reference,DFC_CAPTION,glyphs[i]|flags[i]))return FALSE;
        for(int y=0;y<reference.bottom-reference.top;++y)
            for(int x=bounds.left;x<bounds.right;++x){
                COLORREF native=GetPixel(dc,x,reference.top+y);
                if(native==CLR_INVALID || GetPixel(dc,x,bounds.top+y)!=classic_color(native))
                    return FALSE;
            }
    }
    return GetPixel(dc,5,layout.band.bottom-1)==RGB(212,208,200);
}

int main(void)
{
    HDC screen=GetDC(NULL),dc=screen?CreateCompatibleDC(screen):NULL;
    HBITMAP bitmap=screen?CreateCompatibleBitmap(screen,192,72):NULL;
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
    win2k_caption_colors active=win2k_caption_palette(TRUE),inactive=win2k_caption_palette(FALSE);
    ok=ok && win2k_caption_paint(dc,&rect,L"",font,&active) && GetPixel(dc,0,5)==RGB(0,0,128) &&
        win2k_caption_paint(dc,&rect,L"",font,&inactive) && GetPixel(dc,0,5)==RGB(128,128,128) &&
        compare_buttons(dc,font,96,TRUE) && compare_buttons(dc,font,144,TRUE) &&
        compare_buttons(dc,font,144,FALSE) && compare_buttons(dc,font,192,FALSE);
    SelectObject(dc,previous);DeleteObject(bitmap);DeleteDC(dc);ReleaseDC(NULL,screen);
    if(!ok)return 1;
    puts("WIN2K CAPTION PASS solid gradient text invalid-input native button pixels 96/144/192");return 0;
}
