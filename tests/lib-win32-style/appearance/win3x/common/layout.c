#include "lib-win32-style/appearance/win3x/common/layout.h"
#include "lib-win32-style/appearance/msstyles/classic_presets.h"
#include <stdio.h>
#define CHECK(x) do { if(!(x)){printf("WIN16_LAYOUT_FAIL line=%d\n",__LINE__);return 1;} } while(0)
int main(void)
{
    win16_style_layout layout;
    DWORD style=WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_THICKFRAME;
    CHECK(!win16_style_measure(0,200,96,style,0,FALSE,&layout));
    CHECK(!win16_style_measure(320,200,47,style,0,FALSE,&layout));
    CHECK(!win16_style_measure(320,200,769,style,0,FALSE,&layout));
    CHECK(!win16_style_measure(320,200,96,style,0,FALSE,NULL));
    CHECK(win16_style_measure(320,200,96,style,0,FALSE,&layout));
    CHECK(layout.border==4 && layout.unit==1 && layout.sizeable);
    CHECK(layout.buttons[CLASSIC_BUTTON_SYSTEM].left==4 && layout.buttons[CLASSIC_BUTTON_SYSTEM].right==23 && layout.caption.bottom==24);
    CHECK(layout.buttons[CLASSIC_BUTTON_MAXIMIZE].left==297 && layout.buttons[CLASSIC_BUTTON_MAXIMIZE].right==316 &&
          layout.buttons[CLASSIC_BUTTON_MINIMIZE].left==278 && layout.buttons[CLASSIC_BUTTON_MINIMIZE].right==297);
    CHECK(win16_style_hit(&layout,(POINT){-1,0})==HTNOWHERE);
    CHECK(win16_style_hit(&layout,(POINT){5,5})==HTSYSMENU);
    CHECK(win16_style_hit(&layout,(POINT){280,5})==HTMINBUTTON);
    CHECK(win16_style_hit(&layout,(POINT){300,5})==HTMAXBUTTON);
    CHECK(win16_style_hit(&layout,(POINT){100,5})==HTCAPTION);
    CHECK(win16_style_hit(&layout,(POINT){100,100})==HTCLIENT);
    CHECK(win16_style_hit(&layout,(POINT){0,0})==HTTOPLEFT);
    CHECK(win16_style_hit(&layout,(POINT){319,199})==HTBOTTOMRIGHT);
    CHECK(win16_style_measure(320,200,144,style,0,FALSE,&layout) && layout.unit==2 && layout.border==6);
    CHECK(win16_style_measure(320,200,96,style,0,TRUE,&layout) && !layout.sizeable);
    CHECK(win16_style_hit(&layout,(POINT){0,0})==HTBORDER);
    CHECK(win16_style_measure(320,200,96,WS_BORDER,0,FALSE,&layout));
    CHECK(layout.border==1 && IsRectEmpty(&layout.caption) && layout.client.left==1);
    const classic_button_group buttons=WIN16_TITLE_BUTTON_GROUP;
    style_frame_geometry frame={STYLE_FRAME_LOGICAL,8,5,7,2,30,0};
    CHECK(win16_style_measure_with_buttons(320,200,96,style,0,FALSE,&buttons,NULL,&frame,192,&layout));
    CHECK(layout.border==8 && layout.caption.top==8 && layout.caption.bottom==38 && layout.client.top==38);
    frame.scale=STYLE_FRAME_MONITOR_PIXELS;
    CHECK(win16_style_measure_with_buttons(320,200,96,style,0,FALSE,&buttons,NULL,&frame,192,&layout));
    CHECK(layout.border==4 && layout.client.left==4 && layout.client.right==316 && layout.client.top==34);
    CHECK(win16_style_hit(&layout,(POINT){0,0})==HTTOPLEFT && win16_style_hit(&layout,(POINT){15,34})==HTCLIENT);
    puts("WIN16_LAYOUT_OK");return 0;
}
