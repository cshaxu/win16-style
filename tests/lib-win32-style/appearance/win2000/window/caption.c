#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif
#include "lib-win32-style/appearance/win2000/window/caption.h"
#include <stdio.h>

static COLORREF button_color(COLORREF pixel)
{
    if(pixel==GetSysColor(COLOR_3DLIGHT))return GetSysColor(COLOR_3DFACE);
    return pixel;
}

static BOOL compare_buttons(HDC dc,HFONT font,UINT dpi,BOOL active)
{
    const BOOL present[]={TRUE,TRUE,FALSE,TRUE};
    const UINT flags[]={0,DFCS_PUSHED,0,0};
    const UINT glyphs[]={DFCS_CAPTIONMIN,DFCS_CAPTIONRESTORE,0,DFCS_CAPTIONCLOSE};
    style_native_caption_layout layout={.band={0,0,320,style_native_caption_height(dpi,FALSE)}};
    if(!win2k_caption_system_buttons(&layout.band,dpi,FALSE,present,layout.buttons))return FALSE;
    if(layout.band.bottom!=GetSystemMetricsForDpi(SM_CYCAPTION,dpi) ||
       layout.buttons[STYLE_NATIVE_CAPTION_CLOSE].right!=layout.band.right ||
       layout.buttons[STYLE_NATIVE_CAPTION_CLOSE].right-layout.buttons[STYLE_NATIVE_CAPTION_CLOSE].left!=GetSystemMetricsForDpi(SM_CXSIZE,dpi))
        return FALSE;
    win2k_caption_render(dc,&layout,L"",NULL,font,dpi,active,TRUE,flags);
    for(unsigned i=0;i<STYLE_NATIVE_CAPTION_BUTTONS;++i){
        if(!present[i])continue;
        RECT bounds=layout.buttons[i];
        const int top=layout.band.bottom+3;
        RECT reference={bounds.left,top,bounds.right,top+bounds.bottom-bounds.top};
        if(!DrawFrameControl(dc,&reference,DFC_CAPTION,glyphs[i]|flags[i]))return FALSE;
        for(int y=0;y<reference.bottom-reference.top;++y)
            for(int x=bounds.left;x<bounds.right;++x){
                COLORREF native=GetPixel(dc,x,reference.top+y);
                if(native==CLR_INVALID || GetPixel(dc,x,bounds.top+y)!=button_color(native))
                    return FALSE;
            }
    }
    COLORREF fill=RGB(13,24,35);
    style_native_caption_render(dc,&layout,L"",NULL,font,dpi,
        (style_native_caption_colors){fill,fill,RGB(255,255,255),FALSE},NULL,NULL,TRUE,flags,NULL);
    return GetPixel(dc,5,layout.band.bottom-1)==fill;
}

static BOOL compare_icon(HDC dc,HFONT font,UINT dpi)
{
    BYTE mask[32]={0},pixels[32];
    FillMemory(pixels,sizeof(pixels),255);
    HICON icon=CreateIcon(GetModuleHandleW(NULL),16,16,1,1,mask,pixels);
    if(!icon)return FALSE;
    const style_native_caption_layout layout={.band={0,0,120,32},.icon={2,4,30,24}};
    const UINT flags[STYLE_NATIVE_CAPTION_BUTTONS]={0};
    const COLORREF fill=RGB(13,24,35);
    const style_native_caption_colors colors={fill,fill,RGB(0,0,0),FALSE};
    style_native_caption_render(dc,&layout,L"",icon,font,dpi,colors,NULL,NULL,FALSE,flags,NULL);
    const int width=min(28,GetSystemMetricsForDpi(SM_CXSMICON,dpi));
    const int height=min(20,GetSystemMetricsForDpi(SM_CYSMICON,dpi));
    const int left=2+(28-width)/2,top=4+(20-height)/2;
    BOOL result=GetPixel(dc,left,top)==RGB(255,255,255) &&
        GetPixel(dc,left+width-1,top+height-1)==RGB(255,255,255) &&
        GetPixel(dc,left-1,top)==fill && GetPixel(dc,left+width,top)==fill &&
        GetPixel(dc,left,top-1)==fill && GetPixel(dc,left,top+height)==fill;
    style_native_caption_render(dc,&layout,L"",NULL,font,dpi,colors,NULL,NULL,FALSE,flags,NULL);
    result=result && GetPixel(dc,left,top)==fill;
    return DestroyIcon(icon) && result;
}

static BOOL native_fonts(UINT dpi)
{
    NONCLIENTMETRICSW metrics={.cbSize=sizeof(metrics)};
    if(!SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS,sizeof(metrics),&metrics,0,dpi))return FALSE;
    HFONT fonts[]={msstyles_font_create(NULL,dpi,MSSTYLES_FONT_CAPTION),
        msstyles_font_create(NULL,dpi,MSSTYLES_FONT_SMALL_CAPTION)};
    const LOGFONTW *expected[]={&metrics.lfCaptionFont,&metrics.lfSmCaptionFont};
    BOOL ok=TRUE;
    for(unsigned i=0;i<ARRAYSIZE(fonts);++i){
        LOGFONTW actual;
        if(!fonts[i] || GetObjectW(fonts[i],sizeof(actual),&actual)!=sizeof(actual) ||
            actual.lfHeight!=expected[i]->lfHeight || actual.lfWeight!=expected[i]->lfWeight ||
            lstrcmpW(actual.lfFaceName,expected[i]->lfFaceName))ok=FALSE;
        if(fonts[i])DeleteObject(fonts[i]);
    }
    return ok;
}

int main(void)
{
    HDC screen=GetDC(NULL),dc=screen?CreateCompatibleDC(screen):NULL;
    HBITMAP bitmap=screen?CreateCompatibleBitmap(screen,320,112):NULL;
    if(!dc || !bitmap)return 1;
    HGDIOBJ previous=SelectObject(dc,bitmap);
    RECT rect={0,0,96,24};
    style_native_caption_colors colors={RGB(0,0,128),RGB(16,132,208),RGB(255,255,255),FALSE};
    HFONT font=(HFONT)GetStockObject(DEFAULT_GUI_FONT);
    BOOL ok=previous && win2k_caption_paint(dc,&rect,L"",font,&colors) &&
        GetPixel(dc,5,5)==colors.left;
    colors.gradient=TRUE;
    ok=ok && win2k_caption_paint(dc,&rect,L"",font,&colors) &&
        GetPixel(dc,5,5)!=GetPixel(dc,90,5) &&
        win2k_caption_paint(dc,&rect,L"Sample",font,&colors) &&
        !win2k_caption_paint(NULL,&rect,L"",font,&colors);
    style_native_caption_colors active=style_native_caption_palette(TRUE),inactive=style_native_caption_palette(FALSE);
    ok=ok && win2k_caption_paint(dc,&rect,L"",font,&active) && GetPixel(dc,0,5)==GetSysColor(COLOR_ACTIVECAPTION) &&
        win2k_caption_paint(dc,&rect,L"",font,&inactive) && GetPixel(dc,0,5)==GetSysColor(COLOR_INACTIVECAPTION) &&
        compare_buttons(dc,font,96,TRUE) && compare_buttons(dc,font,144,TRUE) &&
        compare_buttons(dc,font,144,FALSE) && compare_buttons(dc,font,192,FALSE) &&
        compare_icon(dc,font,96) && compare_icon(dc,font,144) && compare_icon(dc,font,192);
    const classic_button_group no_close={2,0,2,{{CLASSIC_BUTTON_MINIMIZE,CLASSIC_BUTTON_RIGHT,0},
        {CLASSIC_BUTTON_MAXIMIZE,CLASSIC_BUTTON_RIGHT,0}},0};
    const BOOL available[]={TRUE,TRUE,FALSE,TRUE};
    RECT buttons[STYLE_NATIVE_CAPTION_BUTTONS]={0};
    ok=ok && win2k_caption_buttons_with_group(&rect,96,FALSE,available,&no_close,NULL,buttons) &&
        IsRectEmpty(&buttons[STYLE_NATIVE_CAPTION_CLOSE]) && !IsRectEmpty(&buttons[STYLE_NATIVE_CAPTION_MAXIMIZE]);
    POINT max_point={buttons[STYLE_NATIVE_CAPTION_MAXIMIZE].left,buttons[STYLE_NATIVE_CAPTION_MAXIMIZE].top};
    ok=ok && style_caption_hit(buttons,CLASSIC_BUTTON_MINIMIZE,~0u,max_point)==HTMAXBUTTON;
    SelectObject(dc,previous);DeleteObject(bitmap);DeleteDC(dc);ReleaseDC(NULL,screen);
    if(!ok || !native_fonts(96) || !native_fonts(144))return 1;
    puts("WIN2K CAPTION PASS solid gradient text invalid-input native button pixels 96/144/192");return 0;
}
