#include "vb_text.h"
#include "control_class.h"
#include "lib-win32-style/appearance/msstyles/raster_font.h"

static const style_font_ops font_operations={.acquire=style_raster_font_acquire_stroke_fit,.release=style_raster_font_release};
static BOOL include_text(HWND w,UINT message)
{
    BOOL edit=style_control_vb_edit(w);
    if(message!=WM_PAINT && message!=WM_PRINT && message!=WM_PRINTCLIENT && !edit)return FALSE;
    return message!=WM_NCDESTROY && message!=WM_DESTROY;
}
static style_runtime_text_role text_role(HWND w,void *context)
{
    BOOL (*user_label_target)(HWND)=*(BOOL (**)(HWND))context;
    BOOL label_only=ntw16s_vb_picture(w) || user_label_target(w);
    HWND target=style_control_vb_frame(w) || style_control_vb_edit(w) || label_only ||
        (style_control_vb_class(w,L"ThunderRT6FormDC") || style_control_vb5_class(w,L"ThunderRT5Form"))?w:NULL;
    return (style_runtime_text_role){target,label_only};
}
void win16_control_vb_paint_begin(style_runtime_text_scope *scope,HWND w,UINT message,style_font_cache *fonts,unsigned points,BOOL (*user_label_target)(HWND))
{
    const style_runtime_text_ops ops={&user_label_target,include_text,text_role};
    style_runtime_text_begin(scope,w,message,fonts,&ops);
    scope->font_recipe=points;
}
HFONT win16_control_vb_paint_font(HDC dc)
{
    const style_runtime_text_scope *scope=style_runtime_text_current();HFONT source;
    /* The scope also covers a runtime-owned memory DC during WM_PRINT or
     * double buffering. Never change a DC belonging to another HWND. */
    if(!style_runtime_text_active() || !scope->context || !scope->font_recipe || (scope->report && dc!=scope->report_dc) || (WindowFromDC(dc) && WindowFromDC(dc)!=scope->window))return NULL;
    style_font_cache *fonts=scope->context;
    source=(HFONT)GetCurrentObject(dc,OBJ_FONT);
    if(!source)return NULL;
    if(scope->report){LOGFONTW lf;
        if(GetObjectW(source,sizeof(lf),&lf)!=sizeof(lf) || lf.lfEscapement || lf.lfOrientation)return NULL;}
    return style_font_cache_get(fonts,source,scope->font_recipe,&font_operations);
}

BOOL win16_control_vb_decode_glyphs(HDC dc,const WCHAR *input,UINT count,WCHAR *decoded)
{
    const style_runtime_text_scope *scope=style_runtime_text_current();
    if(!scope || !scope->window || !scope->context || !scope->font_recipe)return FALSE;
    return style_font_cache_decode(scope->context,dc,input,count,decoded,
        (style_font_glyph_range){1252,32,224});
}
