#include "lib-style/win16-window-uwp/frame_layout.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FAIL line %d\n",__LINE__);return 1;}}while(0)
int main(void)
{
    const UINT scales[]={96,120,144,168,192,240,288};
    for(unsigned scale=0;scale<ARRAYSIZE(scales);++scale){
        UINT dpi=scales[scale];
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
        const POINT edges[]={
            {0,0},{outer.cx/2,0},{outer.cx-1,0},
            {0,outer.cy/2},{outer.cx-1,outer.cy/2},
            {0,outer.cy-1},{outer.cx/2,outer.cy-1},{outer.cx-1,outer.cy-1}};
        const LRESULT hits[]={HTTOPLEFT,HTTOP,HTTOPRIGHT,HTLEFT,HTRIGHT,
            HTBOTTOMLEFT,HTBOTTOM,HTBOTTOMRIGHT};
        for(unsigned i=0;i<ARRAYSIZE(edges);++i)CHECK(win16_uwp_frame_hit(&l,edges[i])==hits[i]);
        /* Native invisible margins must resize, but never consume XAML pixels. */
        p=(POINT){content.left-1,outer.cy/2};CHECK(win16_uwp_frame_hit(&l,p)==HTLEFT);
        ++p.x;CHECK(win16_uwp_frame_hit(&l,p)==HTCLIENT);
        p=(POINT){content.right,outer.cy/2};CHECK(win16_uwp_frame_hit(&l,p)==HTRIGHT);
        --p.x;CHECK(win16_uwp_frame_hit(&l,p)==HTCLIENT);
        p=(POINT){outer.cx/2,content.bottom};CHECK(win16_uwp_frame_hit(&l,p)==HTBOTTOM);
        --p.y;CHECK(win16_uwp_frame_hit(&l,p)==HTCLIENT);
        p=(POINT){outer.cx,outer.cy/2};CHECK(win16_uwp_frame_hit(&l,p)==HTNOWHERE);
        p=(POINT){-1,outer.cy/2};CHECK(win16_uwp_frame_hit(&l,p)==HTNOWHERE);
        /* Maximized and fixed-size windows must not advertise resizing. */
        for(unsigned mode=0;mode<2;++mode){
            CHECK(win16_uwp_frame_layout(outer,dpi,mode?WS_OVERLAPPEDWINDOW&~WS_THICKFRAME:WS_OVERLAPPEDWINDOW,!mode,content,&l));
            CHECK(EqualRect(&content,&l.client));
            for(unsigned i=0;i<ARRAYSIZE(edges);++i)CHECK(win16_uwp_frame_hit(&l,edges[i])==HTBORDER);
        }
        content.right=outer.cx+1;CHECK(!win16_uwp_frame_layout(outer,dpi,WS_OVERLAPPEDWINDOW,FALSE,content,&l));
    }
    puts("UWP_FRAME_LAYOUT_PASS dpi=96,120,144,168,192,240,288 edges=8 native-margin=3 fixed-maximized=2");return 0;
}
