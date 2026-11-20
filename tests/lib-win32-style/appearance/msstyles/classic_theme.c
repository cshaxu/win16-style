#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "lib-win32-style/appearance/win2000/window/caption.h"
#include "lib-win32-style/appearance/win2000/common/paint.h"
#include "lib-win32-style/appearance/win2000/control/button_paint.h"
#include "lib-win32-style/appearance/win2000/control/scroll_layout.h"
#include "lib-win32-style/appearance/win3x/window/frame_paint.h"
#include "lib-win32-style/appearance/win3x/menu/menu_bar.h"
#include "lib-win32-style/appearance/win3x/menu/menu_items.h"
#include "lib-win32-style/appearance/win3x/common/font.h"
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
        RECT r={0,0,240,win2k_caption_theme_height(dpi,FALSE,theme)};
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
        win2k_caption_colors fill=win2k_caption_theme_palette(active,theme);
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
    win16_style_theme_palette(&palette,theme);
    win16_frame_paint_state state={.active=active};
    POINT center={120,layout.caption.top+1};
    win16_control_frame_caption(dc,&layout,&palette,&state,NULL,FALSE,FALSE);
    return win16_style_hit(&layout,center)==HTCAPTION && GetPixel(dc,center.x,center.y)==colors.start;
}

static BOOL draw_win2k_border(HDC dc,const DWORD *pixels,const classic_theme *theme)
{
    DWORD reference[240*64];
    win2k_window_border_paint(dc,240,64,4,NULL);
    GdiFlush();
    memcpy(reference,pixels,sizeof(reference));
    win2k_window_border_paint(dc,240,64,4,&theme->frame);
    GdiFlush();
    if(memcmp(reference,pixels,sizeof(reference)))return FALSE;
    const classic_frame_colors variant={RGB(10,74,95),RGB(26,193,180),
        RGB(89,26,175),RGB(187,64,28),TRUE};
    win2k_window_border_paint(dc,240,64,4,&variant);
    if(GetPixel(dc,0,32)!=variant.face || GetPixel(dc,1,32)!=variant.light ||
       GetPixel(dc,238,32)!=variant.shadow || GetPixel(dc,239,32)!=variant.edge)return FALSE;
    win2k_caption_layout caption={.band={4,4,236,23}};
    const UINT flags[WIN2K_CAPTION_PARTS]={0};
    win2k_caption_render_with_colors(dc,&caption,L"",NULL,(HFONT)GetStockObject(DEFAULT_GUI_FONT),
        96,win2k_caption_palette(TRUE),&variant,TRUE,flags);
    return GetPixel(dc,100,22)==variant.face && GetPixel(dc,4,4)==theme->active.start;
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
    for(unsigned flags=0;flags<4;++flags){
        BOOL system=(flags&1)!=0,pressed=(flags&2)!=0;
        RECT bounds={0,0,MulDiv(19,(int)dpi,96),MulDiv(18,(int)dpi,96)};
        int unit=max(1,MulDiv(1,(int)dpi,96));
        win16_style_merged_control(dc,bounds,system,pressed,unit,&palette,NULL);
        GdiFlush();
        memcpy(reference,pixels,sizeof(reference));
        win16_style_merged_control(dc,bounds,system,pressed,unit,&palette,art);
        GdiFlush();
        if(memcmp(reference,pixels,sizeof(reference)))return FALSE;
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

static BOOL paint_native_button(HDC dc,HWND window,BOOL win2000,const classic_push_colors *push,
    const classic_choice_colors *choice,HMODULE module)
{
    style_instance instance={0};
    if(win2000)return win2k_button_paint(window,&instance,dc,push,choice,NULL);
    win16_style_palette palette;
    win16_style_default_palette(&palette);
    style_control_state control={0};
    win16_choice_art art={0};
    const win16_control_choice_view view={&instance,&control,&palette,push,choice,button_type,
        button_fit,button_fit,button_fit,button_fit,button_fit,module,&art,NULL,NULL,NULL};
    win16_control_draw_choice(window,&view,dc);
    return TRUE;
}

static BOOL draw_button(HDC dc,HWND window,const classic_theme *theme,BOOL win2000,
    BOOL disabled,BOOL pressed,BOOL defaulted,enum classic_push_state expected)
{
    EnableWindow(window,!disabled);
    SendMessageW(window,BM_SETSTYLE,defaulted?BS_DEFPUSHBUTTON:BS_PUSHBUTTON,FALSE);
    SendMessageW(window,BM_SETSTATE,pressed,0);
    if(((SendMessageW(window,BM_GETSTATE,0,0)&BST_PUSHED)!=0)!=pressed ||
       IsWindowEnabled(window)==disabled)return FALSE;
    return paint_native_button(dc,window,win2000,&theme->push,&theme->choice,NULL) &&
        GetPixel(dc,40,14)==theme->push.state[expected].face;
}

static BOOL draw_choice_role(HDC dc,const DWORD *pixels,HWND window,const classic_theme *theme,BOOL win2000,
    unsigned radio,HMODULE module)
{
    const UINT modes[]={0,2,3};
    DWORD reference[240*64];
    for(unsigned check=0;check<3-radio;++check){
        for(unsigned i=0;i<ARRAYSIZE(modes);++i){
            unsigned mode=modes[i];
            EnableWindow(window,mode!=3);
            SendMessageW(window,BM_SETCHECK,check,0);
            SendMessageW(window,BM_SETSTATE,mode==2,0);
            if(SendMessageW(window,BM_GETCHECK,0,0)!=(LRESULT)check ||
               !paint_native_button(dc,window,win2000,NULL,NULL,NULL))return FALSE;
            GdiFlush();
            memcpy(reference,pixels,sizeof(reference));
            if(!paint_native_button(dc,window,win2000,NULL,&theme->choice,module))return FALSE;
            GdiFlush();
            if(memcmp(reference,pixels,sizeof(reference)))return FALSE;
            classic_choice_colors altered=theme->choice;
            classic_choice_surface *entry=(radio?altered.radio:altered.check)+4*check+mode;
            if(!entry->fields)return FALSE;
            entry->colors.face=RGB(89,37,102);
            entry->colors.edge=RGB(17,165,123);
            if(!paint_native_button(dc,window,win2000,NULL,&altered,module))return FALSE;
            BOOL saw_face=FALSE,saw_ink=FALSE;
            for(int y=0;y<28;++y)for(int x=0;x<24;++x){
                COLORREF c=GetPixel(dc,x,y);
                saw_face|=c==entry->colors.face;
                saw_ink|=c==entry->colors.edge;
            }
            if(!saw_face || ((check || !win2000) && !saw_ink)){
                fprintf(stderr,"choice state era=%d radio=%u check=%u mode=%u face=%d ink=%d\n",
                    win2000,radio,check,mode,saw_face,saw_ink);
                return FALSE;
            }
        }
    }
    return TRUE;
}

static BOOL draw_choices(HDC dc,const DWORD *pixels,const classic_theme *theme,BOOL win2000,HMODULE module)
{
    HWND parent=CreateWindowExW(0,L"STATIC",L"",WS_OVERLAPPED,0,0,120,64,NULL,NULL,NULL,NULL);
    if(!parent)return FALSE;
    HWND check=CreateWindowExW(0,L"BUTTON",L"",WS_CHILD|BS_AUTO3STATE,0,0,80,28,parent,NULL,NULL,NULL);
    HWND radio=CreateWindowExW(0,L"BUTTON",L"",WS_CHILD|BS_AUTORADIOBUTTON,0,28,80,28,parent,NULL,NULL,NULL);
    BOOL passed=check && radio && draw_choice_role(dc,pixels,check,theme,win2000,0,module) &&
        draw_choice_role(dc,pixels,radio,theme,win2000,1,module);
    DestroyWindow(parent);
    return passed;
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

static int first_ink(HDC dc,int left,int right,int height,COLORREF background)
{
    for(int x=left;x<right;++x)
        for(int y=0;y<height;++y)
            if(GetPixel(dc,x,y)!=background)return x;
    return -1;
}

static BOOL draw_popup(HDC dc,const DWORD *pixels,const classic_theme *theme,const win16_menu_art *art,
    UINT dpi,HFONT font)
{
    WCHAR text[]=L"MMMM\tF10";
    style_menu_item item={.text=text,.submenu=TRUE};
    win16_style_palette palette;
    win16_style_default_palette(&palette);
    const classic_menu_metrics variant={22,12,16,6};
    const classic_menu_metrics *metrics[]={&theme->menu_metrics,&variant};
    unsigned widths[2],heights[2];
    int label[2],shortcut[2],arrow[2];
    win16_style_menu_columns columns={0};
    if(!win16_style_menu_columns_read(dc,font,dpi,text,&columns))return FALSE;
    for(unsigned i=0;i<ARRAYSIZE(metrics);++i){
        MEASUREITEMSTRUCT measure={.CtlType=ODT_MENU,.itemData=(ULONG_PTR)&item};
        if(!win16_style_menu_size(&item,&measure,font,dpi,metrics[i]))return FALSE;
        widths[i]=measure.itemWidth;
        heights[i]=measure.itemHeight;
        int width=win16_style_menu_columns_width(&columns,dpi,TRUE,metrics[i]);
        int label_x=MulDiv(metrics[i]->check_column,(int)dpi,96);
        int shortcut_x=label_x+columns.label+MulDiv(metrics[i]->shortcut_gap,(int)dpi,96);
        int arrow_x=width-MulDiv(11,(int)dpi,96);
        DRAWITEMSTRUCT draw={.CtlType=ODT_MENU,.itemData=(ULONG_PTR)&item,.hDC=dc,
            .rcItem={0,0,width,(LONG)heights[i]},.itemState=ODS_SELECTED};
        if(!win16_style_menu_draw(&item,&draw,font,dpi,&palette,&theme->menu_popup,metrics[i],art))return FALSE;
        COLORREF background=theme->menu_popup.selection;
        if(GetPixel(dc,0,0)!=background)return FALSE;
        label[i]=first_ink(dc,0,shortcut_x,(int)heights[i],background);
        shortcut[i]=first_ink(dc,shortcut_x,arrow_x,(int)heights[i],background);
        arrow[i]=first_ink(dc,arrow_x,width,(int)heights[i],background);
        if(label[i]<label_x || shortcut[i]<shortcut_x || arrow[i]<arrow_x)return FALSE;
        draw.itemState|=ODS_CHECKED;
        DWORD reference[240*64];
        if(!win16_style_menu_draw(&item,&draw,font,dpi,&palette,&theme->menu_popup,metrics[i],NULL))return FALSE;
        GdiFlush();
        memcpy(reference,pixels,sizeof(reference));
        if(!win16_style_menu_draw(&item,&draw,font,dpi,&palette,&theme->menu_popup,metrics[i],art))return FALSE;
        GdiFlush();
        if(memcmp(reference,pixels,sizeof(reference)) ||
           first_ink(dc,0,label_x,(int)heights[i],background)<0)return FALSE;
        win16_menu_art blank=*art;
        memset(blank.check,'1',sizeof(blank.check));
        memset(blank.arrow,'1',sizeof(blank.arrow));
        if(!win16_style_menu_draw(&item,&draw,font,dpi,&palette,&theme->menu_popup,metrics[i],&blank) ||
           first_ink(dc,0,label_x,(int)heights[i],background)>=0 ||
           first_ink(dc,arrow_x,width,(int)heights[i],background)>=0)return FALSE;
    }
    int step=MulDiv(4,(int)dpi,96);
    return widths[1]-widths[0]==(unsigned)(3*step) && heights[1]==heights[0] &&
        label[1]-label[0]==step && shortcut[1]-shortcut[0]==2*step && arrow[1]-arrow[0]==3*step;
}

static BOOL draw_themed_popup(HDC dc,const DWORD *pixels,const classic_theme *theme,
    const win16_menu_art *art,UINT dpi)
{
    msstyles_font alternate=theme->menu_font;
    alternate.points=24;
    HFONT font=win16_style_menu_font_acquire(dpi,&theme->menu_font);
    HFONT legacy=win16_style_menu_font_acquire(dpi,NULL);
    HFONT smaller=win16_style_menu_font_acquire(dpi,&alternate);
    BOOL ok=font && legacy==font && smaller && smaller!=font;
    if(ok){
        LOGFONTW before={0},after={0};
        ok=GetObjectW(font,sizeof(before),&before)==sizeof(before) &&
            GetObjectW(smaller,sizeof(after),&after)==sizeof(after) &&
            after.lfHeight>before.lfHeight && draw_popup(dc,pixels,theme,art,dpi,font);
        if(!ok)fprintf(stderr,"Win16 popup DPI %u base height %ld alternate %ld\n",
            dpi,(long)before.lfHeight,(long)after.lfHeight);
    }
    win16_style_system_release(smaller);
    win16_style_system_release(legacy);
    win16_style_system_release(font);
    return ok;
}

static BOOL render(const classic_theme *theme,BOOL win2000,const win16_caption_art *art,const win16_menu_art *menu_art,HMODULE module)
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
        (!win2000 || draw_win2k_border(dc,bits,theme)) &&
        (win2000 || (draw_caption_art(dc,bits,theme,art,96) && draw_caption_art(dc,bits,theme,art,144))) &&
        window && draw_button(dc,window,&altered,win2000,FALSE,FALSE,FALSE,CLASSIC_PUSH_NORMAL) &&
        draw_button(dc,window,&altered,win2000,FALSE,TRUE,FALSE,CLASSIC_PUSH_PRESSED) &&
        draw_button(dc,window,&altered,win2000,TRUE,FALSE,TRUE,CLASSIC_PUSH_DISABLED) &&
        draw_button(dc,window,&altered,win2000,FALSE,FALSE,TRUE,CLASSIC_PUSH_DEFAULTED) &&
        draw_button(dc,window,&altered,win2000,FALSE,TRUE,TRUE,CLASSIC_PUSH_PRESSED) &&
        draw_choices(dc,bits,theme,win2000,module);
    for(unsigned state=0;passed && state<3;++state)
        passed=draw_scroll(dc,theme,win2000,96,state) && draw_scroll(dc,theme,win2000,144,state);
    if(passed && !win2000)passed=draw_themed_popup(dc,bits,theme,menu_art,96) &&
        draw_themed_popup(dc,bits,theme,menu_art,144);
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
    classic_theme_mapping source={0};
    if(!recolored){
        WCHAR descriptor[MAX_PATH];
        CHECK(swprintf(descriptor,ARRAYSIZE(descriptor),L"%ls/%ls/%ls.theme",root,name,name)>0);
        CHECK(classic_theme_open(descriptor,&source)==S_OK && source.resources);
        CHECK(!memcmp(&source.model,&theme,sizeof(theme)));
        if(gradient){
            for(WORD id=201;id<=203;++id){
                HRSRC dialog=FindResourceW(source.resources,MAKEINTRESOURCEW(id),MAKEINTRESOURCEW(5));
                CHECK(dialog && SizeofResource(source.resources,dialog)>0);
            }
        }
        CHECK(classic_theme_open(descriptor,&source)==E_INVALIDARG);
        CHECK(!memcmp(&source.model,&theme,sizeof(theme)));
    }
    CHECK(theme.gradient==gradient);
    CHECK(theme.menu_bar.present && theme.menu_popup.present);
    CHECK(theme.menu_bar.background==(gradient?RGB(212,208,200):RGB(255,255,255)) &&
        theme.menu_bar.selection==(gradient?RGB(0,0,128):RGB(164,200,240)) &&
        theme.menu_popup.separator==(gradient?RGB(128,128,128):RGB(0,0,0)));
    for(UINT dpi=96;dpi<=192;dpi+=48){
        LOGFONTW title=msstyles_font_at_dpi(&theme.caption_font,dpi);
        CHECK(title.lfHeight==-MulDiv(gradient?8:10,(int)dpi,72) && title.lfWeight==FW_BOLD &&
              title.lfQuality==(gradient?DEFAULT_QUALITY:NONANTIALIASED_QUALITY) &&
              !wcscmp(title.lfFaceName,gradient?L"Tahoma":L"MS Sans Serif"));
        LOGFONTW menu=msstyles_font_at_dpi(&theme.menu_font,dpi);
        if(gradient){
            LOGFONTW small=msstyles_font_at_dpi(&theme.small_caption_font,dpi);
            CHECK(small.lfHeight==-MulDiv(8,(int)dpi,96) && small.lfWeight==FW_BOLD &&
                  menu.lfHeight==-MulDiv(8,(int)dpi,72) && menu.lfWeight==FW_NORMAL);
        }else CHECK(menu.lfHeight==-MulDiv(12,(int)dpi,72) && menu.lfWeight==FW_BOLD &&
            !wcscmp(menu.lfFaceName,L"System"));
    }
    COLORREF start=recolored?RGB(37,77,123):gradient?RGB(0,0,128):RGB(164,200,240);
    CHECK(theme.active.start==start);
    CHECK(theme.active.end==(gradient?RGB(166,202,240):start));
    CHECK(theme.inactive.start==(gradient?RGB(128,128,128):RGB(255,255,255)));
    CHECK(theme.inactive.text==(gradient?RGB(192,192,192):RGB(0,0,0)));
    COLORREF state_face=gradient?RGB(212,208,200):RGB(192,192,192);
    CHECK(theme.frame.present && theme.frame.face==state_face && theme.frame.light==RGB(255,255,255) &&
        theme.frame.shadow==RGB(128,128,128) && theme.frame.edge==RGB(0,0,0));
    CHECK(theme.label.present && theme.label.normal==RGB(0,0,0) && theme.label.disabled==RGB(128,128,128));
    CHECK(theme.group.text.present && theme.group.text.normal==RGB(0,0,0) &&
        theme.group.text.disabled==(gradient?RGB(128,128,128):RGB(0,0,0)) &&
        theme.group.shadow==RGB(128,128,128) && theme.group.highlight==RGB(255,255,255));
    for(size_t i=0;i<ARRAYSIZE(theme.edit);++i)
        CHECK(theme.edit[i].present && theme.edit[i].line==RGB(0,0,0) &&
            theme.edit[i].dark==(gradient?RGB(64,64,64):RGB(0,0,0)));
    CHECK(theme.list.present && theme.list.background==RGB(255,255,255) &&
        theme.list.selection==(gradient?RGB(10,36,106):RGB(164,200,240)) &&
        theme.list.selection_text==(gradient?RGB(255,255,255):RGB(0,0,0)) &&
        theme.list.dark==(gradient?RGB(64,64,64):RGB(0,0,0)));
    CHECK(theme.combo.present && theme.combo.border==theme.list.dark && theme.combo.disabled_border==theme.combo.border);
    for(size_t i=0;i<ARRAYSIZE(theme.combo.button);++i)
        CHECK(theme.combo.button[i]==state_face);
    CHECK(theme.tab.present && theme.tab.pane==state_face && theme.tab.normal==state_face &&
        theme.tab.selected==(gradient?state_face:RGB(255,255,255)) && theme.tab.border==theme.list.dark);
    CHECK(theme.header.present && theme.header.normal==state_face && theme.header.pressed==state_face);
    for(size_t i=0;i<ARRAYSIZE(theme.trackbar);++i)
        CHECK(theme.trackbar[i].present && theme.trackbar[i].channel==RGB(255,255,255) &&
            theme.trackbar[i].thumb==state_face && theme.trackbar[i].edge==theme.list.dark);
    for(size_t i=0;i<ARRAYSIZE(theme.progress);++i){
        const classic_progress_colors *progress=&theme.progress[i];
        CHECK(progress->present && progress->trough==(gradient?state_face:RGB(255,255,255)) &&
            progress->fill==theme.list.selection && progress->extra_states==!gradient);
        if(!gradient)CHECK(progress->border==RGB(0,0,0) && progress->error==RGB(192,0,0) &&
            progress->paused==RGB(192,192,0));
    }
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
    win16_menu_art menu_art={0};
    HMODULE module=source.resources;
    if(!gradient){
        if(!module)module=LoadLibraryExW(path,NULL,LOAD_LIBRARY_AS_DATAFILE|LOAD_LIBRARY_AS_IMAGE_RESOURCE);
        CHECK(module);
        CHECK(SUCCEEDED(win16_caption_art_read(module,&theme.caption_images,&art)) && art.available);
        classic_caption_images wrong=theme.caption_images;
        wrong.id[CLASSIC_IMAGE_SYSTEM][0]=wrong.id[CLASSIC_IMAGE_MINIMIZE][0];
        win16_caption_art prior_art=art;
        CHECK(FAILED(win16_caption_art_read(module,&wrong,&art)) && !memcmp(&prior_art,&art,sizeof(art)));
        CHECK(SUCCEEDED(win16_menu_art_read(module,theme.menu_images,&menu_art)) && menu_art.available);
        win16_menu_art prior_menu=menu_art;
        WORD wrong_menu[]={theme.menu_images[1],theme.menu_images[0]};
        CHECK(FAILED(win16_menu_art_read(module,wrong_menu,&menu_art)) &&
            !memcmp(&prior_menu,&menu_art,sizeof(menu_art)));
    }else CHECK(!theme.menu_images[0] && !theme.menu_images[1]);
    BOOL rendered=render(&theme,gradient,&art,&menu_art,module);
    if(source.resources){
        classic_theme_close(&source);
        CHECK(!source.resources);
    }else if(module)CHECK(FreeLibrary(module));
    CHECK(rendered);
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
