#define _WIN32_WINNT 0x0A00
#include "lib-win32-style/application/control/scroll_geometry.h"
#include "lib-win32-style/appearance/msstyles/classic_theme.h"
#include "lib-win32-style/application/menu/row_text.h"
#include "lib-win32-style/appearance/msstyles/classic_presets.h"
#include "lib-win32-style/appearance/win2000/window/caption.h"
#include "lib-win32-style/appearance/win2000/common/paint.h"
#include "lib-win32-style/appearance/win2000/control/button_paint.h"
#include "lib-win32-style/appearance/win2000/control/edit_paint.h"
#include "lib-win32-style/appearance/win2000/control/list_combo.h"
#include "lib-win32-style/appearance/win2000/control/static_paint.h"
#include "lib-win32-style/appearance/win3x/window/frame_paint.h"
#include "lib-win32-style/appearance/win3x/menu/menu_bar.h"
#include "lib-win32-style/appearance/win3x/menu/menu_items.h"
#include "lib-win32-style/appearance/win3x/common/font.h"
#include "lib-win32-style/appearance/win3x/control/basic_paint.h"
#include "lib-win32-style/appearance/win3x/control/choice_view.h"
#include "lib-win32-style/appearance/win3x/control/combo_paint.h"
#include "lib-win32-style/appearance/win3x/control/control_state.h"
#include "lib-win32-style/appearance/win3x/control/list_paint.h"
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
       !win16_style_measure_with_buttons(240,64,dpi,WS_OVERLAPPEDWINDOW,0,FALSE,&theme->title_buttons,&theme->caption_metrics,
           &theme->border_presentation.geometry,dpi,&layout) ||
       !win16_style_measure_with_buttons(240,64,dpi,WS_OVERLAPPEDWINDOW,0,FALSE,&changed,&theme->caption_metrics,
           NULL,dpi,&alternate) ||
       !EqualRect(&reference.title,&layout.title) || !EqualRect(&reference.buttons[CLASSIC_BUTTON_MINIMIZE],&layout.buttons[CLASSIC_BUTTON_MINIMIZE]) ||
       !EqualRect(&reference.buttons[CLASSIC_BUTTON_MAXIMIZE],&layout.buttons[CLASSIC_BUTTON_MAXIMIZE]) || !EqualRect(&reference.buttons[CLASSIC_BUTTON_SYSTEM],&layout.buttons[CLASSIC_BUTTON_SYSTEM]) ||
       !EqualRect(&alternate.buttons[CLASSIC_BUTTON_MAXIMIZE],&layout.buttons[CLASSIC_BUTTON_MAXIMIZE]) ||
       layout.buttons[CLASSIC_BUTTON_MINIMIZE].left-alternate.buttons[CLASSIC_BUTTON_MINIMIZE].left!=MulDiv(4,(int)dpi,96))return FALSE;
    POINT changed_min={alternate.buttons[CLASSIC_BUTTON_MINIMIZE].left,alternate.buttons[CLASSIC_BUTTON_MINIMIZE].top};
    if(win16_style_hit(&alternate,changed_min)!=HTMINBUTTON ||
       win16_style_hit(&layout,changed_min)!=HTCAPTION)return FALSE;
    changed=theme->title_buttons;
    changed.slots[0].side=CLASSIC_BUTTON_RIGHT;
    if(!win16_style_measure_with_buttons(240,64,dpi,WS_OVERLAPPEDWINDOW,0,FALSE,
        &changed,&theme->caption_metrics,NULL,dpi,&alternate))return FALSE;
    RECT old_system=layout.buttons[CLASSIC_BUTTON_SYSTEM],new_system=alternate.buttons[CLASSIC_BUTTON_SYSTEM];
    POINT old_point={old_system.left,old_system.top},new_point={new_system.left,new_system.top};
    if(win16_style_hit(&layout,old_point)!=HTSYSMENU || win16_style_hit(&alternate,old_point)!=HTCAPTION ||
       win16_style_hit(&alternate,new_point)!=HTSYSMENU ||
       style_caption_hit(alternate.buttons+CLASSIC_BUTTON_SYSTEM,CLASSIC_BUTTON_SYSTEM,0,new_point)!=HTNOWHERE ||
       style_caption_command(style_caption_role(HTSYSMENU),FALSE)!=SC_KEYMENU ||
       style_caption_command(style_caption_role(HTMAXBUTTON),TRUE)!=SC_RESTORE)return FALSE;
    win16_style_palette palette;
    win16_style_default_palette(&palette);
    win16_style_theme_palette(&palette,theme);
    win16_frame_paint_state state={.active=active,.presentation=theme->caption_presentation};
    POINT center={120,layout.caption.top+1};
    win16_control_frame_caption(dc,&layout,&palette,&state,NULL,FALSE,FALSE);
    return win16_style_hit(&layout,center)==HTCAPTION && GetPixel(dc,center.x,center.y)==colors.start;
}

static BOOL draw_win2k_border(HDC dc,const DWORD *pixels,const classic_theme *theme)
{
    DWORD reference[240*64];
    win2k_window_border_paint(dc,240,64,4,NULL,TRUE,FALSE);
    GdiFlush();
    memcpy(reference,pixels,sizeof(reference));
    win2k_window_border_paint(dc,240,64,4,theme,TRUE,FALSE);
    GdiFlush();
    if(memcmp(reference,pixels,sizeof(reference)))return FALSE;
    const classic_frame_colors variant={RGB(10,74,95),RGB(26,193,180),
        RGB(89,26,175),RGB(187,64,28),TRUE};
    classic_theme recolored=*theme;
    recolored.frame=variant;
    win2k_window_border_paint(dc,240,64,4,&recolored,TRUE,FALSE);
    if(GetPixel(dc,0,32)!=variant.face || GetPixel(dc,1,32)!=variant.light ||
       GetPixel(dc,238,32)!=variant.shadow || GetPixel(dc,239,32)!=variant.edge)return FALSE;
    win2k_caption_layout caption={.band={4,4,236,23}};
    const UINT flags[WIN2K_CAPTION_PARTS]={0};
    win2k_caption_render_with_colors(dc,&caption,L"",NULL,(HFONT)GetStockObject(DEFAULT_GUI_FONT),
        96,win2k_caption_palette(TRUE),&variant,&theme->caption_presentation,TRUE,flags,NULL);
    if(GetPixel(dc,100,22)!=variant.face || GetPixel(dc,4,4)!=theme->active.start)return FALSE;
    /* Give the same adapter a clipped fill and choose different stroke positions. */
    recolored.border_presentation=(style_border_presentation){STYLE_BORDER_CLIPPED,STYLE_BORDER_TITLE,
        {STYLE_BORDER_OUTER,STYLE_BORDER_EDGE,STYLE_BORDER_EDGE},
        {STYLE_BORDER_INNER,STYLE_BORDER_SHADOW,STYLE_BORDER_SHADOW},STYLE_BORDER_LIGHT,STYLE_BORDER_KEEP,{0}};
    RECT bounds={0,0,240,64};
    COLORREF client=RGB(19,201,87);
    if(!style_caption_fill(dc,bounds,client,client,FALSE))return FALSE;
    int saved=SaveDC(dc);
    if(!saved)return FALSE;
    ExcludeClipRect(dc,4,4,236,60);
    win2k_window_border_paint(dc,240,64,4,&recolored,FALSE,TRUE);
    RestoreDC(dc,saved);
    return GetPixel(dc,0,32)==variant.edge && GetPixel(dc,3,32)==variant.light &&
        GetPixel(dc,2,32)==theme->inactive.start && GetPixel(dc,4,32)==client;
}

static BOOL draw_win16_border(HDC dc,const DWORD *pixels,const classic_theme *theme)
{
    const RECT bounds={0,0,240,64};
    const COLORREF client=RGB(19,201,87);
    const style_border_palette palette={theme->frame.face,theme->frame.edge,
        theme->frame.light,theme->frame.shadow,theme->active.start};
    const style_border_presentation fallback=WIN16_FRAME_BORDER;
    const style_border_presentation *border=theme->border_presentation.fill?&theme->border_presentation:&fallback;
    DWORD reference[240*64];
    for(unsigned i=0;i<4;++i){
        BOOL dialog=(i&1u)!=0;
        int unit=i<2?1:2,thickness=(dialog?6:4)*unit;
        if(!style_caption_fill(dc,bounds,client,client,FALSE))return FALSE;
        int saved=SaveDC(dc);
        if(!saved)return FALSE;
        ExcludeClipRect(dc,thickness,thickness,240-thickness,64-thickness);
        win16_control_fill(dc,bounds,dialog?palette.title:palette.face);
        win16_control_outline(dc,bounds,unit,palette.edge);
        RECT inner=bounds;
        InflateRect(&inner,-thickness+unit,-thickness+unit);
        win16_control_outline(dc,inner,unit,dialog?palette.light:palette.edge);
        GdiFlush();
        memcpy(reference,pixels,sizeof(reference));
        RestoreDC(dc,saved);
        if(!style_caption_fill(dc,bounds,client,client,FALSE))return FALSE;
        saved=SaveDC(dc);
        if(!saved)return FALSE;
        ExcludeClipRect(dc,thickness,thickness,240-thickness,64-thickness);
        style_frame_border_paint(dc,bounds,thickness,unit,dialog,NULL,border,&palette);
        GdiFlush();
        RestoreDC(dc,saved);
        if(memcmp(reference,pixels,sizeof(reference)) || GetPixel(dc,120,32)!=client)return FALSE;
    }
    const SIZE corners={22,18};
    style_border_presentation marked=*border;
    marked.divider=STYLE_BORDER_SHADOW;
    int saved=SaveDC(dc);
    if(!saved)return FALSE;
    ExcludeClipRect(dc,4,4,236,60);
    style_frame_border_paint(dc,bounds,4,1,FALSE,&corners,&marked,&palette);
    RestoreDC(dc,saved);
    return GetPixel(dc,21,1)==palette.shadow && GetPixel(dc,1,17)==palette.shadow &&
        GetPixel(dc,24,1)==palette.face && GetPixel(dc,120,32)==client;
}

static BOOL draw_caption_presentation(HDC dc,BOOL win2000)
{
    const RECT band={2,2,202,22};
    const COLORREF background=RGB(255,255,255),line=RGB(50,70,100);
    const UINT flags[WIN2K_CAPTION_PARTS]={0};
    HFONT font=(HFONT)GetStockObject(ANSI_FIXED_FONT);
    const style_caption_presentation variants[]={
        {.text_mode=STYLE_CAPTION_TEXT_LEFT_ELLIPSIS,.separator=STYLE_CAPTION_SEPARATOR_INSIDE},
        {.text_mode=STYLE_CAPTION_TEXT_CENTER_CLIP,.separator=STYLE_CAPTION_SEPARATOR_AFTER}
    };
    int first_ink[ARRAYSIZE(variants)];
    for(unsigned i=0;i<ARRAYSIZE(variants);++i){
        RECT clean={0,0,240,64};
        if(!style_caption_fill(dc,clean,background,background,FALSE))return FALSE;
        if(win2000){
            const win2k_caption_layout layout={.band=band};
            const classic_frame_colors frame={.face=line,.present=TRUE};
            win2k_caption_colors colors={background,background,RGB(0,0,0),FALSE};
            win2k_caption_render_with_colors(dc,&layout,L"Test",NULL,font,96,colors,&frame,
                &variants[i],FALSE,flags,NULL);
        }else{
            const win16_style_layout layout={.caption=band,.title=band,.unit=1};
            style_font_pair fonts={.title=font};
            win16_frame_paint_state state={.fonts=&fonts,.active=TRUE,.presentation=variants[i]};
            win16_style_palette colors={.active=background,.edge=line};
            win16_control_frame_caption(dc,&layout,&colors,&state,L"Test",FALSE,FALSE);
        }
        first_ink[i]=band.right;
        for(int x=band.left;x<first_ink[i];++x)
            for(int y=band.top;y<band.bottom-1;++y)
                if(GetPixel(dc,x,y)!=background){first_ink[i]=x;break;}
        if(GetPixel(dc,200,21)!=(i?background:line) || GetPixel(dc,200,22)!=(i?line:background))
            return FALSE;
    }
    return first_ink[0]<15 && first_ink[1]>65 && first_ink[1]<band.right;
}

static BOOL draw_caption_art(HDC dc,const DWORD *pixels,const classic_theme *theme,const classic_caption_art *art,UINT dpi)
{
    win16_style_layout layout;
    if(!win16_style_measure_with_buttons(240,64,dpi,WS_OVERLAPPEDWINDOW,0,FALSE,
        &theme->title_buttons,&theme->caption_metrics,&theme->border_presentation.geometry,dpi,&layout))return FALSE;

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
    RECT changed=layout.buttons[CLASSIC_BUTTON_MINIMIZE];
    win2k_caption_layout other={.band=layout.caption,.buttons={[WIN2K_MINIMIZE]=changed}};
    classic_frame_colors frame={palette.face,palette.light,palette.shadow,palette.edge,TRUE};
    COLORREF band_color=palette.active;
    win2k_caption_colors colors={band_color,band_color,RGB(0,0,0),FALSE};
    const UINT flags[WIN2K_CAPTION_PARTS]={[WIN2K_MINIMIZE]=DFCS_PUSHED};
    const unsigned sources[]={STYLE_CAPTION_BUTTON_INDEXED,STYLE_CAPTION_BUTTON_SYSTEM};
    DWORD signature[ARRAYSIZE(sources)]={0};
    for(unsigned i=0;i<ARRAYSIZE(sources);++i){
        style_caption_presentation paint={STYLE_CAPTION_TEXT_LEFT_ELLIPSIS,
            STYLE_CAPTION_SEPARATOR_NONE,sources[i],STYLE_CAPTION_BUTTON_NO_OUTLINE};
        win16_frame_paint_state state={.active=TRUE,.button={HTMINBUTTON,TRUE},.art=art,.presentation=paint};
        win16_control_frame_caption(dc,&layout,&palette,&state,NULL,FALSE,FALSE);
        GdiFlush();memcpy(reference,pixels,sizeof(reference));
        win2k_caption_render_with_colors(dc,&other,L"",NULL,GetStockObject(DEFAULT_GUI_FONT),
            dpi,colors,&frame,&paint,FALSE,flags,art);
        GdiFlush();
        for(LONG y=changed.top;y<changed.bottom;++y)
            for(LONG x=changed.left;x<changed.right;++x){
                unsigned index=(unsigned)y*240+(unsigned)x;
                if(pixels[index]!=reference[index])return FALSE;
                signature[i]=signature[i]*31u+pixels[index];
            }
    }
    return signature[0]!=signature[1];
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
    const classic_choice_colors *choice,HMODULE module,style_push_profile profile)
{
    style_instance instance={0};
    classic_choice_art art={0};
    if(win2000)return win2k_button_paint(window,&instance,dc,push,choice,NULL,module,&art,
        (classic_control_presentation){.push=profile});
    win16_style_palette palette;
    win16_style_default_palette(&palette);
    style_control_state control={0};
    const win16_control_choice_view view={&instance,&control,&palette,push,choice,button_type,
        button_fit,button_fit,button_fit,button_fit,button_fit,module,&art,NULL,NULL,NULL,{.push=profile}};
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
    return paint_native_button(dc,window,win2000,&theme->push,&theme->choice,NULL,theme->controls.push) &&
        GetPixel(dc,40,14)==theme->push.state[expected].face;
}

static BOOL draw_mixed_button(HDC dc,HWND window,const classic_theme *theme,BOOL win2000)
{
    classic_theme mixed=*theme;
    const COLORREF edge=RGB(25,46,70),face=RGB(133,155,177);
    mixed.push.state[CLASSIC_PUSH_NORMAL]=(classic_surface_colors){face,edge};
    mixed.controls.push=STYLE_PUSH_STEPPED;
    if(!draw_button(dc,window,&mixed,win2000,FALSE,FALSE,FALSE,CLASSIC_PUSH_NORMAL) ||
       GetPixel(dc,0,10)!=edge)return FALSE;
    mixed.controls.push=STYLE_PUSH_RELIEF;
    return draw_button(dc,window,&mixed,win2000,FALSE,FALSE,FALSE,CLASSIC_PUSH_NORMAL) &&
        GetPixel(dc,0,10)==RGB(255,255,255) && GetPixel(dc,79,10)==edge;
}

static BOOL draw_control_lines(HDC dc,const classic_theme *theme,BOOL win2000)
{
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,120,64,NULL,NULL,NULL,NULL);
    if(!parent)return FALSE;
    HWND group=CreateWindowW(L"BUTTON",L"",WS_CHILD|BS_GROUPBOX,0,0,80,28,parent,NULL,NULL,NULL);
    HWND edit=CreateWindowExW(WS_EX_CLIENTEDGE,L"EDIT",L"",WS_CHILD,0,28,80,28,parent,NULL,NULL,NULL);
    RECT before,after;
    BOOL passed=group && edit && GetClientRect(edit,&before);
    style_instance instance={0};
    win16_style_palette palette;
    win16_style_default_palette(&palette);
    style_control_state native={0};
    win2k_palette relief=win2k_standard_palette;
    for(style_line_profile profile=STYLE_LINE_FLAT;passed && profile<=STYLE_LINE_RELIEF;++profile){
        classic_control_presentation controls=theme->controls;
        controls.group=controls.edit=profile;
        if(win2000){
            passed=win2k_button_paint(group,&instance,dc,NULL,NULL,&theme->group,NULL,NULL,controls);
        }else{
            win16_control_choice_view view={&instance,&native,&palette,NULL,NULL,button_type,
                button_fit,button_fit,button_fit,button_fit,button_fit,NULL,NULL,NULL,&theme->group,NULL,controls};
            win16_control_draw_choice(group,&view,dc);
        }
        passed=passed && GetPixel(dc,10,27)==(profile==STYLE_LINE_FLAT?theme->group.shadow:RGB(255,255,255));
        if(win2000)passed=passed && win2k_edit_border(edit,dc,&relief,profile);
        else{
            control_style state={.palette=palette,.controls=controls,.edit_border=TRUE};
            memcpy(state.edit_colors,theme->edit,sizeof(state.edit_colors));
            win16_control_edit_border(edit,&state,dc);
        }
        passed=passed && GetPixel(dc,0,10)==(profile==STYLE_LINE_FLAT?RGB(0,0,0):relief.shadow) &&
            GetClientRect(edit,&after) && EqualRect(&before,&after);
    }
    DestroyWindow(parent);
    return passed;
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
               !paint_native_button(dc,window,win2000,NULL,NULL,NULL,STYLE_PUSH_DEFAULT))return FALSE;
            GdiFlush();
            memcpy(reference,pixels,sizeof(reference));
            if(!paint_native_button(dc,window,win2000,NULL,&theme->choice,module,STYLE_PUSH_DEFAULT))return FALSE;
            GdiFlush();
            if(memcmp(reference,pixels,sizeof(reference)))return FALSE;
            classic_choice_colors altered=theme->choice;
            classic_choice_surface *entry=(radio?altered.radio:altered.check)+4*check+mode;
            if(!entry->fields)return FALSE;
            entry->colors.face=RGB(89,37,102);
            entry->colors.edge=RGB(17,165,123);
            if(!paint_native_button(dc,window,win2000,NULL,&altered,module,STYLE_PUSH_DEFAULT))return FALSE;
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

static BOOL draw_mixed_choice_art(HDC dc,HWND window,const classic_theme *theme,HMODULE module,BOOL radio)
{
    classic_choice_art art={0};
    const classic_choice_surface *entry=classic_choice_entry(&theme->choice,radio,BST_CHECKED,FALSE,0);
    const classic_surface_colors *colors=classic_choice_color(entry);
    if(!colors || !classic_choice_art_get(module,entry->image,&art))return FALSE;
    if(!paint_native_button(dc,window,TRUE,NULL,&theme->choice,module,STYLE_PUSH_DEFAULT))return FALSE;
    const int side=13,top=(28-side)/2;
    for(int y=0;y<CLASSIC_CHOICE_HEIGHT;++y){
        for(int x=0;x<CLASSIC_CHOICE_WIDTH;++x){
            int left=MulDiv(x,side,CLASSIC_CHOICE_WIDTH);
            int right=MulDiv(x+1,side,CLASSIC_CHOICE_WIDTH);
            COLORREF expected=art.cell[y*CLASSIC_CHOICE_WIDTH+x]=='0'?colors->edge:colors->face;
            if(left<right && GetPixel(dc,left,top+y)!=expected)return FALSE;
        }
    }
    return TRUE;
}

static BOOL mixed_choice_art(HDC dc,const classic_theme *theme,HMODULE module)
{
    HWND parent=CreateWindowExW(0,L"STATIC",L"",WS_OVERLAPPED,0,0,120,64,NULL,NULL,NULL,NULL);
    if(!parent)return FALSE;
    BOOL passed=TRUE;
    for(unsigned radio=0;passed && radio<2;++radio){
        HWND control=CreateWindowExW(0,L"BUTTON",L"",WS_CHILD|(radio?BS_AUTORADIOBUTTON:BS_AUTO3STATE),
            0,0,80,28,parent,NULL,NULL,NULL);
        if(!control){passed=FALSE;break;}
        SendMessageW(control,BM_SETCHECK,BST_CHECKED,0);
        passed=draw_mixed_choice_art(dc,control,theme,module,radio);
        DestroyWindow(control);
    }
    DestroyWindow(parent);
    return passed;
}

static BOOL draw_separator(HDC dc,const classic_theme *theme,BOOL relief)
{
    HWND control=CreateWindowW(L"STATIC",L"",WS_CHILD|SS_ETCHEDFRAME,0,0,32,32,HWND_MESSAGE,NULL,NULL,NULL);
    if(!control)return FALSE;
    style_instance instance={0};
    win2k_palette palette=win2k_standard_palette;
    palette.dark=RGB(31,48,65);palette.shadow=RGB(82,99,116);palette.highlight=RGB(133,150,167);
    COLORREF untouched=RGB(219,220,221);
    RECT bounds={0,0,32,32};
    int unit=max(1,MulDiv(1,(int)GetDpiForWindow(control),96));
    BOOL passed=style_paint_color(dc,&bounds,untouched) &&
        win2k_static_paint(control,&instance,dc,&palette,theme->controls.separator) &&
        GetPixel(dc,0,5)==(relief?palette.shadow:palette.dark) &&
        GetPixel(dc,relief?1:unit,5)==(relief?palette.highlight:untouched) &&
        GetPixel(dc,5,5)==untouched;
    DestroyWindow(control);
    return passed;
}

static BOOL draw_combo_surface(HDC dc,const classic_theme *theme,BOOL win2000)
{
    HWND parent=CreateWindowW(L"STATIC",L"",WS_POPUP,0,0,160,80,NULL,NULL,NULL,NULL);
    if(!parent)return FALSE;
    HWND combo=CreateWindowW(L"COMBOBOX",L"",WS_CHILD|CBS_DROPDOWNLIST,0,0,120,70,parent,NULL,NULL,NULL);
    COMBOBOXINFO info={.cbSize=sizeof(info)};
    RECT bounds={0,0,120,64};
    COLORREF background=RGB(37,79,117);
    BOOL passed=combo && GetComboBoxInfo(combo,&info) && style_paint_color(dc,&bounds,RGB(97,33,51));
    if(passed && win2000){
        win2k_palette palette=win2k_standard_palette;
        palette.window=background;
        style_instance instance={0};
        passed=win2k_combo_paint(combo,&instance,dc,&palette,&theme->combo);
    }else if(passed){
        control_style state={.combo_colors=theme->combo,.list_colors=theme->list,.label_colors=theme->label};
        win16_style_default_palette(&state.palette);
        state.list_colors.background=background;
        win16_control_draw_combo(combo,&state,dc,win2k_combo_info,button_fit,button_fit);
    }
    if(passed)passed=GetPixel(dc,info.rcItem.right-4,(info.rcItem.top+info.rcItem.bottom)/2)==background;
    DestroyWindow(parent);
    return passed;
}

static BOOL draw_scroll(HDC dc,const classic_theme *theme,UINT dpi,unsigned state)
{
    RECT bar={0,0,MulDiv(17,(int)dpi,96),60};
    int nominal=classic_scroll_nominal(theme->scroll_metrics,CLASSIC_SCROLL_EXTENT_DEFAULT,&bar,TRUE,dpi);
    style_scroll_layout geometry;
    const SCROLLINFO info={.nMin=0,.nMax=100,.nPage=5,.nPos=30};
    if(!PatBlt(dc,0,0,bar.right,60,WHITENESS) ||
        !style_scroll_measure(&bar,TRUE,nominal,max(1,MulDiv(1,(int)dpi,96)),&info,theme->scroll_metrics.thumb,&geometry))return FALSE;
    POINT p={bar.right/2,geometry.first.bottom/2};
    if(style_scroll_hit(&geometry,p,state==2?ESB_DISABLE_UP:0)!=
       (state==2?STYLE_SCROLL_NONE:STYLE_SCROLL_LINE_BEFORE))return FALSE;
    win2k_palette palette=win2k_standard_palette;
    palette.face=theme->scroll_arrow.face;
    palette.dark=theme->scroll_arrow.edge;
    const UINT flags[]={0,DFCS_PUSHED,DFCS_INACTIVE};
    if(!style_paint_scroll_arrow(dc,&geometry.first,DFCS_SCROLLUP|flags[state],&palette,&theme->scroll_art))return FALSE;
    for(LONG y=geometry.first.top;y<geometry.first.bottom;++y)
        for(LONG x=geometry.first.left;x<geometry.first.right;++x)
            if(GetPixel(dc,x,y)==theme->scroll_arrow.face)return TRUE;
    return FALSE;
}

static BOOL draw_indexed_arrows(HDC dc,const classic_theme *theme)
{
    RECT cell={0,0,STYLE_SCROLL_ARROW_SIDE,STYLE_SCROLL_ARROW_SIDE};
    const style_relief_palette palette={.dark=RGB(19,37,63),.highlight=RGB(213,225,237),
        .shadow=RGB(111,129,147),.face=RGB(177,159,141)};
    const COLORREF roles[]={palette.dark,palette.highlight,palette.shadow,palette.face};
    const UINT flags[]={0,DFCS_PUSHED,DFCS_INACTIVE};
    for(unsigned state=0;state<ARRAYSIZE(flags);++state){
        for(unsigned direction=0;direction<STYLE_SCROLL_ARROW_DIRECTIONS;++direction){
            unsigned asset=state*STYLE_SCROLL_ARROW_DIRECTIONS+direction;
            const char (*original)[18]=win16_control_scroll_glyph((enum win16_control_scroll_asset)asset);
            if(!original || !style_paint_scroll_arrow(dc,&cell,direction|flags[state],&palette,&theme->scroll_art))
                return FALSE;
            for(int y=0;y<STYLE_SCROLL_ARROW_SIDE;++y)for(int x=0;x<STYLE_SCROLL_ARROW_SIDE;++x)
                if(GetPixel(dc,x,y)!=roles[original[y][x]-'0'])return FALSE;
        }
    }
    return TRUE;
}

static int first_ink(HDC dc,int left,int right,int height,COLORREF background)
{
    for(int x=left;x<right;++x)
        for(int y=0;y<height;++y)
            if(GetPixel(dc,x,y)!=background)return x;
    return -1;
}

static BOOL draw_popup(HDC dc,const DWORD *pixels,const classic_theme *theme,const style_menu_art *art,
    UINT dpi,HFONT font)
{
    WCHAR text[]=L"MMMM\tF10";
    style_menu_item item={.text=text,.submenu=TRUE};
    win16_style_palette palette;
    win16_style_default_palette(&palette);
    const classic_menu_metrics variant={22,12,16,6,0};
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
        style_menu_art blank=*art;
        memset(blank.check,'1',sizeof(blank.check));
        memset(blank.arrow,'1',sizeof(blank.arrow));
        if(!win16_style_menu_draw(&item,&draw,font,dpi,&palette,&theme->menu_popup,metrics[i],&blank) ||
           first_ink(dc,0,label_x,(int)heights[i],background)>=0 ||
           first_ink(dc,arrow_x,width,(int)heights[i],background)>=0)return FALSE;
    }
    classic_menu_metrics configured=theme->menu_metrics;
    RECT bounds={0,0,win16_style_menu_columns_width(&columns,dpi,FALSE,&configured)+MulDiv(16,(int)dpi,96),
        columns.height};
    int shortcut_left=MulDiv(configured.check_column,(int)dpi,96)+columns.label+
        MulDiv(configured.shortcut_gap,(int)dpi,96);
    COLORREF background=theme->menu_popup.background;
    if(!win16_style_menu_text_columns(dc,&bounds,font,dpi,&palette,text,0,WIN16_STYLE_MENU_ROW,
        FALSE,&columns,&theme->menu_popup,&configured,art,NULL))return FALSE;
    int left=first_ink(dc,shortcut_left,bounds.right,columns.height,background);
    configured.text_flags|=STYLE_MENU_TEXT_SHORTCUT_RIGHT;
    if(!win16_style_menu_text_columns(dc,&bounds,font,dpi,&palette,text,0,WIN16_STYLE_MENU_ROW,
        FALSE,&columns,&theme->menu_popup,&configured,art,NULL))return FALSE;
    int right=first_ink(dc,shortcut_left,bounds.right,columns.height,background);
    if(left<shortcut_left || right-left<MulDiv(16,(int)dpi,96))return FALSE;
    if(!win16_style_menu_text_columns(dc,&bounds,font,dpi,&palette,text,ODS_DISABLED,WIN16_STYLE_MENU_ROW,
        FALSE,&columns,&theme->menu_popup,&configured,art,NULL))return FALSE;
    DWORD stippled[240*64];
    GdiFlush();
    memcpy(stippled,pixels,sizeof(stippled));
    configured.text_flags&=~STYLE_MENU_TEXT_STIPPLE_DISABLED;
    if(!win16_style_menu_text_columns(dc,&bounds,font,dpi,&palette,text,ODS_DISABLED,WIN16_STYLE_MENU_ROW,
        FALSE,&columns,&theme->menu_popup,&configured,art,NULL))return FALSE;
    GdiFlush();
    if(!memcmp(stippled,pixels,sizeof(stippled)))return FALSE;
    int step=MulDiv(4,(int)dpi,96);
    return widths[1]-widths[0]==(unsigned)(3*step) && heights[1]==heights[0] &&
        label[1]-label[0]==step && shortcut[1]-shortcut[0]==2*step && arrow[1]-arrow[0]==3*step;
}

static BOOL draw_menu_text_options(HDC dc,const DWORD *pixels,const classic_theme *theme,HFONT font,UINT dpi)
{
    static const WCHAR text[]=L"&Menu";
    RECT area={0,0,200,40};
    classic_menu_metrics metrics=theme->menu_metrics;
    win16_style_palette palette;
    win16_style_default_palette(&palette);
    const classic_menu_colors *colors=&theme->menu_bar;
    if(!win16_style_menu_text_columns(dc,&area,font,dpi,&palette,text,0,WIN16_STYLE_MENU_BAR,
        FALSE,NULL,colors,&metrics,NULL,NULL))return FALSE;
    int left=first_ink(dc,0,area.right,area.bottom,colors->background);
    metrics.text_flags|=STYLE_MENU_TEXT_CENTER_BAR;
    if(!win16_style_menu_text_columns(dc,&area,font,dpi,&palette,text,0,WIN16_STYLE_MENU_BAR,
        FALSE,NULL,colors,&metrics,NULL,NULL))return FALSE;
    if(left<0 || first_ink(dc,0,area.right,area.bottom,colors->background)<=left)return FALSE;
    if(!win16_style_menu_text_columns(dc,&area,font,dpi,&palette,text,ODS_DEFAULT,WIN16_STYLE_MENU_ROW,
        FALSE,NULL,colors,&metrics,NULL,NULL))return FALSE;
    DWORD normal[240*64];
    GdiFlush();
    memcpy(normal,pixels,sizeof(normal));
    metrics.text_flags|=STYLE_MENU_TEXT_BOLD_DEFAULT;
    if(!win16_style_menu_text_columns(dc,&area,font,dpi,&palette,text,ODS_DEFAULT,WIN16_STYLE_MENU_ROW,
        FALSE,NULL,colors,&metrics,NULL,NULL))return FALSE;
    GdiFlush();
    if(!memcmp(normal,pixels,sizeof(normal)))return FALSE;
    if(first_ink(dc,0,16,area.bottom,colors->background)>=0)return FALSE;
    metrics.text_flags|=STYLE_MENU_COMMAND_GLYPHS;
    return win16_style_menu_text_columns(dc,&area,font,dpi,&palette,text,ODS_DEFAULT,WIN16_STYLE_MENU_ROW,
        FALSE,NULL,colors,&metrics,NULL,HBMMENU_POPUP_CLOSE) &&
        first_ink(dc,0,16,area.bottom,colors->background)>=0;
}

static BOOL draw_themed_popup(HDC dc,const DWORD *pixels,const classic_theme *theme,
    const style_menu_art *art,UINT dpi)
{
    msstyles_font alternate=theme->menu_font;
    alternate.points=24;
    HFONT font=win16_style_menu_font_acquire(dpi,&theme->menu_font);
    HFONT legacy=win16_style_menu_font_acquire(dpi,NULL);
    HFONT smaller=win16_style_menu_font_acquire(dpi,&alternate);
    alternate.points=9;
    alternate.logical.lfWeight=FW_NORMAL;
    alternate.logical.lfItalic=TRUE;
    lstrcpyW(alternate.logical.lfFaceName,L"Arial");
    HFONT logical=win16_style_menu_font_acquire(dpi,&alternate);
    BOOL ok=font && legacy==font && smaller && smaller!=font && logical && logical!=font;
    if(ok){
        LOGFONTW before={0},after={0},requested={0};
        ok=GetObjectW(font,sizeof(before),&before)==sizeof(before) &&
            GetObjectW(smaller,sizeof(after),&after)==sizeof(after) &&
            GetObjectW(logical,sizeof(requested),&requested)==sizeof(requested) &&
            requested.lfHeight==-MulDiv(9,(int)dpi,72) && requested.lfWeight==FW_NORMAL &&
            requested.lfItalic && !wcscmp(requested.lfFaceName,L"Arial") &&
            after.lfHeight>before.lfHeight && draw_popup(dc,pixels,theme,art,dpi,font) &&
            draw_menu_text_options(dc,pixels,theme,logical,dpi);
        if(!ok)fprintf(stderr,"Win16 popup DPI %u base height %ld alternate %ld\n",
            dpi,(long)before.lfHeight,(long)after.lfHeight);
    }
    win16_style_system_release(logical);
    LOGFONTW released;
    if(logical && GetObjectW(logical,sizeof(released),&released)){
        fprintf(stderr,"Menu font lease remains live after release at DPI %u\n",dpi);
        ok=FALSE;
    }
    win16_style_system_release(smaller);
    win16_style_system_release(legacy);
    win16_style_system_release(font);
    return ok;
}

static BOOL render(const classic_theme *theme,BOOL win2000,const classic_caption_art *art,const style_menu_art *menu_art,HMODULE module)
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
        draw_caption_presentation(dc,win2000) &&
        (win2000?draw_win2k_border(dc,bits,theme):draw_win16_border(dc,bits,theme)) &&
        (win2000 || (draw_caption_art(dc,bits,theme,art,96) && draw_caption_art(dc,bits,theme,art,144))) &&
        window && draw_button(dc,window,&altered,win2000,FALSE,FALSE,FALSE,CLASSIC_PUSH_NORMAL) &&
        draw_button(dc,window,&altered,win2000,FALSE,TRUE,FALSE,CLASSIC_PUSH_PRESSED) &&
        draw_button(dc,window,&altered,win2000,TRUE,FALSE,TRUE,CLASSIC_PUSH_DISABLED) &&
        draw_button(dc,window,&altered,win2000,FALSE,FALSE,TRUE,CLASSIC_PUSH_DEFAULTED) &&
        draw_button(dc,window,&altered,win2000,FALSE,TRUE,TRUE,CLASSIC_PUSH_PRESSED) &&
        draw_mixed_button(dc,window,theme,win2000) &&
        draw_control_lines(dc,theme,win2000) &&
        draw_choices(dc,bits,theme,win2000,module) && draw_separator(dc,theme,win2000) &&
        draw_combo_surface(dc,theme,win2000) &&
        (win2000 || mixed_choice_art(dc,theme,module));
    for(unsigned state=0;passed && state<3;++state)
        passed=draw_scroll(dc,theme,96,state) && draw_scroll(dc,theme,144,state);
    if(passed && !win2000)passed=draw_indexed_arrows(dc,theme);
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
    CHECK(theme.scroll_art.source==(gradient?STYLE_SCROLL_ARROW_SYMBOL:STYLE_SCROLL_ARROW_INDEXED));
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
    classic_caption_art art={0};
    style_menu_art menu_art={0};
    HMODULE module=source.resources;
    if(!gradient){
        if(!module)module=LoadLibraryExW(path,NULL,LOAD_LIBRARY_AS_DATAFILE|LOAD_LIBRARY_AS_IMAGE_RESOURCE);
        CHECK(module);
        CHECK(SUCCEEDED(classic_caption_art_read(module,&theme.caption_images,&art)) && art.available);
        classic_caption_images wrong=theme.caption_images;
        wrong.id[CLASSIC_IMAGE_SYSTEM][0]=wrong.id[CLASSIC_IMAGE_MINIMIZE][0];
        classic_caption_art prior_art=art;
        CHECK(FAILED(classic_caption_art_read(module,&wrong,&art)) && !memcmp(&prior_art,&art,sizeof(art)));
        CHECK(SUCCEEDED(style_menu_art_read(module,theme.menu_images,&menu_art)) && menu_art.available);
        style_menu_art prior_menu=menu_art;
        WORD wrong_menu[]={theme.menu_images[1],theme.menu_images[0]};
        CHECK(FAILED(style_menu_art_read(module,wrong_menu,&menu_art)) &&
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

static BOOL print_folder(PCWSTR folder,PCWSTR display_name,classic_theme_family family,void *context)
{
    (void)context;wprintf(L"CLASSIC THEME FOLDER %ls family=%u display=%ls\n",folder,(unsigned)family,display_name);
    return TRUE;
}

static int check_identity(PCWSTR folder,classic_theme_family selected)
{
    HMODULE module=GetModuleHandleW(NULL);
    WCHAR actual[MAX_PATH];
    unsigned id=classic_theme_folder_id(folder);
    CHECK(id>=CLASSIC_THEME_PROFILE_FIRST_CUSTOM && id<CLASSIC_THEME_PROFILE_CAP);
    CHECK(SUCCEEDED(classic_theme_profile_folder(module,id,actual)) && !_wcsicmp(folder,actual));
    classic_theme_mapping source={0};
    CHECK(SUCCEEDED(classic_theme_open_profile(module,id,&source)) && source.family==selected);
    classic_theme_close(&source);
    classic_theme_set_process_profile(id,selected);
    CHECK(SUCCEEDED(classic_theme_open_packaged(module,selected,FALSE,&source)) && source.family==selected);
    classic_theme_close(&source);
    printf("CLASSIC THEME ID id=%u adapter=%u\n",id,(unsigned)selected);
    return 0;
}

int wmain(int argc,WCHAR **argv)
{
    if(argc==2 && !wcscmp(argv[1],L"--discover")){
        classic_theme_visit_folders(GetModuleHandleW(NULL),print_folder,NULL);
        return 0;
    }
    if(argc==4 && !wcscmp(argv[1],L"--identity")){
        if(!wcscmp(argv[3],L"0"))return check_identity(argv[2],CLASSIC_THEME_WIN16);
        if(!wcscmp(argv[3],L"1"))return check_identity(argv[2],CLASSIC_THEME_WIN2K);
        return 2;
    }
    BOOL recolored=argc==3 && !wcscmp(argv[2],L"--recolor");
    if(argc!=2 && !recolored)return 2;
    return check(argv[1],L"Win16",FALSE,recolored) || check(argv[1],L"Win2K",TRUE,recolored);
}
