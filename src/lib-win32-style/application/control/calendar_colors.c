#include "calendar_colors.h"
#include <commctrl.h>

static const int system_color[STYLE_CALENDAR_COLOR_COUNT]={
    COLOR_WINDOW,COLOR_WINDOWTEXT,COLOR_ACTIVECAPTION,COLOR_CAPTIONTEXT,COLOR_WINDOW,COLOR_GRAYTEXT
};

void style_calendar_colors_apply(HWND window,const style_instance *instance,style_calendar_colors *colors,
    BOOL picker,const COLORREF *palette)
{
    UINT get=picker?DTM_GETMCCOLOR:MCM_GETCOLOR,set=picker?DTM_SETMCCOLOR:MCM_SETCOLOR;
    for(unsigned role=0;role<STYLE_CALENDAR_COLOR_COUNT && !instance->detached;++role){
        COLORREF original=(COLORREF)SendMessageW(window,get,role,0);
        if(instance->detached || original!=GetSysColor(system_color[role]) || original==palette[role])continue;
        colors->old[role]=original;
        colors->applied[role]=palette[role];
        colors->owned|=1u<<role;
        colors->setting=TRUE;
        SendMessageW(window,set,role,palette[role]);
        colors->setting=FALSE;
    }
}

void style_calendar_colors_observe(style_calendar_colors *colors,BOOL picker,UINT message,WPARAM role)
{
    if(message==(picker?DTM_SETMCCOLOR:MCM_SETCOLOR) && role<STYLE_CALENDAR_COLOR_COUNT && !colors->setting)
        colors->owned&=~(1u<<role);
}

void style_calendar_colors_restore(HWND window,style_calendar_colors *colors,BOOL picker)
{
    UINT get=picker?DTM_GETMCCOLOR:MCM_GETCOLOR,set=picker?DTM_SETMCCOLOR:MCM_SETCOLOR;
    for(unsigned role=0;role<STYLE_CALENDAR_COLOR_COUNT && IsWindow(window);++role){
        if((colors->owned&(1u<<role)) && (COLORREF)SendMessageW(window,get,role,0)==colors->applied[role])
            SendMessageW(window,set,role,colors->old[role]);
    }
    colors->owned=0;
}
