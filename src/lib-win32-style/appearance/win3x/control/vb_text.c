#include "vb_text.h"
#include "control_class.h"
#include "../common/font.h"
#include <stdlib.h>
typedef struct ntw16s_vb_glyphmap {WCHAR chars[224];WORD glyphs[224];BOOL valid;} ntw16s_vb_glyphmap;
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
void win16_control_vb_paint_begin(style_runtime_text_scope *scope,HWND w,UINT message,ntw16s_vb_fonts *fonts,BOOL (*user_label_target)(HWND))
{
    const style_runtime_text_ops ops={&user_label_target,include_text,text_role};
    style_runtime_text_begin(scope,w,message,fonts,&ops);
}
void ntw16s_vb_fonts_clear(ntw16s_vb_fonts *fonts)
{
    for(unsigned i=0;i<fonts->count;++i){win16_style_system_release(fonts->font[i]);free(fonts->glyphs[i]);}
    ZeroMemory(fonts,sizeof(*fonts));
}
HFONT ntw16s_vb_paint_font(HDC dc)
{
    const style_runtime_text_scope *scope=style_runtime_text_current();HFONT source;
    /* The scope also covers a runtime-owned memory DC during WM_PRINT or
     * double buffering. Never change a DC belonging to another HWND. */
    if(!style_runtime_text_active() || (scope->report && dc!=scope->report_dc) || (WindowFromDC(dc) && WindowFromDC(dc)!=scope->window))return NULL;
    ntw16s_vb_fonts *fonts=scope->context;
    source=(HFONT)GetCurrentObject(dc,OBJ_FONT);
    if(!source)return NULL;
    if(scope->report){LOGFONTW lf;
        if(GetObjectW(source,sizeof(lf),&lf)!=sizeof(lf) || lf.lfEscapement || lf.lfOrientation)return NULL;}
    for(unsigned i=0;i<fonts->count;++i)if(fonts->source[i]==source)return fonts->font[i];
    if(fonts->count==ARRAYSIZE(fonts->font))return NULL;
    /* Report rows can be 11px too: reuse the existing stroke-preserving
     * host System lease; existing controls keep their established policy. */
    HFONT font=win16_style_vb_font_acquire(source);
    if(font){fonts->source[fonts->count]=source;fonts->font[fonts->count++]=font;}
    return font;
}

BOOL ntw16s_vb_decode_glyphs(HDC dc,const WCHAR *input,UINT count,WCHAR *decoded)
{
    if(!style_runtime_text_current() || !style_runtime_text_current()->window)return FALSE;
    ntw16s_vb_fonts *fonts=style_runtime_text_current()->context;HFONT source=(HFONT)GetCurrentObject(dc,OBJ_FONT);unsigned slot;
    for(slot=0;slot<fonts->count;++slot)if(fonts->source[slot]==source)break;
    if(slot==fonts->count)return FALSE;
    ntw16s_vb_glyphmap *map=fonts->glyphs[slot];
    if(!map){
        char bytes[224];map=calloc(1,sizeof(*map));if(!map)return FALSE;
        for(unsigned i=0;i<224;++i)bytes[i]=(char)(i+32);
        map->valid=MultiByteToWideChar(1252,0,bytes,224,map->chars,224)==224 &&
            GetGlyphIndicesW(dc,map->chars,224,map->glyphs,GGI_MARK_NONEXISTING_GLYPHS)!=GDI_ERROR;
        fonts->glyphs[slot]=map;
    }
    if(!map->valid)return FALSE;
    for(UINT i=0;i<count;++i){unsigned j;
        for(j=0;j<224;++j)if(map->glyphs[j] && map->glyphs[j]!=0xffff && map->glyphs[j]==(WORD)input[i])break;
        if(j==224)return FALSE;
        decoded[i]=map->chars[j];
    }
    return TRUE;
}
