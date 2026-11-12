#include "lib-style/common/bitmap.h"
#include <stdio.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"BITMAP_FAIL line=%d error=%lu\n",__LINE__,GetLastError());return 1;} } while(0)
int main(void)
{
    HDC reference=GetDC(NULL);CHECK(reference);
    style_bitmap bitmap={0};SIZE a={32,20},b={16,12},invalid={0,1};
    CHECK(style_bitmap_ensure(&bitmap,reference,a));
    HDC dc=bitmap.dc;HBITMAP image=bitmap.bitmap;
    CHECK(bitmap.pixels && SetPixel(dc,4,5,RGB(10,20,30))!=CLR_INVALID);
    CHECK(style_bitmap_ensure(&bitmap,reference,a) && bitmap.dc==dc && bitmap.bitmap==image);
    CHECK(GetPixel(dc,4,5)==RGB(10,20,30));
    CHECK(!style_bitmap_ensure(&bitmap,reference,invalid) && GetLastError()==ERROR_INVALID_PARAMETER && bitmap.dc==dc);
    CHECK(style_bitmap_ensure(&bitmap,reference,b) && bitmap.size.cx==16 && bitmap.size.cy==12);
    style_bitmap_dispose(&bitmap);style_bitmap_dispose(&bitmap);
    CHECK(!bitmap.dc && !bitmap.bitmap && !bitmap.old_bitmap && !bitmap.pixels);
    CHECK(ReleaseDC(NULL,reference));
    puts("BITMAP_OK reference=1 pixels=1 reuse=1 invalid=1 resize=1 cleanup=1");return 0;
}
