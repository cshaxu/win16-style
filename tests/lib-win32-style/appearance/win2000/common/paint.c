/* Original-image/source contracts, not a second implementation of DrawEdge. */
#include "lib-win32-style/appearance/win2000/common/palette.h"
#include <stdio.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"WIN2K_PAINT_FAIL line=%d error=%lu: %s\n",__LINE__,GetLastError(),#x);return 1;}}while(0)
static unsigned long long glyph_hash(HDC dc)
{
    unsigned long long hash=14695981039346656037ull;
    for(int y=0;y<13;++y)for(int x=0;x<13;++x){
        COLORREF color=GetPixel(dc,x,y);
        const BYTE rgb[3]={GetRValue(color),GetGValue(color),GetBValue(color)};
        for(unsigned channel=0;channel<3;++channel)hash=(hash^rgb[channel])*1099511628211ull;
    }
    return hash;
}
static const win2k_palette reference={
    RGB(212,208,200),RGB(212,208,200),RGB(255,255,255),RGB(128,128,128),RGB(64,64,64),
    RGB(0,0,0),RGB(128,128,128),RGB(255,255,255),RGB(10,36,106),RGB(255,255,255)};
int main(void)
{
    HDC dc=CreateCompatibleDC(NULL);void *pixels;
    BITMAPINFO info={.bmiHeader={sizeof(BITMAPINFOHEADER),40,-30,1,32,BI_RGB}};
    HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&pixels,NULL,0);
    CHECK(dc && bitmap);HGDIOBJ previous=SelectObject(dc,bitmap);CHECK(previous && previous!=HGDI_ERROR);
    const RECT client={5,2,105,42};
    RECT glyph,label;
    CHECK(style_choice_layout(dc,client,144,0,(style_choice_metrics){20,16,6,STYLE_CHOICE_DPI},&glyph,&label));
    CHECK(EqualRect(&glyph,&(RECT){5,10,35,34}) && label.left==44);
    CHECK(style_choice_layout(dc,client,144,BS_LEFTTEXT,(style_choice_metrics){20,16,6,0},&glyph,&label));
    CHECK(EqualRect(&glyph,&(RECT){85,14,105,30}) && label.right==79);
    SIZE zero;
    CHECK(GetTextExtentPoint32W(dc,L"0",1,&zero) && style_choice_layout(dc,client,96,BS_TOP,
        (style_choice_metrics){13,13,0,STYLE_CHOICE_FONT_GAP|STYLE_CHOICE_ALIGN},&glyph,&label));
    CHECK(glyph.top==client.top+(zero.cy+zero.cy/4-13)/2 && label.left==glyph.right+zero.cx/2);
    const win2k_palette *p=&reference;RECT r={0,0,40,30};
    SetDCBrushColor(dc,RGB(1,2,3));SetTextColor(dc,RGB(4,5,6));SetBkColor(dc,RGB(7,8,9));
    CHECK(style_paint_push_frame(dc,&r,DFCS_ADJUSTRECT,TRUE,1,STYLE_PUSH_RELIEF,p));
    CHECK(r.left==3 && r.top==3 && r.right==37 && r.bottom==27);
    /* Accessibility Options original: default OK at121,410, frame pixel
     * roles are black outer / white soft highlight / standard face. */
    CHECK(GetPixel(dc,0,10)==RGB(0,0,0) && GetPixel(dc,1,10)==RGB(255,255,255));
    CHECK(GetPixel(dc,2,10)==RGB(212,208,200) && GetPixel(dc,3,10)==RGB(212,208,200));
    CHECK(GetPixel(dc,39,10)==RGB(0,0,0) && GetPixel(dc,38,10)==RGB(64,64,64));
    CHECK(GetPixel(dc,37,10)==RGB(128,128,128));
    CHECK(GetDCBrushColor(dc)==RGB(1,2,3) && GetTextColor(dc)==RGB(4,5,6) && GetBkColor(dc)==RGB(7,8,9));
    r=(RECT){0,0,40,30};CHECK(style_paint_push_frame(dc,&r,DFCS_ADJUSTRECT,FALSE,1,STYLE_PUSH_RELIEF,p));
    CHECK(r.left==2 && GetPixel(dc,0,0)==RGB(255,255,255) && GetPixel(dc,39,0)==RGB(64,64,64));
    CHECK(GetPixel(dc,1,8)==RGB(212,208,200) && GetPixel(dc,38,8)==RGB(128,128,128));
    r=(RECT){0,0,40,30};CHECK(style_paint_push_frame(dc,&r,DFCS_PUSHED,FALSE,1,STYLE_PUSH_RELIEF,p));
    CHECK(GetPixel(dc,0,8)==RGB(64,64,64) && GetPixel(dc,1,8)==RGB(128,128,128));
    CHECK(GetPixel(dc,39,8)==RGB(255,255,255));
    r=(RECT){0,0,40,30};CHECK(style_paint_push_frame(dc,&r,DFCS_PUSHED,TRUE,1,STYLE_PUSH_RELIEF,p));
    CHECK(GetPixel(dc,0,8)==RGB(0,0,0) && GetPixel(dc,1,8)==RGB(128,128,128) && GetPixel(dc,2,8)==RGB(212,208,200));
    win2k_palette different=*p;different.window=RGB(20,40,60);different.highlight=RGB(70,80,90);
    r=(RECT){0,0,40,30};CHECK(style_paint_relief(dc,&r,EDGE_RAISED,BF_RECT|BF_MONO|BF_MIDDLE,&different));
    CHECK(GetPixel(dc,1,8)==different.window && GetPixel(dc,5,8)==different.window);
    CHECK(style_paint_relief(dc,&r,EDGE_RAISED,BF_TOP|BF_RIGHT|BF_ADJUST,p));
    CHECK(r.left==0 && r.top==2 && r.right==38 && r.bottom==30);
    CHECK(GetPixel(dc,39,0)==p->dark && GetPixel(dc,10,0)==p->light && GetPixel(dc,10,1)==p->highlight);
    CHECK(GetPixel(dc,10,2)==different.window && GetPixel(dc,0,8)==p->text);
    CHECK(!style_paint_relief(dc,&r,EDGE_RAISED,BF_DIAGONAL_ENDBOTTOMRIGHT,p));
    /* Genuine Effects/Mouse screenshot crops, not renderer-generated goldens.
     * URLs, source image SHA256 and crop coordinates: t27-s3-choice-controls.json. */
    const UINT glyph_states[]={DFCS_BUTTONCHECK,DFCS_BUTTONCHECK|DFCS_CHECKED,DFCS_BUTTONRADIO,DFCS_BUTTONRADIO|DFCS_CHECKED};
    const unsigned long long golden[]={0xc68eca65c0304287ull,0x2f492bf0b4662f9aull,0x1bfea9cb15233217ull,0xf36f079a80b8802bull};
    RECT cell={0,0,13,13},canvas={0,0,40,30};HGDIOBJ borrowed_font=GetCurrentObject(dc,OBJ_FONT);
    for(unsigned sample=0;sample<4;++sample){
        CHECK(style_paint_color(dc,&canvas,p->face)&&style_paint_choice_symbols(dc,&cell,glyph_states[sample],p,NULL));
        CHECK(glyph_hash(dc)==golden[sample] && GetCurrentObject(dc,OBJ_FONT)==borrowed_font);
        win2k_palette old_dark=*p;old_dark.dark=RGB(0,0,0);
        CHECK(style_paint_color(dc,&canvas,p->face)&&style_paint_choice_symbols(dc,&cell,glyph_states[sample],&old_dark,NULL));
        CHECK(glyph_hash(dc)!=golden[sample]);
    }
    CHECK(style_paint_choice_symbols(dc,&cell,DFCS_BUTTONCHECK|DFCS_CHECKED|DFCS_INACTIVE,p,NULL));
    CHECK(GetPixel(dc,6,6)==RGB(128,128,128) && GetPixel(dc,3,3)==RGB(212,208,200));
    CHECK(style_paint_choice_symbols(dc,&cell,DFCS_BUTTON3STATE|DFCS_CHECKED,p,NULL));
    COLORREF a=GetPixel(dc,2,2),b=GetPixel(dc,3,2);
    CHECK(a!=b && (a==p->face || a==p->highlight) && (b==p->face || b==p->highlight));
    CHECK(GetPixel(dc,6,6)==p->shadow);
    CHECK(style_paint_choice_symbols(dc,&cell,DFCS_BUTTONCHECK|DFCS_FLAT,p,NULL)&&GetPixel(dc,1,1)==p->text);
    CHECK(!style_paint_choice_symbols(dc,&cell,DFCS_BUTTONRADIO|DFCS_HOT,p,NULL));
    DWORD gdi=GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS);CHECK(gdi);
    for(unsigned repeat=0;repeat<64;++repeat)CHECK(style_paint_choice_symbols(dc,&cell,DFCS_BUTTONRADIO|DFCS_CHECKED,p,NULL));
    CHECK(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)==gdi);
    /* Original Display Properties normal down-arrow: seven/five/three/one
     * black pixels, recorded independently in t27-s6-list-combo.json. */
    RECT arrow={0,0,17,17};unsigned rows[17]={0},total=0,first=17;
    CHECK(style_paint_scroll_system_arrow(dc,&arrow,DFCS_SCROLLCOMBOBOX,p));
    for(unsigned y=0;y<17;++y)for(unsigned x=0;x<17;++x)if(GetPixel(dc,(int)x,(int)y)==p->text){++rows[y];++total;if(first==17)first=y;}
    CHECK(total==16 && first<14 && rows[first]==7 && rows[first+1]==5 && rows[first+2]==3 && rows[first+3]==1);
    CHECK(GetPixel(dc,0,8)==p->light && GetPixel(dc,1,8)==p->highlight && GetPixel(dc,16,8)==p->dark);
    CHECK(GetCurrentObject(dc,OBJ_FONT)==borrowed_font);
    CHECK(style_paint_scroll_system_arrow(dc,&arrow,DFCS_SCROLLDOWN|DFCS_PUSHED|DFCS_FLAT,p));
    CHECK(GetPixel(dc,0,8)==p->shadow && GetPixel(dc,1,8)==p->face);
    CHECK(style_paint_scroll_system_arrow(dc,&arrow,DFCS_SCROLLDOWN|DFCS_INACTIVE,p));
    unsigned gray=0;for(int y=3;y<14;++y)for(int x=3;x<14;++x)gray+=GetPixel(dc,x,y)==p->shadow;
    CHECK(gray==16);
    for(unsigned repeat=0;repeat<64;++repeat)CHECK(style_paint_scroll_system_arrow(dc,&arrow,DFCS_SCROLLDOWN,p));
    CHECK(GetGuiResources(GetCurrentProcess(),GR_GDIOBJECTS)==gdi);
    CHECK(!style_paint_scroll_system_arrow(dc,&arrow,DFCS_SCROLLSIZEGRIP,p));
    CHECK(GetTextColor(dc)==RGB(4,5,6)&&GetBkColor(dc)==RGB(7,8,9));
    puts("WIN2K_ARROW_REFERENCE_OK original_down_shape=1 scroll_not_soft=1 pressed=1 disabled=1 borrowed_font_dc=1 gdi_cycles=64");
    puts("WIN2K_GLYPH_REFERENCE_OK samples=4 original_exact=4 old_dark_negative=4 disabled=1 mixed=1 flat=1 gdi_cycles=64");
    CHECK(SelectObject(dc,previous) && DeleteObject(bitmap) && DeleteDC(dc));
    puts("WIN2K_PAINT_OK original_default_samples=1 normal=1 pressed=1 default_pressed=1 mono_window_slot=1 dc_restored=1");return 0;
}
