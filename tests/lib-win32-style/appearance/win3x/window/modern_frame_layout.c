#include "lib-win32-style/application/adapters/uwp/client_frame_layout.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"FAIL line %d\n",__LINE__);return 1;}}while(0)
int main(void)
{
    const UINT scales[]={96,120,144,168,192,240,288};
    for(unsigned scale=0;scale<ARRAYSIZE(scales);++scale){
        UINT dpi=scales[scale];
        SIZE outer={MulDiv(500,dpi,96),MulDiv(400,dpi,96)};
        RECT content={MulDiv(8,dpi,96),MulDiv(40,dpi,96),outer.cx-MulDiv(8,dpi,96),outer.cy-MulDiv(8,dpi,96)};
        style_full_client_layout l,native;RECT overlap;
        CHECK(style_full_client_measure(outer.cx,outer.cy,dpi,WS_OVERLAPPEDWINDOW,0,FALSE,
            NULL,NULL,NULL,dpi,&native));
        CHECK(style_uwp_client_frame_layout(outer,dpi,WS_OVERLAPPEDWINDOW,0,FALSE,content,NULL,NULL,NULL,&l));
        CHECK(EqualRect(&content,&l.client));
        CHECK(EqualRect(&native.caption,&l.caption) && EqualRect(&native.title,&l.title) &&
              EqualRect(&native.buttons[CLASSIC_BUTTON_SYSTEM],&l.buttons[CLASSIC_BUTTON_SYSTEM]) && EqualRect(&native.buttons[CLASSIC_BUTTON_MINIMIZE],&l.buttons[CLASSIC_BUTTON_MINIMIZE]) &&
              EqualRect(&native.buttons[CLASSIC_BUTTON_MAXIMIZE],&l.buttons[CLASSIC_BUTTON_MAXIMIZE]));
        RECT taller=content;taller.top+=MulDiv(15,dpi,96);
        CHECK(style_uwp_client_frame_layout(outer,dpi,WS_OVERLAPPEDWINDOW,0,FALSE,taller,NULL,NULL,NULL,&l));
        CHECK(EqualRect(&taller,&l.client) && EqualRect(&native.caption,&l.caption) &&
              EqualRect(&native.buttons[CLASSIC_BUTTON_MAXIMIZE],&l.buttons[CLASSIC_BUTTON_MAXIMIZE]));
        CHECK(style_uwp_client_frame_layout(outer,dpi,WS_OVERLAPPEDWINDOW,0,FALSE,content,NULL,NULL,NULL,&l));
        CHECK(!IntersectRect(&overlap,&l.title,&l.buttons[CLASSIC_BUTTON_MAXIMIZE]));
        CHECK(!IntersectRect(&overlap,&l.buttons[CLASSIC_BUTTON_MINIMIZE],&l.buttons[CLASSIC_BUTTON_MAXIMIZE]));
        POINT p={content.left+1,content.top+1};CHECK(style_uwp_client_frame_hit(&l,p)==HTCLIENT);
        p=(POINT){l.title.left+1,l.title.top+1};CHECK(style_uwp_client_frame_hit(&l,p)==HTCAPTION);
        const POINT edges[]={
            {0,0},{outer.cx/2,0},{outer.cx-1,0},
            {0,outer.cy/2},{outer.cx-1,outer.cy/2},
            {0,outer.cy-1},{outer.cx/2,outer.cy-1},{outer.cx-1,outer.cy-1}};
        const LRESULT hits[]={HTTOPLEFT,HTTOP,HTTOPRIGHT,HTLEFT,HTRIGHT,
            HTBOTTOMLEFT,HTBOTTOM,HTBOTTOMRIGHT};
        for(unsigned i=0;i<ARRAYSIZE(edges);++i)CHECK(style_uwp_client_frame_hit(&l,edges[i])==hits[i]);
        /* Native invisible margins must resize, but never consume XAML pixels. */
        p=(POINT){content.left-1,outer.cy/2};CHECK(style_uwp_client_frame_hit(&l,p)==HTLEFT);
        ++p.x;CHECK(style_uwp_client_frame_hit(&l,p)==HTCLIENT);
        p=(POINT){content.right,outer.cy/2};CHECK(style_uwp_client_frame_hit(&l,p)==HTRIGHT);
        --p.x;CHECK(style_uwp_client_frame_hit(&l,p)==HTCLIENT);
        p=(POINT){outer.cx/2,content.bottom};CHECK(style_uwp_client_frame_hit(&l,p)==HTBOTTOM);
        --p.y;CHECK(style_uwp_client_frame_hit(&l,p)==HTCLIENT);
        p=(POINT){outer.cx,outer.cy/2};CHECK(style_uwp_client_frame_hit(&l,p)==HTNOWHERE);
        p=(POINT){-1,outer.cy/2};CHECK(style_uwp_client_frame_hit(&l,p)==HTNOWHERE);
        /* Maximized and fixed-size windows must not advertise resizing. */
        for(unsigned mode=0;mode<2;++mode){
            CHECK(style_uwp_client_frame_layout(outer,dpi,mode?WS_OVERLAPPEDWINDOW&~WS_THICKFRAME:WS_OVERLAPPEDWINDOW,0,
                !mode,content,NULL,NULL,NULL,&l));
            CHECK(EqualRect(&content,&l.client));
            for(unsigned i=0;i<ARRAYSIZE(edges);++i)CHECK(style_uwp_client_frame_hit(&l,edges[i])==HTBORDER);
        }
        const classic_button_group group={3,0,0,{{CLASSIC_BUTTON_SYSTEM,CLASSIC_BUTTON_LEFT,0},
            {CLASSIC_BUTTON_MINIMIZE,CLASSIC_BUTTON_RIGHT,0},
            {CLASSIC_BUTTON_MAXIMIZE,CLASSIC_BUTTON_RIGHT,0}},0};
        const classic_caption_metrics metrics={.height=22,.button={23,22}};
        const style_frame_geometry frame={STYLE_FRAME_MONITOR_PIXELS,2,2,2,1,20,1,0};
        CHECK(style_full_client_measure(outer.cx,outer.cy,dpi,WS_OVERLAPPEDWINDOW,0,FALSE,
            &group,&metrics,&frame,dpi,&native));
        CHECK(style_uwp_client_frame_layout(outer,dpi,WS_OVERLAPPEDWINDOW,0,FALSE,content,&group,&metrics,&frame,&l));
        CHECK(l.border==2 && EqualRect(&content,&l.client) && EqualRect(&native.caption,&l.caption) &&
            EqualRect(&native.buttons[CLASSIC_BUTTON_SYSTEM],&l.buttons[CLASSIC_BUTTON_SYSTEM]) && EqualRect(&native.buttons[CLASSIC_BUTTON_MINIMIZE],&l.buttons[CLASSIC_BUTTON_MINIMIZE]) &&
            EqualRect(&native.buttons[CLASSIC_BUTTON_MAXIMIZE],&l.buttons[CLASSIC_BUTTON_MAXIMIZE]));
        const int corner=l.border+MulDiv(23,(int)dpi,96);
        CHECK(style_uwp_client_frame_hit(&l,(POINT){corner-1,outer.cy-1})==HTBOTTOMLEFT);
        CHECK(style_uwp_client_frame_hit(&l,(POINT){corner,outer.cy-1})==HTBOTTOM);
        CHECK(l.buttons[CLASSIC_BUTTON_MAXIMIZE].right-l.buttons[CLASSIC_BUTTON_MAXIMIZE].left==MulDiv(23,(int)dpi,96));
        p=(POINT){l.buttons[CLASSIC_BUTTON_MAXIMIZE].left+1,l.buttons[CLASSIC_BUTTON_MAXIMIZE].top+1};
        CHECK(style_uwp_client_frame_hit(&l,p)==HTMAXBUTTON);
        const classic_button_group mixed={3,0,0,{{CLASSIC_BUTTON_SYSTEM,CLASSIC_BUTTON_LEFT,0},
            {CLASSIC_BUTTON_HELP,CLASSIC_BUTTON_RIGHT,0},{CLASSIC_BUTTON_CLOSE,CLASSIC_BUTTON_RIGHT,2}},0};
        const DWORD dialog=WS_CAPTION|WS_SYSMENU|WS_THICKFRAME;
        CHECK(style_full_client_measure(outer.cx,outer.cy,dpi,dialog,WS_EX_CONTEXTHELP,FALSE,
            &mixed,&metrics,&frame,dpi,&native));
        CHECK(style_uwp_client_frame_layout(outer,dpi,dialog,WS_EX_CONTEXTHELP,FALSE,content,&mixed,&metrics,&frame,&l));
        for(enum classic_button_role role=CLASSIC_BUTTON_HELP;role<=CLASSIC_BUTTON_CLOSE;++role){
            RECT slot=l.buttons[role];
            int hit=role==CLASSIC_BUTTON_HELP?HTHELP:HTCLOSE;
            CHECK(EqualRect(&native.buttons[role],&slot) && !IsRectEmpty(&slot));
            CHECK(style_uwp_client_frame_hit(&l,(POINT){slot.left,slot.top})==hit);
            CHECK(style_caption_gesture(hit,WM_LBUTTONUP,FALSE,FALSE,TRUE)==
                (role==CLASSIC_BUTTON_HELP?SC_CONTEXTHELP:SC_CLOSE));
        }
        CHECK(style_uwp_client_frame_layout(outer,dpi,dialog,0,FALSE,content,&mixed,&metrics,&frame,&l));
        CHECK(IsRectEmpty(&l.buttons[CLASSIC_BUTTON_HELP]) && !IsRectEmpty(&l.buttons[CLASSIC_BUTTON_CLOSE]));
        const classic_button_group omitted={0};
        CHECK(style_uwp_client_frame_layout(outer,dpi,WS_OVERLAPPEDWINDOW,0,FALSE,content,&omitted,&metrics,&frame,&l));
        CHECK(l.border==2 && EqualRect(&native.caption,&l.caption) &&
            IsRectEmpty(&l.buttons[CLASSIC_BUTTON_SYSTEM]) && EqualRect(&l.title,&l.caption));
        CHECK(style_caption_gesture(HTSYSMENU,WM_LBUTTONUP,FALSE,TRUE,TRUE)==SC_KEYMENU);
        CHECK(style_caption_gesture(HTSYSMENU,WM_LBUTTONDBLCLK,FALSE,TRUE,FALSE)==0);
        CHECK(style_caption_gesture(HTCAPTION,WM_LBUTTONDBLCLK,TRUE,TRUE,TRUE)==SC_RESTORE);
        content.right=outer.cx+1;
        CHECK(!style_uwp_client_frame_layout(outer,dpi,WS_OVERLAPPEDWINDOW,0,FALSE,content,NULL,NULL,NULL,&l));
    }
    puts("UWP_FRAME_LAYOUT_PASS dpi=96,120,144,168,192,240,288 edges=8 native-margin=3 fixed-maximized=2");return 0;
}
