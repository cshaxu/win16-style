#include "report_paint.h"

BOOL style_observed_listview_supported(HWND window)
{
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    return !(style&(LVS_OWNERDATA|LVS_OWNERDRAWFIXED));
}

BOOL style_observed_listview_plain(HWND window)
{
    if(!style_observed_listview_supported(window) || SendMessageW(window,LVM_ISGROUPVIEWENABLED,0,0))return FALSE;
    WCHAR source[1]={0};
    LVBKIMAGEW background={.pszImage=source,.cchImageMax=ARRAYSIZE(source)};
    SendMessageW(window,LVM_GETBKIMAGEW,0,(LPARAM)&background);
    return (background.ulFlags&LVBKIF_SOURCE_MASK)==LVBKIF_SOURCE_NONE;
}

BOOL style_observed_treeview_supported(HWND window)
{
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    return !(style&TVS_RTLREADING);
}

/* comctl32 tvpaint.c TV_DrawItem and lvrept.c ListView_RDrawItem own layout,
 * app text/image access and custom-draw decisions. Supply standard selection
 * colors only at default item-draw stages, with the native renderer still active. */
static void selection_colors(HWND window,const NMCUSTOMDRAW *draw,const style_observed_report_draw *parameters,
    COLORREF *text,COLORREF *background)
{
    const style_relief_palette *p=&parameters->palette;
    enum classic_report_state state;
    if(!IsWindowEnabled(window)){
        state=CLASSIC_REPORT_DISABLED;
        *text=p->gray;
        *background=p->face;
    }else if(draw->uItemState&(CDIS_SELECTED|CDIS_DROPHILITED)){
        if(GetFocus()==window || (draw->uItemState&CDIS_DROPHILITED)){
            state=CLASSIC_REPORT_SELECTED;
            *text=p->selection_text;
            *background=p->selection;
        }else{
            state=CLASSIC_REPORT_UNFOCUSED;
            *text=p->text;
            *background=p->face;
        }
    }else return;
    const classic_report_item_colors *colors=&parameters->colors[state];
    if(colors->has_text)*text=colors->text;
    if(colors->has_background)*background=colors->background;
}

void style_observed_listview_colors(HWND window,NMCUSTOMDRAW *draw,const void *context)
{
    const style_observed_report_draw *parameters=context;
    const style_instance *instance=parameters->instance;
    NMLVCUSTOMDRAW *item=(NMLVCUSTOMDRAW *)draw;
    if((draw->uItemState&CDIS_SELECTED) && !(draw->uItemState&CDIS_DROPHILITED)){
        DWORD extended=(DWORD)SendMessageW(window,LVM_GETEXTENDEDLISTVIEWSTYLE,0,0);
        if(instance->detached ||
           (extended&(LVS_EX_FULLROWSELECT|LVS_EX_BORDERSELECT|LVS_EX_TRANSPARENTBKGND|LVS_EX_TRANSPARENTSHADOWTEXT)))
            return;
        DWORD view=(DWORD)GetWindowLongPtrW(window,GWL_STYLE)&LVS_TYPEMASK;
        if(SendMessageW(window,LVM_GETIMAGELIST,view==LVS_ICON?LVSIL_NORMAL:LVSIL_SMALL,0) || instance->detached)return;
        selection_colors(window,draw,parameters,&item->clrText,&item->clrTextBk);
        /* Selected native items can ignore clrTextBk. This affects only the
         * current draw notification. The item's LVIS_SELECTED stays native. */
        draw->uItemState&=~CDIS_SELECTED;
        return;
    }
    selection_colors(window,draw,parameters,&item->clrText,&item->clrTextBk);
}

void style_observed_treeview_colors(HWND window,NMCUSTOMDRAW *draw,const void *context)
{
    const style_observed_report_draw *parameters=context;
    NMTVCUSTOMDRAW *item=(NMTVCUSTOMDRAW *)draw;
    selection_colors(window,draw,parameters,&item->clrText,&item->clrTextBk);
}
