/* Ordinary Toolbar drawing roles from Win2000 toolbar.c. Native rendering
 * still owns the bitmap list, the label, and the placement of both. */
#include "bars.h"
#include "../common/paint.h"

static BOOL supported(HWND window)
{
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    return !(style&(CCS_VERT|TBSTYLE_CUSTOMERASE|TBSTYLE_TRANSPARENT));
}

static void prepare(HWND window,NMCUSTOMDRAW *draw,const void *context)
{
    (void)window;
    const win2k_bar_draw *parameters=context;
    NMTBCUSTOMDRAW *item=(NMTBCUSTOMDRAW *)draw;
    const win2k_palette *palette=&parameters->palette;
    if(item->clrBtnFace==GetSysColor(COLOR_BTNFACE))
        item->clrBtnFace=(draw->uItemState&(CDIS_SELECTED|CDIS_CHECKED))?parameters->pressed_face:palette->face;
    if(item->clrBtnHighlight==GetSysColor(COLOR_BTNHIGHLIGHT))item->clrBtnHighlight=palette->highlight;
    if(item->clrText==GetSysColor(COLOR_BTNTEXT))item->clrText=palette->text;
}

static void paint(HWND window,const NMCUSTOMDRAW *draw,const void *context)
{
    const win2k_bar_draw *parameters=context;
    const win2k_palette *palette=&parameters->palette;
    const style_instance *instance=parameters->instance;
    RECT bounds=draw->rc;
    UINT edge,flags=BF_RECT;
    DWORD style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    LRESULT index=SendMessageW(window,TB_COMMANDTOINDEX,draw->dwItemSpec,0);
    TBBUTTON button;
    if(!draw->hdc || index<0 || instance->detached ||
        !SendMessageW(window,TB_GETBUTTON,(WPARAM)index,(LPARAM)&button) ||
        instance->detached || (button.fsStyle&(BTNS_DROPDOWN|BTNS_WHOLEDROPDOWN|BTNS_SEP)))return;
    BOOL pressed=(draw->uItemState&(CDIS_SELECTED|CDIS_CHECKED))!=0;
    if(style&TBSTYLE_FLAT){
        if(draw->uItemState&CDIS_DISABLED)return;
        edge=pressed?BDR_SUNKENOUTER:BDR_RAISEDINNER;
        if(!pressed && !(draw->uItemState&CDIS_HOT))return;
    }else{
        edge=pressed?EDGE_SUNKEN:EDGE_RAISED;
        flags|=BF_SOFT;
    }
    style_paint_relief(draw->hdc,&bounds,edge,flags,palette);
}

static void separators(HWND window,const NMCUSTOMDRAW *draw,const void *context)
{
    const win2k_bar_draw *parameters=context;
    const style_instance *instance=parameters->instance;
    if(!(GetWindowLongPtrW(window,GWL_STYLE)&TBSTYLE_FLAT) || !draw->hdc)return;
    int inset=GetSystemMetricsForDpi(SM_CYEDGE,GetDpiForWindow(window));
    LRESULT count=SendMessageW(window,TB_BUTTONCOUNT,0,0);
    for(LRESULT i=0;i<count && i<1024 && !instance->detached;++i){
        TBBUTTON item;
        RECT rect;
        if(!SendMessageW(window,TB_GETBUTTON,(WPARAM)i,(LPARAM)&item) || instance->detached ||
            !(item.fsStyle&BTNS_SEP) || (item.fsState&(TBSTATE_WRAP|TBSTATE_HIDDEN)) ||
            !SendMessageW(window,TB_GETITEMRECT,(WPARAM)i,(LPARAM)&rect) || instance->detached)continue;
        rect.left+=(rect.right-rect.left-1)/2;
        InflateRect(&rect,0,-inset);
        if(rect.right-rect.left>=2 && rect.top<rect.bottom)
            style_paint_relief(draw->hdc,&rect,EDGE_ETCHED,BF_LEFT,&parameters->palette);
    }
}

const style_control_notify_ops win2k_toolbar_notify={
    .supported=supported,.prepare_item=prepare,.finish_item=paint,.finish_paint=separators
};
