#include "lib-win32-style/application/graphics/font_cache.h"
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <wchar.h>

static unsigned heap_calls,heap_live,heap_fail,decode_calls,heap_free_count;
static void *heap_freed[2];
static LPVOID WINAPI test_heap_alloc(HANDLE heap,DWORD flags,SIZE_T size)
{
    ++heap_calls;if(heap_fail==heap_calls)return NULL;
    void *memory=HeapAlloc(heap,flags,size);if(memory)++heap_live;return memory;
}
static BOOL WINAPI test_heap_free(HANDLE heap,DWORD flags,LPVOID memory)
{
    BOOL result=HeapFree(heap,flags,memory);
    if(result){--heap_live;if(heap_free_count<2)heap_freed[heap_free_count]=memory;++heap_free_count;}
    return result;
}
#define HeapAlloc test_heap_alloc
#define HeapFree test_heap_free
#include "lib-win32-style/application/graphics/font_cache.c"
#undef HeapAlloc
#undef HeapFree
static BOOL decode_ok(HDC dc,const WCHAR *text,UINT count,WCHAR *decoded)
{(void)dc;++decode_calls;if(count)memcpy(decoded,text,count*sizeof(WCHAR));return TRUE;}
static BOOL decode_fail(HDC dc,const WCHAR *text,UINT count,WCHAR *decoded)
{(void)dc;(void)text;(void)count;(void)decoded;++decode_calls;return FALSE;}
static BOOL glyph_text_resources(void)
{
    style_glyph_text run={0};
    if(!style_glyph_text_prepare(&run,NULL,L"",0,1252,decode_ok) || heap_calls || decode_calls!=1)return FALSE;
    style_glyph_text_end(&run);
    for(unsigned fail=1;fail<=2;++fail){
        heap_calls=decode_calls=0;heap_fail=fail;
        if(style_glyph_text_prepare(&run,NULL,L"AB",2,1252,decode_ok) || heap_live!=fail-1 ||
            decode_calls!=(fail==1?0u:1u))return FALSE;
        style_glyph_text_end(&run);if(heap_live)return FALSE;
    }
    heap_calls=decode_calls=heap_fail=0;
    if(style_glyph_text_prepare(&run,NULL,L"AB",2,1252,decode_fail) || heap_calls!=1 || heap_live!=1)return FALSE;
    style_glyph_text_end(&run);
    if(style_glyph_text_prepare(&run,NULL,L"\x4e2d",1,1252,decode_ok) || heap_live!=2)return FALSE;
    style_glyph_text_end(&run);if(heap_live)return FALSE;
    heap_calls=heap_free_count=0;
    if(!style_glyph_text_prepare(&run,NULL,L"AB",2,1252,decode_ok) || heap_live!=2 ||
        run.encoded[0]!='A' || run.encoded[1]!='B' || run.decoded[0]!='A')return FALSE;
    void *decoded=run.decoded,*encoded=run.encoded;
    style_glyph_text_release_ansi(&run);
    if(heap_live!=1 || run.encoded || run.decoded!=decoded || heap_freed[0]!=encoded)return FALSE;
    style_glyph_text_end(&run);style_glyph_text_end(&run);
    if(heap_live || run.decoded || heap_free_count!=2 || heap_freed[1]!=decoded)return FALSE;
    heap_free_count=0;
    if(!style_glyph_text_prepare(&run,NULL,L"AB",2,1252,decode_ok))return FALSE;
    decoded=run.decoded;encoded=run.encoded;style_glyph_text_end(&run);
    if(heap_live || heap_free_count!=2 || heap_freed[0]!=decoded || heap_freed[1]!=encoded)return FALSE;
    heap_calls=0;
    if(style_glyph_text_prepare(&run,NULL,L"",UINT_MAX,1252,decode_ok) || heap_calls || heap_live)return FALSE;
    return TRUE;
}

#define CHECK(x) do{if(!(x)){fprintf(stderr,"FONT CACHE FAIL line=%d error=%lu\n",__LINE__,(unsigned long)GetLastError());return 1;}}while(0)
typedef struct lease {HFONT font;unsigned refs;} lease;
static lease leases[64];
static unsigned creates,retains,releases;
static BOOL fail_create,fail_retain;
static HFONT acquire(HFONT source,unsigned recipe)
{(void)recipe;
    ++creates;if(fail_create)return NULL;
    LOGFONTW log;if(GetObjectW(source,sizeof(log),&log)!=sizeof(log))return NULL;
    HFONT font=CreateFontIndirectW(&log);if(!font)return NULL;
    for(unsigned i=0;i<ARRAYSIZE(leases);++i)if(!leases[i].refs){leases[i]=(lease){font,1};return font;}
    DeleteObject(font);return NULL;
}
static BOOL retain(HFONT font)
{
    ++retains;if(fail_retain)return FALSE;
    for(unsigned i=0;i<ARRAYSIZE(leases);++i)if(leases[i].font==font && leases[i].refs){++leases[i].refs;return TRUE;}
    return FALSE;
}
static void release(HFONT font)
{
    for(unsigned i=0;i<ARRAYSIZE(leases);++i)if(font && leases[i].font==font && leases[i].refs){
        ++releases;if(!--leases[i].refs){DeleteObject(font);leases[i].font=NULL;}return;
    }
}
static HFONT borrowed(HFONT source,unsigned recipe){(void)recipe;return source;}
static const style_font_ops operations={acquire,retain,release},borrowed_operations={borrowed,retain,release};
static BOOL alive(HFONT font)
{LOGFONTW log;return GetObjectW(font,sizeof(log),&log)==sizeof(log);}
static unsigned pool_creates,memory_creates;
static HFONT copy_font(HFONT source)
{LOGFONTW log;return GetObjectW(source,sizeof(log),&log)==sizeof(log)?CreateFontIndirectW(&log):NULL;}
static HFONT pool_create(HFONT source,UINT key,void *context,style_font_pool_entry **owned)
{
    (void)key;++pool_creates;
    if(context)return source;
    style_font_pool_entry *entry=calloc(1,sizeof(*entry));if(!entry)return NULL;
    HFONT font=fail_create?NULL:copy_font(source);if(!font){free(entry);return NULL;}
    entry->lease.font=font;*owned=entry;return font;
}
static BOOL memory_create(void *context,style_memory_font *value)
{
    ++memory_creates;if(fail_create)return FALSE;
    value->font=copy_font(context);return value->font!=NULL;
}
int main(void)
{
    if(!glyph_text_resources())return 2;
    puts("GLYPH_TEXT_OWNER_OK partial_failure=1 source_preserved=1 release_order=1 zero=1 heap=0");
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);
    HFONT sources[33],cached[32];style_font_cache cache={0};
    HGDIOBJ original=GetCurrentObject(dc,OBJ_FONT);
    for(unsigned i=0;i<ARRAYSIZE(sources);++i){
        sources[i]=CreateFontW(-8-(int)i,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,ANSI_CHARSET,
            OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,L"Tahoma");CHECK(sources[i]);
        if(!i){fail_create=TRUE;CHECK(!style_font_cache_get(&cache,sources[i],0,&operations) && !cache.count);fail_create=FALSE;}
        unsigned before=creates;HFONT font=style_font_cache_get(&cache,sources[i],0,&operations);
        if(i==32){CHECK(!font && cache.count==32 && creates==before);continue;}
        cached[i]=font;CHECK(font && cache.count==i+1 && alive(sources[i]));
        CHECK(style_font_cache_get(&cache,sources[i],0,&operations)==font && creates==before+1);
    }
    CHECK(SelectObject(dc,sources[0]));
    WORD glyphs[5];WCHAR decoded[5];
    CHECK(GetGlyphIndicesW(dc,L"01234",5,glyphs,GGI_MARK_NONEXISTING_GLYPHS)!=GDI_ERROR);
    style_font_glyph_range range={1252,32,96};
    CHECK(style_font_cache_decode(&cache,dc,(const WCHAR *)glyphs,5,decoded,range) && !wmemcmp(decoded,L"01234",5));
    CHECK(style_font_cache_decode(&cache,dc,(const WCHAR *)glyphs,5,decoded,range));
    WORD unknown=0xfffe;CHECK(!style_font_cache_decode(&cache,dc,(const WCHAR *)&unknown,1,decoded,range));
    CHECK(!style_font_cache_decode(&cache,dc,(const WCHAR *)glyphs,5,decoded,(style_font_glyph_range){1252,32,225}));
    CHECK(SelectObject(dc,sources[32]));CHECK(!style_font_cache_decode(&cache,dc,(const WCHAR *)glyphs,5,decoded,range));
    style_font_cache_dispose(&cache);style_font_cache_dispose(&cache);CHECK(!cache.count && !cache.ops);
    for(unsigned i=0;i<32;++i)CHECK(!alive(cached[i]) && alive(sources[i]));

    style_font_snapshot_cache snapshot={0};void *token=NULL;int owner;
    CHECK(SelectObject(dc,sources[0]));
    HFONT first=style_font_snapshot_borrow(&snapshot,dc,&owner,&token,0,&operations);
    CHECK(first && token==&owner && snapshot.borrows==1);
    CHECK(style_font_snapshot_borrow(&snapshot,dc,&owner,&token,0,&operations)==first && snapshot.borrows==2);
    snapshot.invalid=TRUE;CHECK(!style_font_snapshot_borrow(&snapshot,dc,&owner,&token,0,&operations));
    style_font_snapshot_return(&snapshot,first);CHECK(snapshot.borrows==1 && alive(first));
    style_font_snapshot_return(&snapshot,first);CHECK(!snapshot.borrows && !snapshot.font && !alive(first));
    first=style_font_snapshot_borrow(&snapshot,dc,&owner,&token,0,&operations);CHECK(first);
    style_font_snapshot_return(&snapshot,first);
    CHECK(SelectObject(dc,sources[1]));fail_create=TRUE;
    CHECK(!style_font_snapshot_borrow(&snapshot,dc,&owner,&token,0,&operations) && !snapshot.font && !alive(first));
    fail_create=FALSE;fail_retain=TRUE;
    CHECK(!style_font_snapshot_borrow(&snapshot,dc,&owner,&token,0,&operations) && snapshot.font && !snapshot.borrows);
    HFONT pending=snapshot.font;fail_retain=FALSE;
    CHECK(style_font_snapshot_borrow(&snapshot,dc,&owner,&token,0,&operations)==pending);
    style_font_snapshot_return(&snapshot,pending);style_font_snapshot_dispose(&snapshot);
    CHECK(!alive(pending) && !snapshot.ops);
    unsigned before_retains=retains;
    CHECK(style_font_snapshot_borrow(&snapshot,dc,&owner,&token,0,&borrowed_operations)==sources[1] && retains==before_retains);
    style_font_snapshot_return(&snapshot,sources[1]);style_font_snapshot_dispose(&snapshot);CHECK(alive(sources[1]));

    style_font_pool pool={0};fail_create=TRUE;
    CHECK(!style_font_pool_get(&pool,sources[0],7,FALSE,pool_create,NULL) && !pool.head);
    fail_create=FALSE;
    HFONT shared=style_font_pool_get(&pool,sources[0],7,FALSE,pool_create,NULL);CHECK(shared);
    unsigned before_pool=pool_creates;
    CHECK(style_font_pool_get(&pool,shared,7,FALSE,pool_create,NULL)==shared && pool_creates==before_pool);
    CHECK(style_font_pool_source(&pool,shared)==sources[0] && style_font_pool_try_retain(&pool,shared));
    style_font_pool_release(&pool,shared);style_font_pool_retain(&pool,shared);style_font_pool_release(&pool,shared);
    pool.head->lease.references=UINT_MAX;
    CHECK(!style_font_pool_get(&pool,sources[0],7,FALSE,pool_create,NULL) && pool_creates==before_pool);
    CHECK(!style_font_pool_try_retain(&pool,shared));pool.head->lease.references=2;
    HFONT isolated=style_font_pool_get(&pool,sources[0],7,TRUE,pool_create,NULL);
    HFONT recipe=style_font_pool_get(&pool,sources[0],8,FALSE,pool_create,NULL);
    CHECK(isolated && recipe && isolated!=shared && recipe!=shared);
    style_font_pool_release(&pool,shared);CHECK(alive(shared));
    style_font_pool_release(&pool,isolated);style_font_pool_release(&pool,recipe);
    CHECK(!alive(isolated) && !alive(recipe));
    style_font_pool_release(&pool,shared);CHECK(!pool.head && !alive(shared));
    CHECK(style_font_pool_get(&pool,sources[0],7,FALSE,pool_create,&owner)==sources[0] && !pool.head);
    style_font_pool_release(&pool,sources[0]);CHECK(alive(sources[0]));
    style_memory_font memory={0};fail_create=TRUE;
    CHECK(!style_memory_font_acquire(&memory,memory_create,sources[0]) && !memory.references);
    fail_create=FALSE;HFONT single=style_memory_font_acquire(&memory,memory_create,sources[0]);CHECK(single);
    unsigned before_memory=memory_creates;
    CHECK(style_memory_font_acquire(&memory,memory_create,sources[0])==single && memory_creates==before_memory);
    memory.references=UINT_MAX;
    CHECK(!style_memory_font_acquire(&memory,memory_create,sources[0]) && memory_creates==before_memory);
    memory.references=2;style_memory_font_release(&memory,sources[0]);CHECK(memory.references==2);
    style_memory_font_release(&memory,single);CHECK(alive(single));
    style_memory_font_release(&memory,single);CHECK(!memory.references && !memory.font && !alive(single));
    CHECK(SelectObject(dc,original));
    for(unsigned i=0;i<ARRAYSIZE(sources);++i)CHECK(alive(sources[i]) && DeleteObject(sources[i]));
    for(unsigned i=0;i<ARRAYSIZE(leases);++i)CHECK(!leases[i].refs);
    CHECK(DeleteDC(dc));
    printf("FONT CACHE PASS capacity=32 sources_preserved=33 glyphs=1 failure=1 snapshot_borrows=1 deferred_invalid=1 replacement_failure=1 retain_failure=1 borrowed_source=1 pool=1 isolated=1 recipe=1 source_normalized=1 overflow=1 memory_font=1 creates=%u releases=%u\n",creates,releases);
    return 0;
}
