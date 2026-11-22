#ifndef STYLE_GRAPHICS_DIB_STRIP_H
#define STYLE_GRAPHICS_DIB_STRIP_H

#include <windows.h>

typedef struct style_dib_strip {
    const BYTE *bits;
    const RGBQUAD *palette;
    DWORD colors;
    unsigned width,height,stride,count,depth;
    BOOL top_down,horizontal,transparent;
    COLORREF color_key;
    RECT margins; /* Left, top, right, bottom extent in source pixels. */
} style_dib_strip;

/* Borrows an uncompressed Windows DIB. Failure leaves the output unchanged. */
BOOL style_dib_strip_read(const void *data,DWORD bytes,unsigned count,BOOL horizontal,style_dib_strip *output);
BOOL style_dib_strip_draw(HDC dc,RECT bounds,const style_dib_strip *strip,unsigned state,UINT dpi);

#endif