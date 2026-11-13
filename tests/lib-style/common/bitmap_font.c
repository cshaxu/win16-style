#include "lib-style/common/bitmap_font.h"
#include <stdio.h>
#define CHECK(x) do { if(!(x)){printf("BITMAP_FONT_FAIL line=%d\n",__LINE__);return 1;} } while(0)
static unsigned word(const BYTE *data,unsigned offset)
{return (unsigned)data[offset]|((unsigned)data[offset+1]<<8);}
static DWORD dword(const BYTE *data,unsigned offset)
{DWORD value;memcpy(&value,data+offset,sizeof(value));return value;}
static int verify(style_bitmap_font_metrics metrics)
{
    BYTE data[4096]={0};const unsigned start=118+225*4;
    unsigned cursor=start+metrics.height*7;
    style_bitmap_font_word(data,118,9);style_bitmap_font_word(data,120,start);
    unsigned size=style_bitmap_font_finish(data,cursor,"Test Font",&metrics);
    CHECK(word(data,0)==0x200 && dword(data,2)==size && size==cursor+10);
    CHECK(word(data,68)==metrics.points && word(data,70)==96 && word(data,72)==96);
    CHECK(word(data,74)==metrics.ascent && word(data,76)==metrics.internal_leading &&
          word(data,78)==metrics.external_leading && word(data,83)==metrics.weight);
    CHECK(word(data,88)==metrics.height && data[90]==1 && word(data,91)==metrics.total_width/224 &&
          word(data,93)==metrics.max_width && data[95]==32 && data[96]==255 && data[97]=='?'-32);
    CHECK(word(data,99)==7 && dword(data,105)==cursor && dword(data,113)==start);
    CHECK(word(data,118)==9 && word(data,120)==start && word(data,118+224*4)==0 &&
          word(data,120+224*4)==cursor && !strcmp((const char *)data+cursor,"Test Font"));
    return 0;
}
int main(void)
{
    style_bitmap_font_metrics legacy={20,15,2,0,700,15,224*8,12};
    style_bitmap_font_metrics scaled={14,10,1,3,400,8,224*6,9};
    CHECK(!verify(legacy) && !verify(scaled));puts("BITMAP_FONT_OK");return 0;
}
