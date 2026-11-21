#define _WIN32_WINNT 0x0A00
#include "lib-win32-style/appearance/win3x/common/layout.h"
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
    int native_width=GetSystemMetricsForDpi(SM_CYSIZEFRAME,96)+GetSystemMetricsForDpi(SM_CXPADDEDBORDER,96);
    CHECK(layout.border==native_width && layout.client.left==native_width &&
        layout.caption.bottom==layout.client.top);
    for(unsigned role=CLASSIC_BUTTON_SYSTEM;role<=CLASSIC_BUTTON_CLOSE;++role)
        CHECK(IsRectEmpty(&layout.buttons[role]));
    CHECK(layout.caption.bottom-layout.caption.top==GetSystemMetricsForDpi(SM_CYCAPTION,96));
    CHECK(EqualRect(&layout.title,&layout.caption));
    const classic_button_group buttons={3,0,0,{{CLASSIC_BUTTON_SYSTEM,CLASSIC_BUTTON_LEFT,0},
            {CLASSIC_BUTTON_MINIMIZE,CLASSIC_BUTTON_RIGHT,0},
            {CLASSIC_BUTTON_MAXIMIZE,CLASSIC_BUTTON_RIGHT,0}},0};
    const classic_caption_metrics stock_metrics={.height=18,.button={19,18}};
    const style_frame_geometry stock={STYLE_FRAME_LOGICAL,4,3,6,1,20,1,1};
    CHECK(win16_style_measure_with_buttons(320,200,96,style,0,FALSE,&buttons,&stock_metrics,&stock,96,&layout));
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
    CHECK(win16_style_measure_with_buttons(320,200,144,style,0,FALSE,&buttons,&stock_metrics,&stock,144,&layout));
    CHECK(layout.unit==2 && layout.border==6);
    CHECK(win16_style_measure(320,200,96,style,0,TRUE,&layout) && !layout.sizeable);
    CHECK(win16_style_hit(&layout,(POINT){0,0})==HTBORDER);
    CHECK(win16_style_measure(320,200,96,WS_BORDER,0,FALSE,&layout));
    CHECK(layout.border==1 && IsRectEmpty(&layout.caption) && layout.client.left==1);
    style_frame_geometry frame={STYLE_FRAME_LOGICAL,8,5,7,2,30,0,0};
    CHECK(win16_style_measure_with_buttons(320,200,96,style,0,FALSE,&buttons,NULL,&frame,192,&layout));
    CHECK(layout.border==8 && layout.caption.top==8 && layout.caption.bottom==38 && layout.client.top==38);
    frame.scale=STYLE_FRAME_MONITOR_PIXELS;
    CHECK(win16_style_measure_with_buttons(320,200,96,style,0,FALSE,&buttons,NULL,&frame,192,&layout));
    CHECK(layout.border==4 && layout.client.left==4 && layout.client.right==316 && layout.client.top==34);
    CHECK(win16_style_hit(&layout,(POINT){0,0})==HTTOPLEFT && win16_style_hit(&layout,(POINT){15,34})==HTCLIENT);
    classic_button_group no_system=buttons;
    no_system.slots[0]=buttons.slots[1];
    no_system.slots[1]=buttons.slots[2];
    --no_system.count;
    CHECK(win16_style_measure_with_buttons(320,200,96,style,0,FALSE,&no_system,NULL,&frame,192,&layout));
    CHECK(IsRectEmpty(&layout.buttons[CLASSIC_BUTTON_SYSTEM]) && layout.title.left==layout.caption.left);
    CHECK(win16_style_hit(&layout,(POINT){5,5})==HTCAPTION);
    CHECK(win16_style_hit(&layout,(POINT){300,5})==HTMAXBUTTON);
    const classic_caption_metrics metrics={.height=21,.button={27,17}};
    CHECK(win16_style_measure_with_buttons(480,300,144,style,0,FALSE,&buttons,&metrics,&frame,192,&layout));
    CHECK(layout.button_size.cx==41 && layout.button_size.cy==26 &&
        layout.buttons[CLASSIC_BUTTON_SYSTEM].right-layout.buttons[CLASSIC_BUTTON_SYSTEM].left==41);
    classic_button_group fitted=buttons;
    fitted.layout=CLASSIC_BUTTON_LAYOUT_EXPLICIT;
    CHECK(win16_style_measure_with_buttons(320,200,96,style,0,FALSE,&fitted,&stock_metrics,&stock,96,&layout));
    CHECK(layout.buttons[CLASSIC_BUTTON_SYSTEM].top==5 && layout.buttons[CLASSIC_BUTTON_SYSTEM].bottom==23);
    CHECK(win16_style_hit(&layout,(POINT){5,4})==HTCAPTION);
    CHECK(!win16_style_measure_with_buttons(45,200,96,style,0,FALSE,&fitted,&stock_metrics,&stock,96,&layout));
    fitted.layout|=CLASSIC_BUTTON_LAYOUT_FILL|CLASSIC_BUTTON_LAYOUT_CLIP;
    CHECK(win16_style_measure_with_buttons(45,200,96,style,0,FALSE,&fitted,&stock_metrics,&stock,96,&layout));
    CHECK(layout.buttons[CLASSIC_BUTTON_SYSTEM].top==4 && layout.buttons[CLASSIC_BUTTON_SYSTEM].bottom==24);
    CHECK(win16_style_hit(&layout,(POINT){5,4})==HTSYSMENU);
    puts("WIN16_LAYOUT_OK");return 0;
}
