#include "vb_text.h"
#include "control_class.h"
#include "font.h"
#include <stdlib.h>
typedef struct ntw16s_vb_glyphmap {WCHAR chars[224];WORD glyphs[224];BOOL valid;} ntw16s_vb_glyphmap;
static _Thread_local ntw16s_vb_paint *current,*native_current;
void ntw16s_vb_paint_begin(ntw16s_vb_paint *scope,HWND w,UINT message,ntw16s_vb_fonts *fonts)
{
    ZeroMemory(scope,sizeof(*scope));
    if(message==WM_PAINT || message==WM_PRINT || message==WM_PRINTCLIENT || message==WM_ERASEBKGND){
        scope->native_previous=native_current;scope->native_window=w;scope->native_active=TRUE;native_current=scope;
    }
    BOOL edit=ntw16s_vb_edit(w);
    /* Native EDIT also draws synchronously during text, selection and focus
     * messages. Keep that real USER path inside the same bounded scope. */
    if(message!=WM_PAINT && message!=WM_PRINT && message!=WM_PRINTCLIENT && !edit)return;
    if(message==WM_NCDESTROY || message==WM_DESTROY)return;
    scope->previous=current;scope->active=TRUE;scope->fonts=fonts;current=scope;
    /* An attached non-VB child is a barrier during nested WM_PRINT. */
    if(ntw16s_vb_frame(w) || ntw16s_vb_edit(w) ||
            ntw16s_vb_class(w,L"ThunderRT6FormDC"))scope->window=w;
}
void ntw16s_vb_paint_end(ntw16s_vb_paint *scope)
{
    if(scope->native_active)native_current=scope->native_previous;
    if(!scope->active)return;
    current=scope->previous;
}
BOOL ntw16s_native_paint_target(HDC dc,HWND *w)
{*w=native_current?native_current->native_window:NULL;return dc && native_current && native_current->native_dc==dc;}
void ntw16s_vb_fonts_clear(ntw16s_vb_fonts *fonts)
{
    for(unsigned i=0;i<fonts->count;++i){win16_style_system_release(fonts->font[i]);free(fonts->glyphs[i]);}
    ZeroMemory(fonts,sizeof(*fonts));
}
void ntw16s_report_paint_enable(ntw16s_vb_paint *scope,HWND w,HMODULE module,const BOOL *cancelled)
{
    if(!scope->active || !module)return;
    scope->report=TRUE;scope->window=w;scope->report_module=module;scope->cancelled=cancelled;
}
void ntw16s_report_paint_bind(HWND w,HDC dc)
{
    if(current && current->report && current->window==w)current->report_dc=dc;
}
BOOL ntw16s_report_paint_active(void)
{return current && current->report;}
BOOL ntw16s_report_paint_caller(HMODULE module)
{return current && current->report && (module==current->report_module || module==GetModuleHandleW(NULL));}
BOOL ntw16s_vb_paint_active(void)
{return current && current->window && (!current->cancelled || !*current->cancelled);}
HFONT ntw16s_vb_paint_font(HDC dc)
{
    ntw16s_vb_paint *scope=current;HFONT source;
    /* The scope also covers a runtime-owned memory DC during WM_PRINT or
     * double buffering. Never change a DC belonging to another HWND. */
    if(!ntw16s_vb_paint_active() || (scope->report && dc!=scope->report_dc) || (WindowFromDC(dc) && WindowFromDC(dc)!=scope->window))return NULL;
    ntw16s_vb_fonts *fonts=scope->fonts;
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
    if(!current || !current->window)return FALSE;
    ntw16s_vb_fonts *fonts=current->fonts;HFONT source=(HFONT)GetCurrentObject(dc,OBJ_FONT);unsigned slot;
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
