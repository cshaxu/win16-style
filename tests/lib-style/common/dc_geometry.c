#include "lib-style/common/dc_geometry.h"
#include <stdio.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"DC_GEOMETRY_FAIL line=%d\n",__LINE__);return 1;} } while(0)
int main(void)
{
    HDC dc=CreateCompatibleDC(NULL);CHECK(dc);
    CHECK(style_dc_pixel_viewport(dc));
    CHECK(style_dc_origin_pixels(dc));
    CHECK(SetWindowOrgEx(dc,1,0,NULL) && !style_dc_origin_pixels(dc));
    CHECK(SetWindowOrgEx(dc,0,0,NULL) && style_dc_origin_pixels(dc));
    CHECK(SetViewportOrgEx(dc,1,0,NULL) && !style_dc_pixel_viewport(dc));
    CHECK(!style_dc_origin_pixels(dc));
    CHECK(SetViewportOrgEx(dc,0,0,NULL) && style_dc_pixel_viewport(dc));
    CHECK(SetMapMode(dc,MM_LOMETRIC) && !style_dc_pixel_viewport(dc));
    CHECK(!style_dc_origin_pixels(dc));
    CHECK(SetMapMode(dc,MM_TEXT) && style_dc_pixel_viewport(dc));
    CHECK(SetLayout(dc,LAYOUT_RTL)!=GDI_ERROR && !style_dc_pixel_viewport(dc));
    CHECK(!style_dc_origin_pixels(dc));
    CHECK(DeleteDC(dc));
    puts("DC_GEOMETRY_OK viewport=1 mapping=1 rtl=1");return 0;
}
