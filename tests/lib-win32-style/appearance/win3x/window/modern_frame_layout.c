#include "lib-win32-style/appearance/win3x/window/modern_frame_layout.h"
#include "lib-win32-style/appearance/msstyles/classic_presets.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FAIL line %d\n",__LINE__);return 1;}}while(0)
int main(void)
{
    const UINT scales[]={96,120,144,168,192,240,288};
    for(unsigned scale=0;scale<ARRAYSIZE(scales);++scale){
        UINT dpi=scales[scale];
        SIZE outer={MulDiv(500,dpi,96),MulDiv(400,dpi,96)};
        RECT content={MulDiv(8,dpi,96),MulDiv(40,dpi,96),outer.cx-MulDiv(8,dpi,96),outer.cy-MulDiv(8,dpi,96)};
        win16_style_layout l,native;RECT overlap;
        CHECK(win16_style_measure(outer.cx,outer.cy,dpi,WS_OVERLAPPEDWINDOW,0,FALSE,&native));
        CHECK(win16_uwp_frame_layout(outer,dpi,WS_OVERLAPPEDWINDOW,FALSE,content,NULL,NULL,NULL,&l));
        CHECK(EqualRect(&content,&l.client));
        CHECK(EqualRect(&native.caption,&l.caption) && EqualRect(&native.title,&l.title) &&
              EqualRect(&native.buttons[CLASSIC_BUTTON_SYSTEM],&l.buttons[CLASSIC_BUTTON_SYSTEM]) && EqualRect(&native.buttons[CLASSIC_BUTTON_MINIMIZE],&l.buttons[CLASSIC_BUTTON_MINIMIZE]) &&
              EqualRect(&native.buttons[CLASSIC_BUTTON_MAXIMIZE],&l.buttons[CLASSIC_BUTTON_MAXIMIZE]));
        RECT taller=content;taller.top+=MulDiv(15,dpi,96);
        CHECK(win16_uwp_frame_layout(outer,dpi,WS_OVERLAPPEDWINDOW,FALSE,taller,NULL,NULL,NULL,&l));
        CHECK(EqualRect(&taller,&l.client) && EqualRect(&native.caption,&l.caption) &&
              EqualRect(&native.buttons[CLASSIC_BUTTON_MAXIMIZE],&l.buttons[CLASSIC_BUTTON_MAXIMIZE]));
        CHECK(win16_uwp_frame_layout(outer,dpi,WS_OVERLAPPEDWINDOW,FALSE,content,NULL,NULL,NULL,&l));
        CHECK(!IntersectRect(&overlap,&l.title,&l.buttons[CLASSIC_BUTTON_MAXIMIZE]));
        CHECK(!IntersectRect(&overlap,&l.buttons[CLASSIC_BUTTON_MINIMIZE],&l.buttons[CLASSIC_BUTTON_MAXIMIZE]));
        POINT p={content.left+1,content.top+1};CHECK(win16_uwp_frame_hit(&l,p)==HTCLIENT);
        p=(POINT){l.buttons[CLASSIC_BUTTON_MINIMIZE].left+1,l.buttons[CLASSIC_BUTTON_MINIMIZE].top+1};CHECK(win16_uwp_frame_hit(&l,p)==HTMINBUTTON);
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
            CHECK(win16_uwp_frame_layout(outer,dpi,mode?WS_OVERLAPPEDWINDOW&~WS_THICKFRAME:WS_OVERLAPPEDWINDOW,
                !mode,content,NULL,NULL,NULL,&l));
            CHECK(EqualRect(&content,&l.client));
            for(unsigned i=0;i<ARRAYSIZE(edges);++i)CHECK(win16_uwp_frame_hit(&l,edges[i])==HTBORDER);
        }
        const classic_button_group group=WIN16_TITLE_BUTTON_GROUP;
        const classic_caption_metrics metrics={.height=22,.button={23,22}};
        const style_frame_geometry frame={STYLE_FRAME_MONITOR_PIXELS,2,2,2,1,20,1};
        CHECK(win16_style_measure_with_buttons(outer.cx,outer.cy,dpi,WS_OVERLAPPEDWINDOW,0,FALSE,
            &group,&metrics,&frame,dpi,&native));
        CHECK(win16_uwp_frame_layout(outer,dpi,WS_OVERLAPPEDWINDOW,FALSE,content,&group,&metrics,&frame,&l));
        CHECK(l.border==2 && EqualRect(&content,&l.client) && EqualRect(&native.caption,&l.caption) &&
            EqualRect(&native.buttons[CLASSIC_BUTTON_SYSTEM],&l.buttons[CLASSIC_BUTTON_SYSTEM]) && EqualRect(&native.buttons[CLASSIC_BUTTON_MINIMIZE],&l.buttons[CLASSIC_BUTTON_MINIMIZE]) &&
            EqualRect(&native.buttons[CLASSIC_BUTTON_MAXIMIZE],&l.buttons[CLASSIC_BUTTON_MAXIMIZE]));
        CHECK(l.buttons[CLASSIC_BUTTON_MAXIMIZE].right-l.buttons[CLASSIC_BUTTON_MAXIMIZE].left==MulDiv(23,(int)dpi,96));
        p=(POINT){l.buttons[CLASSIC_BUTTON_MAXIMIZE].left+1,l.buttons[CLASSIC_BUTTON_MAXIMIZE].top+1};
        CHECK(win16_uwp_frame_hit(&l,p)==HTMAXBUTTON);
        CHECK(style_caption_gesture(HTSYSMENU,WM_LBUTTONUP,FALSE,TRUE,TRUE)==SC_KEYMENU);
        CHECK(style_caption_gesture(HTSYSMENU,WM_LBUTTONDBLCLK,FALSE,TRUE,FALSE)==0);
        CHECK(style_caption_gesture(HTCAPTION,WM_LBUTTONDBLCLK,TRUE,TRUE,TRUE)==SC_RESTORE);
        content.right=outer.cx+1;
        CHECK(!win16_uwp_frame_layout(outer,dpi,WS_OVERLAPPEDWINDOW,FALSE,content,NULL,NULL,NULL,&l));
    }
    puts("UWP_FRAME_LAYOUT_PASS dpi=96,120,144,168,192,240,288 edges=8 native-margin=3 fixed-maximized=2");return 0;
}
