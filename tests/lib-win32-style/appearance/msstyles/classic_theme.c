#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "lib-win32-style/appearance/win2000/window/caption.h"
#include "lib-win32-style/appearance/win2000/common/paint.h"
#include "lib-win32-style/appearance/win2000/control/button_paint.h"
#include "lib-win32-style/appearance/win2000/control/scroll_layout.h"
#include "lib-win32-style/appearance/win3x/window/frame_paint.h"
#include "lib-win32-style/appearance/win3x/control/basic_paint.h"
#include "lib-win32-style/appearance/win3x/control/choice_view.h"
#include "lib-win32-style/appearance/win3x/control/scroll_layout.h"
#include "lib-win32-style/appearance/win3x/resources/scroll_assets.h"
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#define CHECK(expr) do{if(!(expr)){fprintf(stderr,"line %d: %s\n",__LINE__,#expr);return 1;}}while(0)

static BOOL draw_band(HDC dc,const classic_theme *theme,BOOL win2000,BOOL active,UINT dpi)
{
    classic_caption_colors colors=active?theme->active:theme->inactive;
    if(win2000){
        RECT r={0,0,240,win2k_caption_height(dpi,FALSE)};
        const BOOL present[]={TRUE,TRUE,FALSE,TRUE};
        RECT actual[4],reference[4],alternate[4];
        classic_button_group changed=theme->title_buttons;
        changed.slots[3].gap_before+=2;
        if(!win2k_caption_buttons(&r,dpi,FALSE,present,reference) ||
           !win2k_caption_buttons_with_group(&r,dpi,FALSE,present,&theme->title_buttons,&theme->caption_metrics,actual) ||
           !win2k_caption_buttons_with_group(&r,dpi,FALSE,present,&changed,&theme->caption_metrics,alternate) ||
           memcmp(actual,reference,sizeof(actual)) ||
           !EqualRect(&actual[WIN2K_CLOSE],&alternate[WIN2K_CLOSE]) ||
           actual[WIN2K_MAXIMIZE].left-alternate[WIN2K_MAXIMIZE].left!=MulDiv(4,(int)dpi,96)-MulDiv(2,(int)dpi,96))
            return FALSE;
        win2k_caption_colors fill={colors.start,colors.end,colors.text,theme->gradient};
        return win2k_caption_paint(dc,&r,L"",(HFONT)GetStockObject(DEFAULT_GUI_FONT),&fill) &&
            GetPixel(dc,0,5)==colors.start && GetPixel(dc,239,5)!=colors.start;
    }
    win16_style_layout layout,reference,alternate;
    classic_button_group changed=theme->title_buttons;
    changed.slots[2].gap_before=4;
    if(!win16_style_measure(240,64,dpi,WS_OVERLAPPEDWINDOW,0,FALSE,&reference) ||
       !win16_style_measure_with_buttons(240,64,dpi,WS_OVERLAPPEDWINDOW,0,FALSE,&theme->title_buttons,&theme->caption_metrics,&layout) ||
       !win16_style_measure_with_buttons(240,64,dpi,WS_OVERLAPPEDWINDOW,0,FALSE,&changed,&theme->caption_metrics,&alternate) ||
       !EqualRect(&reference.title,&layout.title) || !EqualRect(&reference.minimize,&layout.minimize) ||
       !EqualRect(&reference.maximize,&layout.maximize) || !EqualRect(&reference.system,&layout.system) ||
       !EqualRect(&alternate.maximize,&layout.maximize) ||
       layout.minimize.left-alternate.minimize.left!=MulDiv(4,(int)dpi,96))return FALSE;
    POINT changed_min={alternate.minimize.left,alternate.minimize.top};
    if(win16_style_hit(&alternate,changed_min)!=HTMINBUTTON ||
       win16_style_hit(&layout,changed_min)!=HTCAPTION)return FALSE;
    win16_style_palette palette;
    win16_style_default_palette(&palette);
    palette.active=theme->active.start;
    palette.inactive=theme->inactive.start;
    win16_frame_paint_state state={.active=active};
    POINT center={120,layout.caption.top+1};
    win16_control_frame_caption(dc,&layout,&palette,&state,NULL,FALSE,FALSE);
    return win16_style_hit(&layout,center)==HTCAPTION && GetPixel(dc,center.x,center.y)==colors.start;
}

static BOOL draw_caption_art(HDC dc,const DWORD *pixels,const classic_theme *theme,const win16_caption_art *art,UINT dpi)
{
    win16_style_layout layout;
    if(!win16_style_measure_with_buttons(240,64,dpi,WS_OVERLAPPEDWINDOW,0,FALSE,
        &theme->title_buttons,&theme->caption_metrics,&layout))return FALSE;

    win16_style_palette palette;
    win16_style_default_palette(&palette);
    palette.edge=RGB(5,20,34);
    palette.light=RGB(29,60,91);
    palette.shadow=RGB(93,55,64);
    palette.face=RGB(117,39,46);
    const int hits[]={HTMINBUTTON,HTMAXBUTTON,HTSYSMENU};
    DWORD reference[240*64];
    for(unsigned flags=0;flags<8;++flags){
        win16_frame_paint_state state={0};
        BOOL pressed=(flags&1)!=0,child=(flags&2)!=0,zoomed=(flags&4)!=0;
        for(unsigned i=0;i<ARRAYSIZE(hits);++i){
            state.button=(style_caption_button){pressed?hits[i]:0,pressed};
            state.art=NULL;
            win16_control_frame_caption(dc,&layout,&palette,&state,NULL,child,zoomed);
            GdiFlush();
            memcpy(reference,pixels,sizeof(reference));
            state.art=art;
            win16_control_frame_caption(dc,&layout,&palette,&state,NULL,child,zoomed);
            GdiFlush();
            if(memcmp(reference,pixels,sizeof(reference)))return FALSE;
        }
    }
    return TRUE;
}

static UINT button_type(HWND window,DWORD style)
{
    (void)window;
    return style&BS_TYPEMASK;
}

static BOOL button_fit(HWND window)
{
    (void)window;
    return FALSE;
}

static BOOL draw_button(HDC dc,HWND window,const classic_theme *theme,BOOL win2000,
    BOOL disabled,BOOL pressed,BOOL defaulted,enum classic_push_state expected)
{
    EnableWindow(window,!disabled);
    SendMessageW(window,BM_SETSTYLE,defaulted?BS_DEFPUSHBUTTON:BS_PUSHBUTTON,FALSE);
    SendMessageW(window,BM_SETSTATE,pressed,0);
    if(((SendMessageW(window,BM_GETSTATE,0,0)&BST_PUSHED)!=0)!=pressed ||
       IsWindowEnabled(window)==disabled)return FALSE;
    style_instance instance={0};
    if(win2000){
        if(!win2k_button_paint(window,&instance,dc,&theme->push))return FALSE;
    }else{
        win16_style_palette palette;
        win16_style_default_palette(&palette);
        style_control_state control={0};
        const win16_control_choice_view view={&instance,&control,&palette,&theme->push,button_type,
            button_fit,button_fit,button_fit,button_fit,button_fit};
        win16_control_draw_choice(window,&view,dc);
    }
    return GetPixel(dc,40,14)==theme->push.state[expected].face;
}

static BOOL draw_scroll(HDC dc,const classic_theme *theme,BOOL win2000,UINT dpi,unsigned state)
{
    RECT bar={0,0,MulDiv(17,(int)dpi,96),60};
    int nominal=classic_scroll_nominal(theme->scroll_metrics,CLASSIC_SCROLL_EXTENT_DEFAULT,&bar,TRUE,dpi);
    style_scroll_layout geometry;
    if(!PatBlt(dc,0,0,bar.right,60,WHITENESS))return FALSE;
    if(win2000){
        const SCROLLINFO info={.nMin=0,.nMax=100,.nPage=5,.nPos=30};
        if(!win2k_scroll_measure(&bar,TRUE,nominal,&info,&geometry))return FALSE;
        win2k_palette palette=win2k_standard_palette;
        palette.face=theme->scroll_arrow.face;
        palette.dark=theme->scroll_arrow.edge;
        const UINT flags[]={0,DFCS_PUSHED,DFCS_INACTIVE};
        if(!win2k_scroll_arrow_button(dc,&geometry.first,DFCS_SCROLLUP|flags[state],&palette))return FALSE;
    }else{
        const win16_scroll_metrics metrics={nominal,nominal,1};
        if(!win16_scroll_measure(&bar,TRUE,&metrics,0,100,30,&geometry))return FALSE;
        win16_style_palette palette;
        win16_style_default_palette(&palette);
        palette.face=theme->scroll_arrow.face;
        palette.edge=theme->scroll_arrow.edge;
        const enum win16_control_scroll_asset asset[]={WIN16_SCROLL_UP,WIN16_SCROLL_UP_PRESSED,
            WIN16_SCROLL_UP_DISABLED};
        POINT p={nominal/2,nominal/2};
        if(win16_scroll_hit(&geometry,p,state==2?ESB_DISABLE_UP:0)!=
           (state==2?STYLE_SCROLL_NONE:STYLE_SCROLL_LINE_BEFORE))return FALSE;
        win16_control_scroll_arrow(dc,geometry.first,win16_control_scroll_glyph(asset[state]),&palette);
    }
    for(LONG y=geometry.first.top;y<geometry.first.bottom;++y)
        for(LONG x=geometry.first.left;x<geometry.first.right;++x)
            if(GetPixel(dc,x,y)==theme->scroll_arrow.face)return TRUE;
    return FALSE;
}

static BOOL render(const classic_theme *theme,BOOL win2000,const win16_caption_art *art)
{
    HDC dc=CreateCompatibleDC(NULL);
    if(!dc)return FALSE;
    BITMAPINFO info={.bmiHeader={.biSize=sizeof(BITMAPINFOHEADER),.biWidth=240,.biHeight=-64,
        .biPlanes=1,.biBitCount=32,.biCompression=BI_RGB}};
    void *bits;
    HBITMAP bitmap=CreateDIBSection(dc,&info,DIB_RGB_COLORS,&bits,NULL,0);
    HGDIOBJ saved=bitmap?SelectObject(dc,bitmap):NULL;
    HWND window=CreateWindowExW(0,L"BUTTON",L"",WS_CHILD,0,0,80,28,HWND_MESSAGE,NULL,NULL,NULL);
    classic_theme altered=*theme;
    altered.push.state[CLASSIC_PUSH_PRESSED].face=RGB(37,86,173);
    altered.push.state[CLASSIC_PUSH_DISABLED].face=RGB(173,52,39);
    altered.push.state[CLASSIC_PUSH_DEFAULTED].face=RGB(40,152,63);
    BOOL passed=saved && saved!=HGDI_ERROR &&
        draw_band(dc,theme,win2000,TRUE,96) && draw_band(dc,theme,win2000,FALSE,96) &&
        draw_band(dc,theme,win2000,TRUE,144) && draw_band(dc,theme,win2000,FALSE,144) &&
        (win2000 || (draw_caption_art(dc,bits,theme,art,96) && draw_caption_art(dc,bits,theme,art,144))) &&
        window && draw_button(dc,window,&altered,win2000,FALSE,FALSE,FALSE,CLASSIC_PUSH_NORMAL) &&
        draw_button(dc,window,&altered,win2000,FALSE,TRUE,FALSE,CLASSIC_PUSH_PRESSED) &&
        draw_button(dc,window,&altered,win2000,TRUE,FALSE,TRUE,CLASSIC_PUSH_DISABLED) &&
        draw_button(dc,window,&altered,win2000,FALSE,FALSE,TRUE,CLASSIC_PUSH_DEFAULTED) &&
        draw_button(dc,window,&altered,win2000,FALSE,TRUE,TRUE,CLASSIC_PUSH_PRESSED);
    for(unsigned state=0;passed && state<3;++state)
        passed=draw_scroll(dc,theme,win2000,96,state) && draw_scroll(dc,theme,win2000,144,state);
    if(window)DestroyWindow(window);
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
    COLORREF state_face=gradient?RGB(212,208,200):RGB(192,192,192);
    for(unsigned state=0;state<CLASSIC_PUSH_STATE_COUNT;++state)
        CHECK(theme.push.state[state].face==(recolored?RGB(251,37,52):state_face) &&
              theme.push.state[state].edge==(gradient?RGB(64,64,64):RGB(0,0,0)));
    CHECK(theme.scroll_arrow.face==(recolored?RGB(89,97,163):state_face));
    CHECK(theme.scroll_arrow.edge==(gradient?RGB(64,64,64):RGB(0,0,0)));
    CHECK(theme.scroll.tracks_and_thumbs && theme.scroll.own_sizebox==gradient);
    CHECK(theme.caption_metrics.height==(gradient?19:18) &&
          theme.caption_metrics.button.cx==(gradient?16:19) &&
          theme.caption_metrics.button.cy==(gradient?14:18));
    for(unsigned axis=0;axis<2;++axis){
        CHECK(theme.scroll.track[axis].face==state_face &&
              theme.scroll.track[axis].highlight==RGB(255,255,255));
        CHECK(theme.scroll.thumb[axis].face==state_face &&
              theme.scroll.thumb[axis].dark==theme.scroll_arrow.edge);
    }
    if(gradient)CHECK(theme.scroll.sizebox.face==state_face);
    CHECK(theme.title_buttons.count==(gradient?4:3));
    win16_caption_art art={0};
    if(!gradient){
        HMODULE module=LoadLibraryExW(path,NULL,LOAD_LIBRARY_AS_DATAFILE|LOAD_LIBRARY_AS_IMAGE_RESOURCE);
        CHECK(module);
        CHECK(SUCCEEDED(win16_caption_art_read(module,&theme.caption_images,&art)) && art.available);
        classic_caption_images wrong=theme.caption_images;
        wrong.id[CLASSIC_IMAGE_SYSTEM][0]=wrong.id[CLASSIC_IMAGE_MINIMIZE][0];
        win16_caption_art prior_art=art;
        CHECK(FAILED(win16_caption_art_read(module,&wrong,&art)) && !memcmp(&prior_art,&art,sizeof(art)));
        CHECK(FreeLibrary(module));
    }
    CHECK(render(&theme,gradient,&art));
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
    return check(argv[1],L"Win16",FALSE,recolored) || check(argv[1],L"Win2K",TRUE,recolored);
}
