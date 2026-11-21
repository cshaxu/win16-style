#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
static void *watched;static unsigned freed_watched;
static void watched_free(void *p);
#define free watched_free
#include "../../src/lib-win32-style/application/graphics/font_cache.c"
#include "../../src/lib-win32-style/appearance/msstyles/raster_font.c"
#undef free
static void watched_free(void *p){if(p&&p==watched)++freed_watched;free(p);}
#define CHECK(x) do{if(!(x)){fprintf(stderr,"STOCK_FAIL line=%d expr=%s\n",__LINE__,#x);return 1;}}while(0)
int main(void)
{
    HFONT source=CreateFontW(-18,0,0,0,FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,0,0,L"Arial");
    CHECK(source);
    HFONT shared=style_raster_font_acquire(source,12);
    CHECK(shared && shared!=source && variants.head);
    watched=variants.head;
    HFONT stock=(HFONT)GetStockObject(SYSTEM_FONT);
    CHECK(stock && style_raster_font_acquire_isolated(stock,12)==stock);
    CHECK(!freed_watched && variants.head==watched);
    CHECK(style_raster_font_source(shared)==source);

    LOGFONTA realized={0};
    CHECK(GetObjectA(shared,sizeof(realized),&realized)==sizeof(realized));
    CHECK(style_memory_font_has_face(realized.lfFaceName));
    CHECK(!style_memory_font_has_face("Style Raster bogus"));
    HFONT clone=CreateFontIndirectA(&realized);
    CHECK(clone && clone!=shared && style_raster_font_acquire(clone,12)==clone);
    style_raster_font_release(clone);
    CHECK(DeleteObject(clone));
    CHECK(style_raster_font_try_retain(shared));
    style_raster_font_release(shared);
    CHECK(style_memory_font_has_face(realized.lfFaceName));
    style_raster_font_release(shared);
    CHECK(!style_memory_font_has_face(realized.lfFaceName) && freed_watched==1);

    CHECK(DeleteObject(source));
    puts("STOCK_VARIANT_OWNERSHIP_OK");
    return 0;
}
