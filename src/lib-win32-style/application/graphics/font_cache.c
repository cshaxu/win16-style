#include "font_cache.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

typedef struct style_font_glyph_map {WCHAR chars[224];WORD glyphs[224];BOOL valid;} style_font_glyph_map;

HFONT style_font_cache_get(style_font_cache *cache,HFONT source,unsigned recipe,const style_font_ops *ops)
{
    for(unsigned i=0;i<cache->count;++i)if(cache->source[i]==source)return cache->font[i];
    if(cache->count==ARRAYSIZE(cache->font))return NULL;
    HFONT font=ops->acquire(source,recipe);
    if(font){cache->ops=ops;cache->source[cache->count]=source;cache->font[cache->count++]=font;}
    return font;
}
void style_font_cache_dispose(style_font_cache *cache)
{
    for(unsigned i=0;i<cache->count;++i){cache->ops->release(cache->font[i]);free(cache->glyphs[i]);}
    ZeroMemory(cache,sizeof(*cache));
}
BOOL style_font_cache_decode(style_font_cache *cache,HDC dc,const WCHAR *input,UINT count,WCHAR *decoded,
    style_font_glyph_range range)
{
    HFONT source=(HFONT)GetCurrentObject(dc,OBJ_FONT);unsigned slot;
    for(slot=0;slot<cache->count;++slot)if(cache->source[slot]==source)break;
    if(slot==cache->count || !range.count || range.count>224 || range.count>256u-range.first)return FALSE;
    style_font_glyph_map *map=cache->glyphs[slot];
    if(!map){
        char bytes[224];map=calloc(1,sizeof(*map));if(!map)return FALSE;
        for(unsigned i=0;i<range.count;++i)bytes[i]=(char)(i+range.first);
        map->valid=MultiByteToWideChar(range.code_page,0,bytes,(int)range.count,map->chars,(int)range.count)==(int)range.count &&
            GetGlyphIndicesW(dc,map->chars,(int)range.count,map->glyphs,GGI_MARK_NONEXISTING_GLYPHS)!=GDI_ERROR;
        cache->glyphs[slot]=map;
    }
    if(!map->valid)return FALSE;
    for(UINT i=0;i<count;++i){unsigned j;
        for(j=0;j<range.count;++j)if(map->glyphs[j] && map->glyphs[j]!=0xffff && map->glyphs[j]==(WORD)input[i])break;
        if(j==range.count)return FALSE;
        decoded[i]=map->chars[j];
    }
    return TRUE;
}

HFONT style_font_snapshot_borrow(style_font_snapshot_cache *cache,HDC dc,void *owner,void **token,unsigned recipe,const style_font_ops *ops)
{
    HFONT source;LOGFONTW log={0};TEXTMETRICW tm={0};int widths[96]={0};
    source=(HFONT)GetCurrentObject(dc,OBJ_FONT);
    if(!source || GetObjectW(source,sizeof(log),&log)!=sizeof(log) || !GetTextMetricsW(dc,&tm))return NULL;
    for(unsigned i=0;i<96;++i){WCHAR character=(WCHAR)(32+i);SIZE extent;
        if(!GetTextExtentPoint32W(dc,&character,1,&extent))return NULL;
        widths[i]=extent.cx;
    }
    BOOL changed=cache->invalid || cache->source!=source || memcmp(&cache->log,&log,sizeof(log)) || memcmp(&cache->metrics,&tm,sizeof(tm)) || memcmp(cache->widths,widths,sizeof(widths));
    cache->ops=ops;
    if(changed){
        if(cache->borrows)return NULL;
        ops->release(cache->font);cache->font=NULL;cache->source=NULL;
        cache->invalid=FALSE;HFONT replacement=ops->acquire(source,recipe);if(!replacement)return NULL;
        cache->source=source;cache->font=replacement;cache->log=log;cache->metrics=tm;memcpy(cache->widths,widths,sizeof(widths));
    }
    if(!cache->font || cache->borrows==UINT_MAX)return NULL;
    if(cache->font!=source && !ops->retain(cache->font))return NULL;
    ++cache->borrows;*token=owner;return cache->font;
}
void style_font_snapshot_return(style_font_snapshot_cache *cache,HFONT font)
{
    if(!cache || !cache->borrows)return;
    cache->ops->release(font);--cache->borrows;
    if(!cache->borrows && cache->invalid){cache->ops->release(cache->font);cache->font=NULL;cache->source=NULL;}
}
void style_font_snapshot_dispose(style_font_snapshot_cache *cache)
{
    if(cache->ops)cache->ops->release(cache->font);
    ZeroMemory(cache,sizeof(*cache));
}

static SRWLOCK face_lock=SRWLOCK_INIT;
static style_memory_font *registered_faces;

void style_memory_font_register_face(style_memory_font *lease,const char *face)
{
    lstrcpynA(lease->face,face,ARRAYSIZE(lease->face));
    AcquireSRWLockExclusive(&face_lock);
    lease->next_face=registered_faces;
    registered_faces=lease;
    ReleaseSRWLockExclusive(&face_lock);
}

BOOL style_memory_font_has_face(const char *face)
{
    AcquireSRWLockShared(&face_lock);
    const style_memory_font *entry=registered_faces;
    while(entry && strcmp(entry->face,face))entry=entry->next_face;
    BOOL found=entry!=NULL;
    ReleaseSRWLockShared(&face_lock);
    return found;
}

HFONT style_memory_font_acquire(style_memory_font *lease,BOOL (*create)(void *,style_memory_font *),void *context)
{
    if(!lease->references && !create(context,lease))return NULL;
    if(lease->references==UINT_MAX)return NULL;
    ++lease->references;return lease->font;
}
void style_memory_font_release(style_memory_font *lease,HFONT font)
{
    if(!lease->references || font!=lease->font)return;
    if(!--lease->references){
        if(lease->face[0]){
            AcquireSRWLockExclusive(&face_lock);
            style_memory_font **entry=&registered_faces;
            while(*entry!=lease)entry=&(*entry)->next_face;
            *entry=lease->next_face;
            ReleaseSRWLockExclusive(&face_lock);
        }
        DeleteObject(lease->font);
        if(lease->resource)RemoveFontMemResourceEx(lease->resource);
        *lease=(style_memory_font){0};
    }
}
HFONT style_font_pool_source(const style_font_pool *pool,HFONT font)
{
    for(style_font_pool_entry *v=pool->head;v;v=v->next)if(v->lease.font==font)return v->source;
    return font;
}
HFONT style_font_pool_get(style_font_pool *pool,HFONT source,UINT key,BOOL isolated,
    style_font_pool_factory create,void *context)
{
    source=style_font_pool_source(pool,source);
    if(!isolated)for(style_font_pool_entry *v=pool->head;v;v=v->next)if(!v->isolated && v->source==source && v->key==key){
        if(v->lease.references==UINT_MAX)return NULL;
        ++v->lease.references;return v->lease.font;
    }
    style_font_pool_entry *v=NULL;HFONT font=create(source,key,context,&v);
    if(v){
        v->source=source;v->key=key;v->isolated=isolated;v->lease.references=1;
        v->next=pool->head;pool->head=v;
    }
    return font;
}
void style_font_pool_release(style_font_pool *pool,HFONT font)
{
    style_font_pool_entry **p=&pool->head;
    while(*p){style_font_pool_entry *v=*p;
        if(v->lease.font==font){
            BOOL last=v->lease.references==1;
            if(last)*p=v->next;
            style_memory_font_release(&v->lease,font);if(last)free(v);return;
        }
        p=&v->next;
    }
}
BOOL style_font_pool_try_retain(style_font_pool *pool,HFONT font)
{
    for(style_font_pool_entry *v=pool->head;v;v=v->next)if(v->lease.font==font){
        if(v->lease.references==UINT_MAX)return FALSE;
        ++v->lease.references;return TRUE;
    }
    return FALSE;
}
void style_font_pool_retain(style_font_pool *pool,HFONT font)
{
    for(style_font_pool_entry *v=pool->head;v;v=v->next)if(v->lease.font==font){++v->lease.references;return;}
}

BOOL style_glyph_text_prepare(style_glyph_text *run,HDC dc,LPCWSTR text,UINT count,UINT code_page,
    BOOL (*decode)(HDC,const WCHAR *,UINT,WCHAR *))
{
    if(count>INT_MAX)return FALSE;
    BOOL valid=TRUE;
    if(count)run->decoded=HeapAlloc(GetProcessHeap(),0,count*sizeof(WCHAR));
    if(count && !run->decoded)valid=FALSE;
    if(valid)valid=decode(dc,text,count,run->decoded);
    if(valid && count){
        BOOL used_default=FALSE;run->encoded=HeapAlloc(GetProcessHeap(),0,count);
        if(!run->encoded || WideCharToMultiByte(code_page,WC_NO_BEST_FIT_CHARS,run->decoded,(int)count,
            run->encoded,(int)count,NULL,&used_default)!=(int)count || used_default)valid=FALSE;
    }
    return valid;
}
void style_glyph_text_release_ansi(style_glyph_text *run)
{
    if(run->encoded){HeapFree(GetProcessHeap(),0,run->encoded);run->encoded=NULL;}
}
void style_glyph_text_end(style_glyph_text *run)
{
    if(run->decoded){HeapFree(GetProcessHeap(),0,run->decoded);run->decoded=NULL;}
    style_glyph_text_release_ansi(run);
}
