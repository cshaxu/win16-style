#ifndef STYLE_COMMON_BITMAP_H
#define STYLE_COMMON_BITMAP_H
#include <windows.h>
/* Zero-initialize before use; keep the same reference DC while reusing. */
typedef struct style_bitmap {
    HDC dc;HBITMAP bitmap;HGDIOBJ old_bitmap;BYTE *pixels;SIZE size;
} style_bitmap;
BOOL style_bitmap_ensure(style_bitmap *bitmap,HDC reference,SIZE size);
void style_bitmap_dispose(style_bitmap *bitmap);
#endif
