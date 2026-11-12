#include "status_font.h"
#include "../win16-common/font.h"
#include <limits.h>
#include <string.h>

HFONT win16_control_status_font_borrow(win16_control_status_font_cache *cache,HDC dc,void *owner,void **token)
{
    HFONT source;LOGFONTW log={0};TEXTMETRICW tm={0};int widths[96]={0};
    source=(HFONT)GetCurrentObject(dc,OBJ_FONT);
    if(!source || GetObjectW(source,sizeof(log),&log)!=sizeof(log) || !GetTextMetricsW(dc,&tm))return NULL;
    for(unsigned i=0;i<96;++i){WCHAR character=(WCHAR)(32+i);SIZE extent;
        if(!GetTextExtentPoint32W(dc,&character,1,&extent))return NULL;
        widths[i]=extent.cx;
    }
    BOOL changed=cache->invalid || cache->source!=source || memcmp(&cache->log,&log,sizeof(log)) || memcmp(&cache->metrics,&tm,sizeof(tm)) || memcmp(cache->widths,widths,sizeof(widths));
    if(changed){
        if(cache->borrows)return NULL;
        win16_style_system_release(cache->font);cache->font=NULL;cache->source=NULL;
        cache->invalid=FALSE;HFONT replacement=win16_style_system_acquire_isolated(source);if(!replacement)return NULL;
        cache->source=source;cache->font=replacement;cache->log=log;cache->metrics=tm;memcpy(cache->widths,widths,sizeof(widths));
    }
    if(!cache->font || cache->borrows==UINT_MAX)return NULL;
    if(cache->font!=source && !win16_style_system_try_retain(cache->font))return NULL;
    ++cache->borrows;*token=owner;return cache->font;
}
void win16_control_status_font_return(win16_control_status_font_cache *cache,HFONT font)
{
    if(!cache || !cache->borrows)return;
    win16_style_system_release(font);--cache->borrows;
    if(!cache->borrows && cache->invalid){win16_style_system_release(cache->font);cache->font=NULL;cache->source=NULL;}
}
