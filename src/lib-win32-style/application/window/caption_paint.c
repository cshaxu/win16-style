#include "caption_paint.h"
#include "../graphics/bitmap.h"

style_caption_raster style_caption_button_raster(const classic_caption_art *art,
    enum classic_button_role role,BOOL maximized,BOOL child,UINT flags)
{
    if(!art || !art->available || (flags&DFCS_INACTIVE))return (style_caption_raster){0};
    unsigned image,width=CLASSIC_CAPTION_ART_WIDTH;
    switch(role){
    case CLASSIC_BUTTON_SYSTEM:
        image=child?CLASSIC_IMAGE_ROLE_COUNT:CLASSIC_IMAGE_SYSTEM;
        width=CLASSIC_SYSTEM_ART_WIDTH;
        break;
    case CLASSIC_BUTTON_MINIMIZE:image=CLASSIC_IMAGE_MINIMIZE;break;
    case CLASSIC_BUTTON_MAXIMIZE:image=maximized?CLASSIC_IMAGE_RESTORE:CLASSIC_IMAGE_MAXIMIZE;break;
    default:return (style_caption_raster){0};
    }
    return (style_caption_raster){art->roles[image][(flags&DFCS_PUSHED)!=0],
        width,CLASSIC_CAPTION_ART_HEIGHT,width};
}

static DWORD pixel(COLORREF color)
{return (DWORD)GetRValue(color)<<16 | (DWORD)GetGValue(color)<<8 | GetBValue(color);}

void style_caption_raster_paint(HDC dc,RECT bounds,const style_caption_raster *raster,const COLORREF colors[4])
{
    DWORD pixels[19*18];
    if(IsRectEmpty(&bounds) || !raster->roles || !raster->width ||
       raster->height>ARRAYSIZE(pixels)/raster->width || !raster->height || raster->stride<raster->width)
        return;
    for(unsigned y=0;y<raster->height;++y){
        for(unsigned x=0;x<raster->width;++x){
            unsigned role=raster->roles[y*raster->stride+x];
            if(role>=4)return;
            pixels[y*raster->width+x]=pixel(colors[role]);
        }
    }
    BITMAPINFO bitmap={.bmiHeader={.biSize=sizeof(BITMAPINFOHEADER),.biWidth=(LONG)raster->width,
        .biHeight=-(LONG)raster->height,.biPlanes=1,.biBitCount=32}};
    SetStretchBltMode(dc,COLORONCOLOR);
    StretchDIBits(dc,bounds.left,bounds.top,bounds.right-bounds.left,bounds.bottom-bounds.top,
        0,0,(int)raster->width,(int)raster->height,pixels,&bitmap,DIB_RGB_COLORS,SRCCOPY);
}

void style_caption_system_paint(HDC dc,RECT bounds,UINT glyph,const COLORREF colors[4])
{
    if(IsRectEmpty(&bounds))return;
    style_bitmap bitmap={0};
    SIZE size={bounds.right-bounds.left,bounds.bottom-bounds.top};
    if(!style_bitmap_ensure(&bitmap,dc,size))return;
    RECT local={0,0,size.cx,size.cy};
    if(DrawFrameControl(bitmap.dc,&local,DFC_CAPTION,glyph)){
        static const int indices[]={COLOR_3DFACE,COLOR_3DSHADOW,COLOR_3DDKSHADOW,COLOR_3DLIGHT};
        DWORD from[ARRAYSIZE(indices)],to[ARRAYSIZE(indices)];
        for(unsigned i=0;i<ARRAYSIZE(indices);++i){
            from[i]=pixel(GetSysColor(indices[i]));
            to[i]=pixel(colors[i]);
        }
        GdiFlush();
        DWORD *pixels=(DWORD *)bitmap.pixels;
        size_t count=(size_t)size.cx*(size_t)size.cy;
        for(size_t i=0;i<count;++i)for(unsigned j=0;j<ARRAYSIZE(indices);++j)
            if((pixels[i]&0xffffff)==from[j]){pixels[i]=to[j];break;}
        BitBlt(dc,bounds.left,bounds.top,size.cx,size.cy,bitmap.dc,0,0,SRCCOPY);
    }
    style_bitmap_dispose(&bitmap);
}

BOOL style_caption_button_paint(HDC dc,RECT bounds,enum classic_button_role role,
    BOOL maximized,UINT flags,int unit,const style_caption_presentation *presentation,
    const style_caption_raster *raster,const COLORREF colors[4])
{
    if(IsRectEmpty(&bounds))return TRUE;
    if(raster && raster->roles && (role==CLASSIC_BUTTON_SYSTEM ||
       presentation->button_source!=STYLE_CAPTION_BUTTON_SYSTEM)){
        RECT image=bounds;
        if(presentation->button_outline==STYLE_CAPTION_BUTTON_EDGE_OUTLINE){
            COLORREF face=role==CLASSIC_BUTTON_SYSTEM && (flags&DFCS_PUSHED)?colors[2]:colors[3];
            HBRUSH brush=GetStockObject(DC_BRUSH);
            SetDCBrushColor(dc,face);FillRect(dc,&bounds,brush);
            SetDCBrushColor(dc,colors[0]);
            const RECT strokes[]={
                {bounds.left,bounds.top,bounds.right,bounds.top+unit},
                {bounds.left,bounds.bottom-unit,bounds.right,bounds.bottom},
                {bounds.right-unit,bounds.top,bounds.right,bounds.bottom},
                {bounds.left,bounds.top,bounds.left+unit,bounds.bottom}
            };
            for(unsigned i=0;i<ARRAYSIZE(strokes);++i)FillRect(dc,&strokes[i],brush);
            if(role==CLASSIC_BUTTON_SYSTEM)image.right-=unit;
        }
        style_caption_raster_paint(dc,image,raster,colors);
        return TRUE;
    }
    if(role==CLASSIC_BUTTON_SYSTEM){
        HICON icon=LoadIcon(NULL,IDI_APPLICATION);
        int size=min(bounds.right-bounds.left,bounds.bottom-bounds.top);
        return icon && DrawIconEx(dc,(bounds.left+bounds.right-size)/2,
            (bounds.top+bounds.bottom-size)/2,icon,size,size,0,NULL,DI_NORMAL);
    }
    UINT glyph;
    switch(role){
    case CLASSIC_BUTTON_MINIMIZE:glyph=DFCS_CAPTIONMIN;break;
    case CLASSIC_BUTTON_MAXIMIZE:glyph=maximized?DFCS_CAPTIONRESTORE:DFCS_CAPTIONMAX;break;
    case CLASSIC_BUTTON_HELP:glyph=DFCS_CAPTIONHELP;break;
    case CLASSIC_BUTTON_CLOSE:glyph=DFCS_CAPTIONCLOSE;break;
    default:return FALSE;
    }
    const COLORREF system_colors[]={colors[3],colors[2],colors[0],colors[3]};
    style_caption_system_paint(dc,bounds,glyph|flags,system_colors);
    return TRUE;
}

BOOL style_caption_fill(HDC dc,RECT bounds,COLORREF start,COLORREF end,BOOL gradient)
{
    if(IsRectEmpty(&bounds))return FALSE;
    if(!gradient){
        HBRUSH brush=CreateSolidBrush(start);
        if(!brush)return FALSE;
        BOOL filled=FillRect(dc,&bounds,brush)!=0;
        DeleteObject(brush);
        return filled;
    }
    TRIVERTEX vertices[]={
        {bounds.left,bounds.top,(COLOR16)(GetRValue(start)<<8),
            (COLOR16)(GetGValue(start)<<8),(COLOR16)(GetBValue(start)<<8),0},
        {bounds.right,bounds.bottom,(COLOR16)(GetRValue(end)<<8),
            (COLOR16)(GetGValue(end)<<8),(COLOR16)(GetBValue(end)<<8),0}
    };
    GRADIENT_RECT range={0,1};
    return GradientFill(dc,vertices,2,&range,1,GRADIENT_FILL_RECT_H);
}

static RECT caption_band(HDC dc,RECT band,int fill_right,COLORREF start,COLORREF end,
    BOOL gradient,unsigned separator,int unit,COLORREF line)
{
    RECT border=band;
    if(separator==STYLE_CAPTION_SEPARATOR_INSIDE && band.bottom-band.top>unit){
        border.top=band.bottom-unit;
        band.bottom=border.top;
        style_caption_fill(dc,border,line,line,FALSE);
    }
    RECT fill=band;
    fill.right=min(band.right,max(band.left,fill_right));
    style_caption_fill(dc,fill,start,end,gradient);
    band.left=fill.right;
    if(!IsRectEmpty(&band))style_caption_fill(dc,band,end,end,FALSE);
    if(separator==STYLE_CAPTION_SEPARATOR_AFTER){
        border.top=border.bottom;
        border.bottom+=unit;
        style_caption_fill(dc,border,line,line,FALSE);
    }
    return fill;
}

BOOL style_caption_title(HDC dc,RECT bounds,PCWSTR title,HFONT font,COLORREF color,unsigned mode)
{
    if(!title || !*title || IsRectEmpty(&bounds))return TRUE;
    if(!SelectObject(dc,font) || !SetBkMode(dc,TRANSPARENT) || SetTextColor(dc,color)==CLR_INVALID)
        return FALSE;
    if(mode==STYLE_CAPTION_TEXT_CENTER_CLIP){
        SIZE extent;
        int length=lstrlenW(title);
        return GetTextExtentPoint32W(dc,title,length,&extent) &&
            ExtTextOutW(dc,bounds.left+max(1,(bounds.right-bounds.left-extent.cx)/2),
                bounds.top+max(0,(bounds.bottom-bounds.top-extent.cy)/2),
                ETO_CLIPPED,&bounds,title,(UINT)length,NULL);
    }
    return DrawTextW(dc,title,-1,&bounds,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOPREFIX)!=0;
}

BOOL style_caption_icon_visible(unsigned mode,BOOL system_button,DWORD style,DWORD extended)
{
    return !system_button && (style&WS_SYSMENU) && !(extended&WS_EX_TOOLWINDOW) &&
        (mode==STYLE_CAPTION_ICON_NATIVE || mode==STYLE_CAPTION_ICON_DEFAULT);
}

BOOL style_caption_icon_bounds(RECT band,int limit,UINT dpi,RECT *icon)
{
    if(IsRectEmpty(&band))return FALSE;
    int width=GetSystemMetricsForDpi(SM_CXSMICON,dpi)+MulDiv(4,(int)dpi,96);
    if(width<=0 || width>=limit-band.left)return FALSE;
    *icon=band;
    icon->right=band.left+width;
    return TRUE;
}

static BOOL paint_sprite_button(HDC dc,const style_caption_render_model *model,enum classic_button_role role)
{
    if(!model->sprites)return FALSE;
    const style_frame_sprite_button *item=role==CLASSIC_BUTTON_MAXIMIZE && model->maximized?
        &model->sprites->restore:&model->sprites->buttons[role];
    if(!item->face.bits)return FALSE;
    UINT flags=model->flags[role];
    unsigned state=flags&DFCS_INACTIVE?3u:flags&DFCS_PUSHED?2u:flags&DFCS_HOT?1u:0u;
    if(!model->active && item->face.count>=8)state+=4;
    RECT bounds=model->buttons[role];
    if(!style_dib_strip_draw(dc,bounds,&item->face,state,model->dpi))return FALSE;
    int dpi=(int)model->dpi;
    bounds.left+=MulDiv(item->content.left,dpi,96);
    bounds.top+=MulDiv(item->content.top,dpi,96);
    bounds.right-=MulDiv(item->content.right,dpi,96);
    bounds.bottom-=MulDiv(item->content.bottom,dpi,96);
    int width=bounds.right-bounds.left,height=bounds.bottom-bounds.top;
    for(size_t i=ARRAYSIZE(item->glyphs);i>0;--i){
        const style_dib_strip *glyph=&item->glyphs[i-1];
        if(!glyph->bits || (int)glyph->width>width || (int)glyph->height>height)continue;
        RECT centered={bounds.left+(width-(int)glyph->width)/2,bounds.top+(height-(int)glyph->height)/2,0,0};
        centered.right=centered.left+(int)glyph->width;
        centered.bottom=centered.top+(int)glyph->height;
        (void)style_dib_strip_draw(dc,centered,glyph,state,model->dpi);
        break;
    }
    return TRUE;
}

void style_caption_render(HDC dc,PCWSTR title,HFONT font,const style_caption_render_model *model,
    const style_caption_presentation *presentation,const classic_caption_art *art)
{
    const COLORREF *colors=model->button_colors;
    COLORREF separator=presentation->separator==STYLE_CAPTION_SEPARATOR_AFTER?colors[0]:colors[3];
    RECT band=caption_band(dc,model->band,model->text.right,model->start,model->end,
        model->gradient,presentation->separator,model->unit,separator);
    if(model->sprites){
        const style_dib_strip *caption=model->maximized?&model->sprites->maximized:&model->sprites->caption;
        RECT image=IsRectEmpty(&model->sprite_band)?model->band:model->sprite_band;
        (void)style_dib_strip_draw(dc,image,caption,model->active?0u:1u,model->dpi);
    }
    RECT text=model->text;
    const int inset=presentation->text_inset_plus_one?presentation->text_inset_plus_one-1:
        (presentation->text_mode==STYLE_CAPTION_TEXT_CENTER_CLIP?0:2);
    text.left+=inset;
    text.bottom=min(text.bottom,band.bottom);
    style_caption_title(dc,text,title,font,model->ink,presentation->text_mode);
    for(enum classic_button_role role=CLASSIC_BUTTON_SYSTEM;role<=CLASSIC_BUTTON_CLOSE;++role){
        if(IsRectEmpty(&model->buttons[role]))continue;
        if(paint_sprite_button(dc,model,role))continue;
        UINT flags=model->flags[role];
        style_caption_raster raster=style_caption_button_raster(art,role,model->maximized,model->child,flags);
        style_caption_button_paint(dc,model->buttons[role],role,model->maximized,flags,
            model->unit,presentation,&raster,colors);
    }
    if(model->icon && !IsRectEmpty(&model->icon_bounds)){
        RECT bounds=model->icon_bounds;
        int width=min(bounds.right-bounds.left,model->icon_size.cx);
        int height=min(bounds.bottom-bounds.top,model->icon_size.cy);
        if(width>0 && height>0)
            DrawIconEx(dc,bounds.left+(bounds.right-bounds.left-width)/2,
                bounds.top+(bounds.bottom-bounds.top-height)/2,model->icon,width,height,0,NULL,DI_NORMAL);
    }
}
