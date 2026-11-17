#include "lib-win32-style/appearance/win2000/control/scroll_layout.h"
#include <limits.h>
#include <stdio.h>

#define CHECK(e) do {if(!(e)){fprintf(stderr,"WIN2K_SCROLL_LAYOUT_FAIL %d: %s\n",__LINE__,#e);return 1;}} while(0)

int main(void)
{
    style_scroll_layout layout;
    RECT vertical={12,-20,29,140};
    SCROLLINFO info={.cbSize=sizeof(info),.nMin=0,.nMax=99,.nPage=25,.nPos=75};
    CHECK(win2k_scroll_measure(&vertical,TRUE,17,&info,&layout));
    CHECK(layout.first.bottom==-3 && layout.last.top==123 && layout.maximum==75);
    CHECK(layout.thumb.top==91 && layout.thumb.bottom==123 && layout.travel==94 && layout.thumb_draggable);

    info.nPage=1;
    info.nPos=0;
    CHECK(win2k_scroll_measure(&vertical,TRUE,17,&info,&layout));
    CHECK(layout.thumb.top==-3 && layout.thumb.bottom==5 && layout.maximum==99);
    info.nPage=0;
    CHECK(win2k_scroll_measure(&vertical,TRUE,17,&info,&layout));
    CHECK(layout.thumb.top==-3 && layout.thumb.bottom==14);

    RECT short_bar={3,5,32,22};
    CHECK(win2k_scroll_measure(&short_bar,FALSE,17,&info,&layout));
    CHECK(layout.first.right==17 && layout.last.left==18 && !layout.thumb_fits && !layout.thumb_draggable);

    RECT horizontal={-50,40,150,57};
    info.nMin=LONG_MIN;
    info.nMax=LONG_MAX;
    info.nPage=1;
    CHECK(win2k_scroll_measure(&horizontal,FALSE,17,&info,&layout));
    CHECK(layout.minimum_pixel==-33 && layout.travel==158 && layout.thumb.left==46 && layout.thumb.right==54);
    CHECK(layout.maximum==LONG_MAX && layout.position==0);

    info.nMin=0;
    info.nMax=99;
    info.nPage=999;
    CHECK(win2k_scroll_measure(&horizontal,FALSE,17,&info,&layout));
    CHECK(layout.thumb.left==-33 && layout.thumb.right==133 && layout.thumb_fits && !layout.thumb_draggable);
    CHECK(!win2k_scroll_measure(&horizontal,FALSE,0,&info,&layout));
    puts("WIN2K_SCROLL_LAYOUT_OK proportional=1 minimum=1 fixed=1 short=1 signed_range=1 full_page=1");
    return 0;
}
