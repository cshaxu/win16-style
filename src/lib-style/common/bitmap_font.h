#ifndef STYLE_BITMAP_FONT_H
#define STYLE_BITMAP_FONT_H
#include <windows.h>
#include <string.h>

/* ANSI FNT v2 serialization. The caller owns the zeroed buffer, glyph data,
 * face name and all resource lifetime; it chooses the actual font metrics. */
typedef struct style_bitmap_font_metrics {
    unsigned height,ascent,internal_leading,external_leading,weight,points;
    unsigned total_width,max_width;
} style_bitmap_font_metrics;
static inline void style_bitmap_font_word(BYTE *data,unsigned offset,unsigned value)
{data[offset]=(BYTE)value;data[offset+1]=(BYTE)(value>>8);}
static inline void style_bitmap_font_dword(BYTE *data,unsigned offset,DWORD value)
{memcpy(data+offset,&value,4);}
static inline unsigned style_bitmap_font_finish(BYTE *data,unsigned cursor,const char *face,
    const style_bitmap_font_metrics *m)
{
    const unsigned bitmap_start=118+225*4;
    style_bitmap_font_word(data,118+224*4,0);style_bitmap_font_word(data,120+224*4,cursor);
    style_bitmap_font_word(data,0,0x200);style_bitmap_font_word(data,68,m->points);
    style_bitmap_font_word(data,70,96);style_bitmap_font_word(data,72,96);
    style_bitmap_font_word(data,74,m->ascent);style_bitmap_font_word(data,76,m->internal_leading);
    style_bitmap_font_word(data,78,m->external_leading);style_bitmap_font_word(data,83,m->weight);
    style_bitmap_font_word(data,88,m->height);data[90]=1;
    style_bitmap_font_word(data,91,m->total_width/224);style_bitmap_font_word(data,93,m->max_width);
    data[95]=32;data[96]=255;data[97]='?'-32;
    style_bitmap_font_word(data,99,(cursor-bitmap_start)/m->height);
    style_bitmap_font_dword(data,105,cursor);style_bitmap_font_dword(data,113,bitmap_start);
    size_t length=strlen(face)+1;memcpy(data+cursor,face,length);cursor+=(unsigned)length;
    style_bitmap_font_dword(data,2,cursor);return cursor;
}
#endif
