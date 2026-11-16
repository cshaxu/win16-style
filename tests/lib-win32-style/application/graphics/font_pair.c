#include "lib-win32-style/application/graphics/font_pair.h"
#include <stdio.h>

static unsigned released;
static HFONT stock;
static void release(const style_font_pair *pair)
{
    if(pair->title){DeleteObject(pair->title);++released;}
    if(pair->menu && pair->menu!=stock){DeleteObject(pair->menu);++released;}
}
static HFONT create(void)
{return CreateFontW(-12,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,0,0,0,0,L"Arial");}
int main(void)
{
    style_font_pair pair={0},first={create(),create(),96};LOGFONTW info;
    stock=(HFONT)GetStockObject(SYSTEM_FONT);
    if(!style_font_pair_commit(&pair,first,release))return 1;
    HFONT partial=create();
    if(!partial){style_font_pair_dispose(&pair,release);return 2;}
    if(style_font_pair_commit(&pair,(style_font_pair){partial,NULL,144},release) ||
       released!=1 || pair.title!=first.title || pair.menu!=first.menu || pair.dpi!=96 ||
       !GetObjectW(first.title,sizeof(info),&info) || !GetObjectW(first.menu,sizeof(info),&info) ||
       GetObjectW(partial,sizeof(info),&info)){
        style_font_pair_dispose(&pair,release);return 3;
    }
    style_font_pair next={create(),stock,144};
    if(!style_font_pair_commit(&pair,next,release)){style_font_pair_dispose(&pair,release);return 4;}
    if(released!=3 || pair.title!=next.title || pair.menu!=stock || pair.dpi!=144 ||
       GetObjectW(first.title,sizeof(info),&info) || GetObjectW(first.menu,sizeof(info),&info)){
        style_font_pair_dispose(&pair,release);return 5;
    }
    style_font_pair_dispose(&pair,release);style_font_pair_dispose(&pair,release);
    BOOL ok=released==4 && !pair.title && !pair.menu && !pair.dpi && GetObjectW(stock,sizeof(info),&info)>0;
    printf("FONT PAIR bits=%u ok=%d released=%u\n",(unsigned)(sizeof(void *)*8),ok,released);
    return ok?0:6;
}
