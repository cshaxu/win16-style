#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "lib-win32-style/appearance/win2000/window/caption.h"
#include "lib-win32-style/appearance/win3x/window/frame_paint.h"
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define CHECK(expr) do{if(!(expr)){fprintf(stderr,"line %d: %s\n",__LINE__,#expr);return 1;}}while(0)

static BOOL draw_band(HDC dc,const classic_theme *theme,BOOL win2000,BOOL active,UINT dpi)
{
    classic_caption_colors colors=active?theme->active:theme->inactive;
    if(win2000){
        RECT r={0,0,240,win2k_caption_height(dpi,FALSE)};
        win2k_caption_colors fill={colors.start,colors.end,colors.text,theme->gradient};
        return win2k_caption_paint(dc,&r,L"",(HFONT)GetStockObject(DEFAULT_GUI_FONT),&fill) &&
            GetPixel(dc,0,5)==colors.start && GetPixel(dc,239,5)!=colors.start;
    }
    win16_style_layout layout;
    if(!win16_style_measure(240,64,dpi,WS_OVERLAPPEDWINDOW,0,FALSE,&layout))return FALSE;
    win16_style_palette palette;
    win16_style_default_palette(&palette);
    palette.active=theme->active.start;
    palette.inactive=theme->inactive.start;
    win16_frame_paint_state state={.active=active};
    POINT center={120,layout.caption.top+1};
    win16_control_frame_caption(dc,&layout,&palette,&state,NULL,FALSE,FALSE);
    return win16_style_hit(&layout,center)==HTCAPTION && GetPixel(dc,center.x,center.y)==colors.start;
}

static BOOL render(const classic_theme *theme,BOOL win2000)
{
    HDC dc=CreateCompatibleDC(NULL);
    if(!dc)return FALSE;
    BITMAPINFO info={.bmiHeader={.biSize=sizeof(BITMAPINFOHEADER),.biWidth=240,.biHeight=-64,
        .biPlanes=1,.biBitCount=32,.biCompression=BI_RGB}};
    void *bits;
    HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,NULL,0);
    HGDIOBJ saved=bitmap?SelectObject(dc,bitmap):NULL;
    BOOL passed=saved && saved!=HGDI_ERROR &&
        draw_band(dc,theme,win2000,TRUE,96) && draw_band(dc,theme,win2000,FALSE,96) &&
        draw_band(dc,theme,win2000,TRUE,144) && draw_band(dc,theme,win2000,FALSE,144);
    if(saved && saved!=HGDI_ERROR)SelectObject(dc,saved);
    if(bitmap)DeleteObject(bitmap);
    DeleteDC(dc);
    return passed;
}

static int check(PCWSTR root,PCWSTR name,BOOL gradient,BOOL recolored)
{
    WCHAR path[MAX_PATH];
    CHECK(swprintf(path,ARRAYSIZE(path),L"%ls/%ls/%ls.msstyles",root,name,name)>0);
    classic_theme theme={0};
    HRESULT result=classic_theme_load(path,L"NormalSize",L"NormalColor",&theme);
    if(FAILED(result))fprintf(stderr,"classic theme load HRESULT=%08lx\n",(unsigned long)result);
    CHECK(result==S_OK);
    CHECK(theme.gradient==gradient);
    COLORREF start=recolored?RGB(37,77,123):gradient?RGB(0,0,128):RGB(164,200,240);
    CHECK(theme.active.start==start);
    CHECK(theme.active.end==(gradient?RGB(166,202,240):start));
    CHECK(theme.inactive.start==(gradient?RGB(128,128,128):RGB(255,255,255)));
    CHECK(theme.inactive.text==(gradient?RGB(192,192,192):RGB(0,0,0)));
    CHECK(theme.button.face==(gradient?RGB(212,208,200):RGB(192,192,192)));
    CHECK(theme.scroll_arrow.edge==(gradient?RGB(64,64,64):RGB(0,0,0)));
    CHECK(theme.title_buttons.count==(gradient?4:3));
    CHECK(render(&theme,gradient));
    unsigned char prior[sizeof(theme)];
    memcpy(prior,&theme,sizeof(theme));
    CHECK(classic_theme_load(path,L"MissingSize",L"NormalColor",&theme)==
        HRESULT_FROM_WIN32(ERROR_RESOURCE_NAME_NOT_FOUND));
    CHECK(!memcmp(prior,&theme,sizeof(theme)));
    return 0;
}

int wmain(int argc,WCHAR **argv)
{
    BOOL recolored=argc==3 && !wcscmp(argv[2],L"--recolor");
    if(argc!=2 && !recolored)return 2;
    return check(argv[1],L"Win16",FALSE,recolored) || check(argv[1],L"Win2000",TRUE,recolored);
}
