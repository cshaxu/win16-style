#include "v3_frame.h"
#include <stdlib.h>
#include <wchar.h>

static BOOL is(msstyles_v3_text text,PCWSTR name)
{return text.length==wcslen(name) && !_wcsnicmp(text.data,name,text.length);}

static BOOL vector(const msstyles_v3_theme *theme,PCWSTR section,PCWSTR key,int *out,unsigned count,int minimum)
{
    WCHAR input[96];
    msstyles_v3_text value;
    if(!msstyles_v3_property(theme,section,key,&value) || !value.length || value.length>=ARRAYSIZE(input))
        return FALSE;
    wmemcpy(input,value.data,value.length);
    input[value.length]=0;
    WCHAR *start=input;
    for(unsigned i=0;i<count;++i){
        while(*start==L' ' || *start==L'\t' || *start==L',')++start;
        WCHAR *end;
        long n=wcstol(start,&end,10);
        if(end==start || n<minimum || n>8192)return FALSE;
        if(i+1<count && *end!=L' ' && *end!=L'\t' && *end!=L',')return FALSE;
        out[i]=(int)n;
        start=end;
    }
    return !*start;
}

static BOOL numbers(const msstyles_v3_theme *theme,PCWSTR section,PCWSTR key,int *out,unsigned count)
{return vector(theme,section,key,out,count,0);}

static BOOL color(const msstyles_v3_theme *theme,PCWSTR section,PCWSTR key,COLORREF *out)
{
    int rgb[3];
    if(!numbers(theme,section,key,rgb,3) || rgb[0]>255 || rgb[1]>255 || rgb[2]>255)return FALSE;
    *out=RGB(rgb[0],rgb[1],rgb[2]);return TRUE;
}

static BOOL image(const msstyles_v3_theme *theme,PCWSTR section,PCWSTR key,unsigned count,
    BOOL horizontal,style_dib_strip *out)
{
    msstyles_v3_text path;
    const void *data;
    DWORD bytes;
    return msstyles_v3_property(theme,section,key,&path) &&
        msstyles_v3_bitmap(theme,path,&data,&bytes) &&
        style_dib_strip_read(data,bytes,count,horizontal,out);
}

static BOOL image_true(const msstyles_v3_theme *theme,PCWSTR section,PCWSTR key)
{
    msstyles_v3_text value;
    return msstyles_v3_property(theme,section,key,&value) && is(value,L"True");
}

static BOOL strip(const msstyles_v3_theme *theme,PCWSTR section,style_dib_strip *out)
{
    int rows=0,margins[4];
    msstyles_v3_text layout;
    if(!numbers(theme,section,L"ImageCount",&rows,1) || rows<1 || rows>32)return FALSE;
    BOOL horizontal=msstyles_v3_property(theme,section,L"ImageLayout",&layout) && is(layout,L"Horizontal");
    if(!image(theme,section,L"ImageFile",(unsigned)rows,horizontal,out))return FALSE;
    if(numbers(theme,section,L"SizingMargins",margins,4)){
        if(margins[0]+margins[1]>(int)out->width || margins[2]+margins[3]>(int)out->height)return FALSE;
        out->margins=(RECT){margins[0],margins[2],margins[1],margins[3]};
    }
    out->transparent=image_true(theme,section,L"Transparent");
    out->color_key=RGB(255,0,255);
    (void)color(theme,section,L"TransparentColor",&out->color_key);
    return TRUE;
}

BOOL msstyles_v3_push_image(const msstyles_v3_theme *theme,style_push_sprite *sprite)
{
    static const WCHAR section[]=L"Button.PushButton";
    style_push_sprite value={0};
    int m[4];
    if(!strip(theme,section,&value.image) || value.image.count<5 ||
       !numbers(theme,section,L"ContentMargins",m,4))return FALSE;
    value.margins=(RECT){m[0],m[2],m[1],m[3]};
    *sprite=value;return TRUE;
}

static BOOL button(const msstyles_v3_theme *theme,PCWSTR section,style_frame_sprite_button *button)
{
    if(!strip(theme,section,&button->face) || button->face.count<4)return FALSE;
    msstyles_v3_text anchor;
    int offset[2];
    if(msstyles_v3_property(theme,section,L"OffsetType",&anchor) && is(anchor,L"TopRight") &&
       vector(theme,section,L"Offset",offset,2,-255) &&
       offset[0]<=-(int)button->face.width && offset[1]>=0){
        button->top_right=(POINT){offset[0],offset[1]};
        button->anchored=TRUE;
    }
    int m[4];
    if(numbers(theme,section,L"ContentMargins",m,4))
        button->content=(RECT){m[0],m[2],m[1],m[3]};
    for(unsigned i=0;i<ARRAYSIZE(button->glyphs);++i){
        WCHAR key[]=L"ImageFile1";
        key[ARRAYSIZE(key)-2]+=(WCHAR)i;
        style_dib_strip *glyph=&button->glyphs[i];
        if(!image(theme,section,key,button->face.count,button->face.horizontal,glyph))continue;
        glyph->transparent=image_true(theme,section,L"GlyphTransparent");
        glyph->color_key=RGB(255,0,255);
        (void)color(theme,section,L"GlyphTransparentColor",&glyph->color_key);
    }
    return TRUE;
}

BOOL msstyles_v3_frame_images(const msstyles_v3_theme *theme,style_frame_sprites *sprites)
{
    style_frame_sprites images={0};
    if(!strip(theme,L"Window.Caption",&images.caption) || images.caption.count<2 ||
       !strip(theme,L"Window.FrameLeft",&images.left) || images.left.count<2 ||
       !strip(theme,L"Window.FrameRight",&images.right) || images.right.count<2 ||
       !strip(theme,L"Window.FrameBottom",&images.bottom) || images.bottom.count<2 ||
       !button(theme,L"Window.MinButton",&images.buttons[CLASSIC_BUTTON_MINIMIZE]) ||
       !button(theme,L"Window.MaxButton",&images.buttons[CLASSIC_BUTTON_MAXIMIZE]) ||
       !button(theme,L"Window.CloseButton",&images.buttons[CLASSIC_BUTTON_CLOSE]) ||
       !button(theme,L"Window.RestoreButton",&images.restore))return FALSE;
    if(!strip(theme,L"Window.MaxCaption",&images.maximized) || images.maximized.count<2)
        images.maximized=images.caption;
    if(!button(theme,L"Window.HelpButton",&images.buttons[CLASSIC_BUTTON_HELP]))
        images.buttons[CLASSIC_BUTTON_HELP]=(style_frame_sprite_button){0};
    *sprites=images;
    return TRUE;
}

static void font(const msstyles_v3_theme *theme,PCWSTR key,msstyles_font *out)
{
    msstyles_v3_text value;
    WCHAR input[96];
    if(!msstyles_v3_property(theme,L"SysMetrics",key,&value) || value.length>=ARRAYSIZE(input))return;
    wmemcpy(input,value.data,value.length);input[value.length]=0;
    WCHAR *comma=wcschr(input,L',');
    if(!comma || comma==input || (size_t)(comma-input)>=LF_FACESIZE)return;
    *comma=0;
    WCHAR *end;
    long points=wcstol(comma+1,&end,10);
    if(points<=0 || points>48)return;
    msstyles_font parsed={.points=(unsigned)points};
    parsed.logical.lfWeight=wcsstr(end,L"bold")?FW_BOLD:FW_NORMAL;
    parsed.logical.lfItalic=wcsstr(end,L"italic")!=NULL;
    wcsncpy(parsed.logical.lfFaceName,input,ARRAYSIZE(parsed.logical.lfFaceName)-1);
    *out=parsed;
}

BOOL msstyles_v3_frame_model(const msstyles_v3_theme *theme,classic_theme *model)
{
    style_frame_sprites images;
    if(!msstyles_v3_frame_images(theme,&images))return FALSE;
    classic_theme decoded={0};
    classic_frame_colors *frame=&decoded.frame;
    int caption,narrow=0;
    if(!numbers(theme,L"SysMetrics",L"CaptionBarHeight",&caption,1) || caption<1 || caption>255 ||
       images.left.width>255 || !color(theme,L"SysMetrics",L"BtnFace",&frame->face) ||
       !color(theme,L"SysMetrics",L"ActiveCaption",&decoded.active.start) ||
       !color(theme,L"SysMetrics",L"InactiveCaption",&decoded.inactive.start) ||
       !color(theme,L"SysMetrics",L"CaptionText",&decoded.active.text) ||
       !color(theme,L"SysMetrics",L"InactiveCaptionText",&decoded.inactive.text))return FALSE;
    decoded.active.end=decoded.active.start;decoded.inactive.end=decoded.inactive.start;
    frame->present=TRUE;
    frame->light=frame->shadow=frame->edge=frame->face;
    (void)color(theme,L"Globals",L"EdgeHighLightColor",&frame->light);
    (void)color(theme,L"Globals",L"EdgeShadowColor",&frame->shadow);
    (void)color(theme,L"Globals",L"EdgeDkShadowColor",&frame->edge);
    decoded.dialog_policy=(classic_dialog_policy){.window_provider=CLASSIC_WINDOW_PROVIDER_NONCLIENT,
        .color=CLASSIC_STOCK_NATIVE,.font=CLASSIC_STOCK_NATIVE,.find=CLASSIC_STOCK_NATIVE};
    decoded.border_presentation.geometry=(style_frame_geometry){.scale=STYLE_FRAME_LOGICAL,
        .sizeable=(uint8_t)images.left.width,.fixed=(uint8_t)images.left.width,
        .dialog=(uint8_t)images.left.width};
    decoded.caption_presentation=(style_caption_presentation){.text_mode=STYLE_CAPTION_TEXT_LEFT_ELLIPSIS,
        .icon_mode=STYLE_CAPTION_ICON_NATIVE};
    const style_dib_strip *close=&images.buttons[CLASSIC_BUTTON_CLOSE].face;
    (void)numbers(theme,L"SysMetrics",L"SmCaptionBarHeight",&narrow,1);
    decoded.caption_metrics=(classic_caption_metrics){caption,narrow,
        {(LONG)close->width,(LONG)close->height},{0,0}};
    int close_offset[2],max_offset[2],min_offset[2];
    if(!vector(theme,L"Window.CloseButton",L"Offset",close_offset,2,-255) ||
       !vector(theme,L"Window.MaxButton",L"Offset",max_offset,2,-255) ||
       !vector(theme,L"Window.MinButton",L"Offset",min_offset,2,-255))return FALSE;
    int right=-close_offset[0]-(int)close->width;
    int close_gap=max(0,close_offset[0]-max_offset[0]-(int)close->width);
    int button_gap=max(0,max_offset[0]-min_offset[0]-(int)close->width);
    if(right<0 || right>255 || close_gap>255 || button_gap>255)return FALSE;
    decoded.title_buttons=(classic_button_group){.count=4,.right_margin=(uint16_t)right,
        .slots={{CLASSIC_BUTTON_MINIMIZE,CLASSIC_BUTTON_RIGHT,0},
            {CLASSIC_BUTTON_MAXIMIZE,CLASSIC_BUTTON_RIGHT,(uint16_t)button_gap},
            {CLASSIC_BUTTON_HELP,CLASSIC_BUTTON_RIGHT,(uint16_t)button_gap},
            {CLASSIC_BUTTON_CLOSE,CLASSIC_BUTTON_RIGHT,(uint16_t)close_gap}},
        .layout=CLASSIC_BUTTON_LAYOUT_EXPLICIT|CLASSIC_BUTTON_LAYOUT_CLIP};
    font(theme,L"CaptionFont",&decoded.caption_font);
    font(theme,L"SmCaptionFont",&decoded.small_caption_font);
    font(theme,L"MenuFont",&decoded.menu_font);
    *model=decoded;
    return TRUE;
}