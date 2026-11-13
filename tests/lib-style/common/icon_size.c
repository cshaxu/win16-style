#include "lib-style/common/icon_size.h"
#include <stdio.h>
#define CHECK(x) do { if(!(x)){printf("ICON_SIZE_FAIL line=%d\n",__LINE__);return 1;} } while(0)
int main(void)
{
    HBITMAP mask=CreateBitmap(8,8,1,1,NULL),color=CreateBitmap(8,8,1,32,NULL);
    HBITMAP mono=CreateBitmap(8,16,1,1,NULL);CHECK(mask && color && mono);
    ICONINFO info={.fIcon=TRUE,.hbmMask=mask,.hbmColor=color};
    HICON colored=CreateIconIndirect(&info);info.hbmMask=mono;info.hbmColor=NULL;
    HICON monochrome=CreateIconIndirect(&info);CHECK(colored && monochrome);
    int w=0,h=0;CHECK(style_icon_size(colored,&w,&h) && w==8 && h==8);
    w=h=0;CHECK(style_icon_size(monochrome,&w,&h) && w==8 && h==8);
    w=12;h=13;CHECK(!style_icon_size(NULL,&w,&h) && w==12 && h==13);
    CHECK(DestroyIcon(colored) && DestroyIcon(monochrome));
    CHECK(DeleteObject(mask) && DeleteObject(color) && DeleteObject(mono));
    puts("ICON_SIZE_OK color=1 monochrome=1 invalid=1 cleanup=1");return 0;
}
