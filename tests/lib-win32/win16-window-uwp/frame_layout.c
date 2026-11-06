#include "lib-win32/win16-window-uwp/frame_layout.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FAIL line %d\n",__LINE__);return 1;}}while(0)
int main(void)
{
    for(UINT dpi=96;dpi<=192;dpi+=48){
        SIZE outer={MulDiv(500,dpi,96),MulDiv(400,dpi,96)};
        RECT content={MulDiv(8,dpi,96),MulDiv(40,dpi,96),outer.cx-MulDiv(8,dpi,96),outer.cy-MulDiv(8,dpi,96)};
        win16_style_layout l;RECT overlap;
        CHECK(win16_uwp_frame_layout(outer,dpi,WS_OVERLAPPEDWINDOW,FALSE,content,&l));
        CHECK(EqualRect(&content,&l.client));
        CHECK(!IntersectRect(&overlap,&l.title,&l.maximize));
        CHECK(!IntersectRect(&overlap,&l.minimize,&l.maximize));
        POINT p={content.left+1,content.top+1};CHECK(win16_uwp_frame_hit(&l,p)==HTCLIENT);
        p=(POINT){l.minimize.left+1,l.minimize.top+1};CHECK(win16_uwp_frame_hit(&l,p)==HTMINBUTTON);
        p=(POINT){l.title.left+1,l.title.top+1};CHECK(win16_uwp_frame_hit(&l,p)==HTCAPTION);
        content.right=outer.cx+1;CHECK(!win16_uwp_frame_layout(outer,dpi,WS_OVERLAPPEDWINDOW,FALSE,content,&l));
    }
    puts("UWP_FRAME_LAYOUT_PASS dpi=96,144,192");return 0;
}
