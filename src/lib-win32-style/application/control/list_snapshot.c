#include "list_snapshot.h"
#include <stdlib.h>
void style_control_list_paint(HWND window,style_instance *instance,HDC dc,const style_control_list_painter *painter)
{
    if(!dc)return;
    style_control_list_snapshot snapshot;
    snapshot.style=(DWORD)GetWindowLongPtrW(window,GWL_STYLE);
    snapshot.first=(int)SendMessageW(window,LB_GETTOPINDEX,0,0);
    snapshot.count=(int)SendMessageW(window,LB_GETCOUNT,0,0);
    snapshot.caret=(int)SendMessageW(window,LB_GETCARETINDEX,0,0);
    snapshot.font=painter->font(window,painter->context);
    if(instance->detached)return;
    int saved=SaveDC(dc);if(!saved)return;
    GetClientRect(window,&snapshot.client);
    IntersectClipRect(dc,0,0,snapshot.client.right,snapshot.client.bottom);
    painter->begin(window,painter->context,dc,&snapshot);
    for(int index=snapshot.first;!instance->detached && index>=0 && index<snapshot.count;++index){
        style_control_list_item item={.index=index};
        if(SendMessageW(window,LB_GETITEMRECT,index,(LPARAM)&item.row)==LB_ERR)break;
        /* Native multicolumn indices advance down, then to the right. A
         * below-client row does not make the next column invisible. */
        if(snapshot.style&LBS_MULTICOLUMN){if(item.row.left>=snapshot.client.right)break;}
        else if(item.row.top>=snapshot.client.bottom)break;
        /* USER's single-column rect includes xOrigin at its right edge. */
        if(!(snapshot.style&LBS_MULTICOLUMN))OffsetRect(&item.row,snapshot.client.right-item.row.right,0);
        if(!IntersectRect(&item.visible,&item.row,&snapshot.client))continue;
        LRESULT length=SendMessageW(window,LB_GETTEXTLEN,index,0);
        if(instance->detached || length==LB_ERR)break;
        WCHAR *text=calloc((size_t)length+1,sizeof(WCHAR));if(!text)break;
        if(SendMessageW(window,LB_GETTEXT,index,(LPARAM)text)==LB_ERR){free(text);break;}
        item.text=text;item.length=(int)length;
        item.selected=(snapshot.style&(LBS_MULTIPLESEL|LBS_EXTENDEDSEL))?SendMessageW(window,LB_GETSEL,index,0):
            SendMessageW(window,LB_GETCURSEL,0,0)==index;
        if(instance->detached){free(text);break;}
        int row_saved=SaveDC(dc);if(!row_saved){free(text);break;}
        IntersectClipRect(dc,item.visible.left,item.visible.top,item.visible.right,item.visible.bottom);
        painter->item(window,painter->context,dc,&snapshot,&item);
        RestoreDC(dc,row_saved);free(text);
    }
    RestoreDC(dc,saved);
}

static HFONT string_font(HWND window,void *context)
{
    const style_list_appearance *style=context;
    return style->font_override?style->font_override:(HFONT)SendMessageW(window,WM_GETFONT,0,0);
}

static void string_background(HWND window,void *context,HDC dc,const style_control_list_snapshot *snapshot)
{
    (void)window;
    const style_list_appearance *style=context;
    HBRUSH brush=style->parent_brush;
    if(!brush){
        SetDCBrushColor(dc,style->background);
        brush=(HBRUSH)GetStockObject(DC_BRUSH);
    }
    FillRect(dc,&snapshot->client,brush);
    SelectObject(dc,snapshot->font?snapshot->font:GetStockObject(SYSTEM_FONT));
    SetBkMode(dc,TRANSPARENT);
}

static void string_row(HWND window,void *context,HDC dc,const style_control_list_snapshot *snapshot,
    const style_control_list_item *item)
{
    const style_list_appearance *style=context;
    BOOL selected=item->selected>0;
    if(selected || !style->parent_brush){
        SetDCBrushColor(dc,selected?style->selection:style->background);
        FillRect(dc,&item->row,(HBRUSH)GetStockObject(DC_BRUSH));
    }
    SetTextColor(dc,!IsWindowEnabled(window)?style->disabled_text:selected?style->selected_text:style->text);
    RECT label=item->row;
    if(!(snapshot->style&LBS_MULTICOLUMN))label.left+=2;
    if(style->use_tabs && (snapshot->style&LBS_USETABSTOPS))
        TabbedTextOutW(dc,label.left,label.top,item->text,item->length,style->tab_count,style->tabs,item->row.left);
    else DrawTextW(dc,item->text,item->length,&label,DT_LEFT|DT_SINGLELINE|DT_NOPREFIX);
    if(GetFocus()==window && snapshot->caret==item->index &&
       (!style->honor_focus_cues || !(SendMessageW(window,WM_QUERYUISTATE,0,0)&UISF_HIDEFOCUS)))
        DrawFocusRect(dc,&item->row);
}

void style_control_list_strings(HWND window,style_instance *instance,HDC dc,const style_list_appearance *style)
{
    const style_control_list_painter painter={(void *)style,string_font,string_background,string_row};
    style_control_list_paint(window,instance,dc,&painter);
}
