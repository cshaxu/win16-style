#include "list_combo.h"
#include "background.h"
#include "../common/palette.h"
#include "lib-win32-style/application/control/combo_snapshot.h"
#include "lib-win32-style/application/control/list_snapshot.h"
#include <oleacc.h>

static HFONT native_font(HWND window,void *context)
{
    (void)context;
    return (HFONT)SendMessageW(window,WM_GETFONT,0,0);
}

static void select_font(HDC dc,HFONT font)
{
    SelectObject(dc,font?font:GetStockObject(SYSTEM_FONT));
}

void win2k_list_paint(HWND window,style_instance *instance,HWND parent,HDC dc,const win2k_palette *p)
{
    int saved=SaveDC(dc);
    if(!saved)return;
    HBRUSH brush=win2k_control_color_brush(window,parent,instance,dc,WM_CTLCOLORLISTBOX,p,p->window);
    if(!instance->detached && !brush){
        SetDCBrushColor(dc,p->window);
        brush=(HBRUSH)GetStockObject(DC_BRUSH);
    }
    if(brush){
        const style_list_appearance appearance={
            .parent_brush=brush,.selection=p->selection,.text=GetTextColor(dc),
            .selected_text=p->selection_text,.disabled_text=p->gray,.honor_focus_cues=TRUE
        };
        style_control_list_strings(window,instance,dc,&appearance);
    }
    RestoreDC(dc,saved);
}

BOOL win2k_combo_info(HWND window,PCOMBOBOXINFO info)
{
    return GetComboBoxInfo(window,info);
}

BOOL win2k_combo_paint(HWND window,style_instance *instance,HDC dc,const win2k_palette *p,const classic_combo_colors *colors)
{
    const style_control_combo_reader reader={NULL,win2k_combo_info,native_font};
    style_control_combo_snapshot snapshot;
    int saved=SaveDC(dc);
    if(!saved)return FALSE;
    if(!style_control_combo_snapshot_read(window,instance,&reader,&snapshot)){
        RestoreDC(dc,saved);
        return FALSE;
    }
    RECT frame;
    GetClientRect(window,&frame);
    RECT item=snapshot.info.rcItem;
    if(snapshot.simple)frame.bottom=min(frame.bottom,item.bottom+item.top);
    IntersectClipRect(dc,frame.left,frame.top,frame.right,frame.bottom);
    if(snapshot.editable)ExcludeClipRect(dc,item.left,item.top,item.right,item.bottom);
    BOOL enabled=IsWindowEnabled(window);
    BOOL read_only=snapshot.editable &&
        (GetWindowLongPtrW(snapshot.info.hwndItem,GWL_STYLE)&ES_READONLY);
    HBRUSH brush=win2k_control_color_brush(window,GetParent(window),instance,dc,
        enabled && !read_only?WM_CTLCOLOREDIT:WM_CTLCOLORSTATIC,p,enabled && !read_only?p->window:p->face);
    BOOL painted=FALSE;
    if(instance->detached)goto done;
    if(brush)FillRect(dc,&frame,brush);
    else style_paint_color(dc,&frame,enabled && !read_only?p->window:p->face);
    win2k_palette chrome=*p;
    if(colors->present)chrome.dark=enabled?colors->border:colors->disabled_border;
    painted=style_paint_relief(dc,&frame,EDGE_SUNKEN,BF_RECT|BF_ADJUST,&chrome);
    if(!snapshot.simple){
        chrome=*p;
        UINT state=DFCS_SCROLLCOMBOBOX;
        if(!enabled)state|=DFCS_INACTIVE;
        BOOL pressed=(snapshot.info.stateButton&STATE_SYSTEM_PRESSED)!=0;
        if(pressed)state|=DFCS_PUSHED|DFCS_FLAT;
        if(colors->present)chrome.face=colors->button[classic_combo_state_for(enabled,pressed)];
        painted=style_paint_scroll_system_arrow(dc,&snapshot.info.rcButton,state,&chrome) && painted;
    }
    if(!snapshot.editable){
        BOOL focused=enabled && GetFocus()==window && !snapshot.pressed;
        if(focused)style_paint_color(dc,&item,p->selection);
        select_font(dc,snapshot.font);
        if(!brush || focused || !enabled)
            SetTextColor(dc,focused?p->selection_text:enabled?p->text:p->gray);
        SetBkMode(dc,TRANSPARENT);
        RECT label=item;
        label.left+=2;
        DrawTextW(dc,snapshot.text,-1,&label,DT_LEFT|DT_VCENTER|DT_SINGLELINE|DT_NOPREFIX);
        if(focused && !(SendMessageW(window,WM_QUERYUISTATE,0,0)&UISF_HIDEFOCUS))DrawFocusRect(dc,&item);
    }
done:
    style_control_combo_snapshot_clear(&snapshot);
    RestoreDC(dc,saved);
    return painted;
}
