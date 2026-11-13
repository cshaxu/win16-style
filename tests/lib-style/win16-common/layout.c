#include "lib-style/win16-common/layout.h"
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
    CHECK(layout.system.left==4 && layout.system.right==23 && layout.caption.bottom==24);
    CHECK(layout.maximize.left==297 && layout.maximize.right==316 &&
          layout.minimize.left==278 && layout.minimize.right==297);
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
    puts("WIN16_LAYOUT_OK");return 0;
}
