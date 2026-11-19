#include "caption.h"
#include "lib-win32-style/application/graphics/bitmap.h"
#include "lib-win32-style/appearance/msstyles/classic_presets.h"

win2k_caption_colors win2k_caption_palette(BOOL active)
{
    return active
        ?(win2k_caption_colors){RGB(0,0,128),RGB(166,202,240),RGB(255,255,255),TRUE}
        :(win2k_caption_colors){RGB(128,128,128),RGB(181,181,181),RGB(192,192,192),TRUE};
}

HFONT win2k_caption_font(UINT dpi,BOOL tool)
{
    int height=MulDiv(8,(int)dpi,tool?96:72);
    return CreateFontW(-height,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");
}

int win2k_caption_height(UINT dpi,BOOL tool)
{
    return MulDiv(tool?16:19,(int)dpi,96);
}

static SIZE button_size(BOOL tool)
{
    return tool?(SIZE){15,15}:(SIZE){16,14};
}

BOOL win2k_caption_buttons(const RECT *title,UINT dpi,BOOL tool,
                          const BOOL present[WIN2K_CAPTION_PARTS],RECT bounds[WIN2K_CAPTION_PARTS])
{
    static const classic_button_group group=WIN2000_TITLE_BUTTON_GROUP;
    return win2k_caption_buttons_with_group(title,dpi,tool,present,&group,NULL,bounds);
}

BOOL win2k_caption_buttons_with_group(const RECT *title,UINT dpi,BOOL tool,
    const BOOL present[WIN2K_CAPTION_PARTS],const classic_button_group *group,
    const classic_caption_metrics *metrics,RECT bounds[WIN2K_CAPTION_PARTS])
{
    SIZE size=button_size(tool);
    if(metrics){
        SIZE defined=tool?metrics->tool_button:metrics->button;
        if(defined.cx>0 && defined.cy>0)size=defined;
    }
    size=(SIZE){MulDiv(size.cx,(int)dpi,96),MulDiv(size.cy,(int)dpi,96)};
    static const unsigned roles[]={CLASSIC_BUTTON_MINIMIZE,CLASSIC_BUTTON_MAXIMIZE,
        CLASSIC_BUTTON_HELP,CLASSIC_BUTTON_CLOSE};
    unsigned mask=0;
    for(unsigned i=0;i<ARRAYSIZE(roles);++i)if(present[i])mask|=1u<<roles[i];
    RECT slots[CLASSIC_BUTTON_CLOSE+1];
    if(!style_caption_slots(group,title,dpi,size,mask,FALSE,slots))return FALSE;
    for(unsigned i=0;i<ARRAYSIZE(roles);++i)bounds[i]=slots[roles[i]];
    return TRUE;
}

BOOL win2k_caption_paint(HDC dc,const RECT *bounds,PCWSTR title,HFONT font,
                         const win2k_caption_colors *colors)
{
    if(!dc || !bounds || !title || !font || !colors ||
       bounds->right<=bounds->left || bounds->bottom<=bounds->top)return FALSE;
    int saved=SaveDC(dc);if(!saved)return FALSE;
    BOOL ok=TRUE;
    if(colors->gradient){
        TRIVERTEX vertices[2]={
            {bounds->left,bounds->top,(COLOR16)(GetRValue(colors->left)<<8),
             (COLOR16)(GetGValue(colors->left)<<8),(COLOR16)(GetBValue(colors->left)<<8),0},
            {bounds->right,bounds->bottom,(COLOR16)(GetRValue(colors->right)<<8),
             (COLOR16)(GetGValue(colors->right)<<8),(COLOR16)(GetBValue(colors->right)<<8),0}};
        GRADIENT_RECT gradient={0,1};
        ok=GradientFill(dc,vertices,2,&gradient,1,GRADIENT_FILL_RECT_H);
    }else{
        HBRUSH brush=CreateSolidBrush(colors->left);
        if(!brush)ok=FALSE;
        else{ok=FillRect(dc,bounds,brush)!=0;DeleteObject(brush);}
    }
    if(ok){
        RECT text=*bounds;
        if(!SelectObject(dc,font) || SetBkMode(dc,TRANSPARENT)==0 || SetTextColor(dc,colors->text)==CLR_INVALID)ok=FALSE;
        else if(*title && !DrawTextW(dc,title,-1,&text,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOPREFIX))ok=FALSE;
    }
    if(!RestoreDC(dc,saved))ok=FALSE;
    return ok;
}

/* DrawFrameControl uses the host's palette; remap its classic shades locally. */
static void paint_button(HDC target,const RECT *rect,UINT state)
{
    int width=rect->right-rect->left,height=rect->bottom-rect->top;
    if(width<=0 || height<=0)return;
    style_bitmap bitmap={0};SIZE size={width,height};
    if(style_bitmap_ensure(&bitmap,target,size)){
        HDC memory=bitmap.dc;
        RECT local={0,0,size.cx,size.cy};
        if(DrawFrameControl(memory,&local,DFC_CAPTION,state)){
            static const int indices[]={COLOR_3DFACE,COLOR_3DSHADOW,COLOR_3DDKSHADOW,COLOR_3DLIGHT};
            static const COLORREF values[]={RGB(212,208,200),RGB(128,128,128),RGB(0,0,0),RGB(212,208,200)};
            DWORD from[ARRAYSIZE(indices)],to[ARRAYSIZE(indices)];
            for(unsigned i=0;i<ARRAYSIZE(indices);++i){
                COLORREF c=GetSysColor(indices[i]),v=values[i];
                from[i]=(DWORD)GetRValue(c)<<16 | (DWORD)GetGValue(c)<<8 | GetBValue(c);
                to[i]=(DWORD)GetRValue(v)<<16 | (DWORD)GetGValue(v)<<8 | GetBValue(v);
            }
            GdiFlush();DWORD *pixels=(DWORD *)bitmap.pixels;
            for(int i=0;i<size.cx*size.cy;++i)for(unsigned j=0;j<ARRAYSIZE(indices);++j)
                if((pixels[i]&0xffffff)==from[j]){pixels[i]=to[j];break;}
            BitBlt(target,rect->left,rect->top,width,height,memory,0,0,SRCCOPY);
        }
    }
    style_bitmap_dispose(&bitmap);
}

void win2k_window_border_paint(HDC dc,int width,int height,int thickness)
{
    static const COLORREF colors[]={RGB(212,208,200),RGB(0,0,0),RGB(255,255,255),RGB(128,128,128)};
    if(thickness>0 && width>2*thickness && height>2*thickness){
        HBRUSH face=CreateSolidBrush(RGB(212,208,200));
        if(face){
            RECT top={0,0,width,thickness},bottom={0,height-thickness,width,height};
            RECT left={0,thickness,thickness,height-thickness},right={width-thickness,thickness,width,height-thickness};
            FillRect(dc,&top,face);FillRect(dc,&bottom,face);FillRect(dc,&left,face);FillRect(dc,&right,face);
            DeleteObject(face);
        }
    }
    for(int inset=0;inset<2;++inset){
        HBRUSH light=CreateSolidBrush(colors[inset*2]),dark=CreateSolidBrush(colors[inset*2+1]);
        if(light && dark && width>2*inset && height>2*inset){
            RECT top={inset,inset,width-inset,inset+1},left={inset,inset+1,inset+1,height-inset};
            RECT bottom={inset,height-inset-1,width-inset,height-inset},right={width-inset-1,inset,width-inset,height-inset};
            FillRect(dc,&top,light);FillRect(dc,&left,light);
            FillRect(dc,&bottom,dark);FillRect(dc,&right,dark);
        }
        if(light)DeleteObject(light);
        if(dark)DeleteObject(dark);
    }
}

static void paint_icon(HDC dc,const RECT *icon,HICON image,UINT dpi)
{
    int width=min(icon->right-icon->left,GetSystemMetricsForDpi(SM_CXSMICON,dpi));
    int height=min(icon->bottom-icon->top,GetSystemMetricsForDpi(SM_CYSMICON,dpi));
    if(image && width>0 && height>0)DrawIconEx(dc,icon->left+(icon->right-icon->left-width)/2,
        icon->top+(icon->bottom-icon->top-height)/2,image,width,height,0,NULL,DI_NORMAL);
}

void win2k_caption_render(HDC dc,const win2k_caption_layout *layout,PCWSTR title,HICON icon,
                         HFONT font,UINT dpi,BOOL active,BOOL maximized,
                         const UINT button_flags[WIN2K_CAPTION_PARTS])
{
    win2k_caption_render_with_colors(dc,layout,title,icon,font,dpi,win2k_caption_palette(active),
        maximized,button_flags);
}

void win2k_caption_render_with_colors(HDC dc,const win2k_caption_layout *layout,PCWSTR title,HICON icon,
    HFONT font,UINT dpi,win2k_caption_colors colors,BOOL maximized,const UINT button_flags[WIN2K_CAPTION_PARTS])
{
    RECT band=layout->band;
    if(IsRectEmpty(&band))return;
    int saved=SaveDC(dc);
    if(!saved)return;
    int separator=max(1,MulDiv(1,(int)dpi,96));
    if(band.bottom-band.top>separator){
        RECT line=band;
        line.top=band.bottom-separator;
        SetDCBrushColor(dc,RGB(212,208,200));
        FillRect(dc,&line,GetStockObject(DC_BRUSH));
        band.bottom=line.top;
    }
    RECT gradient=band;
    for(unsigned i=0;i<WIN2K_CAPTION_PARTS;++i){
        RECT r=layout->buttons[i];
        if(!IsRectEmpty(&r) && r.left>=band.left)gradient.right=min(gradient.right,r.left);
    }
    win2k_caption_paint(dc,&gradient,L"",font,&colors);
    RECT tail=band;
    tail.left=gradient.right;
    win2k_caption_colors solid={colors.right,colors.right,colors.text,FALSE};
    win2k_caption_paint(dc,&tail,L"",font,&solid);
    RECT text=gradient;
    text.left=max(text.left,layout->icon.right)+2;
    if(!IsRectEmpty(&text)){
        SelectObject(dc,font);
        SetBkMode(dc,TRANSPARENT);
        SetTextColor(dc,colors.text);
        DrawTextW(dc,title,-1,&text,DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS|DT_NOPREFIX);
    }
    static const UINT glyphs[]={DFCS_CAPTIONMIN,DFCS_CAPTIONMAX,DFCS_CAPTIONHELP,DFCS_CAPTIONCLOSE};
    for(unsigned i=0;i<WIN2K_CAPTION_PARTS;++i){
        UINT glyph=i==WIN2K_MAXIMIZE && maximized?DFCS_CAPTIONRESTORE:glyphs[i];
        paint_button(dc,&layout->buttons[i],glyph|button_flags[i]);
    }
    if(icon && !IsRectEmpty(&layout->icon))paint_icon(dc,&layout->icon,icon,dpi);
    RestoreDC(dc,saved);
}
