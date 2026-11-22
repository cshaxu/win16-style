/* monthcal.c owns both native registrations. MCPaint uses active
 * caption roles and DPDrawDropdownButton uses the normal scroll-down frame. */
#include "date_paint.h"
#include "lib-win32-style/application/graphics/paint_frame.h"

BOOL style_observed_date_supported(HWND window)
{
    return !(GetWindowLongPtrW(window,GWL_STYLE)&(DTS_UPDOWN|DTS_SHOWNONE|DTS_APPCANPARSE));
}

BOOL style_observed_month_supported(HWND window)
{
    return !(GetWindowLongPtrW(window,GWL_STYLE)&MCS_MULTISELECT);
}

void style_observed_date_colors(HWND window,const style_instance *instance,style_calendar_colors *colors,BOOL picker,
    const style_relief_palette *p,const classic_calendar_colors *theme)
{
    COLORREF roles[STYLE_CALENDAR_COLOR_COUNT]={p->window,p->text,p->selection,p->selection_text,p->window,p->gray};
    static const unsigned native_role[CLASSIC_CALENDAR_ROLE_COUNT]={
        [CLASSIC_CALENDAR_OUTSIDE]=MCSC_BACKGROUND,[CLASSIC_CALENDAR_TEXT]=MCSC_TEXT,
        [CLASSIC_CALENDAR_SELECTED]=MCSC_TITLEBK,[CLASSIC_CALENDAR_SELECTED_TEXT]=MCSC_TITLETEXT,
        [CLASSIC_CALENDAR_GRID]=MCSC_MONTHBK,[CLASSIC_CALENDAR_TRAILING_TEXT]=MCSC_TRAILINGTEXT
    };
    for(unsigned i=0;i<ARRAYSIZE(native_role);++i)
        if(theme->present&(1u<<i))roles[native_role[i]]=theme->values[i];
    style_calendar_colors_apply(window,instance,colors,picker,roles);
}

void style_observed_date_button(HWND window,const style_instance *instance,HDC dc,const RECT *dirty,
    const style_control_native_button *button,const style_relief_palette *palette,const style_scroll_arrow_art *art)
{
    if(instance->detached || GetMapMode(dc)!=MM_TEXT || (GetLayout(dc)&LAYOUT_RTL))return;
    UINT state=DFCS_SCROLLDOWN;
    if(!IsWindowEnabled(window))state|=DFCS_INACTIVE;
    else if(button->pressed)state|=DFCS_PUSHED|DFCS_FLAT;
    int saved=SaveDC(dc);
    if(!saved)return;
    if(!dirty || IntersectClipRect(dc,dirty->left,dirty->top,dirty->right,dirty->bottom)>NULLREGION)
        style_paint_scroll_arrow(dc,&button->bounds,state,palette,art);
    RestoreDC(dc,saved);
}
